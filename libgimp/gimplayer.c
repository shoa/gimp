/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * gimplayer.c
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "gimp.h"

gint32
gimp_layer_new (gint32                image_ID,
		gchar                *name,
		gint                  width,
		gint                  height,
		GimpImageType         type,
		gdouble               opacity,
		GimpLayerModeEffects  mode)
{
  return _gimp_layer_new (image_ID,
			  width,
			  height,
			  type,
			  name,
			  opacity,
			  mode);
}

gint32
gimp_layer_copy (gint32  layer_ID)
{
  return _gimp_layer_copy (layer_ID, FALSE);
}
