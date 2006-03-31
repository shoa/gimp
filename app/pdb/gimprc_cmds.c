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

#include <string.h>

#include <glib-object.h>

#include "libgimpconfig/gimpconfig.h"
#include "libgimpmodule/gimpmodule.h"

#include "pdb-types.h"
#include "gimpprocedure.h"
#include "procedural_db.h"
#include "core/gimpparamspecs.h"

#include "config/gimprc.h"
#include "core/gimp.h"
#include "core/gimptemplate.h"

static GimpProcedure gimprc_query_proc;
static GimpProcedure gimprc_set_proc;
static GimpProcedure get_default_comment_proc;
static GimpProcedure get_monitor_resolution_proc;
static GimpProcedure get_theme_dir_proc;
static GimpProcedure get_color_configuration_proc;
static GimpProcedure get_module_load_inhibit_proc;

void
register_gimprc_procs (Gimp *gimp)
{
  GimpProcedure *procedure;

  /*
   * gimprc_query
   */
  procedure = gimp_procedure_init (&gimprc_query_proc, 1, 1);
  gimp_procedure_add_argument (procedure,
                               GIMP_PDB_STRING,
                               gimp_param_spec_string ("token",
                                                       "token",
                                                       "The token to query for",
                                                       FALSE, FALSE,
                                                       NULL,
                                                       GIMP_PARAM_READWRITE));
  gimp_procedure_add_return_value (procedure,
                                   GIMP_PDB_STRING,
                                   gimp_param_spec_string ("value",
                                                           "value",
                                                           "The value associated with the queried token",
                                                           FALSE, FALSE,
                                                           NULL,
                                                           GIMP_PARAM_READWRITE));
  procedural_db_register (gimp, procedure);

  /*
   * gimprc_set
   */
  procedure = gimp_procedure_init (&gimprc_set_proc, 2, 0);
  gimp_procedure_add_argument (procedure,
                               GIMP_PDB_STRING,
                               gimp_param_spec_string ("token",
                                                       "token",
                                                       "The token to add or modify",
                                                       FALSE, FALSE,
                                                       NULL,
                                                       GIMP_PARAM_READWRITE));
  gimp_procedure_add_argument (procedure,
                               GIMP_PDB_STRING,
                               gimp_param_spec_string ("value",
                                                       "value",
                                                       "The value to set the token to",
                                                       FALSE, FALSE,
                                                       NULL,
                                                       GIMP_PARAM_READWRITE));
  procedural_db_register (gimp, procedure);

  /*
   * get_default_comment
   */
  procedure = gimp_procedure_init (&get_default_comment_proc, 0, 1);
  gimp_procedure_add_return_value (procedure,
                                   GIMP_PDB_STRING,
                                   gimp_param_spec_string ("comment",
                                                           "comment",
                                                           "Default Image Comment",
                                                           FALSE, FALSE,
                                                           NULL,
                                                           GIMP_PARAM_READWRITE));
  procedural_db_register (gimp, procedure);

  /*
   * get_monitor_resolution
   */
  procedure = gimp_procedure_init (&get_monitor_resolution_proc, 0, 2);
  gimp_procedure_add_return_value (procedure,
                                   GIMP_PDB_FLOAT,
                                   g_param_spec_double ("xres",
                                                        "xres",
                                                        "X resolution",
                                                        -G_MAXDOUBLE, G_MAXDOUBLE, 0,
                                                        GIMP_PARAM_READWRITE));
  gimp_procedure_add_return_value (procedure,
                                   GIMP_PDB_FLOAT,
                                   g_param_spec_double ("yres",
                                                        "yres",
                                                        "Y resolution",
                                                        -G_MAXDOUBLE, G_MAXDOUBLE, 0,
                                                        GIMP_PARAM_READWRITE));
  procedural_db_register (gimp, procedure);

  /*
   * get_theme_dir
   */
  procedure = gimp_procedure_init (&get_theme_dir_proc, 0, 1);
  gimp_procedure_add_return_value (procedure,
                                   GIMP_PDB_STRING,
                                   gimp_param_spec_string ("theme-dir",
                                                           "theme dir",
                                                           "The GUI theme dir",
                                                           FALSE, FALSE,
                                                           NULL,
                                                           GIMP_PARAM_READWRITE));
  procedural_db_register (gimp, procedure);

  /*
   * get_color_configuration
   */
  procedure = gimp_procedure_init (&get_color_configuration_proc, 0, 1);
  gimp_procedure_add_return_value (procedure,
                                   GIMP_PDB_STRING,
                                   gimp_param_spec_string ("config",
                                                           "config",
                                                           "Serialized color management configuration",
                                                           FALSE, FALSE,
                                                           NULL,
                                                           GIMP_PARAM_READWRITE));
  procedural_db_register (gimp, procedure);

  /*
   * get_module_load_inhibit
   */
  procedure = gimp_procedure_init (&get_module_load_inhibit_proc, 0, 1);
  gimp_procedure_add_return_value (procedure,
                                   GIMP_PDB_STRING,
                                   gimp_param_spec_string ("load-inhibit",
                                                           "load inhibit",
                                                           "The list of modules",
                                                           FALSE, FALSE,
                                                           NULL,
                                                           GIMP_PARAM_READWRITE));
  procedural_db_register (gimp, procedure);

}

static Argument *
gimprc_query_invoker (GimpProcedure *procedure,
                      Gimp          *gimp,
                      GimpContext   *context,
                      GimpProgress  *progress,
                      Argument      *args)
{
  gboolean success = TRUE;
  Argument *return_vals;
  gchar *token;
  gchar *value = NULL;

  token = (gchar *) g_value_get_string (&args[0].value);

  if (success)
    {
      if (strlen (token))
        {
          /*  use edit_config because unknown tokens are set there  */
          value = gimp_rc_query (GIMP_RC (gimp->edit_config), token);

          if (! value)
            success = FALSE;
        }
      else
        success = FALSE;
    }

  return_vals = gimp_procedure_get_return_values (procedure, success);

  if (success)
    g_value_take_string (&return_vals[1].value, value);

  return return_vals;
}

static GimpProcedure gimprc_query_proc =
{
  TRUE, TRUE,
  "gimp-gimprc-query",
  "gimp-gimprc-query",
  "Queries the gimprc file parser for information on a specified token.",
  "This procedure is used to locate additional information contained in the gimprc file considered extraneous to the operation of the GIMP. Plug-ins that need configuration information can expect it will be stored in the user gimprc file and can use this procedure to retrieve it. This query procedure will return the value associated with the specified token. This corresponds _only_ to entries with the format: (<token> <value>). The value must be a string. Entries not corresponding to this format will cause warnings to be issued on gimprc parsing and will not be queryable.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1997",
  NULL,
  GIMP_INTERNAL,
  0, NULL, 0, NULL,
  { { gimprc_query_invoker } }
};

static Argument *
gimprc_set_invoker (GimpProcedure *procedure,
                    Gimp          *gimp,
                    GimpContext   *context,
                    GimpProgress  *progress,
                    Argument      *args)
{
  gboolean success = TRUE;
  gchar *token;
  gchar *value;

  token = (gchar *) g_value_get_string (&args[0].value);
  value = (gchar *) g_value_get_string (&args[1].value);

  if (success)
    {
      if (strlen (token))
        {
          /*  use edit_config because that's the one that gets saved  */
          gimp_rc_set_unknown_token (GIMP_RC (gimp->edit_config), token, value);
        }
      else
        success = FALSE;
    }

  return gimp_procedure_get_return_values (procedure, success);
}

static GimpProcedure gimprc_set_proc =
{
  TRUE, TRUE,
  "gimp-gimprc-set",
  "gimp-gimprc-set",
  "Sets a gimprc token to a value and saves it in the gimprc.",
  "This procedure is used to add or change additional information in the gimprc file that is considered extraneous to the operation of the GIMP. Plug-ins that need configuration information can use this function to store it, and gimp_gimprc_query to retrieve it. This will accept _only_ string values in UTF-8 encoding.",
  "Seth Burgess",
  "Seth Burgess",
  "1999",
  NULL,
  GIMP_INTERNAL,
  0, NULL, 0, NULL,
  { { gimprc_set_invoker } }
};

static Argument *
get_default_comment_invoker (GimpProcedure *procedure,
                             Gimp          *gimp,
                             GimpContext   *context,
                             GimpProgress  *progress,
                             Argument      *args)
{
  Argument *return_vals;
  gchar *comment = NULL;

  comment = g_strdup (gimp->config->default_image->comment);

  return_vals = gimp_procedure_get_return_values (procedure, TRUE);
  g_value_take_string (&return_vals[1].value, comment);

  return return_vals;
}

static GimpProcedure get_default_comment_proc =
{
  TRUE, TRUE,
  "gimp-get-default-comment",
  "gimp-get-default-comment",
  "Get the default image comment as specified in the Preferences.",
  "Returns a copy of the default image comment.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  NULL,
  GIMP_INTERNAL,
  0, NULL, 0, NULL,
  { { get_default_comment_invoker } }
};

static Argument *
get_monitor_resolution_invoker (GimpProcedure *procedure,
                                Gimp          *gimp,
                                GimpContext   *context,
                                GimpProgress  *progress,
                                Argument      *args)
{
  Argument *return_vals;
  gdouble xres = 0.0;
  gdouble yres = 0.0;

  xres = GIMP_DISPLAY_CONFIG (gimp->config)->monitor_xres;
  yres = GIMP_DISPLAY_CONFIG (gimp->config)->monitor_yres;

  return_vals = gimp_procedure_get_return_values (procedure, TRUE);

  g_value_set_double (&return_vals[1].value, xres);
  g_value_set_double (&return_vals[2].value, yres);

  return return_vals;
}

static GimpProcedure get_monitor_resolution_proc =
{
  TRUE, TRUE,
  "gimp-get-monitor-resolution",
  "gimp-get-monitor-resolution",
  "Get the monitor resolution as specified in the Preferences.",
  "Returns the resolution of the monitor in pixels/inch. This value is taken from the Preferences (or the windowing system if this is set in the Preferences) and there's no guarantee for the value to be reasonable.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  NULL,
  GIMP_INTERNAL,
  0, NULL, 0, NULL,
  { { get_monitor_resolution_invoker } }
};

static Argument *
get_theme_dir_invoker (GimpProcedure *procedure,
                       Gimp          *gimp,
                       GimpContext   *context,
                       GimpProgress  *progress,
                       Argument      *args)
{
  Argument *return_vals;
  gchar *theme_dir = NULL;

  theme_dir = g_strdup (gimp_get_theme_dir (gimp));

  return_vals = gimp_procedure_get_return_values (procedure, TRUE);
  g_value_take_string (&return_vals[1].value, theme_dir);

  return return_vals;
}

static GimpProcedure get_theme_dir_proc =
{
  TRUE, TRUE,
  "gimp-get-theme-dir",
  "gimp-get-theme-dir",
  "Get the directory of the current GUI theme.",
  "Returns a copy of the current GUI theme dir.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  NULL,
  GIMP_INTERNAL,
  0, NULL, 0, NULL,
  { { get_theme_dir_invoker } }
};

static Argument *
get_color_configuration_invoker (GimpProcedure *procedure,
                                 Gimp          *gimp,
                                 GimpContext   *context,
                                 GimpProgress  *progress,
                                 Argument      *args)
{
  Argument *return_vals;
  gchar *config = NULL;

  config = gimp_config_serialize_to_string (GIMP_CONFIG (gimp->config->color_management), NULL);

  return_vals = gimp_procedure_get_return_values (procedure, TRUE);
  g_value_take_string (&return_vals[1].value, config);

  return return_vals;
}

static GimpProcedure get_color_configuration_proc =
{
  TRUE, TRUE,
  "gimp-get-color-configuration",
  "gimp-get-color-configuration",
  "Get a serialized version of the color management configuration.",
  "Returns a string that can be deserialized into a GimpColorConfig object representing the current color management configuration.",
  "Sven Neumann <sven@gimp.org>",
  "Sven Neumann",
  "2005",
  NULL,
  GIMP_INTERNAL,
  0, NULL, 0, NULL,
  { { get_color_configuration_invoker } }
};

static Argument *
get_module_load_inhibit_invoker (GimpProcedure *procedure,
                                 Gimp          *gimp,
                                 GimpContext   *context,
                                 GimpProgress  *progress,
                                 Argument      *args)
{
  Argument *return_vals;
  gchar *load_inhibit = NULL;

  load_inhibit = g_strdup (gimp_module_db_get_load_inhibit (gimp->module_db));

  return_vals = gimp_procedure_get_return_values (procedure, TRUE);
  g_value_take_string (&return_vals[1].value, load_inhibit);

  return return_vals;
}

static GimpProcedure get_module_load_inhibit_proc =
{
  TRUE, TRUE,
  "gimp-get-module-load-inhibit",
  "gimp-get-module-load-inhibit",
  "Get the list of modules which should not be loaded.",
  "Returns a copy of the list of modules which should not be loaded.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  NULL,
  GIMP_INTERNAL,
  0, NULL, 0, NULL,
  { { get_module_load_inhibit_invoker } }
};
