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

#include <glib-object.h>

#include "base-types.h"

#include "base-config.h"

#ifdef  __GNUC__
#define INIT_MEMBER(m,v) .m = v
#else
/* declaration order required ! */
#define INIT_MEMBER(m,v) v
#endif

static GimpBaseConfig  static_base_config =
{
  INIT_MEMBER(temp_path        , NULL),
  INIT_MEMBER(swap_path        , NULL),

  INIT_MEMBER(tile_cache_size  , 33554432),
  INIT_MEMBER(stingy_memory_use, FALSE),
  INIT_MEMBER(num_processors   , 1)
};

#undef INIT_MEMBER

GimpBaseConfig *base_config = &static_base_config;
gboolean        use_mmx     = FALSE;
