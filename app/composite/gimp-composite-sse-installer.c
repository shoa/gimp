/* THIS FILE IS AUTOMATICALLY GENERATED.  DO NOT EDIT */
/* REGENERATE BY USING make-installer.py */
#include "config.h"
#include <glib-object.h>
#include <stdlib.h>
#include <stdio.h>
#include "base/base-types.h"
#include "gimp-composite.h"

#include "gimp-composite-sse.h"

static struct install_table {
  GimpCompositeOperation mode;
  GimpPixelFormat A;
  GimpPixelFormat B;
  GimpPixelFormat D;
  void (*function)(GimpCompositeContext *);
} _gimp_composite_sse[] = {
#if defined(COMPILE_SSE_IS_OKAY)
 { GIMP_COMPOSITE_MULTIPLY, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, gimp_composite_multiply_rgba8_rgba8_rgba8_sse }, 
 { GIMP_COMPOSITE_SCREEN, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, gimp_composite_screen_rgba8_rgba8_rgba8_sse }, 
 { GIMP_COMPOSITE_DIFFERENCE, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, gimp_composite_difference_rgba8_rgba8_rgba8_sse }, 
 { GIMP_COMPOSITE_ADDITION, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, gimp_composite_addition_rgba8_rgba8_rgba8_sse }, 
 { GIMP_COMPOSITE_SUBTRACT, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, gimp_composite_subtract_rgba8_rgba8_rgba8_sse }, 
 { GIMP_COMPOSITE_DARKEN, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, gimp_composite_darken_rgba8_rgba8_rgba8_sse }, 
 { GIMP_COMPOSITE_LIGHTEN, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, gimp_composite_lighten_rgba8_rgba8_rgba8_sse }, 
 { GIMP_COMPOSITE_BURN, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, gimp_composite_burn_rgba8_rgba8_rgba8_sse }, 
 { GIMP_COMPOSITE_GRAIN_EXTRACT, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, gimp_composite_grain_extract_rgba8_rgba8_rgba8_sse }, 
 { GIMP_COMPOSITE_GRAIN_MERGE, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, gimp_composite_grain_merge_rgba8_rgba8_rgba8_sse }, 
 { GIMP_COMPOSITE_SWAP, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, gimp_composite_swap_rgba8_rgba8_rgba8_sse }, 
 { GIMP_COMPOSITE_SCALE, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, GIMP_PIXELFORMAT_RGBA8, gimp_composite_scale_rgba8_rgba8_rgba8_sse }, 
#endif
 { 0, 0, 0, 0, NULL }
};

gboolean
gimp_composite_sse_install (void)
{
  static struct install_table *t = _gimp_composite_sse;

  if (gimp_composite_sse_init ())
    {
      for (t = &_gimp_composite_sse[0]; t->function != NULL; t++)
        {
          gimp_composite_function[t->mode][t->A][t->B][t->D] = t->function;
        }
      return (TRUE);
    }

  return (FALSE);
}
