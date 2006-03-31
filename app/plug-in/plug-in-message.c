/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * plug-in-message.c
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

#include "libgimpbase/gimpbase.h"
#include "libgimpbase/gimpprotocol.h"
#include "libgimpbase/gimpwire.h"

#include "plug-in-types.h"

#include "base/tile.h"
#include "base/tile-manager.h"

#include "core/gimp.h"
#include "core/gimpdrawable.h"

#include "pdb/gimpprocedure.h"
#include "pdb/procedural_db.h"

#include "plug-in.h"
#include "plug-ins.h"
#include "plug-in-def.h"
#include "plug-in-params.h"
#include "plug-in-proc-def.h"
#include "plug-in-shm.h"


typedef struct _PlugInBlocked PlugInBlocked;

struct _PlugInBlocked
{
  PlugIn *plug_in;
  gchar  *proc_name;
};


/*  local function prototypes  */

static void plug_in_handle_quit             (PlugIn          *plug_in);
static void plug_in_handle_tile_req         (PlugIn          *plug_in,
                                             GPTileReq       *tile_req);
static void plug_in_handle_proc_run         (PlugIn          *plug_in,
                                             GPProcRun       *proc_run);
static void plug_in_handle_proc_return_priv (PlugIn          *plug_in,
                                             GPProcReturn    *proc_return,
                                             gboolean         temp_proc);
static void plug_in_handle_proc_return      (PlugIn          *plug_in,
                                             GPProcReturn    *proc_return);
static void plug_in_handle_temp_proc_return (PlugIn          *plug_in,
                                             GPProcReturn    *proc_return);
static void plug_in_handle_proc_install     (PlugIn          *plug_in,
                                             GPProcInstall   *proc_install);
static void plug_in_handle_proc_uninstall   (PlugIn          *plug_in,
                                             GPProcUninstall *proc_uninstall);
static void plug_in_handle_extension_ack    (PlugIn          *plug_in);
static void plug_in_handle_has_init         (PlugIn          *plug_in);


/*  private variables  */

static GSList *blocked_plug_ins = NULL;


/*  public functions  */

void
plug_in_handle_message (PlugIn          *plug_in,
                        GimpWireMessage *msg)
{
  switch (msg->type)
    {
    case GP_QUIT:
      plug_in_handle_quit (plug_in);
      break;

    case GP_CONFIG:
      g_message ("Plug-In \"%s\"\n(%s)\n\n"
		 "sent a CONFIG message.  This should not happen.",
                 gimp_filename_to_utf8 (plug_in->name),
		 gimp_filename_to_utf8 (plug_in->prog));
      plug_in_close (plug_in, TRUE);
      break;

    case GP_TILE_REQ:
      plug_in_handle_tile_req (plug_in, msg->data);
      break;

    case GP_TILE_ACK:
      g_message ("Plug-In \"%s\"\n(%s)\n\n"
		 "sent a TILE_ACK message.  This should not happen.",
                 gimp_filename_to_utf8 (plug_in->name),
		 gimp_filename_to_utf8 (plug_in->prog));
      plug_in_close (plug_in, TRUE);
      break;

    case GP_TILE_DATA:
      g_message ("Plug-In \"%s\"\n(%s)\n\n"
		 "sent a TILE_DATA message.  This should not happen.",
                 gimp_filename_to_utf8 (plug_in->name),
		 gimp_filename_to_utf8 (plug_in->prog));
      plug_in_close (plug_in, TRUE);
      break;

    case GP_PROC_RUN:
      plug_in_handle_proc_run (plug_in, msg->data);
      break;

    case GP_PROC_RETURN:
      plug_in_handle_proc_return (plug_in, msg->data);
      break;

    case GP_TEMP_PROC_RUN:
      g_message ("Plug-In \"%s\"\n(%s)\n\n"
		 "sent a TEMP_PROC_RUN message.  This should not happen.",
                 gimp_filename_to_utf8 (plug_in->name),
		 gimp_filename_to_utf8 (plug_in->prog));
      plug_in_close (plug_in, TRUE);
      break;

    case GP_TEMP_PROC_RETURN:
      plug_in_handle_temp_proc_return (plug_in, msg->data);
      break;

    case GP_PROC_INSTALL:
      plug_in_handle_proc_install (plug_in, msg->data);
      break;

    case GP_PROC_UNINSTALL:
      plug_in_handle_proc_uninstall (plug_in, msg->data);
      break;

    case GP_EXTENSION_ACK:
      plug_in_handle_extension_ack (plug_in);
      break;

    case GP_HAS_INIT:
      plug_in_handle_has_init (plug_in);
      break;
    }
}


/*  private functions  */

static void
plug_in_handle_quit (PlugIn *plug_in)
{
  plug_in_close (plug_in, FALSE);
}

static void
plug_in_handle_tile_req (PlugIn    *plug_in,
                         GPTileReq *tile_req)
{
  GPTileData       tile_data;
  GPTileData      *tile_info;
  GimpWireMessage  msg;
  GimpDrawable    *drawable;
  TileManager     *tm;
  Tile            *tile;
  gint             shm_ID;

  shm_ID = plug_in_shm_get_ID (plug_in->gimp);

  if (tile_req->drawable_ID == -1)
    {
      /*  this branch communicates with libgimp/gimptile.c:gimp_tile_put()  */

      tile_data.drawable_ID = -1;
      tile_data.tile_num    = 0;
      tile_data.shadow      = 0;
      tile_data.bpp         = 0;
      tile_data.width       = 0;
      tile_data.height      = 0;
      tile_data.use_shm     = (shm_ID == -1) ? FALSE : TRUE;
      tile_data.data        = NULL;

      if (! gp_tile_data_write (plug_in->my_write, &tile_data, plug_in))
	{
	  g_warning ("plug_in_handle_tile_req: ERROR");
	  plug_in_close (plug_in, TRUE);
	  return;
	}

      if (! gimp_wire_read_msg (plug_in->my_read, &msg, plug_in))
	{
	  g_warning ("plug_in_handle_tile_req: ERROR");
	  plug_in_close (plug_in, TRUE);
	  return;
	}

      if (msg.type != GP_TILE_DATA)
	{
	  g_warning ("expected tile data and received: %d", msg.type);
	  plug_in_close (plug_in, TRUE);
	  return;
	}

      tile_info = msg.data;

      drawable = (GimpDrawable *) gimp_item_get_by_ID (plug_in->gimp,
                                                       tile_info->drawable_ID);

      if (! drawable)
	{
          g_message ("Plug-In \"%s\"\n(%s)\n\n"
                     "requested invalid drawable (killing)",
                     gimp_filename_to_utf8 (plug_in->name),
		     gimp_filename_to_utf8 (plug_in->prog));
	  plug_in_close (plug_in, TRUE);
	  return;
	}

      if (tile_info->shadow)
	tm = gimp_drawable_shadow (drawable);
      else
	tm = gimp_drawable_data (drawable);

      tile = tile_manager_get (tm, tile_info->tile_num, TRUE, TRUE);

      if (! tile)
	{
          g_message ("Plug-In \"%s\"\n(%s)\n\n"
                     "requested invalid tile (killing)",
                     gimp_filename_to_utf8 (plug_in->name),
		     gimp_filename_to_utf8 (plug_in->prog));
	  plug_in_close (plug_in, TRUE);
	  return;
	}

      if (tile_data.use_shm)
	memcpy (tile_data_pointer (tile, 0, 0),
                plug_in_shm_get_addr (plug_in->gimp),
                tile_size (tile));
      else
	memcpy (tile_data_pointer (tile, 0, 0),
                tile_info->data,
                tile_size (tile));

      tile_release (tile, TRUE);
      gimp_wire_destroy (&msg);

      if (! gp_tile_ack_write (plug_in->my_write, plug_in))
	{
	  g_warning ("plug_in_handle_tile_req: ERROR");
	  plug_in_close (plug_in, TRUE);
	  return;
	}
    }
  else
    {
      /*  this branch communicates with libgimp/gimptile.c:gimp_tile_get()  */

      drawable = (GimpDrawable *) gimp_item_get_by_ID (plug_in->gimp,
                                                       tile_req->drawable_ID);

      if (! drawable)
	{
          g_message ("Plug-In \"%s\"\n(%s)\n\n"
                     "requested invalid drawable (killing)",
                     gimp_filename_to_utf8 (plug_in->name),
		     gimp_filename_to_utf8 (plug_in->prog));
	  plug_in_close (plug_in, TRUE);
	  return;
	}

      if (tile_req->shadow)
	tm = gimp_drawable_shadow (drawable);
      else
	tm = gimp_drawable_data (drawable);

      tile = tile_manager_get (tm, tile_req->tile_num, TRUE, FALSE);

      if (! tile)
	{
          g_message ("Plug-In \"%s\"\n(%s)\n\n"
                     "requested invalid tile (killing)",
                     gimp_filename_to_utf8 (plug_in->name),
		     gimp_filename_to_utf8 (plug_in->prog));
	  plug_in_close (plug_in, TRUE);
	  return;
	}

      tile_data.drawable_ID = tile_req->drawable_ID;
      tile_data.tile_num    = tile_req->tile_num;
      tile_data.shadow      = tile_req->shadow;
      tile_data.bpp         = tile_bpp (tile);
      tile_data.width       = tile_ewidth (tile);
      tile_data.height      = tile_eheight (tile);
      tile_data.use_shm     = (shm_ID == -1) ? FALSE : TRUE;

      if (tile_data.use_shm)
	memcpy (plug_in_shm_get_addr (plug_in->gimp),
                tile_data_pointer (tile, 0, 0),
                tile_size (tile));
      else
	tile_data.data = tile_data_pointer (tile, 0, 0);

      if (! gp_tile_data_write (plug_in->my_write, &tile_data, plug_in))
	{
	  g_message ("plug_in_handle_tile_req: ERROR");
	  plug_in_close (plug_in, TRUE);
	  return;
	}

      tile_release (tile, FALSE);

      if (! gimp_wire_read_msg (plug_in->my_read, &msg, plug_in))
	{
	  g_message ("plug_in_handle_tile_req: ERROR");
	  plug_in_close (plug_in, TRUE);
	  return;
	}

      if (msg.type != GP_TILE_ACK)
	{
	  g_warning ("expected tile ack and received: %d", msg.type);
	  plug_in_close (plug_in, TRUE);
	  return;
	}

      gimp_wire_destroy (&msg);
    }
}

static void
plug_in_handle_proc_run (PlugIn    *plug_in,
                         GPProcRun *proc_run)
{
  PlugInProcFrame *proc_frame;
  gchar           *canonical;
  const gchar     *proc_name     = NULL;
  GimpProcedure   *procedure;
  Argument        *args          = NULL;
  gint             n_args        = 0;
  Argument        *return_vals   = NULL;
  gint             n_return_vals = 0;

  canonical = gimp_canonicalize_identifier (proc_run->name);

  proc_frame = plug_in_get_proc_frame (plug_in);

  procedure = procedural_db_lookup (plug_in->gimp, canonical);

  if (! procedure)
    {
      proc_name = g_hash_table_lookup (plug_in->gimp->procedural_compat_ht,
                                       canonical);

      if (proc_name)
        {
          procedure = procedural_db_lookup (plug_in->gimp, proc_name);

          if (plug_in->gimp->pdb_compat_mode == GIMP_PDB_COMPAT_WARN)
            {
              g_message ("WARNING: Plug-In \"%s\"\n(%s)\n"
                         "called deprecated procedure '%s'.\n"
                         "It should call '%s' instead!",
                         gimp_filename_to_utf8 (plug_in->name),
			 gimp_filename_to_utf8 (plug_in->prog),
                         canonical, proc_name);
            }
        }
    }
  else if (procedure->deprecated)
    {
      if (plug_in->gimp->pdb_compat_mode == GIMP_PDB_COMPAT_WARN)
        {
          if (! strcmp (procedure->deprecated, "NONE"))
            {
              g_message ("WARNING: Plug-In \"%s\"\n(%s)\n"
                         "called deprecated procedure '%s'.",
                         gimp_filename_to_utf8 (plug_in->name),
                         gimp_filename_to_utf8 (plug_in->prog),
                         canonical);
            }
          else
            {
              g_message ("WARNING: Plug-In \"%s\"\n(%s)\n"
                         "called deprecated procedure '%s'.\n"
                         "It should call '%s' instead!",
                         gimp_filename_to_utf8 (plug_in->name),
                         gimp_filename_to_utf8 (plug_in->prog),
                         canonical, procedure->deprecated);
            }
        }
      else if (plug_in->gimp->pdb_compat_mode == GIMP_PDB_COMPAT_OFF)
        {
          procedure = NULL;
        }
    }

  if (! proc_name)
    proc_name = canonical;

  if (procedure)
    {
      n_args = proc_run->nparams;
      args   = plug_in_params_to_args (procedure->args, procedure->num_args,
                                       proc_run->params, n_args,
                                       FALSE);
    }

  plug_in_push (plug_in->gimp, plug_in);

  /*  Execute the procedure even if procedural_db_lookup() returned NULL,
   *  procedural_db_execute() will return appropriate error return_vals.
   */
  return_vals = procedural_db_execute (plug_in->gimp,
                                       proc_frame->context_stack ?
                                       proc_frame->context_stack->data :
                                       proc_frame->main_context,
                                       proc_frame->progress,
                                       proc_name,
                                       args, n_args,
                                       &n_return_vals);

  plug_in_pop (plug_in->gimp);

  g_free (canonical);

  if (return_vals)
    {
      GPProcReturn proc_return;

      /*  Return the name we got called with, *not* proc_name or canonical,
       *  since proc_name may have been remapped by gimp->procedural_compat_ht
       *  and canonical may be different too.
       */
      proc_return.name    = proc_run->name;
      proc_return.nparams = n_return_vals;
      proc_return.params  = plug_in_args_to_params (return_vals, n_return_vals,
                                                    FALSE);

      if (! gp_proc_return_write (plug_in->my_write, &proc_return, plug_in))
	{
	  g_warning ("plug_in_handle_proc_run: ERROR");
	  plug_in_close (plug_in, TRUE);
	  return;
	}

      procedural_db_destroy_args (args, n_args, FALSE);
      procedural_db_destroy_args (return_vals, n_return_vals, TRUE);
      plug_in_params_destroy (proc_return.params, proc_return.nparams, FALSE);
    }
  else
    {
      PlugInBlocked *blocked;

      g_warning ("%s: EEEEEEEEEK! \n"
                 "You managed to trigger a code path that \n"
                 "should be dead. Please report this to bugs.gimp.org.",
                 G_STRFUNC);

      blocked = g_new0 (PlugInBlocked, 1);

      blocked->plug_in   = plug_in;
      blocked->proc_name = g_strdup (proc_run->name);

      blocked_plug_ins = g_slist_prepend (blocked_plug_ins, blocked);
    }
}

static void
plug_in_handle_proc_return_priv (PlugIn       *plug_in,
                                 GPProcReturn *proc_return,
                                 gboolean      temp_proc)
{
  PlugInProcFrame *proc_frame;

  if (temp_proc)
    proc_frame = plug_in->temp_proc_frames->data;
  else
    proc_frame = &plug_in->main_proc_frame;

  if (proc_frame->main_loop)
    {
      proc_frame->return_vals = plug_in_params_to_args (proc_frame->procedure->values,
                                                        proc_frame->procedure->num_values,
                                                        proc_return->params,
                                                        proc_return->nparams,
                                                        TRUE);
      proc_frame->n_return_vals = proc_return->nparams;
    }
  else
    {
      GSList *list;

      for (list = blocked_plug_ins; list; list = g_slist_next (list))
	{
          PlugInBlocked *blocked;

	  blocked = (PlugInBlocked *) list->data;

	  if (blocked->proc_name && proc_return->name &&
	      strcmp (blocked->proc_name, proc_return->name) == 0)
	    {
	      if (! gp_proc_return_write (blocked->plug_in->my_write,
                                          proc_return,
                                          blocked->plug_in))
		{
		  g_message ("plug_in_handle_proc_run: ERROR");
		  plug_in_close (blocked->plug_in, TRUE);
		  return;
		}

	      blocked_plug_ins = g_slist_remove (blocked_plug_ins, blocked);
	      g_free (blocked->proc_name);
	      g_free (blocked);
	      break;
	    }
	}
    }
}

static void
plug_in_handle_proc_return (PlugIn       *plug_in,
                            GPProcReturn *proc_return)
{
  plug_in_handle_proc_return_priv (plug_in, proc_return, FALSE);

  if (plug_in->main_proc_frame.main_loop)
    g_main_loop_quit (plug_in->main_proc_frame.main_loop);

  plug_in_close (plug_in, FALSE);
}

static void
plug_in_handle_temp_proc_return (PlugIn       *plug_in,
                                 GPProcReturn *proc_return)
{
  if (plug_in->temp_proc_frames)
    {
      plug_in_handle_proc_return_priv (plug_in, proc_return, TRUE);

      plug_in_main_loop_quit (plug_in);
      plug_in_proc_frame_pop (plug_in);
    }
  else
    {
      g_message ("Plug-In \"%s\"\n(%s)\n\n"
		 "sent a TEMP_PROC_RETURN message while not running "
                 "a temporary procedure.  This should not happen.",
                 gimp_filename_to_utf8 (plug_in->name),
		 gimp_filename_to_utf8 (plug_in->prog));
      plug_in_close (plug_in, TRUE);
    }
}

static void
plug_in_handle_proc_install (PlugIn        *plug_in,
                             GPProcInstall *proc_install)
{
  PlugInDef     *plug_in_def = NULL;
  PlugInProcDef *proc_def    = NULL;
  GimpProcedure *procedure   = NULL;
  gchar         *canonical;
  gchar         *prog        = NULL;
  gboolean       valid_utf8  = FALSE;
  gint           i;

  canonical = gimp_canonicalize_identifier (proc_install->name);

  /*  Argument checking
   *   --only sanity check arguments when the procedure requests a menu path
   */

  if (proc_install->menu_path && proc_install->menu_path[0] == '<')
    {
      GError *error = NULL;

      if (! plug_in_param_defs_check (plug_in->name,
                                      plug_in->prog,
                                      canonical,
                                      proc_install->menu_path,
                                      proc_install->params,
                                      proc_install->nparams,
                                      proc_install->return_vals,
                                      proc_install->nreturn_vals,
                                      &error))
        {
          g_message (error->message);
          g_clear_error (&error);

          g_free (canonical);

          return;
        }
    }

  /*  Sanity check for array arguments  */

  for (i = 1; i < proc_install->nparams; i++)
    {
      if ((proc_install->params[i].type == GIMP_PDB_INT32ARRAY ||
	   proc_install->params[i].type == GIMP_PDB_INT8ARRAY  ||
	   proc_install->params[i].type == GIMP_PDB_FLOATARRAY ||
	   proc_install->params[i].type == GIMP_PDB_STRINGARRAY)
          &&
	  proc_install->params[i-1].type != GIMP_PDB_INT32)
	{
	  g_message ("Plug-In \"%s\"\n(%s)\n\n"
                     "attempted to install procedure \"%s\" "
                     "which fails to comply with the array parameter "
                     "passing standard.  Argument %d is noncompliant.",
                     gimp_filename_to_utf8 (plug_in->name),
		     gimp_filename_to_utf8 (plug_in->prog),
                     canonical, i);
          g_free (canonical);
	  return;
	}
    }

  /*  Sanity check strings for UTF-8 validity  */

  if ((proc_install->menu_path == NULL ||
       g_utf8_validate (proc_install->menu_path, -1, NULL)) &&
      (g_utf8_validate (canonical, -1, NULL))               &&
      (proc_install->blurb == NULL ||
       g_utf8_validate (proc_install->blurb, -1, NULL))     &&
      (proc_install->help == NULL ||
       g_utf8_validate (proc_install->help, -1, NULL))      &&
      (proc_install->author == NULL ||
       g_utf8_validate (proc_install->author, -1, NULL))    &&
      (proc_install->copyright == NULL ||
       g_utf8_validate (proc_install->copyright, -1, NULL)) &&
      (proc_install->date == NULL ||
       g_utf8_validate (proc_install->date, -1, NULL)))
    {
      valid_utf8 = TRUE;

      for (i = 0; i < proc_install->nparams && valid_utf8; i++)
        {
          if (! (g_utf8_validate (proc_install->params[i].name, -1, NULL) &&
                 (proc_install->params[i].description == NULL ||
                  g_utf8_validate (proc_install->params[i].description, -1, NULL))))
            valid_utf8 = FALSE;
        }

      for (i = 0; i < proc_install->nreturn_vals && valid_utf8; i++)
        {
          if (! (g_utf8_validate (proc_install->return_vals[i].name, -1, NULL) &&
                 (proc_install->return_vals[i].description == NULL ||
                  g_utf8_validate (proc_install->return_vals[i].description, -1, NULL))))
            valid_utf8 = FALSE;
        }
    }

  if (! valid_utf8)
    {
      g_message ("Plug-In \"%s\"\n(%s)\n\n"
                 "attempted to install a procedure with invalid UTF-8 strings.",
                 gimp_filename_to_utf8 (plug_in->name),
		 gimp_filename_to_utf8 (plug_in->prog));
      g_free (canonical);
      return;
    }

  /*  Initialization  */

  switch (proc_install->type)
    {
    case GIMP_PLUGIN:
    case GIMP_EXTENSION:
      plug_in_def = plug_in->plug_in_def;
      prog        = plug_in_def->prog;

      proc_def = plug_in_proc_def_find (plug_in_def->proc_defs, canonical);

      if (proc_def)
        {
          plug_in_def->proc_defs = g_slist_remove (plug_in_def->proc_defs,
                                                   proc_def);
          plug_in_proc_def_free (proc_def);
        }
      break;

    case GIMP_TEMPORARY:
      plug_in_def = NULL;
      prog        = "none";

      proc_def = plug_in_proc_def_find (plug_in->temp_proc_defs, canonical);

      if (proc_def)
        {
          plug_in->temp_proc_defs = g_slist_remove (plug_in->temp_proc_defs,
                                                    proc_def);
          plug_ins_temp_proc_def_remove (plug_in->gimp, proc_def);
        }
      break;
    }

  proc_def = plug_in_proc_def_new ();

  if (proc_install->menu_path)
    {
      if (proc_install->menu_path[0] == '<')
        proc_def->menu_paths =
          g_list_append (proc_def->menu_paths,
                         g_strdup (proc_install->menu_path));
      else
        proc_def->menu_label = g_strdup (proc_install->menu_path);
    }

  proc_def->prog            = g_strdup (prog);
  proc_def->extensions      = NULL;
  proc_def->prefixes        = NULL;
  proc_def->magics          = NULL;
  proc_def->image_types     = g_strdup (proc_install->image_types);
  proc_def->image_types_val = plug_ins_image_types_parse (proc_def->image_types);
  /* Install temp one use todays time */
  proc_def->mtime           = time (NULL);

  /* Remember if this proc was installed while initing a plug-in */
  proc_def->installed_during_init = plug_in->init;

  /*  The procedural database procedure  */

  procedure = proc_def->procedure;

  gimp_procedure_set_strings (procedure,
                              canonical,
                              proc_install->name,
                              proc_install->blurb,
                              proc_install->help,
                              proc_install->author,
                              proc_install->copyright,
                              proc_install->date,
                              NULL);

  g_free (canonical);

  procedure->proc_type = proc_install->type;

  gimp_procedure_init (procedure,
                       proc_install->nparams, proc_install->nreturn_vals);

  for (i = 0; i < proc_install->nparams; i++)
    {
      canonical = gimp_canonicalize_identifier (proc_install->params[i].name);

      gimp_procedure_add_compat_arg (procedure,
                                     plug_in->gimp,
                                     proc_install->params[i].type,
                                     canonical,
                                     proc_install->params[i].description);

      g_free (canonical);
    }

  for (i = 0; i < proc_install->nreturn_vals; i++)
    {
      canonical = gimp_canonicalize_identifier (proc_install->return_vals[i].name);

      gimp_procedure_add_compat_value (procedure,
                                       plug_in->gimp,
                                       proc_install->return_vals[i].type,
                                       canonical,
                                       proc_install->return_vals[i].description);

      g_free (canonical);
    }

  switch (proc_install->type)
    {
    case GIMP_PLUGIN:
    case GIMP_EXTENSION:
      plug_in_def->proc_defs = g_slist_prepend (plug_in_def->proc_defs,
                                                proc_def);
      break;

    case GIMP_TEMPORARY:
      plug_in->temp_proc_defs = g_slist_prepend (plug_in->temp_proc_defs,
                                                 proc_def);

      procedure->exec_method.temporary.plug_in = plug_in;

      plug_ins_temp_proc_def_add (plug_in->gimp, proc_def);
      break;
    }
}

static void
plug_in_handle_proc_uninstall (PlugIn          *plug_in,
                               GPProcUninstall *proc_uninstall)
{
  PlugInProcDef *proc_def;
  gchar         *canonical;

  canonical = gimp_canonicalize_identifier (proc_uninstall->name);

  proc_def = plug_in_proc_def_find (plug_in->temp_proc_defs, canonical);

  if (proc_def)
    {
      plug_in->temp_proc_defs = g_slist_remove (plug_in->temp_proc_defs,
                                                proc_def);
      plug_ins_temp_proc_def_remove (plug_in->gimp, proc_def);
    }

  g_free (canonical);
}

static void
plug_in_handle_extension_ack (PlugIn *plug_in)
{
  if (plug_in->ext_main_loop)
    {
      g_main_loop_quit (plug_in->ext_main_loop);
    }
  else
    {
      g_message ("Plug-In \"%s\"\n(%s)\n\n"
		 "sent an EXTENSION_ACK message while not being started "
                 "as an extension.  This should not happen.",
                 gimp_filename_to_utf8 (plug_in->name),
		 gimp_filename_to_utf8 (plug_in->prog));
      plug_in_close (plug_in, TRUE);
    }
}

static void
plug_in_handle_has_init (PlugIn *plug_in)
{
  if (plug_in->query)
    {
      plug_in_def_set_has_init (plug_in->plug_in_def, TRUE);
    }
  else
    {
      g_message ("Plug-In \"%s\"\n(%s)\n\n"
		 "sent an HAS_INIT message while not in query().  "
                 "This should not happen.",
                 gimp_filename_to_utf8 (plug_in->name),
		 gimp_filename_to_utf8 (plug_in->prog));
      plug_in_close (plug_in, TRUE);
    }
}
