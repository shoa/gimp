/* LIBGIMP - The GIMP Library 
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball                
 *
 * gimpcolornotebook.h
 * Copyright (C) 2002 Michael Natterer <mitch@gimp.org>
 *
 * based on color_notebook module
 * Copyright (C) 1998 Austin Donnelly <austin@greenend.org.uk>
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

#ifndef __GIMP_COLOR_NOTEBOOK_H__
#define __GIMP_COLOR_NOTEBOOK_H__

#include <libgimpwidgets/gimpcolorselector.h>

G_BEGIN_DECLS


#define GIMP_TYPE_COLOR_NOTEBOOK            (gimp_color_notebook_get_type ())
#define GIMP_COLOR_NOTEBOOK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_COLOR_NOTEBOOK, GimpColorNotebook))
#define GIMP_COLOR_NOTEBOOK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_COLOR_NOTEBOOK, GimpColorNotebookClass))
#define GIMP_IS_COLOR_NOTEBOOK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIMP_TYPE_COLOR_NOTEBOOK))
#define GIMP_IS_COLOR_NOTEBOOK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_COLOR_NOTEBOOK))
#define GIMP_COLOR_NOTEBOOK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GIMP_TYPE_COLOR_NOTEBOOK, GimpColorNotebookClass))


typedef struct _GimpColorNotebookClass GimpColorNotebookClass;

struct _GimpColorNotebook
{
  GimpColorSelector  parent_instance;

  GtkWidget         *notebook;

  GList             *selectors;
  GimpColorSelector *cur_page;
};

struct _GimpColorNotebookClass
{
  GimpColorSelectorClass  parent_class;
};


GType       gimp_color_notebook_get_type     (void) G_GNUC_CONST;

GtkWidget * gimp_color_notebook_set_has_page (GimpColorNotebook *notebook,
                                              GType              page_type,
                                              gboolean           has_page);


G_END_DECLS

#endif /* __GIMP_COLOR_NOTEBOOK_H__ */
