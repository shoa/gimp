/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * gimpdrawable_pdb.c
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

/* NOTE: This file is autogenerated by pdbgen.pl */

#include "gimp.h"

void
gimp_drawable_merge_shadow (gint32   drawable_ID,
			    gboolean undo)
{
  GParam *return_vals;
  gint nreturn_vals;

  return_vals = gimp_run_procedure ("gimp_drawable_merge_shadow",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_INT32, undo,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);
}

void
gimp_drawable_fill (gint32       drawable_ID,
		    GimpFillType fill_type)
{
  GParam *return_vals;
  gint nreturn_vals;

  return_vals = gimp_run_procedure ("gimp_drawable_fill",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_INT32, fill_type,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);
}

void
gimp_drawable_update (gint32 drawable_ID,
		      gint   x,
		      gint   y,
		      gint   width,
		      gint   height)
{
  GParam *return_vals;
  gint nreturn_vals;

  return_vals = gimp_run_procedure ("gimp_drawable_update",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_INT32, x,
				    PARAM_INT32, y,
				    PARAM_INT32, width,
				    PARAM_INT32, height,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);
}

gboolean
gimp_drawable_mask_bounds (gint32  drawable_ID,
			   gint   *x1,
			   gint   *y1,
			   gint   *x2,
			   gint   *y2)
{
  GParam *return_vals;
  gint nreturn_vals;
  gboolean non_empty = FALSE;

  return_vals = gimp_run_procedure ("gimp_drawable_mask_bounds",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_END);

  *x1 = *y1 = *x2 = *y2 = 0;
  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    {
      non_empty = return_vals[1].data.d_int32;
      *x1 = return_vals[2].data.d_int32;
      *y1 = return_vals[3].data.d_int32;
      *x2 = return_vals[4].data.d_int32;
      *y2 = return_vals[5].data.d_int32;
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return non_empty;
}

gint32
gimp_drawable_image (gint32 drawable_ID)
{
  GParam *return_vals;
  gint nreturn_vals;
  gint32 image_ID = -1;

  return_vals = gimp_run_procedure ("gimp_drawable_image",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    image_ID = return_vals[1].data.d_image;

  gimp_destroy_params (return_vals, nreturn_vals);

  return image_ID;
}

GimpImageType
gimp_drawable_type (gint32 drawable_ID)
{
  GParam *return_vals;
  gint nreturn_vals;
  GimpImageType type = 0;

  return_vals = gimp_run_procedure ("gimp_drawable_type",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    type = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return type;
}

gboolean
gimp_drawable_has_alpha (gint32 drawable_ID)
{
  GParam *return_vals;
  gint nreturn_vals;
  gboolean has_alpha = TRUE;

  return_vals = gimp_run_procedure ("gimp_drawable_has_alpha",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    has_alpha = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return has_alpha;
}

GimpImageType
gimp_drawable_type_with_alpha (gint32 drawable_ID)
{
  GParam *return_vals;
  gint nreturn_vals;
  GimpImageType type_with_alpha = 0;

  return_vals = gimp_run_procedure ("gimp_drawable_type_with_alpha",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    type_with_alpha = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return type_with_alpha;
}

gboolean
gimp_drawable_is_rgb (gint32 drawable_ID)
{
  GParam *return_vals;
  gint nreturn_vals;
  gboolean color = FALSE;

  return_vals = gimp_run_procedure ("gimp_drawable_is_rgb",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    color = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return color;
}

gboolean
gimp_drawable_is_gray (gint32 drawable_ID)
{
  GParam *return_vals;
  gint nreturn_vals;
  gboolean gray = FALSE;

  return_vals = gimp_run_procedure ("gimp_drawable_is_gray",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    gray = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return gray;
}

gboolean
gimp_drawable_is_indexed (gint32 drawable_ID)
{
  GParam *return_vals;
  gint nreturn_vals;
  gboolean indexed = FALSE;

  return_vals = gimp_run_procedure ("gimp_drawable_is_indexed",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    indexed = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return indexed;
}

gint
gimp_drawable_bytes (gint32 drawable_ID)
{
  GParam *return_vals;
  gint nreturn_vals;
  gint bytes = 0;

  return_vals = gimp_run_procedure ("gimp_drawable_bytes",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    bytes = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return bytes;
}

gint
gimp_drawable_width (gint32 drawable_ID)
{
  GParam *return_vals;
  gint nreturn_vals;
  gint width = 0;

  return_vals = gimp_run_procedure ("gimp_drawable_width",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    width = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return width;
}

gint
gimp_drawable_height (gint32 drawable_ID)
{
  GParam *return_vals;
  gint nreturn_vals;
  gint height = 0;

  return_vals = gimp_run_procedure ("gimp_drawable_height",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    height = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return height;
}

void
gimp_drawable_offsets (gint32  drawable_ID,
		       gint   *offset_x,
		       gint   *offset_y)
{
  GParam *return_vals;
  gint nreturn_vals;

  return_vals = gimp_run_procedure ("gimp_drawable_offsets",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_END);

  *offset_x = *offset_y = 0;
  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    {
      *offset_x = return_vals[1].data.d_int32;
      *offset_y = return_vals[2].data.d_int32;
    }

  gimp_destroy_params (return_vals, nreturn_vals);
}

gboolean
gimp_drawable_is_layer (gint32 drawable_ID)
{
  GParam *return_vals;
  gint nreturn_vals;
  gboolean layer = FALSE;

  return_vals = gimp_run_procedure ("gimp_drawable_is_layer",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    layer = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return layer;
}

gboolean
gimp_drawable_is_layer_mask (gint32 drawable_ID)
{
  GParam *return_vals;
  gint nreturn_vals;
  gboolean layer_mask = FALSE;

  return_vals = gimp_run_procedure ("gimp_drawable_is_layer_mask",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    layer_mask = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return layer_mask;
}

gboolean
gimp_drawable_is_channel (gint32 drawable_ID)
{
  GParam *return_vals;
  gint nreturn_vals;
  gboolean channel = FALSE;

  return_vals = gimp_run_procedure ("gimp_drawable_is_channel",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    channel = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return channel;
}

void
_gimp_drawable_thumbnail (gint32   drawable_ID,
			  gint     width,
			  gint     height,
			  gint    *ret_width,
			  gint    *ret_height,
			  gint    *bpp,
			  gint    *thumbnail_data_count,
			  guint8 **thumbnail_data)
{
  GParam *return_vals;
  gint nreturn_vals;

  return_vals = gimp_run_procedure ("gimp_drawable_thumbnail",
				    &nreturn_vals,
				    PARAM_DRAWABLE, drawable_ID,
				    PARAM_INT32, width,
				    PARAM_INT32, height,
				    PARAM_END);

  *ret_width = 0;
  *ret_height = 0;
  *thumbnail_data_count = 0;
  *thumbnail_data = NULL;
  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    {
      *ret_width = return_vals[1].data.d_int32;
      *ret_height = return_vals[2].data.d_int32;
      *bpp = return_vals[3].data.d_int32;
      *thumbnail_data_count = return_vals[4].data.d_int32;
      *thumbnail_data = g_new (guint8, *thumbnail_data_count);
      memcpy (*thumbnail_data, return_vals[5].data.d_int8array,
	      *thumbnail_data_count * sizeof (guint8));
    }

  gimp_destroy_params (return_vals, nreturn_vals);
}
