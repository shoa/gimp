/* The GIMP -- an image manipulation program
 * Copyright (C) 1995, 1996, 1997 Spencer Kimball and Peter Mattis
 * Copyright (C) 1997 Josh MacDonald
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

#include <gtk/gtk.h>

#include "libgimpwidgets/gimpwidgets.h"

#include "dialogs-types.h"

#include "core/gimp.h"
#include "core/gimpimage.h"
#include "core/gimpprogress.h"

#include "plug-in/plug-in-proc-def.h"

#include "file/file-save.h"
#include "file/file-utils.h"

#include "widgets/gimpactiongroup.h"
#include "widgets/gimpfiledialog.h"
#include "widgets/gimphelp-ids.h"
#include "widgets/gimpmessagebox.h"
#include "widgets/gimpmessagedialog.h"

#include "file-save-dialog.h"

#include "gimp-intl.h"


/*  local function prototypes  */

static void       file_save_dialog_response      (GtkWidget      *save_dialog,
                                                  gint            response_id,
                                                  Gimp           *gimp);
static gboolean   file_save_dialog_check_uri     (GtkWidget      *save_dialog,
                                                  Gimp           *gimp,
                                                  gchar         **ret_uri,
                                                  gchar         **ret_basename,
                                                  PlugInProcDef **ret_save_proc);
static gboolean   file_save_dialog_use_extension (GtkWidget      *save_dialog,
                                                  const gchar    *uri);
static gboolean   file_save_dialog_save_image    (GtkWidget      *save_dialog,
                                                  GimpImage      *gimage,
                                                  const gchar    *uri,
                                                  PlugInProcDef  *save_proc,
                                                  gboolean        save_a_copy);


/*  public functions  */

GtkWidget *
file_save_dialog_new (Gimp *gimp)
{
  GtkWidget   *dialog;
  const gchar *uri;

  g_return_val_if_fail (GIMP_IS_GIMP (gimp), NULL);

  dialog = gimp_file_dialog_new (gimp,
                                 GTK_FILE_CHOOSER_ACTION_SAVE,
                                 _("Save Image"), "gimp-file-save",
                                 GTK_STOCK_SAVE,
                                 GIMP_HELP_FILE_SAVE);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  uri = g_object_get_data (G_OBJECT (gimp), "gimp-file-save-last-uri");

  if (uri)
    {
      gchar *folder_uri = g_path_get_dirname (uri);

#ifdef __GNUC__
#warning: FIXME: should use set_uri() but idle stuff in the file chooser seems to override set_current_name() when called immediately after set_uri()
#endif

      if (folder_uri)
        {
          gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (dialog),
                                                   folder_uri);
          g_free (folder_uri);
        }

      gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "");
    }

  g_signal_connect (dialog, "response",
                    G_CALLBACK (file_save_dialog_response),
                    gimp);

  return dialog;
}


/*  private functions  */

static void
file_save_dialog_response (GtkWidget *save_dialog,
                           gint       response_id,
                           Gimp      *gimp)
{
  GimpFileDialog *dialog = GIMP_FILE_DIALOG (save_dialog);
  gchar          *uri;
  gchar          *basename;
  PlugInProcDef  *save_proc;

  if (response_id != GTK_RESPONSE_OK)
    {
      if (! dialog->busy)
        gtk_widget_hide (save_dialog);

      return;
    }

  gimp_file_dialog_set_sensitive (dialog, FALSE);

  if (file_save_dialog_check_uri (save_dialog, gimp,
                                  &uri, &basename, &save_proc))
    {
      if (file_save_dialog_save_image (save_dialog,
                                       dialog->gimage,
                                       uri,
                                       save_proc,
                                       dialog->save_a_copy))
        {
          gtk_widget_hide (save_dialog);
        }

      g_free (uri);
      g_free (basename);
    }

  gimp_file_dialog_set_sensitive (dialog, TRUE);
}

static gboolean
file_save_dialog_check_uri (GtkWidget      *save_dialog,
                            Gimp           *gimp,
                            gchar         **ret_uri,
                            gchar         **ret_basename,
                            PlugInProcDef **ret_save_proc)
{
  GimpFileDialog *dialog = GIMP_FILE_DIALOG (save_dialog);
  gchar          *uri;
  gchar          *basename;
  PlugInProcDef  *save_proc;
  PlugInProcDef  *uri_proc;
  PlugInProcDef  *basename_proc;

  uri = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (save_dialog));

  if (! (uri && strlen (uri)))
    return FALSE;

  basename = g_path_get_basename (uri);

  save_proc     = dialog->file_proc;
  uri_proc      = file_utils_find_proc (gimp->save_procs, uri);
  basename_proc = file_utils_find_proc (gimp->save_procs, basename);

  g_print ("\n\n%s: URI = %s\n",
           G_STRFUNC, uri);
  g_print ("%s: basename = %s\n",
           G_STRFUNC, basename);
  g_print ("%s: selected save_proc: %s\n",
           G_STRFUNC, save_proc && save_proc->menu_label ? save_proc->menu_label : "NULL");
  g_print ("%s: URI save_proc: %s\n",
           G_STRFUNC, uri_proc ? uri_proc->menu_label : "NULL");
  g_print ("%s: basename save_proc: %s\n\n",
           G_STRFUNC, basename_proc && basename_proc->menu_label ? basename_proc->menu_label : "NULL");


  /*  first check if the user entered an extension at all  */
  if (! basename_proc)
    {
      g_print ("%s: basename has no valid extension\n",
               G_STRFUNC);

      if (! strchr (basename, '.'))
        {
          const gchar *ext = NULL;

          g_print ("%s: basename has no '.', trying to add extension\n",
                   G_STRFUNC);

          if (! save_proc)
            {
              ext = "xcf";
            }
          else if (save_proc->extensions_list)
            {
              ext = save_proc->extensions_list->data;
            }

          if (ext)
            {
              gchar *ext_uri      = g_strconcat (uri,      ".", ext, NULL);
              gchar *ext_basename = g_strconcat (basename, ".", ext, NULL);
              gchar *utf8;

              g_print ("%s: appending .%s to basename\n",
                       G_STRFUNC, ext);

              g_free (uri);
              g_free (basename);

              uri      = ext_uri;
              basename = ext_basename;

              uri_proc      = file_utils_find_proc (gimp->save_procs, uri);
              basename_proc = file_utils_find_proc (gimp->save_procs, basename);

              utf8 = g_filename_to_utf8 (basename, -1, NULL, NULL, NULL);
              gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (save_dialog),
                                                 utf8);
              g_free (utf8);
            }
          else
            {
              g_print ("%s: save_proc has no extensions, continuing without\n",
                       G_STRFUNC);

              /*  there may be file formats with no extension at all, use
               *  the selected proc in this case.
               */
              basename_proc = save_proc;

              if (! uri_proc)
                uri_proc = basename_proc;
            }

          if (! basename_proc)
            {
              g_print ("%s: unable to figure save_proc, bailing out\n",
                       G_STRFUNC);

              g_message (_("The given filename does not have any known "
                           "file extension. Please enter a known file "
                           "extension or select a file format from the "
                           "file format list."));
              g_free (uri);
              g_free (basename);
              return FALSE;
            }
        }
      else if (save_proc && ! save_proc->extensions_list)
        {
          g_print ("%s: basename has '.', but save_proc has no extensions, "
                   "accepting random extension\n",
                   G_STRFUNC);

          /*  accept any random extension if the file format has
           *  no extensions at all
           */
          basename_proc = save_proc;

          if (! uri_proc)
            uri_proc = basename_proc;
        }
    }

  /*  then check if the selected format matches the entered extension  */
  if (! save_proc)
    {
      g_print ("%s: no save_proc was selected from the list\n",
               G_STRFUNC);

      if (! basename_proc)
        {
          g_print ("%s: basename had no useful extension, bailing out\n",
                   G_STRFUNC);

          g_message (_("The given filename does not have any known "
                       "file extension. Please enter a known file "
                       "extension or select a file format from the "
                       "file format list."));
          g_free (uri);
          g_free (basename);
          return FALSE;
        }

      g_print ("%s: use URI's proc '%s' so indirect saving works\n",
               G_STRFUNC, uri_proc->menu_label ? uri_proc->menu_label : "?");

      /*  use the URI's proc if no save proc was selected  */
      save_proc = uri_proc;
    }
  else
    {
      g_print ("%s: save_proc '%s' was selected from the list\n",
               G_STRFUNC, save_proc->menu_label);

      if (save_proc != basename_proc)
        {
          g_print ("%s: however the basename's proc is '%s'\n",
                   G_STRFUNC,
                   basename_proc ? basename_proc->menu_label : "NULL");

          if (uri_proc != basename_proc)
            {
              g_print ("%s: that's impossible for remote URIs, bailing out\n",
                       G_STRFUNC);

              /*  remote URI  */

              g_message (_("Saving remote files needs to determine the "
                           "file format from the file extension. Please "
                           "enter a file extension that matches the selected "
                           "file format or enter no file extension at all."));
              g_free (uri);
              g_free (basename);
              return FALSE;
            }
          else
            {
              g_print ("%s: ask the user if she really wants that filename\n",
                       G_STRFUNC);

              /*  local URI  */

              if (! file_save_dialog_use_extension (save_dialog, uri))
                {
                  g_free (uri);
                  g_free (basename);
                  return FALSE;
                }
            }
        }
      else if (save_proc != uri_proc)
        {
          g_print ("%s: use URI's proc '%s' so indirect saving works\n",
                   G_STRFUNC, uri_proc->menu_label);

          /*  need to use the URI's proc for saving because e.g.
           *  the GIF plug-in can't save a GIF to sftp://
           */
          save_proc = uri_proc;
        }
    }

  if (! save_proc)
    {
      g_print ("%s: EEEEEEK\n", G_STRFUNC);

      g_message ("Yay! You found a bug. Please report this at "
                 "http://bugzilla.gnome.org/ and paste the console "
                 "output to the 'Additional Comments' field");

      return FALSE;
    }

  *ret_uri       = uri;
  *ret_basename  = basename;
  *ret_save_proc = save_proc;

  return TRUE;
}

static gboolean
file_save_dialog_use_extension (GtkWidget   *save_dialog,
                                const gchar *uri)
{
  GtkWidget *dialog;
  gboolean   use_name = FALSE;

  dialog = gimp_message_dialog_new (_("Extension Mismatch"),
                                    GIMP_STOCK_QUESTION,
                                    save_dialog, GTK_DIALOG_DESTROY_WITH_PARENT,
                                    gimp_standard_help_func, NULL,

                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    GTK_STOCK_SAVE,   GTK_RESPONSE_OK,

                                    NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  gimp_message_box_set_primary_text (GIMP_MESSAGE_DIALOG (dialog)->box,
                                     _("The given file extension does "
                                       "not match the chosen file type."));

  gimp_message_box_set_text (GIMP_MESSAGE_DIALOG (dialog)->box,
                             _("Do you want to save the image using this "
                               "name anyway?"));

  gtk_dialog_set_response_sensitive (GTK_DIALOG (save_dialog),
                                     GTK_RESPONSE_CANCEL, FALSE);

  g_object_ref (dialog);

  use_name = (gimp_dialog_run (GIMP_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);
  g_object_unref (dialog);

  gtk_dialog_set_response_sensitive (GTK_DIALOG (save_dialog),
                                     GTK_RESPONSE_CANCEL, TRUE);

  return use_name;
}

static gboolean
file_save_dialog_save_image (GtkWidget     *save_dialog,
                             GimpImage     *gimage,
                             const gchar   *uri,
                             PlugInProcDef *save_proc,
                             gboolean       save_a_copy)
{
  GimpPDBStatusType  status;
  GError            *error   = NULL;
  GList             *list;
  gboolean           success = TRUE;

  for (list = gimp_action_groups_from_name ("file");
       list;
       list = g_list_next (list))
    {
      gimp_action_group_set_action_sensitive (list->data, "file-quit", FALSE);
    }

  g_object_ref (gimage);

  status = file_save (gimage, gimp_get_user_context (gimage->gimp),
                      GIMP_PROGRESS (save_dialog),
                      uri, save_proc,
                      GIMP_RUN_INTERACTIVE, save_a_copy, &error);

  if (status == GIMP_PDB_SUCCESS)
    g_object_set_data_full (G_OBJECT (gimage->gimp), "gimp-file-save-last-uri",
                            g_strdup (uri), (GDestroyNotify) g_free);

  g_object_unref (gimage);

  if (status != GIMP_PDB_SUCCESS &&
      status != GIMP_PDB_CANCEL)
    {
      gchar *filename = file_utils_uri_display_name (uri);

      g_message (_("Saving '%s' failed:\n\n%s"),
                 filename, error->message);
      g_clear_error (&error);

      g_free (filename);

      success = FALSE;
    }

  for (list = gimp_action_groups_from_name ("file");
       list;
       list = g_list_next (list))
    {
      gimp_action_group_set_action_sensitive (list->data, "file-quit", TRUE);
    }

  return success;
}
