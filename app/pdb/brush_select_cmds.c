/* The GIMP -- an image manipulation program
 * Copyright (C) 1995-2003 Spencer Kimball and Peter Mattis
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
#include "gui/gui-types.h"
#include "procedural_db.h"

#include "core/gimp.h"
#include "core/gimpcontainer.h"
#include "core/gimpcontext.h"
#include "core/gimpdatafactory.h"
#include "gui/brush-select.h"
#include "widgets/gimpbrushfactoryview.h"

static ProcRecord brushes_popup_proc;
static ProcRecord brushes_close_popup_proc;
static ProcRecord brushes_set_popup_proc;

void
register_brush_select_procs (Gimp *gimp)
{
  procedural_db_register (gimp, &brushes_popup_proc);
  procedural_db_register (gimp, &brushes_close_popup_proc);
  procedural_db_register (gimp, &brushes_set_popup_proc);
}

static Argument *
brushes_popup_invoker (Gimp     *gimp,
                       Argument *args)
{
  gboolean success = TRUE;
  gchar *brush_callback;
  gchar *popup_title;
  gchar *initial_brush;
  gdouble opacity;
  gint32 spacing;
  gint32 paint_mode;
  ProcRecord *proc;

  brush_callback = (gchar *) args[0].value.pdb_pointer;
  if (brush_callback == NULL || !g_utf8_validate (brush_callback, -1, NULL))
    success = FALSE;

  popup_title = (gchar *) args[1].value.pdb_pointer;
  if (popup_title == NULL || !g_utf8_validate (popup_title, -1, NULL))
    success = FALSE;

  initial_brush = (gchar *) args[2].value.pdb_pointer;
  if (initial_brush && !g_utf8_validate (initial_brush, -1, NULL))
    success = FALSE;

  opacity = args[3].value.pdb_float;
  if (opacity < 0.0 || opacity > 100.0)
    success = FALSE;

  spacing = args[4].value.pdb_int;
  if (spacing > 1000)
    success = FALSE;

  paint_mode = args[5].value.pdb_int;
  if (paint_mode < GIMP_NORMAL_MODE || paint_mode > GIMP_COLOR_ERASE_MODE)
    success = FALSE;

  if (success)
    {
      if (! gimp->no_interface &&
	  (proc = procedural_db_lookup (gimp, brush_callback)))
	{
	  brush_select_new (gimp, popup_title,
			    initial_brush,
			    opacity / 100.0,
			    paint_mode,
			    spacing,
			    brush_callback);
	}
      else
	success = FALSE;
    }

  return procedural_db_return_args (&brushes_popup_proc, success);
}

static ProcArg brushes_popup_inargs[] =
{
  {
    GIMP_PDB_STRING,
    "brush_callback",
    "The callback PDB proc to call when brush selection is made"
  },
  {
    GIMP_PDB_STRING,
    "popup_title",
    "Title to give the brush popup window"
  },
  {
    GIMP_PDB_STRING,
    "initial_brush",
    "The name of the brush to set as the first selected"
  },
  {
    GIMP_PDB_FLOAT,
    "opacity",
    "The initial opacity of the brush"
  },
  {
    GIMP_PDB_INT32,
    "spacing",
    "The initial spacing of the brush (if < 0 then use brush default spacing)"
  },
  {
    GIMP_PDB_INT32,
    "paint_mode",
    "The initial paint mode: { GIMP_NORMAL_MODE (0), GIMP_DISSOLVE_MODE (1), GIMP_BEHIND_MODE (2), GIMP_MULTIPLY_MODE (3), GIMP_SCREEN_MODE (4), GIMP_OVERLAY_MODE (5), GIMP_DIFFERENCE_MODE (6), GIMP_ADDITION_MODE (7), GIMP_SUBTRACT_MODE (8), GIMP_DARKEN_ONLY_MODE (9), GIMP_LIGHTEN_ONLY_MODE (10), GIMP_HUE_MODE (11), GIMP_SATURATION_MODE (12), GIMP_COLOR_MODE (13), GIMP_VALUE_MODE (14), GIMP_DIVIDE_MODE (15), GIMP_DODGE_MODE (16), GIMP_BURN_MODE (17), GIMP_HARDLIGHT_MODE (18), GIMP_SOFTLIGHT_MODE (19), GIMP_GRAIN_EXTRACT_MODE (20), GIMP_GRAIN_MERGE_MODE (21), GIMP_COLOR_ERASE_MODE (22) }"
  }
};

static ProcRecord brushes_popup_proc =
{
  "gimp_brushes_popup",
  "Invokes the Gimp brush selection.",
  "This procedure popups the brush selection dialog.",
  "Andy Thomas",
  "Andy Thomas",
  "1998",
  GIMP_INTERNAL,
  6,
  brushes_popup_inargs,
  0,
  NULL,
  { { brushes_popup_invoker } }
};

static Argument *
brushes_close_popup_invoker (Gimp     *gimp,
                             Argument *args)
{
  gboolean success = TRUE;
  gchar *brush_callback;
  ProcRecord *proc;
  BrushSelect *brush_select;

  brush_callback = (gchar *) args[0].value.pdb_pointer;
  if (brush_callback == NULL || !g_utf8_validate (brush_callback, -1, NULL))
    success = FALSE;

  if (success)
    {
      if (! gimp->no_interface &&
	  (proc = procedural_db_lookup (gimp, brush_callback)) &&
	  (brush_select = brush_select_get_by_callback (brush_callback)))
	{
	  brush_select_free (brush_select);
	}
      else
	success = FALSE;
    }

  return procedural_db_return_args (&brushes_close_popup_proc, success);
}

static ProcArg brushes_close_popup_inargs[] =
{
  {
    GIMP_PDB_STRING,
    "brush_callback",
    "The name of the callback registered for this popup"
  }
};

static ProcRecord brushes_close_popup_proc =
{
  "gimp_brushes_close_popup",
  "Popdown the Gimp brush selection.",
  "This procedure closes an opened brush selection dialog.",
  "Andy Thomas",
  "Andy Thomas",
  "1998",
  GIMP_INTERNAL,
  1,
  brushes_close_popup_inargs,
  0,
  NULL,
  { { brushes_close_popup_invoker } }
};

static Argument *
brushes_set_popup_invoker (Gimp     *gimp,
                           Argument *args)
{
  gboolean success = TRUE;
  gchar *brush_callback;
  gchar *brush_name;
  gdouble opacity;
  gint32 spacing;
  gint32 paint_mode;
  ProcRecord *proc;
  BrushSelect *brush_select;

  brush_callback = (gchar *) args[0].value.pdb_pointer;
  if (brush_callback == NULL || !g_utf8_validate (brush_callback, -1, NULL))
    success = FALSE;

  brush_name = (gchar *) args[1].value.pdb_pointer;
  if (brush_name == NULL || !g_utf8_validate (brush_name, -1, NULL))
    success = FALSE;

  opacity = args[2].value.pdb_float;
  if (opacity < 0.0 || opacity > 100.0)
    success = FALSE;

  spacing = args[3].value.pdb_int;
  if (spacing > 1000)
    success = FALSE;

  paint_mode = args[4].value.pdb_int;
  if (paint_mode < GIMP_NORMAL_MODE || paint_mode > GIMP_COLOR_ERASE_MODE)
    success = FALSE;

  if (success)
    {
      if (! gimp->no_interface &&
	  (proc = procedural_db_lookup (gimp, brush_callback)) &&
	  (brush_select = brush_select_get_by_callback (brush_callback)))
	{
	  GimpBrush *active = (GimpBrush *)
	    gimp_container_get_child_by_name (gimp->brush_factory->container,
					      brush_name);

	  if (active)
	    {
	      GtkAdjustment *spacing_adj;

	      spacing_adj = GIMP_BRUSH_FACTORY_VIEW (brush_select->view)->spacing_adjustment;

	      gimp_context_set_brush (brush_select->context, active);
	      gimp_context_set_opacity (brush_select->context, opacity / 100.0);
	      gimp_context_set_paint_mode (brush_select->context, paint_mode);

	      if (spacing >= 0)
		gtk_adjustment_set_value (spacing_adj, spacing);

	      gtk_window_present (GTK_WINDOW (brush_select->shell));
	    }
	  else
	    success = FALSE;
	}
      else
	success = FALSE;
    }

  return procedural_db_return_args (&brushes_set_popup_proc, success);
}

static ProcArg brushes_set_popup_inargs[] =
{
  {
    GIMP_PDB_STRING,
    "brush_callback",
    "The name of the callback registered for this popup"
  },
  {
    GIMP_PDB_STRING,
    "brush_name",
    "The name of the brush to set as selected"
  },
  {
    GIMP_PDB_FLOAT,
    "opacity",
    "The initial opacity of the brush"
  },
  {
    GIMP_PDB_INT32,
    "spacing",
    "The initial spacing of the brush (if < 0 then use brush default spacing)"
  },
  {
    GIMP_PDB_INT32,
    "paint_mode",
    "The initial paint mode: { GIMP_NORMAL_MODE (0), GIMP_DISSOLVE_MODE (1), GIMP_BEHIND_MODE (2), GIMP_MULTIPLY_MODE (3), GIMP_SCREEN_MODE (4), GIMP_OVERLAY_MODE (5), GIMP_DIFFERENCE_MODE (6), GIMP_ADDITION_MODE (7), GIMP_SUBTRACT_MODE (8), GIMP_DARKEN_ONLY_MODE (9), GIMP_LIGHTEN_ONLY_MODE (10), GIMP_HUE_MODE (11), GIMP_SATURATION_MODE (12), GIMP_COLOR_MODE (13), GIMP_VALUE_MODE (14), GIMP_DIVIDE_MODE (15), GIMP_DODGE_MODE (16), GIMP_BURN_MODE (17), GIMP_HARDLIGHT_MODE (18), GIMP_SOFTLIGHT_MODE (19), GIMP_GRAIN_EXTRACT_MODE (20), GIMP_GRAIN_MERGE_MODE (21), GIMP_COLOR_ERASE_MODE (22) }"
  }
};

static ProcRecord brushes_set_popup_proc =
{
  "gimp_brushes_set_popup",
  "Sets the current brush selection in a popup.",
  "Sets the current brush selection in a popup.",
  "Andy Thomas",
  "Andy Thomas",
  "1998",
  GIMP_INTERNAL,
  5,
  brushes_set_popup_inargs,
  0,
  NULL,
  { { brushes_set_popup_invoker } }
};
