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

#include <string.h>

#include <glib-object.h>

#include "libgimpmath/gimpmath.h"

#include "core-types.h"

#include "config/gimpcoreconfig.h"

#include "gimp.h"
#include "gimpimage.h"
#include "gimpitem.h"
#include "gimpitem-preview.h"


/*  public functions  */

void
gimp_item_get_preview_size (GimpViewable *viewable,
                            gint          size,
                            gboolean      is_popup,
                            gboolean      dot_for_dot,
                            gint         *width,
                            gint         *height)
{
  GimpItem  *item;
  GimpImage *gimage;

  item   = GIMP_ITEM (viewable);
  gimage = gimp_item_get_image (item);

  if (gimage && ! gimage->gimp->config->layer_previews && ! is_popup)
    {
      *width  = size;
      *height = size;
      return;
    }

  if (gimage && ! is_popup)
    {
      gimp_viewable_calc_preview_size (viewable,
                                       gimage->width,
                                       gimage->height,
                                       size,
                                       size,
                                       dot_for_dot,
                                       gimage->xresolution,
                                       gimage->yresolution,
                                       width,
                                       height,
                                       NULL);
    }
  else
    {
      gimp_viewable_calc_preview_size (viewable,
                                       item->width,
                                       item->height,
                                       size,
                                       size,
                                       dot_for_dot,
                                       1.0,
                                       1.0,
                                       width,
                                       height,
                                       NULL);
    }
}

gboolean
gimp_item_get_popup_size (GimpViewable *viewable,
                          gint          width,
                          gint          height,
                          gboolean      dot_for_dot,
                          gint         *popup_width,
                          gint         *popup_height)
{
  GimpItem  *item;
  GimpImage *gimage;

  item   = GIMP_ITEM (viewable);
  gimage = gimp_item_get_image (item);

  if (gimage && ! gimage->gimp->config->layer_previews)
    return FALSE;

  if (item->width > width || item->height > height)
    {
      gboolean scaling_up;

      gimp_viewable_calc_preview_size (viewable,
                                       item->width,
                                       item->height,
                                       MIN (width  * 2,
                                            GIMP_VIEWABLE_MAX_POPUP_SIZE),
                                       MIN (height * 2,
                                            GIMP_VIEWABLE_MAX_POPUP_SIZE),
                                       dot_for_dot,
                                       gimage ? gimage->xresolution : 1.0,
                                       gimage ? gimage->yresolution : 1.0,
                                       popup_width,
                                       popup_height,
                                       &scaling_up);

      if (scaling_up)
        {
          *popup_width = item->width;
          *popup_width = item->height;
        }

      return TRUE;
    }

  return FALSE;
}
