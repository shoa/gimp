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

#include <gtk/gtk.h>

#include "libgimpcolor/gimpcolor.h"

#include "apptypes.h"

#include "core/gimpcontext.h"

#include "libgimp_glue.h"


gboolean
gimp_palette_set_foreground (const GimpRGB *color)
{
  g_return_val_if_fail (color != NULL, FALSE);

  gimp_context_set_foreground (NULL, color);

  return TRUE;
}

gboolean
gimp_palette_get_foreground (GimpRGB *color)
{
  g_return_val_if_fail (color != NULL, FALSE);

  gimp_context_get_foreground (NULL, color);

  return TRUE;
}

gboolean
gimp_palette_set_background (const GimpRGB *color)
{
  g_return_val_if_fail (color != NULL, FALSE);

  gimp_context_set_background (NULL, color);

  return TRUE;
}

gboolean
gimp_palette_get_background (GimpRGB *color)
{
  g_return_val_if_fail (color != NULL, FALSE);

  gimp_context_get_background (NULL, color);

  return TRUE;
}
