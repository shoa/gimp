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


#include <glib-object.h>

#include "pdb-types.h"
#include "gimpprocedure.h"
#include "procedural_db.h"
#include "core/gimpparamspecs.h"

#include "core/gimpdrawable.h"
#include "core/gimplayer-floating-sel.h"
#include "core/gimplayer.h"

static GimpProcedure floating_sel_remove_proc;
static GimpProcedure floating_sel_anchor_proc;
static GimpProcedure floating_sel_to_layer_proc;
static GimpProcedure floating_sel_attach_proc;
static GimpProcedure floating_sel_rigor_proc;
static GimpProcedure floating_sel_relax_proc;

void
register_floating_sel_procs (Gimp *gimp)
{
  GimpProcedure *procedure;

  /*
   * floating_sel_remove
   */
  procedure = gimp_procedure_init (&floating_sel_remove_proc, 1, 0);
  gimp_procedure_add_argument (procedure,
                               GIMP_PDB_LAYER,
                               gimp_param_spec_item_id ("floating-sel",
                                                        "floating sel",
                                                        "The floating selection",
                                                        gimp,
                                                        GIMP_TYPE_LAYER,
                                                        GIMP_PARAM_READWRITE));
  procedural_db_register (gimp, procedure);

  /*
   * floating_sel_anchor
   */
  procedure = gimp_procedure_init (&floating_sel_anchor_proc, 1, 0);
  gimp_procedure_add_argument (procedure,
                               GIMP_PDB_LAYER,
                               gimp_param_spec_item_id ("floating-sel",
                                                        "floating sel",
                                                        "The floating selection",
                                                        gimp,
                                                        GIMP_TYPE_LAYER,
                                                        GIMP_PARAM_READWRITE));
  procedural_db_register (gimp, procedure);

  /*
   * floating_sel_to_layer
   */
  procedure = gimp_procedure_init (&floating_sel_to_layer_proc, 1, 0);
  gimp_procedure_add_argument (procedure,
                               GIMP_PDB_LAYER,
                               gimp_param_spec_item_id ("floating-sel",
                                                        "floating sel",
                                                        "The floating selection",
                                                        gimp,
                                                        GIMP_TYPE_LAYER,
                                                        GIMP_PARAM_READWRITE));
  procedural_db_register (gimp, procedure);

  /*
   * floating_sel_attach
   */
  procedure = gimp_procedure_init (&floating_sel_attach_proc, 2, 0);
  gimp_procedure_add_argument (procedure,
                               GIMP_PDB_LAYER,
                               gimp_param_spec_item_id ("layer",
                                                        "layer",
                                                        "The layer (is attached as floating selection)",
                                                        gimp,
                                                        GIMP_TYPE_LAYER,
                                                        GIMP_PARAM_READWRITE));
  gimp_procedure_add_argument (procedure,
                               GIMP_PDB_DRAWABLE,
                               gimp_param_spec_item_id ("drawable",
                                                        "drawable",
                                                        "The drawable (where to attach the floating selection)",
                                                        gimp,
                                                        GIMP_TYPE_DRAWABLE,
                                                        GIMP_PARAM_READWRITE));
  procedural_db_register (gimp, procedure);

  /*
   * floating_sel_rigor
   */
  procedure = gimp_procedure_init (&floating_sel_rigor_proc, 2, 0);
  gimp_procedure_add_argument (procedure,
                               GIMP_PDB_LAYER,
                               gimp_param_spec_item_id ("floating-sel",
                                                        "floating sel",
                                                        "The floating selection",
                                                        gimp,
                                                        GIMP_TYPE_LAYER,
                                                        GIMP_PARAM_READWRITE));
  gimp_procedure_add_argument (procedure,
                               GIMP_PDB_INT32,
                               g_param_spec_boolean ("undo",
                                                     "undo",
                                                     "(TRUE or FALSE)",
                                                     FALSE,
                                                     GIMP_PARAM_READWRITE));
  procedural_db_register (gimp, procedure);

  /*
   * floating_sel_relax
   */
  procedure = gimp_procedure_init (&floating_sel_relax_proc, 2, 0);
  gimp_procedure_add_argument (procedure,
                               GIMP_PDB_LAYER,
                               gimp_param_spec_item_id ("floating-sel",
                                                        "floating sel",
                                                        "The floating selection",
                                                        gimp,
                                                        GIMP_TYPE_LAYER,
                                                        GIMP_PARAM_READWRITE));
  gimp_procedure_add_argument (procedure,
                               GIMP_PDB_INT32,
                               g_param_spec_boolean ("undo",
                                                     "undo",
                                                     "(TRUE or FALSE)",
                                                     FALSE,
                                                     GIMP_PARAM_READWRITE));
  procedural_db_register (gimp, procedure);

}

static Argument *
floating_sel_remove_invoker (GimpProcedure *procedure,
                             Gimp          *gimp,
                             GimpContext   *context,
                             GimpProgress  *progress,
                             Argument      *args)
{
  gboolean success = TRUE;
  GimpLayer *floating_sel;

  floating_sel = (GimpLayer *) gimp_value_get_item (&args[0].value, gimp, GIMP_TYPE_LAYER);

  if (success)
    {
      if (gimp_layer_is_floating_sel (floating_sel))
        floating_sel_remove (floating_sel);
      else
        success = FALSE;
    }

  return gimp_procedure_get_return_values (procedure, success);
}

static GimpProcedure floating_sel_remove_proc =
{
  TRUE, TRUE,
  "gimp-floating-sel-remove",
  "gimp-floating-sel-remove",
  "Remove the specified floating selection from its associated drawable.",
  "This procedure removes the floating selection completely, without any side effects. The associated drawable is then set to active.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  NULL,
  GIMP_INTERNAL,
  0, NULL, 0, NULL,
  { { floating_sel_remove_invoker } }
};

static Argument *
floating_sel_anchor_invoker (GimpProcedure *procedure,
                             Gimp          *gimp,
                             GimpContext   *context,
                             GimpProgress  *progress,
                             Argument      *args)
{
  gboolean success = TRUE;
  GimpLayer *floating_sel;

  floating_sel = (GimpLayer *) gimp_value_get_item (&args[0].value, gimp, GIMP_TYPE_LAYER);

  if (success)
    {
      if (gimp_layer_is_floating_sel (floating_sel))
        floating_sel_anchor (floating_sel);
      else
        success = FALSE;
    }

  return gimp_procedure_get_return_values (procedure, success);
}

static GimpProcedure floating_sel_anchor_proc =
{
  TRUE, TRUE,
  "gimp-floating-sel-anchor",
  "gimp-floating-sel-anchor",
  "Anchor the specified floating selection to its associated drawable.",
  "This procedure anchors the floating selection to its associated drawable. This is similar to merging with a merge type of ClipToBottomLayer. The floating selection layer is no longer valid after this operation.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  NULL,
  GIMP_INTERNAL,
  0, NULL, 0, NULL,
  { { floating_sel_anchor_invoker } }
};

static Argument *
floating_sel_to_layer_invoker (GimpProcedure *procedure,
                               Gimp          *gimp,
                               GimpContext   *context,
                               GimpProgress  *progress,
                               Argument      *args)
{
  gboolean success = TRUE;
  GimpLayer *floating_sel;

  floating_sel = (GimpLayer *) gimp_value_get_item (&args[0].value, gimp, GIMP_TYPE_LAYER);

  if (success)
    {
      if (gimp_layer_is_floating_sel (floating_sel))
        floating_sel_to_layer (floating_sel);
      else
        success = FALSE;
    }

  return gimp_procedure_get_return_values (procedure, success);
}

static GimpProcedure floating_sel_to_layer_proc =
{
  TRUE, TRUE,
  "gimp-floating-sel-to-layer",
  "gimp-floating-sel-to-layer",
  "Transforms the specified floating selection into a layer.",
  "This procedure transforms the specified floating selection into a layer with the same offsets and extents. The composited image will look precisely the same, but the floating selection layer will no longer be clipped to the extents of the drawable it was attached to. The floating selection will become the active layer. This procedure will not work if the floating selection has a different base type from the underlying image. This might be the case if the floating selection is above an auxillary channel or a layer mask.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  NULL,
  GIMP_INTERNAL,
  0, NULL, 0, NULL,
  { { floating_sel_to_layer_invoker } }
};

static Argument *
floating_sel_attach_invoker (GimpProcedure *procedure,
                             Gimp          *gimp,
                             GimpContext   *context,
                             GimpProgress  *progress,
                             Argument      *args)
{
  gboolean success = TRUE;
  GimpLayer *layer;
  GimpDrawable *drawable;

  layer = (GimpLayer *) gimp_value_get_item (&args[0].value, gimp, GIMP_TYPE_LAYER);
  drawable = (GimpDrawable *) gimp_value_get_item (&args[1].value, gimp, GIMP_TYPE_DRAWABLE);

  if (success)
    {
      if (gimp_item_is_attached (GIMP_ITEM (drawable)))
        floating_sel_attach (layer, drawable);
      else
        success = FALSE;
    }

  return gimp_procedure_get_return_values (procedure, success);
}

static GimpProcedure floating_sel_attach_proc =
{
  TRUE, TRUE,
  "gimp-floating-sel-attach",
  "gimp-floating-sel-attach",
  "Attach the specified layer as floating to the specified drawable.",
  "This procedure attaches the layer as floating selection to the drawable.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  NULL,
  GIMP_INTERNAL,
  0, NULL, 0, NULL,
  { { floating_sel_attach_invoker } }
};

static Argument *
floating_sel_rigor_invoker (GimpProcedure *procedure,
                            Gimp          *gimp,
                            GimpContext   *context,
                            GimpProgress  *progress,
                            Argument      *args)
{
  gboolean success = TRUE;
  GimpLayer *floating_sel;
  gboolean undo;

  floating_sel = (GimpLayer *) gimp_value_get_item (&args[0].value, gimp, GIMP_TYPE_LAYER);
  undo = g_value_get_boolean (&args[1].value);

  if (success)
    {
      if (gimp_layer_is_floating_sel (floating_sel))
        floating_sel_rigor (floating_sel, undo);
      else
        success = FALSE;
    }

  return gimp_procedure_get_return_values (procedure, success);
}

static GimpProcedure floating_sel_rigor_proc =
{
  TRUE, TRUE,
  "gimp-floating-sel-rigor",
  "gimp-floating-sel-rigor",
  "Rigor the floating selection.",
  "This procedure rigors the floating selection.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  NULL,
  GIMP_INTERNAL,
  0, NULL, 0, NULL,
  { { floating_sel_rigor_invoker } }
};

static Argument *
floating_sel_relax_invoker (GimpProcedure *procedure,
                            Gimp          *gimp,
                            GimpContext   *context,
                            GimpProgress  *progress,
                            Argument      *args)
{
  gboolean success = TRUE;
  GimpLayer *floating_sel;
  gboolean undo;

  floating_sel = (GimpLayer *) gimp_value_get_item (&args[0].value, gimp, GIMP_TYPE_LAYER);
  undo = g_value_get_boolean (&args[1].value);

  if (success)
    {
      if (gimp_layer_is_floating_sel (floating_sel))
        floating_sel_relax (floating_sel, undo);
      else
        success = FALSE;
    }

  return gimp_procedure_get_return_values (procedure, success);
}

static GimpProcedure floating_sel_relax_proc =
{
  TRUE, TRUE,
  "gimp-floating-sel-relax",
  "gimp-floating-sel-relax",
  "Relax the floating selection.",
  "This procedure relaxes the floating selection.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  NULL,
  GIMP_INTERNAL,
  0, NULL, 0, NULL,
  { { floating_sel_relax_invoker } }
};
