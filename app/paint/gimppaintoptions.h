/* The GIMP -- an image manipulation program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#ifndef __GIMP_PAINT_OPTIONS_H__
#define __GIMP_PAINT_OPTIONS_H__


#include "core/gimptooloptions.h"


#define GIMP_PAINT_OPTIONS_CONTEXT_MASK GIMP_CONTEXT_FOREGROUND_MASK | \
                                        GIMP_CONTEXT_BACKGROUND_MASK | \
                                        GIMP_CONTEXT_OPACITY_MASK    | \
                                        GIMP_CONTEXT_PAINT_MODE_MASK | \
                                        GIMP_CONTEXT_BRUSH_MASK


typedef struct _GimpPressureOptions GimpPressureOptions;
typedef struct _GimpFadeOptions     GimpFadeOptions;
typedef struct _GimpGradientOptions GimpGradientOptions;

struct _GimpPressureOptions
{
  gboolean  opacity;
  gboolean  hardness;
  gboolean  rate;
  gboolean  size;
  gboolean  color;
};

struct _GimpFadeOptions
{
  gboolean  use_fade;
  gdouble   fade_length;
  GimpUnit  fade_unit;
};

struct _GimpGradientOptions
{
  gboolean        use_gradient;
  gboolean        gradient_reverse;
  gdouble         gradient_length;
  GimpUnit        gradient_unit;
  GimpRepeatMode  gradient_repeat;
};


#define GIMP_TYPE_PAINT_OPTIONS            (gimp_paint_options_get_type ())
#define GIMP_PAINT_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_PAINT_OPTIONS, GimpPaintOptions))
#define GIMP_PAINT_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_PAINT_OPTIONS, GimpPaintOptionsClass))
#define GIMP_IS_PAINT_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIMP_TYPE_PAINT_OPTIONS))
#define GIMP_IS_PAINT_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_PAINT_OPTIONS))
#define GIMP_PAINT_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GIMP_TYPE_PAINT_OPTIONS, GimpPaintOptionsClass))


typedef struct _GimpPaintOptionsClass GimpPaintOptionsClass;

struct _GimpPaintOptions
{
  GimpToolOptions           parent_instance;

  GimpPaintInfo            *paint_info;

  GimpPaintApplicationMode  application_mode;
  GimpPaintApplicationMode  application_mode_save;

  gboolean                  hard;

  GimpPressureOptions      *pressure_options;
  GimpFadeOptions          *fade_options;
  GimpGradientOptions      *gradient_options;
};

struct _GimpPaintOptionsClass
{
  GimpToolOptionsClass  parent_instance;
};


GType              gimp_paint_options_get_type (void) G_GNUC_CONST;

GimpPaintOptions * gimp_paint_options_new      (GimpPaintInfo    *paint_info);

gdouble            gimp_paint_options_get_fade (GimpPaintOptions *paint_options,
                                                GimpImage        *gimage,
                                                gdouble           pixel_dist);

gboolean gimp_paint_options_get_gradient_color (GimpPaintOptions *paint_options,
                                                GimpImage        *gimage,
                                                gdouble           pressure,
                                                gdouble           pixel_dist,
                                                GimpRGB          *color);

GimpBrushApplicationMode
             gimp_paint_options_get_brush_mode (GimpPaintOptions *paint_options);


#endif  /*  __GIMP_PAINT_OPTIONS_H__  */
