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

#include <gtk/gtk.h>

#include "libgimpwidgets/gimpwidgets.h"

#include "tools-types.h"

#include "config/gimpconfig-params.h"

#include "core/gimp.h"
#include "core/gimpdatafactory.h"
#include "core/gimptoolinfo.h"

#include "widgets/gimpenumcombobox.h"
#include "widgets/gimppropwidgets.h"
#include "widgets/gimpwidgets-utils.h"

#include "gimpblendoptions.h"
#include "gimppaintoptions-gui.h"

#include "gimp-intl.h"


enum
{
  PROP_0,
  PROP_OFFSET,
  PROP_GRADIENT_TYPE,
  PROP_GRADIENT_REPEAT,  /*  overrides a GimpPaintOptions property  */
  PROP_SUPERSAMPLE,
  PROP_SUPERSAMPLE_DEPTH,
  PROP_SUPERSAMPLE_THRESHOLD,
  PROP_DITHER
};


static void   gimp_blend_options_class_init      (GimpBlendOptionsClass *klass);

static void   gimp_blend_options_set_property    (GObject          *object,
                                                  guint             property_id,
                                                  const GValue     *value,
                                                  GParamSpec       *pspec);
static void   gimp_blend_options_get_property    (GObject          *object,
                                                  guint             property_id,
                                                  GValue           *value,
                                                  GParamSpec       *pspec);

static void   blend_options_gradient_type_notify (GimpBlendOptions *options,
                                                  GParamSpec       *pspec,
                                                  GtkWidget        *repeat_combo);


static GimpPaintOptionsClass *parent_class = NULL;


GType
gimp_blend_options_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      static const GTypeInfo info =
      {
        sizeof (GimpBlendOptionsClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) gimp_blend_options_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data     */
        sizeof (GimpBlendOptions),
        0,              /* n_preallocs    */
        (GInstanceInitFunc) NULL
      };

      type = g_type_register_static (GIMP_TYPE_PAINT_OPTIONS,
                                     "GimpBlendOptions",
                                     &info, 0);
    }

  return type;
}

static void
gimp_blend_options_class_init (GimpBlendOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->set_property = gimp_blend_options_set_property;
  object_class->get_property = gimp_blend_options_get_property;

  GIMP_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_OFFSET,
                                   "offset", NULL,
                                   0.0, 100.0, 0.0,
                                   0);
  GIMP_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_GRADIENT_TYPE,
                                 "gradient-type", NULL,
                                 GIMP_TYPE_GRADIENT_TYPE,
                                 GIMP_GRADIENT_LINEAR,
                                 0);
  GIMP_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_GRADIENT_REPEAT,
                                 "gradient-repeat", NULL,
                                 GIMP_TYPE_REPEAT_MODE,
                                 GIMP_REPEAT_NONE,
                                 0);

  GIMP_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SUPERSAMPLE,
                                    "supersample", NULL,
                                    FALSE,
                                    0);
  GIMP_CONFIG_INSTALL_PROP_INT (object_class, PROP_SUPERSAMPLE_DEPTH,
                                "supersample-depth", NULL,
                                0, 6, 3,
                                0);
  GIMP_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_SUPERSAMPLE_THRESHOLD,
                                   "supersample-threshold", NULL,
                                   0.0, 4.0, 0.2,
                                   0);

  GIMP_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_DITHER,
                                    "dither", NULL,
                                    TRUE,
                                    0);
}

static void
gimp_blend_options_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  GimpBlendOptions *options = GIMP_BLEND_OPTIONS (object);

  switch (property_id)
    {
    case PROP_OFFSET:
      options->offset = g_value_get_double (value);
      break;
    case PROP_GRADIENT_TYPE:
      options->gradient_type = g_value_get_enum (value);
      break;
    case PROP_GRADIENT_REPEAT:
      GIMP_PAINT_OPTIONS (options)->gradient_options->gradient_repeat =
        g_value_get_enum (value);
      break;

    case PROP_SUPERSAMPLE:
      options->supersample = g_value_get_boolean (value);
      break;
    case PROP_SUPERSAMPLE_DEPTH:
      options->supersample_depth = g_value_get_int (value);
      break;
    case PROP_SUPERSAMPLE_THRESHOLD:
      options->supersample_threshold = g_value_get_double (value);
      break;

    case PROP_DITHER:
      options->dither = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gimp_blend_options_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  GimpBlendOptions *options = GIMP_BLEND_OPTIONS (object);

  switch (property_id)
    {
    case PROP_OFFSET:
      g_value_set_double (value, options->offset);
      break;
    case PROP_GRADIENT_TYPE:
      g_value_set_enum (value, options->gradient_type);
      break;
    case PROP_GRADIENT_REPEAT:
      g_value_set_enum (value,
                        GIMP_PAINT_OPTIONS (options)->gradient_options->gradient_repeat);
      break;

    case PROP_SUPERSAMPLE:
      g_value_set_boolean (value, options->supersample);
      break;
    case PROP_SUPERSAMPLE_DEPTH:
      g_value_set_int (value, options->supersample_depth);
      break;
    case PROP_SUPERSAMPLE_THRESHOLD:
      g_value_set_double (value, options->supersample_threshold);
      break;

    case PROP_DITHER:
      g_value_set_boolean (value, options->dither);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
gimp_blend_options_gui (GimpToolOptions *tool_options)
{
  GObject   *config = G_OBJECT (tool_options);
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *frame;
  GtkWidget *combo;
  GtkWidget *button;

  vbox = gimp_paint_options_gui (tool_options);

  table = g_object_get_data (G_OBJECT (vbox), GIMP_PAINT_OPTIONS_TABLE_KEY);

  /*  the offset scale  */
  gimp_prop_scale_entry_new (config, "offset",
                             GTK_TABLE (table), 0, 3,
                             _("Offset:"),
                             1.0, 10.0, 1,
                             FALSE, 0.0, 0.0);

  /*  the gradient type menu  */
  combo = gimp_prop_enum_combo_box_new (config, "gradient-type", 0, 0);
  gimp_enum_combo_box_set_stock_prefix (GIMP_ENUM_COMBO_BOX (combo),
                                        "gimp-gradient");
  gimp_table_attach_aligned (GTK_TABLE (table), 0, 4,
                             _("Shape:"), 0.0, 0.5,
                             combo, 2, FALSE);

  /*  the repeat option  */
  combo = gimp_prop_enum_combo_box_new (config, "gradient-repeat", 0, 0);
  gimp_table_attach_aligned (GTK_TABLE (table), 0, 5,
                             _("Repeat:"), 0.0, 0.5,
                             combo, 2, FALSE);

  g_signal_connect (config, "notify::gradient-type",
                    G_CALLBACK (blend_options_gradient_type_notify),
                    combo);

  button = gimp_prop_check_button_new (config, "dither",
                                       _("Dithering"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  /*  supersampling options  */
  frame = gimp_frame_new (NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  button = gimp_prop_check_button_new (config, "supersample",
                                       _("Adaptive supersampling"));
  gtk_frame_set_label_widget (GTK_FRAME (frame), button);
  gtk_widget_show (button);

  table = gtk_table_new (2, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 1);
  gtk_container_add (GTK_CONTAINER (frame), table);
  if (GIMP_BLEND_OPTIONS (config)->supersample)
    gtk_widget_show (table);

  g_signal_connect_object (button, "toggled",
                           G_CALLBACK (gimp_toggle_button_set_visible),
                           table, 0);

  /*  max depth scale  */
  gimp_prop_scale_entry_new (config, "supersample-depth",
                             GTK_TABLE (table), 0, 0,
                             _("Max Depth:"),
                             1.0, 1.0, 0,
                             FALSE, 0.0, 0.0);

  /*  threshold scale  */
  gimp_prop_scale_entry_new (config, "supersample-threshold",
                             GTK_TABLE (table), 0, 1,
                             _("Threshold:"),
                             0.01, 0.1, 2,
                             FALSE, 0.0, 0.0);

  return vbox;
}

static void
blend_options_gradient_type_notify (GimpBlendOptions *options,
                                    GParamSpec       *pspec,
                                    GtkWidget        *repeat_combo)
{
  gtk_widget_set_sensitive (repeat_combo, options->gradient_type < 6);
}
