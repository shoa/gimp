/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib-object.h>

#include "libgimpcolor/gimpcolor.h"
#include "libgimpmath/gimpmath.h"
#include "libgimpbase/gimpbase.h"

#include "core-types.h"

#include "base/pixel-region.h"
#include "base/tile.h"
#include "base/tile-manager.h"

#include "paint-funcs/paint-funcs.h"

#include "gimp.h"
#include "gimpchannel.h"
#include "gimpcontext.h"
#include "gimpdrawable.h"
#include "gimpdrawable-preview.h"
#include "gimpimage.h"
#include "gimpimage-mask.h"
#include "gimplayer.h"
#include "gimplist.h"
#include "gimpmarshal.h"
#include "gimpparasite.h"
#include "gimpparasitelist.h"
#include "gimppreviewcache.h"

#include "undo.h"

#include "libgimp/gimpintl.h"


enum
{
  VISIBILITY_CHANGED,
  REMOVED,
  LAST_SIGNAL
};


/*  local function prototypes  */

static void   gimp_drawable_class_init         (GimpDrawableClass *klass);
static void   gimp_drawable_init               (GimpDrawable      *drawable);

static void   gimp_drawable_finalize           (GObject           *object);

static void   gimp_drawable_name_changed       (GimpObject        *drawable);
static void   gimp_drawable_invalidate_preview (GimpViewable      *viewable);


/*  private variables  */

static guint gimp_drawable_signals[LAST_SIGNAL] = { 0 };

static GimpViewableClass *parent_class = NULL;


GType
gimp_drawable_get_type (void)
{
  static GType drawable_type = 0;

  if (! drawable_type)
    {
      static const GTypeInfo drawable_info =
      {
        sizeof (GimpDrawableClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) gimp_drawable_class_init,
	NULL,           /* class_finalize */
	NULL,           /* class_data     */
	sizeof (GimpDrawable),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) gimp_drawable_init,
      };

      drawable_type = g_type_register_static (GIMP_TYPE_VIEWABLE,
					      "GimpDrawable", 
					      &drawable_info, 0);
    }

  return drawable_type;
}

static void
gimp_drawable_class_init (GimpDrawableClass *klass)
{
  GObjectClass      *object_class;
  GimpObjectClass   *gimp_object_class;
  GimpViewableClass *viewable_class;

  object_class      = G_OBJECT_CLASS (klass);
  gimp_object_class = GIMP_OBJECT_CLASS (klass);
  viewable_class    = GIMP_VIEWABLE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  gimp_drawable_signals[VISIBILITY_CHANGED] =
    g_signal_new ("visibility_changed",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (GimpDrawableClass, visibility_changed),
		  NULL, NULL,
		  gimp_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  gimp_drawable_signals[REMOVED] =
    g_signal_new ("removed",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (GimpDrawableClass, removed),
		  NULL, NULL,
		  gimp_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  object_class->finalize             = gimp_drawable_finalize;

  gimp_object_class->name_changed    = gimp_drawable_name_changed;

  viewable_class->invalidate_preview = gimp_drawable_invalidate_preview;
  viewable_class->get_preview        = gimp_drawable_get_preview;

  klass->visibility_changed          = NULL;
  klass->removed                     = NULL;
}

static void
gimp_drawable_init (GimpDrawable *drawable)
{
  drawable->tiles         = NULL;
  drawable->visible       = FALSE;
  drawable->width         = 0;
  drawable->height        = 0;
  drawable->offset_x      = 0;
  drawable->offset_y      = 0;
  drawable->bytes         = 0;
  drawable->ID            = 0;
  drawable->tattoo        = 0;
  drawable->gimage        = NULL;
  drawable->type          = -1;
  drawable->has_alpha     = FALSE;
  drawable->preview_cache = NULL;
  drawable->preview_valid = FALSE;
  drawable->parasites     = gimp_parasite_list_new ();
  drawable->preview_cache = NULL;
  drawable->preview_valid = FALSE;
}

static void
gimp_drawable_finalize (GObject *object)
{
  GimpDrawable *drawable;

  g_return_if_fail (GIMP_IS_DRAWABLE (object));

  drawable = GIMP_DRAWABLE (object);

  if (drawable->gimage && drawable->gimage->gimp)
    {
      g_hash_table_remove (drawable->gimage->gimp->drawable_table,
			   GINT_TO_POINTER (drawable->ID));
      drawable->gimage = NULL;
    }

  if (drawable->tiles)
    {
      tile_manager_destroy (drawable->tiles);
      drawable->tiles = NULL;
    }

  if (drawable->preview_cache)
    gimp_preview_cache_invalidate (&drawable->preview_cache);

  if (drawable->parasites)
    {
      g_object_unref (G_OBJECT (drawable->parasites));
      drawable->parasites = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gimp_drawable_name_changed (GimpObject *object)
{
  GimpDrawable *drawable;
  GimpDrawable *drawable2;
  GList        *list, *list2, *base_list;
  gint          unique_ext = 0;
  gchar        *ext;
  gchar        *new_name = NULL;

  g_return_if_fail (GIMP_IS_DRAWABLE (object));

  drawable = GIMP_DRAWABLE (object);

  /*  if no other layers to check name against  */
  if (drawable->gimage == NULL || 
      gimp_image_is_empty (drawable->gimage))
    return;

  if (GIMP_IS_LAYER (drawable))
    base_list = GIMP_LIST (drawable->gimage->layers)->list;
  else if (GIMP_IS_CHANNEL (drawable))
    base_list = GIMP_LIST (drawable->gimage->channels)->list;
  else
    base_list = NULL;

  for (list = base_list; 
       list; 
       list = g_list_next (list))
    {
      drawable2 = GIMP_DRAWABLE (list->data);

      if (drawable != drawable2 &&
	  strcmp (gimp_object_get_name (GIMP_OBJECT (drawable)),
		  gimp_object_get_name (GIMP_OBJECT (drawable2))) == 0)
	{
          ext = strrchr (GIMP_OBJECT (drawable)->name, '#');

          if (ext)
            {
	      gchar *ext_str;

	      unique_ext = atoi (ext + 1);

	      ext_str = g_strdup_printf ("%d", unique_ext);

	      /*  check if the extension really is of the form "#<n>"  */
	      if (! strcmp (ext_str, ext + 1))
		{
		  *ext = '\0';
		}
	      else
                {
                  unique_ext = 0;
                }

              g_free (ext_str);
            }
          else
            {
              unique_ext = 0;
            }

	  do
	    {
	      unique_ext++;

	      g_free (new_name);

	      new_name = g_strdup_printf ("%s#%d",
					  GIMP_OBJECT (drawable)->name,
					  unique_ext);

              for (list2 = base_list; list2; list2 = g_list_next (list2))
                {
		  drawable2 = GIMP_DRAWABLE (list2->data);

		  if (drawable == drawable2)
		    continue;

                  if (! strcmp (GIMP_OBJECT (drawable2)->name, new_name))
                    {
                      break;
                    }
                }
            }
          while (list2);

          g_free (GIMP_OBJECT (drawable)->name);

          GIMP_OBJECT (drawable)->name = new_name;

          break;
        }
    }
}

static void
gimp_drawable_invalidate_preview (GimpViewable *viewable)
{
  GimpDrawable *drawable;
  GimpImage    *gimage;

  if (GIMP_VIEWABLE_CLASS (parent_class)->invalidate_preview)
    GIMP_VIEWABLE_CLASS (parent_class)->invalidate_preview (viewable);

  drawable = GIMP_DRAWABLE (viewable);

  drawable->preview_valid = FALSE;

  gimage = gimp_drawable_gimage (drawable);

  if (gimage)
    gimp_viewable_invalidate_preview (GIMP_VIEWABLE (gimage));
}

void
gimp_drawable_configure (GimpDrawable  *drawable,
			 GimpImage     *gimage,
			 gint           width,
			 gint           height, 
			 GimpImageType  type,
			 const gchar   *name)
{
  gint     bpp;
  gboolean alpha;

  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));
  g_return_if_fail (GIMP_IS_IMAGE (gimage));

  drawable->ID = gimage->gimp->next_drawable_ID++;

  g_hash_table_insert (gimage->gimp->drawable_table,
		       GINT_TO_POINTER (drawable->ID),
		       (gpointer) drawable);

  if (!name)
    name = _("Unnamed");

  switch (type)
    {
    case GIMP_RGB_IMAGE:
      bpp = 3; alpha = FALSE; break;
    case GIMP_GRAY_IMAGE:
      bpp = 1; alpha = FALSE; break;
    case GIMP_RGBA_IMAGE:
      bpp = 4; alpha = TRUE; break;
    case GIMP_GRAYA_IMAGE:
      bpp = 2; alpha = TRUE; break;
    case GIMP_INDEXED_IMAGE:
      bpp = 1; alpha = FALSE; break;
    case GIMP_INDEXEDA_IMAGE:
      bpp = 2; alpha = TRUE; break;
    default:
      g_warning ("%s: Layer type %d not supported.", 
		 G_GNUC_PRETTY_FUNCTION, type);
      return;
    }

  drawable->width     = width;
  drawable->height    = height;
  drawable->bytes     = bpp;
  drawable->type      = type;
  drawable->has_alpha = alpha;
  drawable->offset_x  = 0;
  drawable->offset_y  = 0;

  if (drawable->tiles)
    tile_manager_destroy (drawable->tiles);

  drawable->tiles = tile_manager_new (width, height, bpp);

  drawable->visible = TRUE;

  gimp_drawable_set_gimage (drawable, gimage);

  gimp_object_set_name (GIMP_OBJECT (drawable), name);

  /*  preview variables  */
  drawable->preview_cache = NULL;
  drawable->preview_valid = FALSE;
}

gint
gimp_drawable_get_ID (GimpDrawable *drawable)
{
  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), -1);

  return drawable->ID;
}

GimpDrawable *
gimp_drawable_get_by_ID (Gimp *gimp,
			 gint  drawable_id)
{
  g_return_val_if_fail (GIMP_IS_GIMP (gimp), NULL);

  if (gimp->drawable_table == NULL)
    return NULL;

  return (GimpDrawable *) g_hash_table_lookup (gimp->drawable_table, 
					       GINT_TO_POINTER (drawable_id));
}

GimpImage *
gimp_drawable_gimage (const GimpDrawable *drawable)
{
  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), NULL);

  return drawable->gimage;
}

void
gimp_drawable_set_gimage (GimpDrawable *drawable,
			  GimpImage    *gimage)
{
  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));
  g_return_if_fail (! gimage || GIMP_IS_IMAGE (gimage));;

  if (gimage == NULL)
    drawable->tattoo = 0;
  else if (drawable->tattoo == 0 || drawable->gimage != gimage )
    drawable->tattoo = gimp_image_get_new_tattoo (gimage);

  drawable->gimage = gimage;
}

void
gimp_drawable_update (GimpDrawable *drawable,
		      gint          x,
		      gint          y,
		      gint          w,
		      gint          h)
{
  GimpImage *gimage;
  gint       offset_x;
  gint       offset_y;

  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));

  gimage = gimp_drawable_gimage (drawable);

  g_return_if_fail (gimage != NULL);

  gimp_drawable_offsets (drawable, &offset_x, &offset_y);
  x += offset_x;
  y += offset_y;

  gimp_image_update (gimage, x, y, w, h);

  gimp_viewable_invalidate_preview (GIMP_VIEWABLE (drawable));
}

void
gimp_drawable_apply_image (GimpDrawable *drawable, 
			   gint          x1,
			   gint          y1,
			   gint          x2,
			   gint          y2, 
			   TileManager  *tiles,
			   gint          sparse)
{
  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));

  if (! tiles)
    undo_push_image (drawable->gimage, drawable, 
		     x1, y1, x2, y2);
  else
    undo_push_image_mod (drawable->gimage, drawable, 
			 x1, y1, x2, y2, tiles, sparse);
}

void
gimp_drawable_merge_shadow (GimpDrawable *drawable,
			    gboolean      undo)
{
  GimpImage   *gimage;
  PixelRegion  shadowPR;
  gint         x1, y1, x2, y2;

  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));

  gimage = gimp_drawable_gimage (drawable);

  g_return_if_fail (GIMP_IS_IMAGE (gimage));
  g_return_if_fail (gimage->shadow != NULL);

  /*  A useful optimization here is to limit the update to the
   *  extents of the selection mask, as it cannot extend beyond
   *  them.
   */
  gimp_drawable_mask_bounds (drawable, &x1, &y1, &x2, &y2);
  pixel_region_init (&shadowPR, gimage->shadow, x1, y1,
		     (x2 - x1), (y2 - y1), FALSE);
  gimp_image_apply_image (gimage, drawable, &shadowPR, undo, OPAQUE_OPACITY,
			  GIMP_REPLACE_MODE, NULL, x1, y1);
}

void
gimp_drawable_fill (GimpDrawable  *drawable,
		    const GimpRGB *color)
{
  GimpImage   *gimage;
  PixelRegion  destPR;
  guchar       c[MAX_CHANNELS];
  guchar       i;

  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));

  gimage = gimp_drawable_gimage (drawable);

  g_return_if_fail (gimage != NULL);

  switch (gimp_drawable_type (drawable))
    {
    case GIMP_RGB_IMAGE: case GIMP_RGBA_IMAGE:
      gimp_rgba_get_uchar (color,
			   &c[RED_PIX],
			   &c[GREEN_PIX],
			   &c[BLUE_PIX],
			   &c[ALPHA_PIX]);
      if (gimp_drawable_type (drawable) != GIMP_RGBA_IMAGE)
	c[ALPHA_PIX] = 255;
      break;

    case GIMP_GRAY_IMAGE: case GIMP_GRAYA_IMAGE:
      gimp_rgba_get_uchar (color,
			   &c[GRAY_PIX],
			   NULL,
			   NULL,
			   &c[ALPHA_G_PIX]);
      if (gimp_drawable_type (drawable) != GIMP_GRAYA_IMAGE)
	c[ALPHA_G_PIX] = 255;
      break;

    case GIMP_INDEXED_IMAGE: case GIMP_INDEXEDA_IMAGE:
      gimp_rgb_get_uchar (color,
			  &c[RED_PIX],
			  &c[GREEN_PIX],
			  &c[BLUE_PIX]);
      gimp_image_transform_color (gimage, drawable, c, &i, GIMP_RGB);
      c[INDEXED_PIX] = i;
      if (gimp_drawable_type (drawable) == GIMP_INDEXEDA_IMAGE)
	gimp_rgba_get_uchar (color,
			     NULL,
			     NULL,
			     NULL,
			     &c[ALPHA_I_PIX]);
      break;

    default:
      g_warning ("%s: Cannot fill unknown image type.", 
                 G_GNUC_PRETTY_FUNCTION);
      break;
    }

  pixel_region_init (&destPR,
		     gimp_drawable_data (drawable),
		     0, 0,
		     gimp_drawable_width  (drawable),
		     gimp_drawable_height (drawable),
		     TRUE);
  color_region (&destPR, c);

  gimp_drawable_update (drawable,
			0, 0,
			gimp_drawable_width  (drawable),
			gimp_drawable_height (drawable));
}

void
gimp_drawable_fill_by_type (GimpDrawable *drawable,
			    GimpContext  *context,
			    GimpFillType  fill_type)
{
  GimpRGB color;

  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));

  color.a = 1.0;

  switch (fill_type)
    {
    case FOREGROUND_FILL:
      gimp_context_get_foreground (context, &color);
      break;

    case BACKGROUND_FILL:
      gimp_context_get_background (context, &color);
      break;

    case WHITE_FILL:
      gimp_rgb_set (&color, 1.0, 1.0, 1.0);
      break;

    case TRANSPARENT_FILL:
      gimp_rgba_set (&color, 0.0, 0.0, 0.0, 0.0);
      break;

    case NO_FILL:
      return;

    default:
      g_warning ("%s: unknown fill type %d", G_GNUC_PRETTY_FUNCTION, fill_type);
      return;
    }

  gimp_drawable_fill (drawable, &color);
}

gboolean
gimp_drawable_mask_bounds (GimpDrawable *drawable, 
			   gint         *x1,
			   gint         *y1,
			   gint         *x2,
			   gint         *y2)
{
  GimpImage *gimage;
  gint       off_x, off_y;

  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), FALSE);

  gimage = gimp_drawable_gimage (drawable);

  g_return_val_if_fail (gimage != NULL, FALSE);

  if (gimp_image_mask_bounds (gimage, x1, y1, x2, y2))
    {
      gimp_drawable_offsets (drawable, &off_x, &off_y);
      *x1 = CLAMP (*x1 - off_x, 0, gimp_drawable_width  (drawable));
      *y1 = CLAMP (*y1 - off_y, 0, gimp_drawable_height (drawable));
      *x2 = CLAMP (*x2 - off_x, 0, gimp_drawable_width  (drawable));
      *y2 = CLAMP (*y2 - off_y, 0, gimp_drawable_height (drawable));
      return TRUE;
    }
  else
    {
      *x2 = gimp_drawable_width  (drawable);
      *y2 = gimp_drawable_height (drawable);
      return FALSE;
    }
}

/* The removed signal is sent out when the layer is no longer
 * associcated with an image.  It's needed because layers aren't
 * destroyed immediately, but kept around for undo purposes.  Connect
 * to the removed signal to update bits of UI that are tied to a
 * particular layer.
 */
void
gimp_drawable_removed (GimpDrawable *drawable)
{
  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));

  g_signal_emit (G_OBJECT (drawable), gimp_drawable_signals[REMOVED], 0);
}

gboolean
gimp_drawable_has_alpha (const GimpDrawable *drawable)
{
  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), FALSE);

  return drawable->has_alpha;
}

GimpImageType
gimp_drawable_type (const GimpDrawable *drawable)
{
  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), -1);

  return drawable->type;
}

GimpImageType
gimp_drawable_type_with_alpha (const GimpDrawable *drawable)
{
  GimpImageType type;

  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), -1);

  type = gimp_drawable_type (drawable);

  if (gimp_drawable_has_alpha (drawable))
    {
      return type;
    }
  else
    {
      switch (type)
	{
	case GIMP_RGB_IMAGE:
	  return GIMP_RGBA_IMAGE;
	  break;
	case GIMP_GRAY_IMAGE:
	  return GIMP_GRAYA_IMAGE;
	  break;
	case GIMP_INDEXED_IMAGE:
	  return GIMP_INDEXEDA_IMAGE;
	  break;
	default:
	  g_assert_not_reached ();
	  break;
	}
    }

  return 0;
}

gboolean
gimp_drawable_get_visible (const GimpDrawable *drawable)
{
  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), FALSE);

  return drawable->visible;
}

void
gimp_drawable_set_visible (GimpDrawable *drawable,
                           gboolean      visible)
{
  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));

  visible = visible ? TRUE : FALSE;

  if (drawable->visible != visible)
    {
      drawable->visible = visible;

      g_signal_emit (G_OBJECT (drawable),
		     gimp_drawable_signals[VISIBILITY_CHANGED], 0);

      gimp_drawable_update (drawable,
			    0, 0,
			    drawable->width,
			    drawable->height);
    }
}

guchar *
gimp_drawable_get_color_at (GimpDrawable *drawable,
			    gint          x,
			    gint          y)
{
  Tile   *tile;
  guchar *src;
  guchar *dest;

  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (gimp_drawable_gimage (drawable) ||
			! gimp_drawable_is_indexed (drawable), NULL);

  /* do not make this a g_return_if_fail() */
  if ( !(x >= 0 && x < drawable->width && y >= 0 && y < drawable->height))
    return NULL;

  dest = g_new (guchar, 5);

  tile = tile_manager_get_tile (gimp_drawable_data (drawable), x, y,
				TRUE, FALSE);
  src = tile_data_pointer (tile, x % TILE_WIDTH, y % TILE_HEIGHT);

  gimp_image_get_color (gimp_drawable_gimage (drawable),
			gimp_drawable_type (drawable), dest, src);

  if (GIMP_IMAGE_TYPE_HAS_ALPHA (gimp_drawable_type (drawable)))
    dest[3] = src[gimp_drawable_bytes (drawable) - 1];
  else
    dest[3] = 255;

  if (gimp_drawable_is_indexed (drawable))
    dest[4] = src[0];
  else
    dest[4] = 0;

  tile_release (tile, FALSE);

  return dest;
}

GimpParasite *
gimp_drawable_parasite_find (const GimpDrawable *drawable,
			     const gchar        *name)
{
  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), NULL);

  return gimp_parasite_list_find (drawable->parasites, name);
}

static void
gimp_drawable_parasite_list_foreach_func (gchar          *key,
					  GimpParasite   *p,
					  gchar        ***cur)
{
  *(*cur)++ = (gchar *) g_strdup (key);
}

gchar **
gimp_drawable_parasite_list (const GimpDrawable *drawable,
			     gint               *count)
{
  gchar **list;
  gchar **cur;

  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (count != NULL, NULL);

  *count = gimp_parasite_list_length (drawable->parasites);
  cur = list = g_new (gchar *, *count);

  gimp_parasite_list_foreach (drawable->parasites,
			      (GHFunc) gimp_drawable_parasite_list_foreach_func,
			      &cur);

  return list;
}

void
gimp_drawable_parasite_attach (GimpDrawable *drawable,
			       GimpParasite *parasite)
{
  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));

  /* only set the dirty bit manually if we can be saved and the new
     parasite differs from the current one and we arn't undoable */
  if (gimp_parasite_is_undoable (parasite))
    {
      /* do a group in case we have attach_parent set */
      undo_push_group_start (drawable->gimage, PARASITE_ATTACH_UNDO);

      undo_push_drawable_parasite (drawable->gimage, drawable, parasite);
    }
  else if (gimp_parasite_is_persistent (parasite) &&
	   ! gimp_parasite_compare (parasite,
				    gimp_drawable_parasite_find
				    (drawable, gimp_parasite_name (parasite))))
    {
      undo_push_cantundo (drawable->gimage, _("parasite attached to drawable"));
    }

  gimp_parasite_list_add (drawable->parasites, parasite);

  if (gimp_parasite_has_flag (parasite, GIMP_PARASITE_ATTACH_PARENT))
    {
      gimp_parasite_shift_parent (parasite);
      gimp_image_parasite_attach (drawable->gimage, parasite);
    }
  else if (gimp_parasite_has_flag (parasite, GIMP_PARASITE_ATTACH_GRANDPARENT))
    {
      gimp_parasite_shift_parent (parasite);
      gimp_parasite_shift_parent (parasite);
      gimp_parasite_attach (drawable->gimage->gimp, parasite);
    }

  if (gimp_parasite_is_undoable (parasite))
    {
      undo_push_group_end (drawable->gimage);
    }
}

void
gimp_drawable_parasite_detach (GimpDrawable *drawable,
			       const gchar  *parasite)
{
  GimpParasite *p;

  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));

  if (! (p = gimp_parasite_list_find (drawable->parasites, parasite)))
    return;

  if (gimp_parasite_is_undoable (p))
    undo_push_drawable_parasite_remove (drawable->gimage, drawable,
					gimp_parasite_name (p));
  else if (gimp_parasite_is_persistent (p))
    undo_push_cantundo (drawable->gimage, _("parasite detached from drawable"));

  gimp_parasite_list_remove (drawable->parasites, parasite);
}

GimpTattoo
gimp_drawable_get_tattoo (const GimpDrawable *drawable)
{
  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), 0); 

  return drawable->tattoo;
}

void
gimp_drawable_set_tattoo (GimpDrawable *drawable,
			  GimpTattoo    tattoo)
{
  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));

  drawable->tattoo = tattoo;
}

gboolean
gimp_drawable_is_rgb (const GimpDrawable *drawable)
{
  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), FALSE);

  if (gimp_drawable_type (drawable) == GIMP_RGBA_IMAGE ||
      gimp_drawable_type (drawable) == GIMP_RGB_IMAGE)
    return TRUE;
  else
    return FALSE;
}

gboolean
gimp_drawable_is_gray (const GimpDrawable *drawable)
{
  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), FALSE);

  if (gimp_drawable_type (drawable) == GIMP_GRAYA_IMAGE ||
      gimp_drawable_type (drawable) == GIMP_GRAY_IMAGE)
    return TRUE;
  else
    return FALSE;
}

gboolean
gimp_drawable_is_indexed (const GimpDrawable *drawable)
{
  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), FALSE);

  if (gimp_drawable_type (drawable) == GIMP_INDEXEDA_IMAGE ||
      gimp_drawable_type (drawable) == GIMP_INDEXED_IMAGE)
    return TRUE;
  else
    return FALSE;
}

TileManager *
gimp_drawable_data (const GimpDrawable *drawable)
{
  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), NULL);

  return drawable->tiles;
}

TileManager *
gimp_drawable_shadow (GimpDrawable *drawable)
{
  GimpImage *gimage;

  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), NULL);

  if (! (gimage = gimp_drawable_gimage (drawable)))
    return NULL;

  return gimp_image_shadow (gimage, drawable->width, drawable->height, 
			    drawable->bytes);
}

int
gimp_drawable_bytes (const GimpDrawable *drawable)
{
  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), -1);

  return drawable->bytes;
}

gint
gimp_drawable_width (const GimpDrawable *drawable)
{
  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), -1);

  return drawable->width;
}

gint
gimp_drawable_height (const GimpDrawable *drawable)
{
  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), -1);

  return drawable->height;
}

void
gimp_drawable_offsets (const GimpDrawable *drawable,
		       gint               *off_x,
		       gint               *off_y)
{
  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));

  *off_x = drawable->offset_x;
  *off_y = drawable->offset_y;
}

guchar *
gimp_drawable_cmap (const GimpDrawable *drawable)
{
  GimpImage *gimage;

  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), NULL);

  if (! (gimage = gimp_drawable_gimage (drawable)))
    return NULL;

  return gimage->cmap;
}
