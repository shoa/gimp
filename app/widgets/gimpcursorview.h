/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpcursorview.h
 * Copyright (C) 2005 Michael Natterer <mitch@gimp.org>
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

#ifndef __GIMP_CURSOR_VIEW_H__
#define __GIMP_CURSOR_VIEW_H__


#include "gimpeditor.h"


#define GIMP_TYPE_CURSOR_VIEW            (gimp_cursor_view_get_type ())
#define GIMP_CURSOR_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_CURSOR_VIEW, GimpCursorView))
#define GIMP_CURSOR_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_CURSOR_VIEW, GimpCursorViewClass))
#define GIMP_IS_CURSOR_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIMP_TYPE_CURSOR_VIEW))
#define GIMP_IS_CURSOR_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_CURSOR_VIEW))
#define GIMP_CURSOR_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GIMP_TYPE_CURSOR_VIEW, GimpCursorViewClass))


typedef struct _GimpCursorViewClass GimpCursorViewClass;

struct _GimpCursorView
{
  GimpEditor   parent_instance;

  GtkWidget   *coord_hbox;
  GtkWidget   *color_hbox;

  GtkWidget   *pixel_x_label;
  GtkWidget   *pixel_y_label;
  GtkWidget   *unit_x_label;
  GtkWidget   *unit_y_label;
  GtkWidget   *color_frame_1;
  GtkWidget   *color_frame_2;

  gboolean     sample_merged;
};

struct _GimpCursorViewClass
{
  GimpEditorClass  parent_class;
};


GType       gimp_cursor_view_get_type          (void) G_GNUC_CONST;

GtkWidget * gimp_cursor_view_new               (GimpMenuFactory *menu_factory);

void        gimp_cursor_view_set_sample_merged (GimpCursorView  *view,
                                                gboolean         sample_merged);
gboolean    gimp_cursor_view_get_sample_merged (GimpCursorView  *view);

void        gimp_cursor_view_update_cursor     (GimpCursorView  *view,
                                                GimpImage       *image,
                                                GimpUnit         unit,
                                                gdouble          x,
                                                gdouble          y);

#endif /* __GIMP_CURSOR_VIEW_H__ */
