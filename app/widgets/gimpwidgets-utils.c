/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpui.c
 * Copyright (C) 1999 Michael Natterer <mitch@gimp.org>
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

#include "widgets-types.h"

#include "gimpwidgets-utils.h"

#include "libgimp/gimpintl.h"


extern gchar *prog_name;

static void  gimp_message_box_close_callback  (GtkWidget *widget,
					       gpointer   data);

/*
 *  Message Boxes...
 */

typedef struct _MessageBox MessageBox;

struct _MessageBox
{
  GtkWidget   *mbox;
  GtkWidget   *vbox;
  GtkWidget   *repeat_label;
  gchar       *message;
  gint         repeat_count;
  GtkCallback  callback;
  gpointer     data;
};

/*  the maximum number of concurrent dialog boxes */
#define MESSAGE_BOX_MAXIMUM  4

static GList *message_boxes = NULL;

void
gimp_message_box (const gchar *message,
		  GtkCallback  callback,
		  gpointer     data)
{
  MessageBox  *msg_box;
  GtkWidget   *mbox;
  GtkWidget   *hbox;
  GtkWidget   *vbox;
  GtkWidget   *image;
  GtkWidget   *label;
  GList       *list;
  const gchar *stock_id = GTK_STOCK_DIALOG_WARNING;

  if (!message)
    return;

  if (g_list_length (message_boxes) > MESSAGE_BOX_MAXIMUM)
    {
      g_printerr ("%s: %s\n", prog_name, message);
      return;
    }

  for (list = message_boxes; list; list = list->next)
    {
      msg_box = list->data;
      if (strcmp (msg_box->message, message) == 0)
	{
	  msg_box->repeat_count++;
	  if (msg_box->repeat_count > 1)
	    {
	      gchar *text = g_strdup_printf (_("Message repeated %d times."), 
					     msg_box->repeat_count);
	      gtk_label_set_text (GTK_LABEL (msg_box->repeat_label), text);
	      g_free (text);
	    }
	  else
	    {
              GtkWidget *label;

              label = gtk_label_new (_("Message repeated once."));
              gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
	      gtk_box_pack_end (GTK_BOX (msg_box->vbox), label,
                                FALSE, FALSE, 0);
	      gtk_widget_show (label);

	      msg_box->repeat_label = label;
	    }

          gtk_window_present (GTK_WINDOW (msg_box->mbox));
	  return;
	}
    }

  if (g_list_length (message_boxes) == MESSAGE_BOX_MAXIMUM)
    {
      g_printerr ("%s: %s\n", prog_name, message);
      message = _("WARNING:\n"
		  "Too many open message dialogs.\n"
		  "Messages are redirected to stderr.");
      stock_id = GTK_STOCK_DIALOG_WARNING;
    }
  
  msg_box = g_new0 (MessageBox, 1);

  mbox = gimp_dialog_new (_("GIMP Message"), "gimp-message",
			  NULL, NULL,
			  GTK_WIN_POS_MOUSE,
			  FALSE, FALSE, FALSE,

			  GTK_STOCK_OK, gimp_message_box_close_callback,
			  msg_box, NULL, NULL, TRUE, TRUE,

			  NULL);

  hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (mbox)->vbox), hbox);
  gtk_widget_show (hbox);

  image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
  gtk_widget_show (image);  

  vbox = gtk_vbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);  

  label = gtk_label_new (message);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  msg_box->mbox     = mbox;
  msg_box->vbox     = vbox;
  msg_box->message  = g_strdup (message);
  msg_box->callback = callback;
  msg_box->data     = data;

  message_boxes = g_list_append (message_boxes, msg_box);

  gtk_widget_show (mbox);
}

static void
gimp_message_box_close_callback (GtkWidget *widget,
				 gpointer   data)
{
  MessageBox *msg_box;

  msg_box = (MessageBox *) data;

  /*  If there is a valid callback, invoke it  */
  if (msg_box->callback)
    (* msg_box->callback) (widget, msg_box->data);

  /*  Destroy the box  */
  gtk_widget_destroy (msg_box->mbox);
  
  /* make this box available again */
  message_boxes = g_list_remove (message_boxes, msg_box);

  g_free (msg_box->message);
  g_free (msg_box);
}

void
gimp_menu_position (GtkMenu *menu,
		    gint    *x,
		    gint    *y,
		    guint   *button,
		    guint32 *activate_time)
{
  GdkEvent       *current_event;
  GtkRequisition  requisition;
  gint            pointer_x;
  gint            pointer_y;
  gint            screen_width;
  gint            screen_height;

  g_return_if_fail (GTK_IS_MENU (menu));
  g_return_if_fail (x != NULL);
  g_return_if_fail (y != NULL);
  g_return_if_fail (button != NULL);
  g_return_if_fail (activate_time != NULL);

  gdk_window_get_pointer (NULL, &pointer_x, &pointer_y, NULL);

  gtk_widget_size_request (GTK_WIDGET (menu), &requisition);

  screen_width  = gdk_screen_width ()  + 2;
  screen_height = gdk_screen_height () + 2;

  *x = CLAMP (pointer_x, 2, MAX (0, screen_width  - requisition.width));
  *y = CLAMP (pointer_y, 2, MAX (0, screen_height - requisition.height));

  *x += (pointer_x <= *x) ? 2 : -2;
  *y += (pointer_y <= *y) ? 2 : -2;

  *x = MAX (*x, 0);
  *y = MAX (*y, 0);

  current_event = gtk_get_current_event ();

  if (current_event && current_event->type == GDK_BUTTON_PRESS)
    {
      GdkEventButton *bevent;

      bevent = (GdkEventButton *) current_event;

      *button        = bevent->button;
      *activate_time = bevent->time;
    }
  else
    {
      *button        = 0;
      *activate_time = 0;
    }
}


typedef struct _GimpAccelContextData GimpAccelContextData;

struct _GimpAccelContextData
{
  GtkItemFactory          *item_factory;
  GimpGetAccelContextFunc  get_context_func;
  gpointer                 get_context_data;
};

static gboolean
gimp_window_accel_key_press (GtkWidget            *widget,
			     GdkEvent             *event,
			     GimpAccelContextData *context_data)
{
  gpointer accel_context;

  accel_context =
    context_data->get_context_func (context_data->get_context_data);

  g_object_set_data (G_OBJECT (context_data->item_factory),
                     "gimp-accel-context", accel_context);

  return FALSE;
}

static gboolean
gimp_window_accel_key_release (GtkWidget            *widget,
			       GdkEvent             *event,
			       GimpAccelContextData *context_data)
{
  g_object_set_data (G_OBJECT (context_data->item_factory),
                     "gimp-accel-context", NULL);

  return FALSE;
}

void
gimp_window_add_accel_group (GtkWindow               *window,
			     GtkItemFactory          *item_factory,
			     GimpGetAccelContextFunc  get_context_func,
			     gpointer                 get_context_data)
{
  if (get_context_func)
    {
      GimpAccelContextData *context_data;

      context_data = g_new0 (GimpAccelContextData, 1);

      context_data->item_factory     = item_factory;
      context_data->get_context_func = get_context_func;
      context_data->get_context_data = get_context_data;

      g_object_set_data_full (G_OBJECT (window), "gimp-accel-conext-data",
                              context_data,
                              (GDestroyNotify) g_free);

      g_signal_connect (G_OBJECT (window), "key_press_event",
                        G_CALLBACK (gimp_window_accel_key_press),
                        context_data);
      g_signal_connect (G_OBJECT (window), "key_release_event",
                        G_CALLBACK (gimp_window_accel_key_release),
                        context_data);
    }

  gtk_window_add_accel_group (window, item_factory->accel_group);
}

void
gimp_window_remove_accel_group (GtkWindow      *window,
				GtkItemFactory *item_factory)
{
  GimpAccelContextData *context_data;

  context_data = g_object_get_data (G_OBJECT (window),
                                    "gimp-accel-conext-data");

  if (context_data)
    {
      g_signal_handlers_disconnect_by_func (G_OBJECT (window),
                                            G_CALLBACK (gimp_window_accel_key_press),
                                            context_data);
      g_signal_handlers_disconnect_by_func (G_OBJECT (window),
                                            G_CALLBACK (gimp_window_accel_key_release),
                                            context_data);

      g_object_set_data (G_OBJECT (window), "gimp-accel-conext-data", NULL);
    }

  gtk_window_remove_accel_group (window, item_factory->accel_group);
}

gpointer
gimp_widget_get_callback_context (GtkWidget *widget)
{
  GtkItemFactory *ifactory;
  gpointer        popup_context;
  gpointer        accel_context = NULL;

  ifactory = gtk_item_factory_from_widget (widget);

  popup_context = gtk_item_factory_popup_data_from_widget (widget);

  if (ifactory)
    accel_context = g_object_get_data (G_OBJECT (ifactory), 
                                       "gimp-accel-context");

  if (popup_context)
    return popup_context;

  return accel_context;
}

void
gimp_table_attach_stock (GtkTable    *table,
                         gint         column,
                         gint         row,
                         const gchar *stock_id,
                         GtkWidget   *widget)
{
  GtkStockItem  item;
  GtkWidget    *label;
  GtkWidget    *image;

  g_return_if_fail (GTK_IS_TABLE (table));
  g_return_if_fail (stock_id != NULL);

  if (! gtk_stock_lookup (stock_id, &item))
    return;

  image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_BUTTON);
  gtk_table_attach (table, image, column, column + 1, row, row + 1,
                    GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_widget_show (image);
  
  label = gtk_label_new_with_mnemonic (item.label);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (table, label, column + 1, column + 2, row, row + 1,
                    GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
  gtk_widget_show (label);
 
  if (!widget)
    return;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  gtk_table_attach (table, widget, column + 2, column + 3, row, row + 1,
                    GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_widget_show (widget);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);
}
