/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#include <glib.h>		/* Include early for obscure Win32
				   build reasons */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <gtk/gtk.h>

#ifdef G_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "siod/siod.h"

#include "script-fu-types.h"

#include "script-fu-interface.h"
#include "script-fu-scripts.h"
#include "siod-wrapper.h"

#include "script-fu-intl.h"


#define RESPONSE_RESET         1
#define RESPONSE_ABOUT         2

#define TEXT_WIDTH           100
#define COLOR_SAMPLE_WIDTH   100
#define COLOR_SAMPLE_HEIGHT   15
#define SLIDER_WIDTH          80

#define MAX_STRING_LENGTH   4096


/* External functions
 */
extern long  nlength (LISP obj);

/*
 *  Local Functions
 */

static void       script_fu_load_script    (const GimpDatafileData *file_data,
                                            gpointer                user_data);
static gboolean   script_fu_install_script (gpointer                foo,
                                            SFScript               *script,
                                            gpointer                bar);
static gboolean   script_fu_remove_script  (gpointer                foo,
                                            SFScript               *script,
                                            gpointer                bar);
static void       script_fu_script_proc    (const gchar            *name,
                                            gint                    nparams,
                                            const GimpParam        *params,
                                            gint                   *nreturn_vals,
                                            GimpParam             **return_vals);

static SFScript * script_fu_find_script    (const gchar            *script_name);
static void       script_fu_free_script    (SFScript               *script);


/*
 *  Local variables
 */

static GTree *script_list  = NULL;


/*
 *  Function definitions
 */

void
script_fu_find_scripts (void)
{
  gchar *path_str;

  /*  Make sure to clear any existing scripts  */
  if (script_list != NULL)
    {
      g_tree_foreach (script_list,
                      (GTraverseFunc) script_fu_remove_script,
                      NULL);
      g_tree_destroy (script_list);
    }

  script_list = g_tree_new ((GCompareFunc) strcoll);

  path_str = gimp_gimprc_query ("script-fu-path");

  if (path_str == NULL)
    return;

  gimp_datafiles_read_directories (path_str, G_FILE_TEST_IS_REGULAR,
                                   script_fu_load_script,
                                   NULL);

  g_free (path_str);

  /*  now that all scripts are read in and sorted, tell gimp about them  */
  g_tree_foreach (script_list,
                  (GTraverseFunc) script_fu_install_script,
                  NULL);
}

LISP
script_fu_add_script (LISP a)
{
  GimpParamDef *args;
  SFScript     *script;
  gchar        *val;
  gint          i;
  guchar        r, g, b;
  LISP          color_list;
  LISP          adj_list;
  LISP          brush_list;
  LISP          option_list;
  gchar        *s;

  /*  Check the length of a  */
  if (nlength (a) < 7)
    return my_err ("Too few arguments to script-fu-register", NIL);

  /*  Create a new script  */
  script = g_new0 (SFScript, 1);

  /*  Find the script name  */
  val = get_c_string (car (a));
  script->script_name = g_strdup (val);
  a = cdr (a);

  /* transform the function name into a name containing "_" for each "-".
   * this does not hurt anybody, yet improves the life of many... ;)
   */
  script->pdb_name = g_strdup (val);

  for (s = script->pdb_name; *s; s++)
    if (*s == '-')
      *s = '_';

  /*  Find the script description  */
  val = get_c_string (car (a));
  script->description = g_strdup (val);
  a = cdr (a);

  /*  Find the script help  */
  val = get_c_string (car (a));
  script->help = g_strdup (val);
  a = cdr (a);

  /*  Find the script author  */
  val = get_c_string (car (a));
  script->author = g_strdup (val);
  a = cdr (a);

  /*  Find the script copyright  */
  val = get_c_string (car (a));
  script->copyright = g_strdup (val);
  a = cdr (a);

  /*  Find the script date  */
  val = get_c_string (car (a));
  script->date = g_strdup (val);
  a = cdr (a);

  /*  Find the script image types  */
  if (TYPEP (a, tc_cons))
    {
      val = get_c_string (car (a));
      a = cdr (a);
    }
  else
    {
      val = get_c_string (a);
      a = NIL;
    }
  script->img_types = g_strdup (val);

  /*  Check the supplied number of arguments  */
  script->num_args = nlength (a) / 3;

  args = g_new0 (GimpParamDef, script->num_args + 1);
  args[0].type        = GIMP_PDB_INT32;
  args[0].name        = "run_mode";
  args[0].description = "Interactive, non-interactive";

  script->arg_types    = g_new0 (SFArgType, script->num_args);
  script->arg_labels   = g_new0 (gchar *, script->num_args);
  script->arg_defaults = g_new0 (SFArgValue, script->num_args);
  script->arg_values   = g_new0 (SFArgValue, script->num_args);

  if (script->num_args > 0)
    {
      for (i = 0; i < script->num_args; i++)
	{
	  if (a != NIL)
	    {
	      if (!TYPEP (car (a), tc_flonum))
		return my_err ("script-fu-register: argument types must be integer values", NIL);
	      script->arg_types[i] = get_c_long (car (a));
	      a = cdr (a);
	    }
	  else
	    return my_err ("script-fu-register: missing type specifier", NIL);

	  if (a != NIL)
	    {
	      if (!TYPEP (car (a), tc_string))
		return my_err ("script-fu-register: argument labels must be strings", NIL);
	      script->arg_labels[i] = g_strdup (get_c_string (car (a)));
	      a = cdr (a);
	    }
	  else
	    return my_err ("script-fu-register: missing arguments label", NIL);

	  if (a != NIL)
	    {
	      switch (script->arg_types[i])
		{
		case SF_IMAGE:
		case SF_DRAWABLE:
		case SF_LAYER:
		case SF_CHANNEL:
		  if (!TYPEP (car (a), tc_flonum))
		    return my_err ("script-fu-register: drawable defaults must be integer values", NIL);

		  script->arg_defaults[i].sfa_image =
                    get_c_long (car (a));
		  script->arg_values[i].sfa_image =
                    script->arg_defaults[i].sfa_image;

		  switch (script->arg_types[i])
		    {
		    case SF_IMAGE:
		      args[i + 1].type = GIMP_PDB_IMAGE;
		      args[i + 1].name = "image";
		      break;

		    case SF_DRAWABLE:
		      args[i + 1].type = GIMP_PDB_DRAWABLE;
		      args[i + 1].name = "drawable";
		      break;

		    case SF_LAYER:
		      args[i + 1].type = GIMP_PDB_LAYER;
		      args[i + 1].name = "layer";
		      break;

		    case SF_CHANNEL:
		      args[i + 1].type = GIMP_PDB_CHANNEL;
		      args[i + 1].name = "channel";
		      break;

		    default:
		      break;
		    }

		  args[i + 1].description = script->arg_labels[i];
		  break;

		case SF_COLOR:
		  if (!TYPEP (car (a), tc_cons))
		    return my_err ("script-fu-register: color defaults must be a list of 3 integers", NIL);

		  color_list = car (a);
		  r = CLAMP (get_c_long (car (color_list)), 0, 255);
		  color_list = cdr (color_list);
		  g = CLAMP (get_c_long (car (color_list)), 0, 255);
		  color_list = cdr (color_list);
		  b = CLAMP (get_c_long (car (color_list)), 0, 255);

		  gimp_rgb_set_uchar (&script->arg_defaults[i].sfa_color,
				      r, g, b);

		  script->arg_values[i].sfa_color =
		    script->arg_defaults[i].sfa_color;

		  args[i + 1].type        = GIMP_PDB_COLOR;
		  args[i + 1].name        = "color";
		  args[i + 1].description = script->arg_labels[i];
		  break;

		case SF_TOGGLE:
		  if (!TYPEP (car (a), tc_flonum))
		    return my_err ("script-fu-register: toggle default must be an integer value", NIL);

		  script->arg_defaults[i].sfa_toggle =
		    (get_c_long (car (a))) ? TRUE : FALSE;
		  script->arg_values[i].sfa_toggle =
		    script->arg_defaults[i].sfa_toggle;

		  args[i + 1].type        = GIMP_PDB_INT32;
		  args[i + 1].name        = "toggle";
		  args[i + 1].description = script->arg_labels[i];
		  break;

		case SF_VALUE:
		  if (!TYPEP (car (a), tc_string))
		    return my_err ("script-fu-register: value defaults must be string values", NIL);

		  script->arg_defaults[i].sfa_value =
		    g_strdup (get_c_string (car (a)));
		  script->arg_values[i].sfa_value =
		    g_strdup (script->arg_defaults[i].sfa_value);

		  args[i + 1].type        = GIMP_PDB_STRING;
		  args[i + 1].name        = "value";
		  args[i + 1].description = script->arg_labels[i];
		  break;

		case SF_STRING:
		  if (!TYPEP (car (a), tc_string))
		    return my_err ("script-fu-register: string defaults must be string values", NIL);

		  script->arg_defaults[i].sfa_value =
		    g_strdup (get_c_string (car (a)));
		  script->arg_values[i].sfa_value =
		    g_strdup (script->arg_defaults[i].sfa_value);

		  args[i + 1].type        = GIMP_PDB_STRING;
		  args[i + 1].name        = "string";
		  args[i + 1].description = script->arg_labels[i];
		  break;

		case SF_ADJUSTMENT:
		  if (!TYPEP (car (a), tc_cons))
		    return my_err ("script-fu-register: adjustment defaults must be a list", NIL);

		  adj_list = car (a);
		  script->arg_defaults[i].sfa_adjustment.value =
		    get_c_double (car (adj_list));
		  adj_list = cdr (adj_list);
		  script->arg_defaults[i].sfa_adjustment.lower =
		    get_c_double (car (adj_list));
		  adj_list = cdr (adj_list);
		  script->arg_defaults[i].sfa_adjustment.upper =
		    get_c_double (car (adj_list));
		  adj_list = cdr (adj_list);
		  script->arg_defaults[i].sfa_adjustment.step =
		    get_c_double (car (adj_list));
		  adj_list = cdr (adj_list);
		  script->arg_defaults[i].sfa_adjustment.page =
		    get_c_double (car (adj_list));
		  adj_list = cdr (adj_list);
		  script->arg_defaults[i].sfa_adjustment.digits =
		    get_c_long (car (adj_list));
		  adj_list = cdr (adj_list);
		  script->arg_defaults[i].sfa_adjustment.type =
		    get_c_long (car (adj_list));
		  script->arg_values[i].sfa_adjustment.adj = NULL;

		  script->arg_values[i].sfa_adjustment.value =
		    script->arg_defaults[i].sfa_adjustment.value;

		  args[i + 1].type        = GIMP_PDB_FLOAT;
		  args[i + 1].name        = "value";
		  args[i + 1].description = script->arg_labels[i];
		  break;

		case SF_FILENAME:
		  if (!TYPEP (car (a), tc_string))
		    return my_err ("script-fu-register: filename defaults must be string values", NIL);
                  /* fallthrough */

		case SF_DIRNAME:
		  if (!TYPEP (car (a), tc_string))
		    return my_err ("script-fu-register: dirname defaults must be string values", NIL);

		  script->arg_defaults[i].sfa_file.filename =
		    g_strdup (get_c_string (car (a)));

#ifdef G_OS_WIN32
		  /* Replace POSIX slashes with Win32 backslashes. This
		   * is just so script-fus can be written with only
		   * POSIX directory separators.
		   */
		  val = script->arg_defaults[i].sfa_file.filename;
		  while (*val)
		    {
		      if (*val == '/')
			*val = '\\';
		      val++;
		    }
#endif
		  script->arg_values[i].sfa_file.filename =
		    g_strdup (script->arg_defaults[i].sfa_file.filename);
		  script->arg_values[i].sfa_file.file_entry = NULL;

		  args[i + 1].type        = GIMP_PDB_STRING;
		  args[i + 1].name        = (script->arg_types[i] == SF_FILENAME ?
                                             "filename" : "dirname");
		  args[i + 1].description = script->arg_labels[i];
		 break;

		case SF_FONT:
		  if (!TYPEP (car (a), tc_string))
		    return my_err ("script-fu-register: font defaults must be string values", NIL);

		  script->arg_defaults[i].sfa_font =
                    g_strdup (get_c_string (car (a)));
		  script->arg_values[i].sfa_font =
                    g_strdup (script->arg_defaults[i].sfa_font);

		  args[i + 1].type        = GIMP_PDB_STRING;
		  args[i + 1].name        = "font";
		  args[i + 1].description = script->arg_labels[i];
		  break;

		case SF_PALETTE:
		  if (!TYPEP (car (a), tc_string))
		    return my_err ("script-fu-register: palette defaults must be string values", NIL);

		  script->arg_defaults[i].sfa_palette =
                    g_strdup (get_c_string (car (a)));
		  script->arg_values[i].sfa_palette =
                    g_strdup (script->arg_defaults[i].sfa_palette);

		  args[i + 1].type        = GIMP_PDB_STRING;
		  args[i + 1].name        = "palette";
		  args[i + 1].description = script->arg_labels[i];
		  break;

		case SF_PATTERN:
		  if (!TYPEP (car (a), tc_string))
		    return my_err ("script-fu-register: pattern defaults must be string values", NIL);

		  script->arg_defaults[i].sfa_pattern =
		    g_strdup (get_c_string (car (a)));
		  script->arg_values[i].sfa_pattern =
		    g_strdup (script->arg_defaults[i].sfa_pattern);

		  args[i + 1].type        = GIMP_PDB_STRING;
		  args[i + 1].name        = "pattern";
		  args[i + 1].description = script->arg_labels[i];
		  break;

		case SF_BRUSH:
		  if (!TYPEP (car (a), tc_cons))
		    return my_err ("script-fu-register: brush defaults must be a list", NIL);

		  brush_list = car (a);
		  script->arg_defaults[i].sfa_brush.name =
		    g_strdup (get_c_string (car (brush_list)));
		  script->arg_values[i].sfa_brush.name =
		    g_strdup (script->arg_defaults[i].sfa_brush.name);

		  brush_list = cdr (brush_list);
		  script->arg_defaults[i].sfa_brush.opacity =
		    get_c_double (car (brush_list));
		  script->arg_values[i].sfa_brush.opacity =
		    script->arg_defaults[i].sfa_brush.opacity;

		  brush_list = cdr (brush_list);
		  script->arg_defaults[i].sfa_brush.spacing =
		    get_c_long (car (brush_list));
		  script->arg_values[i].sfa_brush.spacing =
		    script->arg_defaults[i].sfa_brush.spacing;

		  brush_list = cdr (brush_list);
		  script->arg_defaults[i].sfa_brush.paint_mode =
		    get_c_long (car (brush_list));
		  script->arg_values[i].sfa_brush.paint_mode =
		    script->arg_defaults[i].sfa_brush.paint_mode;

		  args[i + 1].type        = GIMP_PDB_STRING;
		  args[i + 1].name        = "brush";
		  args[i + 1].description = script->arg_labels[i];
		  break;

		case SF_GRADIENT:
		  if (!TYPEP (car (a), tc_string))
		    return my_err ("script-fu-register: gradient defaults must be string values", NIL);

		  script->arg_defaults[i].sfa_gradient =
		    g_strdup (get_c_string (car (a)));
		  script->arg_values[i].sfa_gradient =
		    g_strdup (script->arg_defaults[i].sfa_pattern);

		  args[i + 1].type        = GIMP_PDB_STRING;
		  args[i + 1].name        = "gradient";
		  args[i + 1].description = script->arg_labels[i];
		  break;

		case SF_OPTION:
		  if (!TYPEP (car (a), tc_cons))
		    return my_err ("script-fu-register: option defaults must be a list", NIL);

		  for (option_list = car (a);
		       option_list;
		       option_list = cdr (option_list))
		    {
		      script->arg_defaults[i].sfa_option.list =
			g_slist_append (script->arg_defaults[i].sfa_option.list,
					g_strdup (get_c_string (car (option_list))));
		    }
		  script->arg_defaults[i].sfa_option.history = 0;
		  script->arg_values[i].sfa_option.history = 0;

		  args[i + 1].type        = GIMP_PDB_INT32;
		  args[i + 1].name        = "option";
		  args[i + 1].description = script->arg_labels[i];
		  break;

		default:
		  break;
		}

	      a = cdr (a);
	    }
	  else
            {
              return my_err ("script-fu-register: missing default argument",
                             NIL);
            }
	}
    }

  script->args = args;
  g_tree_insert (script_list, gettext (script->description), script);

  return NIL;
}

void
script_fu_error_msg (const gchar *command)
{
  g_message (_("Error while executing\n%s\n%s"),
	     command, siod_get_error_msg ());
}


/*  private functions  */

static void
script_fu_load_script (const GimpDatafileData *file_data,
                       gpointer                user_data)
{
  if (gimp_datafiles_check_extension (file_data->filename, ".scm"))
    {
      gchar *command;
      gchar *escaped = g_strescape (file_data->filename, NULL);

      command = g_strdup_printf ("(load \"%s\")", escaped);
      g_free (escaped);

      if (repl_c_string (command, 0, 0, 1) != 0)
        script_fu_error_msg (command);

#ifdef G_OS_WIN32
      /* No, I don't know why, but this is
       * necessary on NT 4.0.
       */
      Sleep(0);
#endif

      g_free (command);
    }
}

/*
 *  The following function is a GTraverseFunction.  Please
 *  note that it frees the script->args structure.  --Sven
 */
static gboolean
script_fu_install_script (gpointer  foo,
			  SFScript *script,
			  gpointer  bar)
{
  gchar *menu_path = NULL;

  /* Allow scripts with no menus */
  if (strncmp (script->description, "<None>", 6) != 0)
    menu_path = script->description;

  gimp_install_temp_proc (script->pdb_name,
                          script->description,
                          script->help,
                          script->author,
                          script->copyright,
                          script->date,
                          menu_path,
                          script->img_types,
                          GIMP_TEMPORARY,
                          script->num_args + 1, 0,
                          script->args, NULL,
                          script_fu_script_proc);

  g_free (script->args);
  script->args = NULL;

  return FALSE;
}

/*
 *  The following function is a GTraverseFunction.
 */
static gboolean
script_fu_remove_script (gpointer  foo,
			 SFScript *script,
			 gpointer  bar)
{
  script_fu_free_script (script);

  return FALSE;
}

static void
script_fu_script_proc (const gchar      *name,
		       gint              nparams,
		       const GimpParam  *params,
		       gint             *nreturn_vals,
		       GimpParam       **return_vals)
{
  static GimpParam   values[1];
  GimpPDBStatusType  status = GIMP_PDB_SUCCESS;
  GimpRunMode        run_mode;
  SFScript          *script;
  gint               min_args;
  gchar             *escaped;

  run_mode = params[0].data.d_int32;

  if (! (script = script_fu_find_script (name)))
    {
      status = GIMP_PDB_CALLING_ERROR;
    }
  else
    {
      if (script->num_args == 0)
	run_mode = GIMP_RUN_NONINTERACTIVE;

      switch (run_mode)
	{
	case GIMP_RUN_INTERACTIVE:
	case GIMP_RUN_WITH_LAST_VALS:
	  /*  Determine whether the script is image based (runs on an image) */
	  if (strncmp (script->description, "<Image>", 7) == 0)
	    {
	      script->arg_values[0].sfa_image    = params[1].data.d_image;
	      script->arg_values[1].sfa_drawable = params[2].data.d_drawable;
	      script->image_based = TRUE;
	    }
	  else
	    script->image_based = FALSE;

	  /*  First acquire information with a dialog  */
	  /*  Skip this part if the script takes no parameters */
	  min_args = (script->image_based) ? 2 : 0;
	  if (script->num_args > min_args)
	    {
	      script_fu_interface (script);
	      break;
	    }
	  /*  else fallthrough  */

	case GIMP_RUN_NONINTERACTIVE:
	  /*  Make sure all the arguments are there!  */
	  if (nparams != (script->num_args + 1))
            status = GIMP_PDB_CALLING_ERROR;

	  if (status == GIMP_PDB_SUCCESS)
	    {
	      guchar color[3];
	      gchar *text = NULL;
	      gchar *command;
	      gchar *c;
	      gchar  buffer[MAX_STRING_LENGTH];
	      gint   length;
	      gint   i;

	      length = strlen (script->script_name) + 3;

	      for (i = 0; i < script->num_args; i++)
		switch (script->arg_types[i])
		  {
		  case SF_IMAGE:
		  case SF_DRAWABLE:
		  case SF_LAYER:
		  case SF_CHANNEL:
		    length += 12;  /*  Maximum size of integer value will not exceed this many characters  */
		    break;

		  case SF_COLOR:
		    length += 16;  /*  Maximum size of color string: '(XXX XXX XXX)  */
		    break;

		  case SF_TOGGLE:
		    length += 6;   /*  Maximum size of (TRUE, FALSE)  */
		    break;

		  case SF_VALUE:
		    length += strlen (params[i + 1].data.d_string) + 1;
		    break;

		  case SF_STRING:
		  case SF_FILENAME:
		  case SF_DIRNAME:
		    escaped = g_strescape (params[i + 1].data.d_string, NULL);
		    length += strlen (escaped) + 3;
		    g_free (escaped);
		    break;

		  case SF_ADJUSTMENT:
		    length += G_ASCII_DTOSTR_BUF_SIZE;
		    break;

		  case SF_FONT:
                  case SF_PALETTE:
		  case SF_PATTERN:
		  case SF_GRADIENT:
		  case SF_BRUSH:
		    length += strlen (params[i + 1].data.d_string) + 3;
		    break;

		  case SF_OPTION:
		    length += strlen (params[i + 1].data.d_string) + 1;
		    break;

		  default:
		    break;
		  }

	      c = command = g_new (gchar, length);

	      if (script->num_args)
                {
                  sprintf (command, "(%s ", script->script_name);
                  c += strlen (script->script_name) + 2;

                  for (i = 0; i < script->num_args; i++)
                    {
                      switch (script->arg_types[i])
                        {
                        case SF_IMAGE:
                        case SF_DRAWABLE:
                        case SF_LAYER:
                        case SF_CHANNEL:
                          g_snprintf (buffer, sizeof (buffer), "%d",
				      params[i + 1].data.d_image);
                          text = buffer;
                          break;

                        case SF_COLOR:
			  gimp_rgb_get_uchar (&params[i + 1].data.d_color,
					      color, color + 1, color + 2);
                          g_snprintf (buffer, sizeof (buffer), "'(%d %d %d)",
				      color[0], color[1], color[2]);
                          text = buffer;
                          break;

                        case SF_TOGGLE:
                          g_snprintf (buffer, sizeof (buffer), "%s",
				      (params[i + 1].data.d_int32) ? "TRUE"
				                                   : "FALSE");
                          text = buffer;
                          break;

                        case SF_VALUE:
                          text = params[i + 1].data.d_string;
                          break;

                        case SF_STRING:
                        case SF_FILENAME:
                        case SF_DIRNAME:
                          escaped = g_strescape (params[i + 1].data.d_string,
                                                 NULL);
                          g_snprintf (buffer, sizeof (buffer), "\"%s\"",
				      escaped);
                          g_free (escaped);
                          text = buffer;
                          break;

                        case SF_ADJUSTMENT:
                          text = g_ascii_dtostr (buffer, sizeof (buffer),
                                                 params[i + 1].data.d_float);
                          break;

                        case SF_FONT:
                        case SF_PALETTE:
                        case SF_PATTERN:
                        case SF_GRADIENT:
                        case SF_BRUSH:
                          g_snprintf (buffer, sizeof (buffer), "\"%s\"",
				      params[i + 1].data.d_string);
                          text = buffer;
                          break;

                        case SF_OPTION:
                          text = params[i + 1].data.d_string;
                          break;

                        default:
                          break;
                        }

                      if (i == script->num_args - 1)
                        sprintf (c, "%s)", text);
                      else
                        sprintf (c, "%s ", text);

                      c += strlen (text) + 1;
                    }
                }
	      else
		sprintf (command, "(%s)", script->script_name);

	      /*  run the command through the interpreter  */
	      if (repl_c_string (command, 0, 0, 1) != 0)
		script_fu_error_msg (command);

	      g_free (command);
	    }
	  break;

	default:
	  break;
	}
    }

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = GIMP_PDB_STATUS;
  values[0].data.d_status = status;
}

/* this is a GTraverseFunction */
static gboolean
script_fu_lookup_script (gpointer      *foo,
			 SFScript      *script,
			 gconstpointer *name)
{
  if (strcmp (script->pdb_name, *name) == 0)
    {
      /* store the script in the name pointer and stop the traversal */
      *name = script;
      return TRUE;
    }

  return FALSE;
}

static SFScript *
script_fu_find_script (const gchar *pdb_name)
{
  gconstpointer script = pdb_name;

  g_tree_foreach (script_list,
                  (GTraverseFunc) script_fu_lookup_script,
                  &script);

  if (script == pdb_name)
    return NULL;

  return (SFScript *) script;
}

static void
script_fu_free_script (SFScript *script)
{
  gint i;

  /*  Uninstall the temporary procedure for this script  */
  gimp_uninstall_temp_proc (script->pdb_name);

  if (script)
    {
      g_free (script->script_name);
      g_free (script->description);
      g_free (script->help);
      g_free (script->author);
      g_free (script->copyright);
      g_free (script->date);
      g_free (script->img_types);

      for (i = 0; i < script->num_args; i++)
	{
	  g_free (script->arg_labels[i]);
	  switch (script->arg_types[i])
	    {
	    case SF_IMAGE:
	    case SF_DRAWABLE:
	    case SF_LAYER:
	    case SF_CHANNEL:
	    case SF_COLOR:
	      break;

	    case SF_VALUE:
	    case SF_STRING:
	      g_free (script->arg_defaults[i].sfa_value);
	      g_free (script->arg_values[i].sfa_value);
	      break;

	    case SF_ADJUSTMENT:
	      break;

	    case SF_FILENAME:
	    case SF_DIRNAME:
	      g_free (script->arg_defaults[i].sfa_file.filename);
	      g_free (script->arg_values[i].sfa_file.filename);
	      break;

	    case SF_FONT:
	      g_free (script->arg_defaults[i].sfa_font);
	      g_free (script->arg_values[i].sfa_font);
	      break;

	    case SF_PALETTE:
	      g_free (script->arg_defaults[i].sfa_palette);
	      g_free (script->arg_values[i].sfa_palette);
	      break;

	    case SF_PATTERN:
	      g_free (script->arg_defaults[i].sfa_pattern);
	      g_free (script->arg_values[i].sfa_pattern);
	      break;

	    case SF_GRADIENT:
	      g_free (script->arg_defaults[i].sfa_gradient);
	      g_free (script->arg_values[i].sfa_gradient);
	      break;

	    case SF_BRUSH:
	      g_free (script->arg_defaults[i].sfa_brush.name);
	      g_free (script->arg_values[i].sfa_brush.name);
	      break;

	    case SF_OPTION:
	      g_slist_foreach (script->arg_defaults[i].sfa_option.list,
			       (GFunc) g_free, NULL);
              g_slist_free (script->arg_defaults[i].sfa_option.list);
	      break;

	    default:
	      break;
	    }
	}

      g_free (script->arg_labels);
      g_free (script->arg_defaults);
      g_free (script->arg_types);
      g_free (script->arg_values);

      g_free (script);
    }
}
