/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2003 Peter Mattis and Spencer Kimball
 *
 * gimpplugin_pdb.c
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

#include "config.h"

#include "gimp.h"

/**
 * gimp_progress_init:
 * @message: Message to use in the progress dialog.

 *
 * Initializes the progress bar for the current plug-in.
 *
 * Initializes the progress bar for the current plug-in. It is only
 * valid to call this procedure from a plug-in.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_progress_init (const gchar *message)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_progress_init",
				    &nreturn_vals,
				    GIMP_PDB_STRING, message,
				    GIMP_PDB_INT32, gimp_default_display (),
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_progress_update:
 * @percentage: Percentage of progress completed which must be between 0.0 and 1.0.
 *
 * Updates the progress bar for the current plug-in.
 *
 * Updates the progress bar for the current plug-in. It is only valid
 * to call this procedure from a plug-in.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_progress_update (gdouble percentage)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_progress_update",
				    &nreturn_vals,
				    GIMP_PDB_FLOAT, percentage,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_plugin_domain_register:
 * @domain_name: The name of the textdomain (must be unique).
 * @domain_path: The absolute path to the compiled message catalog (may be NULL).
 *
 * Registers a textdomain for localisation.
 *
 * This procedure adds a textdomain to the list of domains Gimp
 * searches for strings when translating its menu entries. There is no
 * need to call this function for plug-ins that have their strings
 * included in the gimp-std-plugins domain as that is used by default.
 * If the compiled message catalog is not in the standard location, you
 * may specify an absolute path to another location. This procedure can
 * only be called in the query function of a plug-in and it has to be
 * called before any procedure is installed.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_plugin_domain_register (const gchar *domain_name,
			     const gchar *domain_path)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_plugin_domain_register",
				    &nreturn_vals,
				    GIMP_PDB_STRING, domain_name,
				    GIMP_PDB_STRING, domain_path,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_plugin_help_register:
 * @domain_name: The XML namespace of the plug-in's help pages.
 * @domain_uri: The root URI of the plug-in's help pages.
 *
 * Register a help path for a plug-in.
 *
 * This procedure changes the help rootdir for the plug-in which calls
 * it. All subsequent calls of gimp_help from this plug-in will be
 * interpreted relative to this rootdir.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_plugin_help_register (const gchar *domain_name,
			   const gchar *domain_uri)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_plugin_help_register",
				    &nreturn_vals,
				    GIMP_PDB_STRING, domain_name,
				    GIMP_PDB_STRING, domain_uri,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_plugin_menu_register:
 * @procedure_name: The procedure for which to install the menu path.
 * @menu_path: The procedure's additional menu path.
 *
 * Register an additional menu path for a plug-in procedure.
 *
 * This procedure installs an additional menu entry for the given
 * procedure.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_plugin_menu_register (const gchar *procedure_name,
			   const gchar *menu_path)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_plugin_menu_register",
				    &nreturn_vals,
				    GIMP_PDB_STRING, procedure_name,
				    GIMP_PDB_STRING, menu_path,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}
