/* The GIMP -- an image manipulation program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#include <gtk/gtk.h>

#include "libgimpbase/gimpbase.h"
#include "libgimpwidgets/gimpwidgets.h"

#include "widgets-types.h"

#include "core/gimpstrokeoptions.h"

#include "gimppropwidgets.h"
#include "gimpstrokeeditor.h"

#include "gimp-intl.h"


#define SB_WIDTH 10

enum
{
  PROP_0,
  PROP_OPTIONS
};

static void      gimp_stroke_editor_class_init   (GimpStrokeEditorClass *klass);
static GObject * gimp_stroke_editor_constructor  (GType                  type,
                                                  guint                  n_params,
                                                  GObjectConstructParam *params);
static void      gimp_stroke_editor_set_property (GObject         *object,
                                                  guint            property_id,
                                                  const GValue    *value,
                                                  GParamSpec      *pspec);
static void      gimp_stroke_editor_get_property (GObject         *object,
                                                  guint            property_id,
                                                  GValue          *value,
                                                  GParamSpec      *pspec);
static void      gimp_stroke_editor_finalize     (GObject         *object);


static GtkVBoxClass *parent_class = NULL;


GType
gimp_stroke_editor_get_type (void)
{
  static GType view_type = 0;

  if (! view_type)
    {
      static const GTypeInfo view_info =
      {
        sizeof (GimpStrokeEditorClass),
        NULL,           /* base_init      */
        NULL,           /* base_finalize  */
        (GClassInitFunc) gimp_stroke_editor_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data     */
        sizeof (GimpStrokeEditor),
        0,              /* n_preallocs    */
        NULL            /* instance_init  */
      };

      view_type = g_type_register_static (GTK_TYPE_VBOX,
                                          "GimpStrokeEditor",
                                          &view_info, 0);
    }

  return view_type;
}

static void
gimp_stroke_editor_class_init (GimpStrokeEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->constructor  = gimp_stroke_editor_constructor;
  object_class->set_property = gimp_stroke_editor_set_property;
  object_class->get_property = gimp_stroke_editor_get_property;
  object_class->finalize     = gimp_stroke_editor_finalize;

  g_object_class_install_property (object_class, PROP_OPTIONS,
                                   g_param_spec_object ("options", NULL, NULL,
                                                        GIMP_TYPE_STROKE_OPTIONS,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
gimp_stroke_editor_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  GimpStrokeEditor *editor = GIMP_STROKE_EDITOR (object);

  switch (property_id)
    {
    case PROP_OPTIONS:
      editor->options = GIMP_STROKE_OPTIONS (g_value_dup_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gimp_stroke_editor_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  GimpStrokeEditor *editor = GIMP_STROKE_EDITOR (object);

  switch (property_id)
    {
    case PROP_OPTIONS:
      g_value_set_object (value, editor->options);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static GObject *
gimp_stroke_editor_constructor (GType                   type,
                                guint                   n_params,
                                GObjectConstructParam  *params)
{
  GimpStrokeEditor *editor;
  GtkWidget        *table;
  GtkWidget        *menu;
  GtkWidget        *box;
  GtkWidget        *spinbutton;
  GtkWidget        *button;
  GObject          *object;
  gint              digits;
  gint              row = 0;

  object = G_OBJECT_CLASS (parent_class)->constructor (type, n_params, params);

  editor = GIMP_STROKE_EDITOR (object);

  g_assert (editor->options != NULL);

  table = gtk_table_new (5, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_box_pack_start (GTK_BOX (editor), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  box = gimp_prop_enum_radio_frame_new (G_OBJECT (editor->options), "style",
                                        _("Style"), 0, 0);
  gtk_table_attach (GTK_TABLE (table), box, 0, 3, row, row + 1,
                    GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_widget_show (box);

  row++;

  digits = gimp_unit_get_digits (editor->options->width_unit);
  spinbutton = gimp_prop_spin_button_new (G_OBJECT (editor->options), "width",
                                          1.0, 10.0, digits);
  gimp_table_attach_aligned (GTK_TABLE (table), 0, row,
                             _("Stroke _Width:"), 1.0, 0.5,
                             spinbutton, 1, FALSE);

  menu = gimp_prop_unit_menu_new (G_OBJECT (editor->options), "width-unit",
                                  "%a");
  g_object_set_data (G_OBJECT (menu), "set_digits", spinbutton);
  gtk_table_attach (GTK_TABLE (table), menu, 2, 3, row, row + 1,
                    GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_widget_show (menu);
  row++;

  box = gimp_prop_enum_stock_box_new (G_OBJECT (editor->options), "cap-style",
                                      "gimp-cap", 0, 0);
  gimp_table_attach_aligned (GTK_TABLE (table), 0, row++,
                             _("_Cap Style:"), 1.0, 0.5,
                             box, 2, TRUE);

  box = gimp_prop_enum_stock_box_new (G_OBJECT (editor->options), "join-style",
                                      "gimp-join", 0, 0);
  gimp_table_attach_aligned (GTK_TABLE (table), 0, row++,
                             _("_Join Style:"), 1.0, 0.5,
                             box, 2, TRUE);

  gimp_prop_scale_entry_new (G_OBJECT (editor->options), "miter",
                             GTK_TABLE (table), 0, row++,
                             _("_Miter Limit:"),
                             1.0, 1.0, 1,
                             FALSE, 0.0, 0.0);

  button = gimp_prop_check_button_new (G_OBJECT (editor->options), "antialias",
                                       _("_Antialiasing"));
  gtk_table_attach (GTK_TABLE (table), button, 0, 2, row, row + 1,
                    GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_widget_show (button);
  row++;

  return object;
}

static void
gimp_stroke_editor_finalize (GObject *object)
{
  GimpStrokeEditor *editor = GIMP_STROKE_EDITOR (object);

  if (editor->options)
    {
      g_object_unref (editor->options);
      editor->options = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

GtkWidget *
gimp_stroke_editor_new (GimpStrokeOptions *options)
{
  g_return_val_if_fail (GIMP_IS_STROKE_OPTIONS (options), NULL);

  return GTK_WIDGET (g_object_new (GIMP_TYPE_STROKE_EDITOR,
                                   "options", options,
                                   NULL));
}
