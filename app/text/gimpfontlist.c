/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpfontlist.c
 * Copyright (C) 2003 Michael Natterer <mitch@gimp.org>
 *                    Sven Neumann <sven@gimp.org>
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

#include <string.h>

#include <glib-object.h>
#include <pango/pangoft2.h>

#include "text-types.h"

#include "gimpfont.h"
#include "gimpfontlist.h"

#include "gimp-intl.h"


/* Use fontconfig directly for speed. We can use the pango stuff when/if
 * fontconfig/pango get more efficient.
 */
#define USE_FONTCONFIG_DIRECTLY

#ifdef USE_FONTCONFIG_DIRECTLY
/* PangoFT2 is assumed, so we should have this in our cflags */
#include <fontconfig/fontconfig.h>
#endif


static void   gimp_font_list_class_init   (GimpFontListClass    *klass);
static void   gimp_font_list_init         (GimpFontList         *list);

static void   gimp_font_list_add_font     (GimpFontList         *list,
                                           PangoContext         *context,
                                           PangoFontDescription *desc);

static void   gimp_font_list_load_names   (GimpFontList         *list,
                                           PangoFontMap         *fontmap,
                                           PangoContext         *context);


static GimpListClass *parent_class = NULL;


GType
gimp_font_list_get_type (void)
{
  static GType list_type = 0;

  if (! list_type)
    {
      static const GTypeInfo list_info =
      {
        sizeof (GimpFontListClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) gimp_font_list_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_font     */
        sizeof (GimpFontList),
        0,              /* n_preallocs    */
        (GInstanceInitFunc) gimp_font_list_init,
      };

      list_type = g_type_register_static (GIMP_TYPE_LIST,
                                          "GimpFontList",
                                          &list_info, 0);
    }

  return list_type;
}

static void
gimp_font_list_class_init (GimpFontListClass *klass)
{
  parent_class = g_type_class_peek_parent (klass);
}

static void
gimp_font_list_init (GimpFontList *list)
{
}

GimpContainer *
gimp_font_list_new (gdouble xresolution,
                    gdouble yresolution)
{
  GimpFontList *list;

  g_return_val_if_fail (xresolution > 0.0, NULL);
  g_return_val_if_fail (yresolution > 0.0, NULL);

  list = g_object_new (GIMP_TYPE_FONT_LIST,
                       "children_type", GIMP_TYPE_FONT,
                       "policy",        GIMP_CONTAINER_POLICY_STRONG,
                       NULL);

  list->xresolution = xresolution;
  list->yresolution = yresolution;

  return GIMP_CONTAINER (list);
}

void
gimp_font_list_restore (GimpFontList *list)
{
  PangoFontMap     *fontmap;
  PangoContext     *context;

  g_return_if_fail (GIMP_IS_FONT_LIST (list));

  fontmap = pango_ft2_font_map_new ();
  pango_ft2_font_map_set_resolution (PANGO_FT2_FONT_MAP (fontmap),
                                     list->xresolution, list->yresolution);

  context = pango_ft2_font_map_create_context (PANGO_FT2_FONT_MAP (fontmap));
  g_object_unref (fontmap);

  gimp_container_freeze (GIMP_CONTAINER (list));

  gimp_font_list_load_names (list, fontmap, context);
  g_object_unref (context);

  gimp_list_sort_by_name (GIMP_LIST (list));

  gimp_container_thaw (GIMP_CONTAINER (list));
}

static void
gimp_font_list_add_font (GimpFontList         *list,
                         PangoContext         *context,
                         PangoFontDescription *desc)
{
  GimpFont *font;
  gchar    *name;

  if (desc == NULL)
    return;

  name = pango_font_description_to_string (desc);

  font = g_object_new (GIMP_TYPE_FONT,
                       "name",          name,
                       "pango-context", context,
                       NULL);

  g_free (name);

  gimp_container_add (GIMP_CONTAINER (list), GIMP_OBJECT (font));
  g_object_unref (font);
}

#ifdef USE_FONTCONFIG_DIRECTLY
/* We're really chummy here with the implementation. Oh well. */

/* This is copied straight from make_alias_description in pango, plus
 * the gimp_font_list_add_font bits. */
static void
gimp_font_list_make_alias (GimpFontList *list,
                           PangoContext *context,
                           const gchar  *family,
                           gboolean      bold,
                           gboolean      italic)
{
  PangoFontDescription *desc = pango_font_description_new ();

  pango_font_description_set_family (desc, family);
  pango_font_description_set_style (desc,
                                    italic ?
                                    PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL);
  pango_font_description_set_variant (desc, PANGO_VARIANT_NORMAL);
  pango_font_description_set_weight (desc,
                                     bold ?
                                     PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL);
  pango_font_description_set_stretch (desc, PANGO_STRETCH_NORMAL);

  gimp_font_list_add_font (list, context, desc);

  pango_font_description_free (desc);
}

static void
gimp_font_list_load_aliases (GimpFontList *list,
                             PangoContext *context)
{
  const gchar *families[] = { "Sans", "Serif", "Monospace" };
  gint         i;

  for (i = 0; i < 3; i++)
    {
      gimp_font_list_make_alias (list, context, families[i], FALSE, FALSE);
      gimp_font_list_make_alias (list, context, families[i], TRUE,  FALSE);
      gimp_font_list_make_alias (list, context, families[i], FALSE, TRUE);
      gimp_font_list_make_alias (list, context, families[i], TRUE,  TRUE);
    }
}

/* This is copied straight from _pango_fc_font_desc_from_pattern, minus
 * the size bits.
 * FIXME: Use pango_fc_font_description_from_pattern from 1.4 when we can.
 */
static PangoFontDescription *
gimp_font_list_font_desc_from_pattern (FcPattern *pattern)
{
  PangoFontDescription *desc;
  PangoStyle            style;
  PangoWeight           weight;
  PangoStretch          stretch;
  FcChar8              *s;
  gint                  i;
  FcResult              res;

  desc = pango_font_description_new ();

  res = FcPatternGetString (pattern, FC_FAMILY, 0, (FcChar8 **) &s);
  g_assert (res == FcResultMatch);

  pango_font_description_set_family (desc, (gchar *)s);

  if (FcPatternGetInteger (pattern, FC_SLANT, 0, &i) == FcResultMatch)
    {
      switch (i)
        {
        case FC_SLANT_ROMAN:
          style = PANGO_STYLE_NORMAL;
          break;
        case FC_SLANT_ITALIC:
          style = PANGO_STYLE_ITALIC;
          break;
        case FC_SLANT_OBLIQUE:
          style = PANGO_STYLE_OBLIQUE;
          break;
        default:
          style = PANGO_STYLE_NORMAL;
          break;
        }
    }
  else
    style = PANGO_STYLE_NORMAL;

  pango_font_description_set_style (desc, style);

  if (FcPatternGetInteger (pattern, FC_WEIGHT, 0, &i) == FcResultMatch)
    {
      if (i < FC_WEIGHT_LIGHT)
        weight = PANGO_WEIGHT_ULTRALIGHT;
      else if (i < (FC_WEIGHT_LIGHT + FC_WEIGHT_MEDIUM) / 2)
        weight = PANGO_WEIGHT_LIGHT;
      else if (i < (FC_WEIGHT_MEDIUM + FC_WEIGHT_DEMIBOLD) / 2)
        weight = PANGO_WEIGHT_NORMAL;
      else if (i < (FC_WEIGHT_DEMIBOLD + FC_WEIGHT_BOLD) / 2)
        weight = 600;
      else if (i < (FC_WEIGHT_BOLD + FC_WEIGHT_BLACK) / 2)
        weight = PANGO_WEIGHT_BOLD;
      else
        weight = PANGO_WEIGHT_ULTRABOLD;
    }
  else
    weight = PANGO_WEIGHT_NORMAL;

  pango_font_description_set_weight (desc, weight);
  
  if (FcPatternGetInteger (pattern, FC_WIDTH, 0, &i) == FcResultMatch)
    {
      switch (i)
        {
        case FC_WIDTH_NORMAL:
          stretch = PANGO_STRETCH_NORMAL;
          break;
        case FC_WIDTH_ULTRACONDENSED:
          stretch = PANGO_STRETCH_ULTRA_CONDENSED;
          break;
        case FC_WIDTH_EXTRACONDENSED:
          stretch = PANGO_STRETCH_EXTRA_CONDENSED;
          break;
        case FC_WIDTH_CONDENSED:
          stretch = PANGO_STRETCH_CONDENSED;
          break;
        case FC_WIDTH_SEMICONDENSED:
          stretch = PANGO_STRETCH_SEMI_CONDENSED;
          break;
        case FC_WIDTH_SEMIEXPANDED:
          stretch = PANGO_STRETCH_SEMI_EXPANDED;
          break;
        case FC_WIDTH_EXPANDED:
          stretch = PANGO_STRETCH_EXPANDED;
          break;
        case FC_WIDTH_EXTRAEXPANDED:
          stretch = PANGO_STRETCH_EXTRA_EXPANDED;
          break;
        case FC_WIDTH_ULTRAEXPANDED:
          stretch = PANGO_STRETCH_ULTRA_EXPANDED;
          break;
        default:
          stretch = PANGO_STRETCH_NORMAL;
          break;
        }
    }
  else
    stretch = PANGO_STRETCH_NORMAL;

  pango_font_description_set_stretch (desc, stretch);

  pango_font_description_set_variant (desc, PANGO_VARIANT_NORMAL);

  return desc;
}

static void
gimp_font_list_load_names (GimpFontList *list,
                           PangoFontMap *fontmap,
                           PangoContext *context)
{
  FcObjectSet *os;
  FcPattern   *pat;
  FcFontSet   *fontset;
  gint         i;

  os = FcObjectSetBuild (FC_FAMILY, FC_STYLE,
                         FC_SLANT, FC_WEIGHT, FC_WIDTH,
                         NULL);

  pat = FcPatternCreate ();

  fontset = FcFontList (NULL, pat, os);

  FcPatternDestroy (pat);
  FcObjectSetDestroy (os);

  for (i = 0; i < fontset->nfont; i++)
    {
      PangoFontDescription *desc;

      desc = gimp_font_list_font_desc_from_pattern (fontset->fonts[i]);
      gimp_font_list_add_font (list, context, desc);
      pango_font_description_free (desc);
    }

  /*  only create aliases if there is at least one font available  */
  if (fontset->nfont > 0)
    gimp_font_list_load_aliases (list, context);

  FcFontSetDestroy (fontset);
}

#else  /* ! USE_FONTCONFIG_DIRECTLY  */

static void
gimp_font_list_load_names (GimpFontList *list,
                           PangoFontMap *fontmap,
                           PangoContext *context)
{
  PangoFontFamily **families;
  PangoFontFace   **faces;
  gint              n_families;
  gint              n_faces;
  gint              i, j;

  pango_font_map_list_families (fontmap, &families, &n_families);

  for (i = 0; i < n_families; i++)
    {
      pango_font_family_list_faces (families[i], &faces, &n_faces);

      for (j = 0; j < n_faces; j++)
        {
          PangoFontDescription *desc;

          desc = pango_font_face_describe (faces[j]);
          gimp_font_list_add_font (list, context, desc);
          pango_font_description_free (desc);
        }
    }

  g_free (families);
}
#endif
