/* The GIMP -- an image manipulation program
 * Copyright (C) 1995-2000 Spencer Kimball and Peter Mattis
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

/* NOTE: This file is autogenerated by pdbgen.pl. */

#include "config.h"


#include <gtk/gtk.h>

#include "libgimpbase/gimpbasetypes.h"

#include "pdb-types.h"
#include "procedural_db.h"

#include "core/core-enums.h"
#include "core/gimpdrawable.h"
#include "paint/gimpairbrush.h"
#include "paint/gimpclone.h"
#include "paint/gimpconvolve.h"
#include "paint/gimpdodgeburn.h"
#include "paint/gimperaser.h"
#include "paint/gimppaintbrush.h"
#include "paint/gimppaintcore-stroke.h"
#include "paint/gimppencil.h"
#include "paint/gimpsmudge.h"
#include "paint/paint-enums.h"

#include "libgimpmath/gimpmath.h"

static ProcRecord airbrush_proc;
static ProcRecord airbrush_default_proc;
static ProcRecord clone_proc;
static ProcRecord clone_default_proc;
static ProcRecord convolve_proc;
static ProcRecord convolve_default_proc;
static ProcRecord dodgeburn_proc;
static ProcRecord dodgeburn_default_proc;
static ProcRecord eraser_proc;
static ProcRecord eraser_default_proc;
static ProcRecord paintbrush_proc;
static ProcRecord paintbrush_default_proc;
static ProcRecord pencil_proc;
static ProcRecord smudge_proc;
static ProcRecord smudge_default_proc;

void
register_paint_tools_procs (Gimp *gimp)
{
  procedural_db_register (gimp, &airbrush_proc);
  procedural_db_register (gimp, &airbrush_default_proc);
  procedural_db_register (gimp, &clone_proc);
  procedural_db_register (gimp, &clone_default_proc);
  procedural_db_register (gimp, &convolve_proc);
  procedural_db_register (gimp, &convolve_default_proc);
  procedural_db_register (gimp, &dodgeburn_proc);
  procedural_db_register (gimp, &dodgeburn_default_proc);
  procedural_db_register (gimp, &eraser_proc);
  procedural_db_register (gimp, &eraser_default_proc);
  procedural_db_register (gimp, &paintbrush_proc);
  procedural_db_register (gimp, &paintbrush_default_proc);
  procedural_db_register (gimp, &pencil_proc);
  procedural_db_register (gimp, &smudge_proc);
  procedural_db_register (gimp, &smudge_default_proc);
}

static gboolean
paint_tools_stroke (Gimp             *gimp,
                    GType             core_type,
                    GimpPaintOptions *paint_options,
                    GimpDrawable     *drawable,
                    gint              n_strokes,
                    gdouble          *strokes)
{
  GimpPaintCore *core;
  GimpCoords    *coords;
  gboolean       retval;
  gint           i;

  core = g_object_new (core_type, NULL);

  coords = g_new (GimpCoords, n_strokes);

  for (i = 0; i < n_strokes; i++)
    {
      coords[i].x        = strokes[2 * i];
      coords[i].y        = strokes[2 * i + 1];
      coords[i].pressure = 1.0;
      coords[i].xtilt    = 0.5;
      coords[i].ytilt    = 0.5;
      coords[i].wheel    = 0.5;
    }

  retval = gimp_paint_core_stroke (core, drawable, paint_options,
                                   coords, n_strokes);

  g_free (coords);

  g_object_unref (G_OBJECT (core));

  return retval;
}

static Argument *
airbrush_invoker (Gimp     *gimp,
                  Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gdouble pressure;
  gint32 num_strokes;
  gdouble *strokes;
  GimpAirbrushOptions *options;

  drawable = (GimpDrawable *) gimp_item_get_by_ID (gimp, args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  pressure = args[1].value.pdb_float;
  if (pressure < 0.0 || pressure > 100.0)
    success = FALSE;

  num_strokes = args[2].value.pdb_int;
  if (!(num_strokes < 2))
    num_strokes /= 2;
  else
    success = FALSE;

  strokes = (gdouble *) args[3].value.pdb_pointer;

  if (success)
    {
      options = gimp_airbrush_options_new ();
    
      options->pressure = pressure;
    
      success = paint_tools_stroke (gimp,
				    GIMP_TYPE_AIRBRUSH,
				    (GimpPaintOptions *) options,
				    drawable,
				    num_strokes, strokes);
    }

  return procedural_db_return_args (&airbrush_proc, success);
}

static ProcArg airbrush_inargs[] =
{
  {
    GIMP_PDB_DRAWABLE,
    "drawable",
    "The affected drawable"
  },
  {
    GIMP_PDB_FLOAT,
    "pressure",
    "The pressure of the airbrush strokes (0 <= pressure <= 100)"
  },
  {
    GIMP_PDB_INT32,
    "num_strokes",
    "Number of stroke control points (count each coordinate as 2 points)"
  },
  {
    GIMP_PDB_FLOATARRAY,
    "strokes",
    "Array of stroke coordinates: { s1.x, s1.y, s2.x, s2.y, ..., sn.x, sn.y }"
  }
};

static ProcRecord airbrush_proc =
{
  "gimp_airbrush",
  "Paint in the current brush with varying pressure. Paint application is time-dependent.",
  "This tool simulates the use of an airbrush. Paint pressure represents the relative intensity of the paint application. High pressure results in a thicker layer of paint while low pressure results in a thinner layer.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  4,
  airbrush_inargs,
  0,
  NULL,
  { { airbrush_invoker } }
};

static Argument *
airbrush_default_invoker (Gimp     *gimp,
                          Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gint32 num_strokes;
  gdouble *strokes;
  GimpAirbrushOptions *options;

  drawable = (GimpDrawable *) gimp_item_get_by_ID (gimp, args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  num_strokes = args[1].value.pdb_int;
  if (!(num_strokes < 2))
    num_strokes /= 2;
  else
    success = FALSE;

  strokes = (gdouble *) args[2].value.pdb_pointer;

  if (success)
    {
      options = gimp_airbrush_options_new ();
    
      success = paint_tools_stroke (gimp,
				    GIMP_TYPE_AIRBRUSH,
				    (GimpPaintOptions *) options,
				    drawable,
				    num_strokes, strokes);
    }

  return procedural_db_return_args (&airbrush_default_proc, success);
}

static ProcArg airbrush_default_inargs[] =
{
  {
    GIMP_PDB_DRAWABLE,
    "drawable",
    "The affected drawable"
  },
  {
    GIMP_PDB_INT32,
    "num_strokes",
    "Number of stroke control points (count each coordinate as 2 points)"
  },
  {
    GIMP_PDB_FLOATARRAY,
    "strokes",
    "Array of stroke coordinates: { s1.x, s1.y, s2.x, s2.y, ..., sn.x, sn.y }"
  }
};

static ProcRecord airbrush_default_proc =
{
  "gimp_airbrush_default",
  "Paint in the current brush with varying pressure. Paint application is time-dependent.",
  "This tool simulates the use of an airbrush. It is similar to gimp_airbrush except that the pressure is derived from the airbrush tools options box. It the option has not been set the default for the option will be used.",
  "Andy Thomas",
  "Andy Thomas",
  "1999",
  GIMP_INTERNAL,
  3,
  airbrush_default_inargs,
  0,
  NULL,
  { { airbrush_default_invoker } }
};

static Argument *
clone_invoker (Gimp     *gimp,
               Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  GimpDrawable *src_drawable;
  gint32 clone_type;
  gdouble src_x;
  gdouble src_y;
  gint32 num_strokes;
  gdouble *strokes;
  GimpCloneOptions *options;

  drawable = (GimpDrawable *) gimp_item_get_by_ID (gimp, args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  src_drawable = (GimpDrawable *) gimp_item_get_by_ID (gimp, args[1].value.pdb_int);
  if (src_drawable == NULL)
    success = FALSE;

  clone_type = args[2].value.pdb_int;
  if (clone_type < GIMP_IMAGE_CLONE || clone_type > GIMP_PATTERN_CLONE)
    success = FALSE;

  src_x = args[3].value.pdb_float;

  src_y = args[4].value.pdb_float;

  num_strokes = args[5].value.pdb_int;
  if (!(num_strokes < 2))
    num_strokes /= 2;
  else
    success = FALSE;

  strokes = (gdouble *) args[6].value.pdb_pointer;

  if (success)
    {
      options = gimp_clone_options_new ();
    
      options->type = clone_type;
    
    #if 0
      FIXME
    
      core->src_drawable = src_drawable;
      core->src_x        = srx_x;
      core->src_y        = src_y;
    #endif
    
      success = paint_tools_stroke (gimp,
				    GIMP_TYPE_CLONE,
				    (GimpPaintOptions *) options,
				    drawable,
				    num_strokes, strokes);
    }

  return procedural_db_return_args (&clone_proc, success);
}

static ProcArg clone_inargs[] =
{
  {
    GIMP_PDB_DRAWABLE,
    "drawable",
    "The affected drawable"
  },
  {
    GIMP_PDB_DRAWABLE,
    "src_drawable",
    "The source drawable"
  },
  {
    GIMP_PDB_INT32,
    "clone_type",
    "The type of clone: { GIMP_IMAGE_CLONE (0), GIMP_PATTERN_CLONE (1) }"
  },
  {
    GIMP_PDB_FLOAT,
    "src_x",
    "The x coordinate in the source image"
  },
  {
    GIMP_PDB_FLOAT,
    "src_y",
    "The y coordinate in the source image"
  },
  {
    GIMP_PDB_INT32,
    "num_strokes",
    "Number of stroke control points (count each coordinate as 2 points)"
  },
  {
    GIMP_PDB_FLOATARRAY,
    "strokes",
    "Array of stroke coordinates: { s1.x, s1.y, s2.x, s2.y, ..., sn.x, sn.y }"
  }
};

static ProcRecord clone_proc =
{
  "gimp_clone",
  "Clone from the source to the dest drawable using the current brush",
  "This tool clones (copies) from the source drawable starting at the specified source coordinates to the dest drawable. If the \"clone_type\" argument is set to PATTERN-CLONE, then the current pattern is used as the source and the \"src_drawable\" argument is ignored. Pattern cloning assumes a tileable pattern and mods the sum of the src coordinates and subsequent stroke offsets with the width and height of the pattern. For image cloning, if the sum of the src coordinates and subsequent stroke offsets exceeds the extents of the src drawable, then no paint is transferred. The clone tool is capable of transforming between any image types including RGB->Indexed--although converting from any type to indexed is significantly slower.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  7,
  clone_inargs,
  0,
  NULL,
  { { clone_invoker } }
};

static Argument *
clone_default_invoker (Gimp     *gimp,
                       Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gint32 num_strokes;
  gdouble *strokes;
  GimpCloneOptions *options;

  drawable = (GimpDrawable *) gimp_item_get_by_ID (gimp, args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  num_strokes = args[1].value.pdb_int;
  if (!(num_strokes < 2))
    num_strokes /= 2;
  else
    success = FALSE;

  strokes = (gdouble *) args[2].value.pdb_pointer;

  if (success)
    {
      options = gimp_clone_options_new ();
    
      success = paint_tools_stroke (gimp,
				    GIMP_TYPE_CLONE,
				    (GimpPaintOptions *) options,
				    drawable,
				    num_strokes, strokes);
    }

  return procedural_db_return_args (&clone_default_proc, success);
}

static ProcArg clone_default_inargs[] =
{
  {
    GIMP_PDB_DRAWABLE,
    "drawable",
    "The affected drawable"
  },
  {
    GIMP_PDB_INT32,
    "num_strokes",
    "Number of stroke control points (count each coordinate as 2 points)"
  },
  {
    GIMP_PDB_FLOATARRAY,
    "strokes",
    "Array of stroke coordinates: { s1.x, s1.y, s2.x, s2.y, ..., sn.x, sn.y }"
  }
};

static ProcRecord clone_default_proc =
{
  "gimp_clone_default",
  "Clone from the source to the dest drawable using the current brush",
  "This tool clones (copies) from the source drawable starting at the specified source coordinates to the dest drawable. This function performs exactly the same as the gimp_clone function except that the tools arguments are obtained from the clones option dialog. It this dialog has not been activated then the dialogs default values will be used.",
  "Andy Thomas",
  "Andy Thomas",
  "1999",
  GIMP_INTERNAL,
  3,
  clone_default_inargs,
  0,
  NULL,
  { { clone_default_invoker } }
};

static Argument *
convolve_invoker (Gimp     *gimp,
                  Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gdouble pressure;
  gint32 convolve_type;
  gint32 num_strokes;
  gdouble *strokes;
  GimpConvolveOptions *options;

  drawable = (GimpDrawable *) gimp_item_get_by_ID (gimp, args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  pressure = args[1].value.pdb_float;
  if (pressure < 0.0 || pressure > 100.0)
    success = FALSE;

  convolve_type = args[2].value.pdb_int;
  if (convolve_type < GIMP_BLUR_CONVOLVE || convolve_type > GIMP_SHARPEN_CONVOLVE)
    success = FALSE;

  num_strokes = args[3].value.pdb_int;
  if (!(num_strokes < 2))
    num_strokes /= 2;
  else
    success = FALSE;

  strokes = (gdouble *) args[4].value.pdb_pointer;

  if (success)
    {
      options = gimp_convolve_options_new ();
    
      options->type = convolve_type;
      options->rate = pressure;
    
      success = paint_tools_stroke (gimp,
				    GIMP_TYPE_CONVOLVE,
				    (GimpPaintOptions *) options,
				    drawable,
				    num_strokes, strokes);
    }

  return procedural_db_return_args (&convolve_proc, success);
}

static ProcArg convolve_inargs[] =
{
  {
    GIMP_PDB_DRAWABLE,
    "drawable",
    "The affected drawable"
  },
  {
    GIMP_PDB_FLOAT,
    "pressure",
    "The pressure: 0 <= pressure <= 100"
  },
  {
    GIMP_PDB_INT32,
    "convolve_type",
    "Convolve type: { GIMP_BLUR_CONVOLVE (0), GIMP_SHARPEN_CONVOLVE (1) }"
  },
  {
    GIMP_PDB_INT32,
    "num_strokes",
    "Number of stroke control points (count each coordinate as 2 points)"
  },
  {
    GIMP_PDB_FLOATARRAY,
    "strokes",
    "Array of stroke coordinates: { s1.x, s1.y, s2.x, s2.y, ..., sn.x, sn.y }"
  }
};

static ProcRecord convolve_proc =
{
  "gimp_convolve",
  "Convolve (Blur, Sharpen) using the current brush.",
  "This tool convolves the specified drawable with either a sharpening or blurring kernel. The pressure parameter controls the magnitude of the operation. Like the paintbrush, this tool linearly interpolates between the specified stroke coordinates.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  5,
  convolve_inargs,
  0,
  NULL,
  { { convolve_invoker } }
};

static Argument *
convolve_default_invoker (Gimp     *gimp,
                          Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gint32 num_strokes;
  gdouble *strokes;
  GimpConvolveOptions *options;

  drawable = (GimpDrawable *) gimp_item_get_by_ID (gimp, args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  num_strokes = args[1].value.pdb_int;
  if (!(num_strokes < 2))
    num_strokes /= 2;
  else
    success = FALSE;

  strokes = (gdouble *) args[2].value.pdb_pointer;

  if (success)
    {
      options = gimp_convolve_options_new ();
    
      success = paint_tools_stroke (gimp,
				    GIMP_TYPE_CONVOLVE,
				    (GimpPaintOptions *) options,
				    drawable,
				    num_strokes, strokes);
    }

  return procedural_db_return_args (&convolve_default_proc, success);
}

static ProcArg convolve_default_inargs[] =
{
  {
    GIMP_PDB_DRAWABLE,
    "drawable",
    "The affected drawable"
  },
  {
    GIMP_PDB_INT32,
    "num_strokes",
    "Number of stroke control points (count each coordinate as 2 points)"
  },
  {
    GIMP_PDB_FLOATARRAY,
    "strokes",
    "Array of stroke coordinates: { s1.x, s1.y, s2.x, s2.y, ..., sn.x, sn.y }"
  }
};

static ProcRecord convolve_default_proc =
{
  "gimp_convolve_default",
  "Convolve (Blur, Sharpen) using the current brush.",
  "This tool convolves the specified drawable with either a sharpening or blurring kernel. This function performs exactly the same as the gimp_convolve function except that the tools arguments are obtained from the convolve option dialog. It this dialog has not been activated then the dialogs default values will be used.",
  "Andy Thomas",
  "Andy Thomas",
  "1999",
  GIMP_INTERNAL,
  3,
  convolve_default_inargs,
  0,
  NULL,
  { { convolve_default_invoker } }
};

static Argument *
dodgeburn_invoker (Gimp     *gimp,
                   Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gdouble exposure;
  gint32 dodgeburn_type;
  gint32 dodgeburn_mode;
  gint32 num_strokes;
  gdouble *strokes;
  GimpDodgeBurnOptions *options;

  drawable = (GimpDrawable *) gimp_item_get_by_ID (gimp, args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  exposure = args[1].value.pdb_float;
  if (exposure < 0.0 || exposure > 100.0)
    success = FALSE;

  dodgeburn_type = args[2].value.pdb_int;
  if (dodgeburn_type < GIMP_DODGE || dodgeburn_type > GIMP_BURN)
    success = FALSE;

  dodgeburn_mode = args[3].value.pdb_int;
  if (dodgeburn_mode < GIMP_SHADOWS || dodgeburn_mode > GIMP_HIGHLIGHTS)
    success = FALSE;

  num_strokes = args[4].value.pdb_int;
  if (!(num_strokes < 2))
    num_strokes /= 2;
  else
    success = FALSE;

  strokes = (gdouble *) args[5].value.pdb_pointer;

  if (success)
    {
      options = gimp_dodgeburn_options_new ();
    
      options->exposure = exposure;
      options->type     = dodgeburn_type;
      options->mode     = dodgeburn_mode;
    
      success = paint_tools_stroke (gimp,
				    GIMP_TYPE_DODGEBURN,
				    (GimpPaintOptions *) options,
				    drawable,
				    num_strokes, strokes);
    }

  return procedural_db_return_args (&dodgeburn_proc, success);
}

static ProcArg dodgeburn_inargs[] =
{
  {
    GIMP_PDB_DRAWABLE,
    "drawable",
    "The affected drawable"
  },
  {
    GIMP_PDB_FLOAT,
    "exposure",
    "The exposer of the strokes (0 <= exposure <= 100)"
  },
  {
    GIMP_PDB_INT32,
    "dodgeburn_type",
    "The type either dodge or burn: { GIMP_DODGE (0), GIMP_BURN (1) }"
  },
  {
    GIMP_PDB_INT32,
    "dodgeburn_mode",
    "The mode: { GIMP_SHADOWS (0), GIMP_MIDTONES (1), GIMP_HIGHLIGHTS (2) }"
  },
  {
    GIMP_PDB_INT32,
    "num_strokes",
    "Number of stroke control points (count each coordinate as 2 points)"
  },
  {
    GIMP_PDB_FLOATARRAY,
    "strokes",
    "Array of stroke coordinates: { s1.x, s1.y, s2.x, s2.y, ..., sn.x, sn.y }"
  }
};

static ProcRecord dodgeburn_proc =
{
  "gimp_dodgeburn",
  "Dodgeburn image with varying exposure.",
  "Dodgebure. More details here later.",
  "Andy Thomas",
  "Andy Thomas",
  "1999",
  GIMP_INTERNAL,
  6,
  dodgeburn_inargs,
  0,
  NULL,
  { { dodgeburn_invoker } }
};

static Argument *
dodgeburn_default_invoker (Gimp     *gimp,
                           Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gint32 num_strokes;
  gdouble *strokes;
  GimpDodgeBurnOptions *options;

  drawable = (GimpDrawable *) gimp_item_get_by_ID (gimp, args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  num_strokes = args[1].value.pdb_int;
  if (!(num_strokes < 2))
    num_strokes /= 2;
  else
    success = FALSE;

  strokes = (gdouble *) args[2].value.pdb_pointer;

  if (success)
    {
      options = gimp_dodgeburn_options_new ();
    
      success = paint_tools_stroke (gimp,
				    GIMP_TYPE_DODGEBURN,
				    (GimpPaintOptions *) options,
				    drawable,
				    num_strokes, strokes);
    }

  return procedural_db_return_args (&dodgeburn_default_proc, success);
}

static ProcArg dodgeburn_default_inargs[] =
{
  {
    GIMP_PDB_DRAWABLE,
    "drawable",
    "The affected drawable"
  },
  {
    GIMP_PDB_INT32,
    "num_strokes",
    "Number of stroke control points (count each coordinate as 2 points)"
  },
  {
    GIMP_PDB_FLOATARRAY,
    "strokes",
    "Array of stroke coordinates: { s1.x, s1.y, s2.x, s2.y, ..., sn.x, sn.y }"
  }
};

static ProcRecord dodgeburn_default_proc =
{
  "gimp_dodgeburn_default",
  "Dodgeburn image with varying exposure. This is the same as the gimp_dodgeburn function except that the exposure, type and mode are taken from the tools option dialog. If the dialog has not been activated then the defaults as used by the dialog will be used.",
  "Dodgeburn. More details here later.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  3,
  dodgeburn_default_inargs,
  0,
  NULL,
  { { dodgeburn_default_invoker } }
};

static Argument *
eraser_invoker (Gimp     *gimp,
                Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gint32 num_strokes;
  gdouble *strokes;
  gint32 hardness;
  gint32 method;
  GimpEraserOptions *options;

  drawable = (GimpDrawable *) gimp_item_get_by_ID (gimp, args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  num_strokes = args[1].value.pdb_int;
  if (!(num_strokes < 2))
    num_strokes /= 2;
  else
    success = FALSE;

  strokes = (gdouble *) args[2].value.pdb_pointer;

  hardness = args[3].value.pdb_int;
  if (hardness < GIMP_BRUSH_HARD || hardness > GIMP_BRUSH_SOFT)
    success = FALSE;

  method = args[4].value.pdb_int;
  if (method < GIMP_PAINT_CONSTANT || method > GIMP_PAINT_INCREMENTAL)
    success = FALSE;

  if (success)
    {
      options = gimp_eraser_options_new ();
    
      options->paint_options.incremental = method;
    
      options->hard = hardness;
    
      success = paint_tools_stroke (gimp,
				    GIMP_TYPE_ERASER,
				    (GimpPaintOptions *) options,
				    drawable,
				    num_strokes, strokes);
    }

  return procedural_db_return_args (&eraser_proc, success);
}

static ProcArg eraser_inargs[] =
{
  {
    GIMP_PDB_DRAWABLE,
    "drawable",
    "The affected drawable"
  },
  {
    GIMP_PDB_INT32,
    "num_strokes",
    "Number of stroke control points (count each coordinate as 2 points)"
  },
  {
    GIMP_PDB_FLOATARRAY,
    "strokes",
    "Array of stroke coordinates: { s1.x, s1.y, s2.x, s2.y, ..., sn.x, sn.y }"
  },
  {
    GIMP_PDB_INT32,
    "hardness",
    "GIMP_BRUSH_HARD (0) or GIMP_BRUSH_SOFT (1)"
  },
  {
    GIMP_PDB_INT32,
    "method",
    "GIMP_PAINT_CONSTANT (0) or GIMP_PAINT_INCREMENTAL (1)"
  }
};

static ProcRecord eraser_proc =
{
  "gimp_eraser",
  "Erase using the current brush.",
  "This tool erases using the current brush mask. If the specified drawable contains an alpha channel, then the erased pixels will become transparent. Otherwise, the eraser tool replaces the contents of the drawable with the background color. Like paintbrush, this tool linearly interpolates between the specified stroke coordinates.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  5,
  eraser_inargs,
  0,
  NULL,
  { { eraser_invoker } }
};

static Argument *
eraser_default_invoker (Gimp     *gimp,
                        Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gint32 num_strokes;
  gdouble *strokes;
  GimpEraserOptions *options;

  drawable = (GimpDrawable *) gimp_item_get_by_ID (gimp, args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  num_strokes = args[1].value.pdb_int;
  if (!(num_strokes < 2))
    num_strokes /= 2;
  else
    success = FALSE;

  strokes = (gdouble *) args[2].value.pdb_pointer;

  if (success)
    {
      options = gimp_eraser_options_new ();
    
      success = paint_tools_stroke (gimp,
				    GIMP_TYPE_ERASER,
				    (GimpPaintOptions *) options,
				    drawable,
				    num_strokes, strokes);
    }

  return procedural_db_return_args (&eraser_default_proc, success);
}

static ProcArg eraser_default_inargs[] =
{
  {
    GIMP_PDB_DRAWABLE,
    "drawable",
    "The affected drawable"
  },
  {
    GIMP_PDB_INT32,
    "num_strokes",
    "Number of stroke control points (count each coordinate as 2 points)"
  },
  {
    GIMP_PDB_FLOATARRAY,
    "strokes",
    "Array of stroke coordinates: { s1.x, s1.y, s2.x, s2.y, ..., sn.x, sn.y }"
  }
};

static ProcRecord eraser_default_proc =
{
  "gimp_eraser_default",
  "Erase using the current brush.",
  "This tool erases using the current brush mask. This function performs exactly the same as the gimp_eraser function except that the tools arguments are obtained from the eraser option dialog. It this dialog has not been activated then the dialogs default values will be used.",
  "Andy Thomas",
  "Andy Thomas",
  "1999",
  GIMP_INTERNAL,
  3,
  eraser_default_inargs,
  0,
  NULL,
  { { eraser_default_invoker } }
};

static Argument *
paintbrush_invoker (Gimp     *gimp,
                    Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gdouble fade_out;
  gint32 num_strokes;
  gdouble *strokes;
  gint32 method;
  gdouble gradient_length;
  GimpPaintOptions *options;

  drawable = (GimpDrawable *) gimp_item_get_by_ID (gimp, args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  fade_out = args[1].value.pdb_float;
  if (fade_out < 0.0)
    success = FALSE;

  num_strokes = args[2].value.pdb_int;
  if (!(num_strokes < 2))
    num_strokes /= 2;
  else
    success = FALSE;

  strokes = (gdouble *) args[3].value.pdb_pointer;

  method = args[4].value.pdb_int;
  if (method < GIMP_PAINT_CONSTANT || method > GIMP_PAINT_INCREMENTAL)
    success = FALSE;

  gradient_length = args[5].value.pdb_float;
  if (gradient_length < 0.0)
    success = FALSE;

  if (success)
    {
      options = gimp_paint_options_new ();
    
      options->incremental = method;
    
      options->gradient_options->fade_out        = fade_out;
      options->gradient_options->gradient_length = gradient_length;
    
      success = paint_tools_stroke (gimp,
				    GIMP_TYPE_PAINTBRUSH,
				    (GimpPaintOptions *) options,
				    drawable,
				    num_strokes, strokes);
    }

  return procedural_db_return_args (&paintbrush_proc, success);
}

static ProcArg paintbrush_inargs[] =
{
  {
    GIMP_PDB_DRAWABLE,
    "drawable",
    "The affected drawable"
  },
  {
    GIMP_PDB_FLOAT,
    "fade_out",
    "Fade out parameter: 0 <= fade_out"
  },
  {
    GIMP_PDB_INT32,
    "num_strokes",
    "Number of stroke control points (count each coordinate as 2 points)"
  },
  {
    GIMP_PDB_FLOATARRAY,
    "strokes",
    "Array of stroke coordinates: { s1.x, s1.y, s2.x, s2.y, ..., sn.x, sn.y }"
  },
  {
    GIMP_PDB_INT32,
    "method",
    "GIMP_PAINT_CONSTANT (0) or GIMP_PAINT_INCREMENTAL (1)"
  },
  {
    GIMP_PDB_FLOAT,
    "gradient_length",
    "Length of gradient to draw: 0 <= gradient_length"
  }
};

static ProcRecord paintbrush_proc =
{
  "gimp_paintbrush",
  "Paint in the current brush with optional fade out parameter and pull colors from a gradient.",
  "This tool is the standard paintbrush. It draws linearly interpolated lines through the specified stroke coordinates. It operates on the specified drawable in the foreground color with the active brush. The \"fade_out\" parameter is measured in pixels and allows the brush stroke to linearly fall off. The pressure is set to the maximum at the beginning of the stroke. As the distance of the stroke nears the fade_out value, the pressure will approach zero. The gradient_length is the distance to spread the gradient over. It is measured in pixels. If the gradient_length is 0, no gradient is used.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  6,
  paintbrush_inargs,
  0,
  NULL,
  { { paintbrush_invoker } }
};

static Argument *
paintbrush_default_invoker (Gimp     *gimp,
                            Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gint32 num_strokes;
  gdouble *strokes;
  GimpPaintOptions *options;

  drawable = (GimpDrawable *) gimp_item_get_by_ID (gimp, args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  num_strokes = args[1].value.pdb_int;
  if (!(num_strokes < 2))
    num_strokes /= 2;
  else
    success = FALSE;

  strokes = (gdouble *) args[2].value.pdb_pointer;

  if (success)
    {
      options = gimp_paint_options_new ();
    
      success = paint_tools_stroke (gimp,
				    GIMP_TYPE_PAINTBRUSH,
				    (GimpPaintOptions *) options,
				    drawable,
				    num_strokes, strokes);
    }

  return procedural_db_return_args (&paintbrush_default_proc, success);
}

static ProcArg paintbrush_default_inargs[] =
{
  {
    GIMP_PDB_DRAWABLE,
    "drawable",
    "The affected drawable"
  },
  {
    GIMP_PDB_INT32,
    "num_strokes",
    "Number of stroke control points (count each coordinate as 2 points)"
  },
  {
    GIMP_PDB_FLOATARRAY,
    "strokes",
    "Array of stroke coordinates: { s1.x, s1.y, s2.x, s2.y, ..., sn.x, sn.y }"
  }
};

static ProcRecord paintbrush_default_proc =
{
  "gimp_paintbrush_default",
  "Paint in the current brush. The fade out parameter and pull colors from a gradient parameter are set from the paintbrush options dialog. If this dialog has not been activated then the dialog defaults will be used.",
  "This tool is similar to the standard paintbrush. It draws linearly interpolated lines through the specified stroke coordinates. It operates on the specified drawable in the foreground color with the active brush. The \"fade_out\" parameter is measured in pixels and allows the brush stroke to linearly fall off (value obtained from the option dialog). The pressure is set to the maximum at the beginning of the stroke. As the distance of the stroke nears the fade_out value, the pressure will approach zero. The gradient_length (value obtained from the option dialog) is the distance to spread the gradient over. It is measured in pixels. If the gradient_length is 0, no gradient is used.",
  "Andy Thomas",
  "Andy Thomas",
  "1999",
  GIMP_INTERNAL,
  3,
  paintbrush_default_inargs,
  0,
  NULL,
  { { paintbrush_default_invoker } }
};

static Argument *
pencil_invoker (Gimp     *gimp,
                Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gint32 num_strokes;
  gdouble *strokes;
  GimpPaintOptions *options;

  drawable = (GimpDrawable *) gimp_item_get_by_ID (gimp, args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  num_strokes = args[1].value.pdb_int;
  if (!(num_strokes < 2))
    num_strokes /= 2;
  else
    success = FALSE;

  strokes = (gdouble *) args[2].value.pdb_pointer;

  if (success)
    {
      options = gimp_paint_options_new ();
    
      success = paint_tools_stroke (gimp,
				    GIMP_TYPE_PENCIL,
				    (GimpPaintOptions *) options,
				    drawable,
				    num_strokes, strokes);
    }

  return procedural_db_return_args (&pencil_proc, success);
}

static ProcArg pencil_inargs[] =
{
  {
    GIMP_PDB_DRAWABLE,
    "drawable",
    "The affected drawable"
  },
  {
    GIMP_PDB_INT32,
    "num_strokes",
    "Number of stroke control points (count each coordinate as 2 points)"
  },
  {
    GIMP_PDB_FLOATARRAY,
    "strokes",
    "Array of stroke coordinates: { s1.x, s1.y, s2.x, s2.y, ..., sn.x, sn.y }"
  }
};

static ProcRecord pencil_proc =
{
  "gimp_pencil",
  "Paint in the current brush without sub-pixel sampling.",
  "This tool is the standard pencil. It draws linearly interpolated lines through the specified stroke coordinates. It operates on the specified drawable in the foreground color with the active brush. The brush mask is treated as though it contains only black and white values. Any value below half is treated as black; any above half, as white.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  3,
  pencil_inargs,
  0,
  NULL,
  { { pencil_invoker } }
};

static Argument *
smudge_invoker (Gimp     *gimp,
                Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gdouble pressure;
  gint32 num_strokes;
  gdouble *strokes;
  GimpSmudgeOptions *options;

  drawable = (GimpDrawable *) gimp_item_get_by_ID (gimp, args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  pressure = args[1].value.pdb_float;
  if (pressure < 0.0 || pressure > 100.0)
    success = FALSE;

  num_strokes = args[2].value.pdb_int;
  if (!(num_strokes < 2))
    num_strokes /= 2;
  else
    success = FALSE;

  strokes = (gdouble *) args[3].value.pdb_pointer;

  if (success)
    {
      options = gimp_smudge_options_new ();
    
      options->rate = pressure;
    
      success = paint_tools_stroke (gimp,
				    GIMP_TYPE_SMUDGE,
				    (GimpPaintOptions *) options,
				    drawable,
				    num_strokes, strokes);
    }

  return procedural_db_return_args (&smudge_proc, success);
}

static ProcArg smudge_inargs[] =
{
  {
    GIMP_PDB_DRAWABLE,
    "drawable",
    "The affected drawable"
  },
  {
    GIMP_PDB_FLOAT,
    "pressure",
    "The pressure of the smudge strokes (0 <= pressure <= 100)"
  },
  {
    GIMP_PDB_INT32,
    "num_strokes",
    "Number of stroke control points (count each coordinate as 2 points)"
  },
  {
    GIMP_PDB_FLOATARRAY,
    "strokes",
    "Array of stroke coordinates: { s1.x, s1.y, s2.x, s2.y, ..., sn.x, sn.y }"
  }
};

static ProcRecord smudge_proc =
{
  "gimp_smudge",
  "Smudge image with varying pressure.",
  "This tool simulates a smudge using the current brush. High pressure results in a greater smudge of paint while low pressure results in a lesser smudge.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  4,
  smudge_inargs,
  0,
  NULL,
  { { smudge_invoker } }
};

static Argument *
smudge_default_invoker (Gimp     *gimp,
                        Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gint32 num_strokes;
  gdouble *strokes;
  GimpSmudgeOptions *options;

  drawable = (GimpDrawable *) gimp_item_get_by_ID (gimp, args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  num_strokes = args[1].value.pdb_int;
  if (!(num_strokes < 2))
    num_strokes /= 2;
  else
    success = FALSE;

  strokes = (gdouble *) args[2].value.pdb_pointer;

  if (success)
    {
      options = gimp_smudge_options_new ();
    
      success = paint_tools_stroke (gimp,
				    GIMP_TYPE_SMUDGE,
				    (GimpPaintOptions *) options,
				    drawable,
				    num_strokes, strokes);
    }

  return procedural_db_return_args (&smudge_default_proc, success);
}

static ProcArg smudge_default_inargs[] =
{
  {
    GIMP_PDB_DRAWABLE,
    "drawable",
    "The affected drawable"
  },
  {
    GIMP_PDB_INT32,
    "num_strokes",
    "Number of stroke control points (count each coordinate as 2 points)"
  },
  {
    GIMP_PDB_FLOATARRAY,
    "strokes",
    "Array of stroke coordinates: { s1.x, s1.y, s2.x, s2.y, ..., sn.x, sn.y }"
  }
};

static ProcRecord smudge_default_proc =
{
  "gimp_smudge_default",
  "Smudge image with varying pressure.",
  "This tool simulates a smudge using the current brush. It behaves exactly the same as gimp_smudge except that the pressure value is taken from the smudge tool options or the options default if the tools option dialog has not been activated.",
  "Andy Thomas",
  "Andy Thomas",
  "1999",
  GIMP_INTERNAL,
  3,
  smudge_default_inargs,
  0,
  NULL,
  { { smudge_default_invoker } }
};
