/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * plug-in-file.c
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

#include "plug-in-types.h"

#include "core/gimp.h"
#include "core/gimpparamspecs.h"

#include "pdb/gimp-pdb.h"
#include "pdb/gimppluginprocedure.h"

#include "plug-in.h"
#include "plug-in-def.h"
#include "plug-in-file.h"


/*  public functions  */

gboolean
plug_in_file_register_load_handler (Gimp        *gimp,
                                    const gchar *name,
                                    const gchar *extensions,
                                    const gchar *prefixes,
                                    const gchar *magics)
{
  GimpPlugInProcedure *file_proc;
  GimpProcedure       *procedure;
  GSList              *list;

  g_return_val_if_fail (GIMP_IS_GIMP (gimp), FALSE);
  g_return_val_if_fail (name != NULL, FALSE);

  if (gimp->current_plug_in && gimp->current_plug_in->plug_in_def)
    list = gimp->current_plug_in->plug_in_def->procedures;
  else
    list = gimp->plug_in_procedures;

  file_proc = gimp_plug_in_procedure_find (list, name);

  if (! file_proc)
    {
      g_message ("attempt to register nonexistent load handler \"%s\"",
                 name);
      return FALSE;
    }

  procedure = GIMP_PROCEDURE (file_proc);

  if ((procedure->num_args   < 3)                        ||
      (procedure->num_values < 1)                        ||
      ! GIMP_IS_PARAM_SPEC_INT32    (procedure->args[0]) ||
      ! G_IS_PARAM_SPEC_STRING      (procedure->args[1]) ||
      ! G_IS_PARAM_SPEC_STRING      (procedure->args[2]) ||
      ! GIMP_IS_PARAM_SPEC_IMAGE_ID (procedure->values[0]))
    {
      g_message ("load handler \"%s\" does not take the standard "
                 "load handler args", name);
      return FALSE;
    }

  gimp_plug_in_procedure_set_file_proc (file_proc,
                                        extensions, prefixes, magics);

  if (! g_slist_find (gimp->load_procs, file_proc))
    gimp->load_procs = g_slist_prepend (gimp->load_procs, file_proc);

  return TRUE;
}

gboolean
plug_in_file_register_save_handler (Gimp        *gimp,
                                    const gchar *name,
                                    const gchar *extensions,
                                    const gchar *prefixes)
{
  GimpPlugInProcedure *file_proc;
  GimpProcedure       *procedure;
  GSList              *list;

  g_return_val_if_fail (GIMP_IS_GIMP (gimp), FALSE);
  g_return_val_if_fail (name != NULL, FALSE);

  if (gimp->current_plug_in && gimp->current_plug_in->plug_in_def)
    list = gimp->current_plug_in->plug_in_def->procedures;
  else
    list = gimp->plug_in_procedures;

  file_proc = gimp_plug_in_procedure_find (list, name);

  if (! file_proc)
    {
      g_message ("attempt to register nonexistent save handler \"%s\"",
                 name);
      return FALSE;
    }

  procedure = GIMP_PROCEDURE (file_proc);

  if ((procedure->num_args < 5)                             ||
      ! GIMP_IS_PARAM_SPEC_INT32       (procedure->args[0]) ||
      ! GIMP_IS_PARAM_SPEC_IMAGE_ID    (procedure->args[1]) ||
      ! GIMP_IS_PARAM_SPEC_DRAWABLE_ID (procedure->args[2]) ||
      ! G_IS_PARAM_SPEC_STRING         (procedure->args[3]) ||
      ! G_IS_PARAM_SPEC_STRING         (procedure->args[4]))
    {
      g_message ("save handler \"%s\" does not take the standard "
                 "save handler args", name);
      return FALSE;
    }

  gimp_plug_in_procedure_set_file_proc (file_proc,
                                        extensions, prefixes, NULL);

  if (! g_slist_find (gimp->save_procs, file_proc))
    gimp->save_procs = g_slist_prepend (gimp->save_procs, file_proc);

  return TRUE;
}

gboolean
plug_in_file_register_mime_type (Gimp        *gimp,
                                 const gchar *name,
                                 const gchar *mime_type)
{
  GimpPlugInProcedure *file_proc;
  GSList              *list;

  g_return_val_if_fail (GIMP_IS_GIMP (gimp), FALSE);
  g_return_val_if_fail (name != NULL, FALSE);
  g_return_val_if_fail (mime_type != NULL, FALSE);

  if (gimp->current_plug_in && gimp->current_plug_in->plug_in_def)
    list = gimp->current_plug_in->plug_in_def->procedures;
  else
    list = gimp->plug_in_procedures;

  file_proc = gimp_plug_in_procedure_find (list, name);

  if (! file_proc)
    return FALSE;

  gimp_plug_in_procedure_set_mime_type (file_proc, mime_type);

  return TRUE;
}

gboolean
plug_in_file_register_thumb_loader (Gimp        *gimp,
                                    const gchar *load_proc,
                                    const gchar *thumb_proc)
{
  GimpPlugInProcedure *file_proc;
  GSList              *list;

  g_return_val_if_fail (GIMP_IS_GIMP (gimp), FALSE);
  g_return_val_if_fail (load_proc, FALSE);
  g_return_val_if_fail (thumb_proc, FALSE);

  if (gimp->current_plug_in && gimp->current_plug_in->plug_in_def)
    list = gimp->current_plug_in->plug_in_def->procedures;
  else
    list = gimp->plug_in_procedures;

  file_proc = gimp_plug_in_procedure_find (list, load_proc);

  if (! file_proc)
    return FALSE;

  gimp_plug_in_procedure_set_thumb_loader (file_proc, thumb_proc);

  return TRUE;
}