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

#include "vectors-types.h"

#include "base/temp-buf.h"

#include "gimpvectors.h"
#include "gimpvectors-preview.h"


/*  public functions  */

TempBuf *
gimp_vectors_get_new_preview (GimpViewable *viewable,
                              gint          width,
                              gint          height)
{
  GimpVectors *vectors;
  TempBuf     *temp_buf;
  guchar       white[1] = { 255 };

  vectors = GIMP_VECTORS (viewable);

  temp_buf = temp_buf_new (width, height, 1, 0, 0, white);

  return temp_buf;
}
