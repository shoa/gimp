/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * gimpfileentry.c
 * Copyright (C) 1999-2004 Michael Natterer <mitch@gimp.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <glib.h>         /* Needed here by Win32 gcc compilation */

#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>

#include <gtk/gtk.h>

#include "gimpwidgetstypes.h"

#include "gimpfileentry.h"

#include "libgimp/libgimp-intl.h"


enum
{
  FILENAME_CHANGED,
  LAST_SIGNAL
};


static void   gimp_file_entry_class_init      (GimpFileEntryClass *klass);
static void   gimp_file_entry_init            (GimpFileEntry      *entry);

static void   gimp_file_entry_destroy         (GtkObject          *object);

static void   gimp_file_entry_entry_activate  (GtkWidget          *widget,
                                               GimpFileEntry      *entry);
static gint   gimp_file_entry_entry_focus_out (GtkWidget          *widget,
                                               GdkEvent           *event,
                                               GimpFileEntry      *entry);
static void   gimp_file_entry_browse_clicked  (GtkWidget          *widget,
                                               GimpFileEntry      *entry);
static void   gimp_file_entry_check_filename  (GimpFileEntry      *entry);


static guint gimp_file_entry_signals[LAST_SIGNAL] = { 0 };

static GtkHBoxClass *parent_class = NULL;


GType
gimp_file_entry_get_type (void)
{
  static GType entry_type = 0;

  if (! entry_type)
    {
      static const GTypeInfo entry_info =
      {
        sizeof (GimpFileEntryClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) gimp_file_entry_class_init,
        NULL,		/* class_finalize */
        NULL,		/* class_data     */
        sizeof (GimpFileEntry),
        0,              /* n_preallocs    */
        (GInstanceInitFunc) gimp_file_entry_init,
      };

      entry_type = g_type_register_static (GTK_TYPE_HBOX,
                                           "GimpFileEntry",
                                           &entry_info, 0);
    }

  return entry_type;
}

static void
gimp_file_entry_class_init (GimpFileEntryClass *klass)
{
  GtkObjectClass *object_class = GTK_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  /**
   * GimpFileEntry::filename-changed:
   *
   * This signal is emitted whenever the user changes the filename.
   **/
  gimp_file_entry_signals[FILENAME_CHANGED] =
    g_signal_new ("filename_changed",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (GimpFileEntryClass, filename_changed),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  object_class->destroy   = gimp_file_entry_destroy;

  klass->filename_changed = NULL;
}

static void
gimp_file_entry_init (GimpFileEntry *entry)
{
  entry->title       = NULL;
  entry->file_dialog = NULL;
  entry->check_valid = FALSE;
  entry->file_exists = NULL;

  gtk_box_set_spacing (GTK_BOX (entry), 4);
  gtk_box_set_homogeneous (GTK_BOX (entry), FALSE);

  entry->browse_button = gtk_button_new_with_label (" ... ");
  gtk_box_pack_end (GTK_BOX (entry), entry->browse_button, FALSE, FALSE, 0);
  gtk_widget_show (entry->browse_button);

  g_signal_connect (entry->browse_button, "clicked",
                    G_CALLBACK (gimp_file_entry_browse_clicked),
                    entry);

  entry->entry = gtk_entry_new ();
  gtk_box_pack_end (GTK_BOX (entry), entry->entry, TRUE, TRUE, 0);
  gtk_widget_show (entry->entry);

  g_signal_connect (entry->entry, "activate",
                    G_CALLBACK (gimp_file_entry_entry_activate),
                    entry);
  g_signal_connect (entry->entry, "focus_out_event",
                    G_CALLBACK (gimp_file_entry_entry_focus_out),
                    entry);
}

static void
gimp_file_entry_destroy (GtkObject *object)
{
  GimpFileEntry *entry = GIMP_FILE_ENTRY (object);

  if (entry->file_dialog)
    {
      gtk_widget_destroy (entry->file_dialog);
      entry->file_dialog = NULL;
    }

  if (entry->title)
    {
      g_free (entry->title);
      entry->title = NULL;
    }

  GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

/**
 * gimp_file_entry_new:
 * @title:       The title of the #GtkFileEntry dialog.
 * @filename:    The initial filename.
 * @dir_only:    %TRUE if the file entry should accept directories only.
 * @check_valid: %TRUE if the widget should check if the entered file
 *               really exists.
 *
 * Creates a new #GimpFileEntry widget.
 *
 * Returns: A pointer to the new #GimpFileEntry widget.
 **/
GtkWidget *
gimp_file_entry_new (const gchar *title,
                     const gchar *filename,
                     gboolean     dir_only,
                     gboolean     check_valid)
{
  GimpFileEntry *entry;

  entry = g_object_new (GIMP_TYPE_FILE_ENTRY, NULL);

  entry->title       = g_strdup (title);
  entry->dir_only    = dir_only;
  entry->check_valid = check_valid;

  if (check_valid)
    {
      entry->file_exists = gtk_image_new_from_stock (GTK_STOCK_NO,
                                                     GTK_ICON_SIZE_BUTTON);
      gtk_box_pack_start (GTK_BOX (entry), entry->file_exists, FALSE, FALSE, 0);
      gtk_widget_show (entry->file_exists);
    }

  gimp_file_entry_set_filename (entry, filename);

  return GTK_WIDGET (entry);
}

/**
 * gimp_file_entry_get_filename:
 * @entry: The file entry you want to know the filename from.
 *
 * Note that you have to g_free() the returned string.
 *
 * Returns: The file or directory the user has entered.
 **/
gchar *
gimp_file_entry_get_filename (GimpFileEntry *entry)
{
  gchar *utf8;
  gchar *filename;

  g_return_val_if_fail (GIMP_IS_FILE_ENTRY (entry), NULL);

  utf8 = gtk_editable_get_chars (GTK_EDITABLE (entry->entry), 0, -1);

  filename = g_filename_from_utf8 (utf8, -1, NULL, NULL, NULL);

  g_free (utf8);

  return filename;
}

/**
 * gimp_file_entry_set_filename:
 * @entry:    The file entry you want to set the filename for.
 * @filename: The new filename.
 *
 * If you specified @check_valid as %TRUE in gimp_file_entry_new()
 * the #GimpFileEntry will immediately check the validity of the file
 * name.
 **/
void
gimp_file_entry_set_filename (GimpFileEntry *entry,
                              const gchar   *filename)
{
  gchar *utf8;

  g_return_if_fail (GIMP_IS_FILE_ENTRY (entry));

  if (filename)
    utf8 = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);
  else
    utf8 = g_strdup ("");

  gtk_entry_set_text (GTK_ENTRY (entry->entry), utf8);
  g_free (utf8);

  /*  update everything
   */
  gimp_file_entry_entry_activate (entry->entry, entry);
}

static void
gimp_file_entry_entry_activate (GtkWidget     *widget,
                                GimpFileEntry *entry)
{
  gchar *utf8;
  gchar *filename;
  gint   len;

  /*  filenames still need more sanity checking
   *  (erase double G_DIR_SEPARATORS, ...)
   */
  utf8 = gtk_editable_get_chars (GTK_EDITABLE (widget), 0, -1);
  utf8 = g_strstrip (utf8);

  while (((len = strlen (utf8)) > 1) &&
	 (utf8[len - 1] == G_DIR_SEPARATOR))
    utf8[len - 1] = '\0';

  filename = g_filename_from_utf8 (utf8, -1, NULL, NULL, NULL);

  g_signal_handlers_block_by_func (entry->entry,
                                   gimp_file_entry_entry_activate,
                                   entry);
  gtk_entry_set_text (GTK_ENTRY (entry->entry), utf8);
  g_signal_handlers_unblock_by_func (entry->entry,
                                     gimp_file_entry_entry_activate,
                                     entry);

  if (entry->file_dialog)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION (entry->file_dialog),
				     filename);

  g_free (filename);
  g_free (utf8);

  gimp_file_entry_check_filename (entry);

  gtk_editable_set_position (GTK_EDITABLE (entry->entry), -1);

  g_signal_emit (entry, gimp_file_entry_signals[FILENAME_CHANGED], 0);
}

static gboolean
gimp_file_entry_entry_focus_out (GtkWidget     *widget,
                                 GdkEvent      *event,
                                 GimpFileEntry *entry)
{
  gimp_file_entry_entry_activate (widget, entry);

  return FALSE;
}

/*  local callback of gimp_file_entry_browse_clicked()  */
static void
gimp_file_entry_filesel_response (GtkWidget     *dialog,
                                  gint           response_id,
                                  GimpFileEntry *entry)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      const gchar *filename;

      filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (dialog));
      gimp_file_entry_set_filename (entry, filename);
    }

  gtk_widget_hide (dialog);
}

static void
gimp_file_entry_browse_clicked (GtkWidget     *widget,
                                GimpFileEntry *entry)
{
  gchar *utf8;
  gchar *filename;

  utf8 = gtk_editable_get_chars (GTK_EDITABLE (entry->entry), 0, -1);
  filename = g_filename_from_utf8 (utf8, -1, NULL, NULL, NULL);
  g_free (utf8);

  if (! entry->file_dialog)
    {
      GtkFileSelection *filesel;

      if (entry->dir_only)
	{
          entry->file_dialog = gtk_file_selection_new (entry->title ?
                                                       entry->title :
                                                       _("Select Folder"));

	  /*  hiding these widgets uses internal gtk+ knowledge, but it's
	   *  easier than creating my own directory browser -- michael
	   */
	  gtk_widget_hide
	    (GTK_FILE_SELECTION (entry->file_dialog)->fileop_del_file);
	  gtk_widget_hide
	    (GTK_FILE_SELECTION (entry->file_dialog)->file_list->parent);
	}
      else
        {
          entry->file_dialog = gtk_file_selection_new (entry->title ?
                                                       entry->title :
                                                       _("Select File"));
        }

      filesel = GTK_FILE_SELECTION (entry->file_dialog);

      gtk_window_set_position (GTK_WINDOW (entry->file_dialog),
			       GTK_WIN_POS_MOUSE);
      gtk_window_set_role (GTK_WINDOW (entry->file_dialog),
                           "gimp-file-entry-file-dialog");

      gtk_container_set_border_width (GTK_CONTAINER (filesel), 6);
      gtk_container_set_border_width (GTK_CONTAINER (filesel->button_area), 4);

      g_signal_connect (filesel, "response",
                        G_CALLBACK (gimp_file_entry_filesel_response),
                        entry);
      g_signal_connect (filesel, "delete_event",
                        G_CALLBACK (gtk_true),
                        NULL);

      g_signal_connect_swapped (entry, "unmap",
                                G_CALLBACK (gtk_widget_hide),
                                filesel);
    }

  gtk_file_selection_set_filename (GTK_FILE_SELECTION (entry->file_dialog),
                                   filename);

  gtk_window_set_screen (GTK_WINDOW (entry->file_dialog),
                         gtk_widget_get_screen (widget));

  gtk_window_present (GTK_WINDOW (entry->file_dialog));
}

static void
gimp_file_entry_check_filename (GimpFileEntry *entry)
{
  gchar    *utf8;
  gchar    *filename;
  gboolean  exists;

  if (! entry->check_valid || ! entry->file_exists)
    return;

  utf8 = gtk_editable_get_chars (GTK_EDITABLE (entry->entry), 0, -1);
  filename = g_filename_from_utf8 (utf8, -1, NULL, NULL, NULL);
  g_free (utf8);

  if (entry->dir_only)
    exists = g_file_test (filename, G_FILE_TEST_IS_DIR);
  else
    exists = g_file_test (filename, G_FILE_TEST_IS_REGULAR);

  g_free (filename);

  gtk_image_set_from_stock (GTK_IMAGE (entry->file_exists),
                            exists ? GTK_STOCK_YES : GTK_STOCK_NO,
                            GTK_ICON_SIZE_BUTTON);
}
