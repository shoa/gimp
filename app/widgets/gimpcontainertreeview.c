/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpcontainertreeview.c
 * Copyright (C) 2003 Michael Natterer <mitch@gimp.org>
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

#include "widgets-types.h"

#include "base/temp-buf.h"

#include "core/gimpcontainer.h"
#include "core/gimpcontext.h"
#include "core/gimpviewable.h"

#include "gimpcellrenderertoggle.h"
#include "gimpcellrendererviewable.h"
#include "gimpcontainertreeview.h"
#include "gimpcontainertreeview-dnd.h"
#include "gimpdnd.h"
#include "gimppreviewrenderer.h"
#include "gimpwidgets-utils.h"


enum
{
  COLUMN_RENDERER,
  COLUMN_NAME,
  NUM_COLUMNS
};


static void     gimp_container_tree_view_class_init   (GimpContainerTreeViewClass *klass);
static void     gimp_container_tree_view_init         (GimpContainerTreeView      *view);

static GObject *gimp_container_tree_view_constructor  (GType                   type,
                                                       guint                   n_params,
                                                       GObjectConstructParam  *params);
static void    gimp_container_tree_view_set_container (GimpContainerView      *view,
                                                       GimpContainer          *container);

static gpointer gimp_container_tree_view_insert_item  (GimpContainerView      *view,
						       GimpViewable           *viewable,
						       gint                    index);
static void     gimp_container_tree_view_remove_item  (GimpContainerView      *view,
						       GimpViewable           *viewable,
						       gpointer                insert_data);
static void     gimp_container_tree_view_reorder_item (GimpContainerView      *view,
						       GimpViewable           *viewable,
						       gint                    new_index,
						       gpointer                insert_data);
static void     gimp_container_tree_view_select_item  (GimpContainerView      *view,
						       GimpViewable           *viewable,
						       gpointer                insert_data);
static void     gimp_container_tree_view_clear_items  (GimpContainerView      *view);
static void gimp_container_tree_view_set_preview_size (GimpContainerView      *view);

static void gimp_container_tree_view_selection_changed (GtkTreeSelection      *sel,
                                                        GimpContainerTreeView *tree_view);
static gboolean gimp_container_tree_view_button_press (GtkWidget              *widget,
						       GdkEventButton         *bevent,
						       GimpContainerTreeView  *tree_view);
static void gimp_container_tree_view_renderer_update  (GimpPreviewRenderer    *renderer,
                                                       GimpContainerTreeView  *tree_view);
static void gimp_container_tree_view_name_changed     (GimpObject             *object,
                                                       GimpContainerTreeView  *tree_view);

static GimpViewable * gimp_container_tree_view_drag_viewable (GtkWidget       *widget,
                                                              gpointer         data);


static GimpContainerViewClass *parent_class = NULL;


GType
gimp_container_tree_view_get_type (void)
{
  static GType view_type = 0;

  if (! view_type)
    {
      static const GTypeInfo view_info =
      {
        sizeof (GimpContainerTreeViewClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) gimp_container_tree_view_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (GimpContainerTreeView),
        0,              /* n_preallocs */
        (GInstanceInitFunc) gimp_container_tree_view_init,
      };

      view_type = g_type_register_static (GIMP_TYPE_CONTAINER_VIEW,
                                          "GimpContainerTreeView",
                                          &view_info, 0);
    }

  return view_type;
}

static void
gimp_container_tree_view_class_init (GimpContainerTreeViewClass *klass)
{
  GObjectClass           *object_class;
  GimpContainerViewClass *container_view_class;

  object_class         = G_OBJECT_CLASS (klass);
  container_view_class = GIMP_CONTAINER_VIEW_CLASS (klass);
  
  parent_class = g_type_class_peek_parent (klass);

  object_class->constructor              = gimp_container_tree_view_constructor;

  container_view_class->set_container    = gimp_container_tree_view_set_container;
  container_view_class->insert_item      = gimp_container_tree_view_insert_item;
  container_view_class->remove_item      = gimp_container_tree_view_remove_item;
  container_view_class->reorder_item     = gimp_container_tree_view_reorder_item;
  container_view_class->select_item      = gimp_container_tree_view_select_item;
  container_view_class->clear_items      = gimp_container_tree_view_clear_items;
  container_view_class->set_preview_size = gimp_container_tree_view_set_preview_size;

  container_view_class->insert_data_free = (GDestroyNotify) g_free;

  klass->drop_possible = gimp_container_tree_view_real_drop_possible;
  klass->drop          = gimp_container_tree_view_real_drop;
}

static void
gimp_container_tree_view_init (GimpContainerTreeView *tree_view)
{
  tree_view->n_model_columns = NUM_COLUMNS;

  tree_view->model_columns[COLUMN_RENDERER] = GIMP_TYPE_PREVIEW_RENDERER;
  tree_view->model_columns[COLUMN_NAME]     = G_TYPE_STRING;

  tree_view->model_column_renderer = COLUMN_RENDERER;
  tree_view->model_column_name     = COLUMN_NAME;

  tree_view->preview_border_width = 1;

  tree_view->scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (tree_view->scrolled_win),
                                       GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (tree_view->scrolled_win), 
                                  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (tree_view), tree_view->scrolled_win);
  gtk_widget_show (tree_view->scrolled_win);

  GTK_WIDGET_UNSET_FLAGS (GTK_SCROLLED_WINDOW
			  (tree_view->scrolled_win)->vscrollbar,
                          GTK_CAN_FOCUS);
}

static GObject *
gimp_container_tree_view_constructor (GType                  type,
                                      guint                  n_params,
                                      GObjectConstructParam *params)
{
  GimpContainerTreeView *tree_view;
  GtkListStore          *list;
  GObject               *object;

  object = G_OBJECT_CLASS (parent_class)->constructor (type, n_params, params);

  tree_view = GIMP_CONTAINER_TREE_VIEW (object);

  list = gtk_list_store_newv (tree_view->n_model_columns,
                              tree_view->model_columns);
  tree_view->model = GTK_TREE_MODEL (list);

  tree_view->view =
    GTK_TREE_VIEW (gtk_tree_view_new_with_model (tree_view->model));
  g_object_unref (list);

  gtk_tree_view_set_headers_visible (tree_view->view, FALSE);
  gtk_container_add (GTK_CONTAINER (tree_view->scrolled_win),
                     GTK_WIDGET (tree_view->view));
  gtk_widget_show (GTK_WIDGET (tree_view->view));

  GIMP_CONTAINER_VIEW (tree_view)->dnd_widget = GTK_WIDGET (tree_view->view);

  tree_view->main_column = gtk_tree_view_column_new ();
  gtk_tree_view_insert_column (tree_view->view, tree_view->main_column, 0);

  tree_view->renderer_cell = gimp_cell_renderer_viewable_new ();
  gtk_tree_view_column_pack_start (tree_view->main_column,
                                   tree_view->renderer_cell,
                                   FALSE);

  gtk_tree_view_column_set_attributes (tree_view->main_column,
                                       tree_view->renderer_cell,
                                       "renderer", COLUMN_RENDERER,
                                       NULL);

  tree_view->name_cell = gtk_cell_renderer_text_new ();
  g_object_set (tree_view->name_cell, "xalign", 0.0, NULL);
  gtk_tree_view_column_pack_end (tree_view->main_column,
                                   tree_view->name_cell,
                                   FALSE);

  gtk_tree_view_column_set_attributes (tree_view->main_column,
                                       tree_view->name_cell,
                                       "text", COLUMN_NAME,
                                       NULL);

  tree_view->renderer_cells = g_list_prepend (tree_view->renderer_cells,
                                              tree_view->renderer_cell);

  tree_view->selection = gtk_tree_view_get_selection (tree_view->view);

  g_signal_connect (tree_view->selection, "changed",
                    G_CALLBACK (gimp_container_tree_view_selection_changed),
                    tree_view);

  g_signal_connect (tree_view->view, "drag_leave",
                    G_CALLBACK (gimp_container_tree_view_drag_leave),
                    tree_view);
  g_signal_connect (tree_view->view, "drag_motion",
                    G_CALLBACK (gimp_container_tree_view_drag_motion),
                    tree_view);
  g_signal_connect (tree_view->view, "drag_drop",
                    G_CALLBACK (gimp_container_tree_view_drag_drop),
                    tree_view);

  return object;
}

GtkWidget *
gimp_container_tree_view_new (GimpContainer *container,
                              GimpContext   *context,
			      gint           preview_size,
                              gboolean       reorderable,
			      gint           min_items_x,
			      gint           min_items_y)
{
  GimpContainerTreeView *tree_view;
  GimpContainerView     *view;
  gint                   window_border;

  g_return_val_if_fail (! container || GIMP_IS_CONTAINER (container), NULL);
  g_return_val_if_fail (! context || GIMP_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (preview_size > 0 &&
                        preview_size <= GIMP_PREVIEW_MAX_SIZE, NULL);
  g_return_val_if_fail (min_items_x > 0 && min_items_x <= 64, NULL);
  g_return_val_if_fail (min_items_y > 0 && min_items_y <= 64, NULL);

  tree_view = g_object_new (GIMP_TYPE_CONTAINER_TREE_VIEW, NULL);

  view = GIMP_CONTAINER_VIEW (tree_view);

  view->preview_size = preview_size;
  view->reorderable  = reorderable ? TRUE : FALSE;

  window_border =
    GTK_SCROLLED_WINDOW (tree_view->scrolled_win)->vscrollbar->requisition.width +
    GTK_SCROLLED_WINDOW_GET_CLASS (tree_view->scrolled_win)->scrollbar_spacing +
    tree_view->scrolled_win->style->xthickness * 4;

  gtk_widget_set_size_request (GTK_WIDGET (tree_view),
                               (preview_size + 2) * min_items_x + window_border,
                               (preview_size + 6) * min_items_y + window_border);

  if (container)
    gimp_container_view_set_container (view, container);

  if (context)
    gimp_container_view_set_context (view, context);

  return GTK_WIDGET (tree_view);
}

static void
gimp_container_tree_view_set (GimpContainerTreeView *tree_view,
                              GtkTreeIter           *iter,
                              GimpViewable          *viewable)
{
  GimpContainerView   *view;
  GimpPreviewRenderer *renderer;
  gchar               *name;

  view = GIMP_CONTAINER_VIEW (tree_view);

  if (view->get_name_func)
    name = view->get_name_func (G_OBJECT (viewable), NULL);
  else
    name = g_strdup (gimp_object_get_name (GIMP_OBJECT (viewable)));

  renderer = gimp_preview_renderer_new (G_TYPE_FROM_INSTANCE (viewable),
                                        view->preview_size,
                                        tree_view->preview_border_width,
                                        FALSE);
  gimp_preview_renderer_set_viewable (renderer, viewable);
  gimp_preview_renderer_remove_idle (renderer);

  g_signal_connect (renderer, "update",
                    G_CALLBACK (gimp_container_tree_view_renderer_update),
                    tree_view);

  gtk_list_store_set (GTK_LIST_STORE (tree_view->model), iter,
                      COLUMN_RENDERER, renderer,
                      COLUMN_NAME,     name,
                      -1);

  g_object_unref (renderer);
  g_free (name);
}

/*  GimpContainerView methods  */

static void
gimp_container_tree_view_set_container (GimpContainerView *view,
                                        GimpContainer     *container)
{
  GimpContainerTreeView *tree_view;

  tree_view = GIMP_CONTAINER_TREE_VIEW (view);

  if (view->container)
    {
      gimp_container_remove_handler (view->container,
                                     tree_view->name_changed_handler_id);

      if (! container)
        {
          if (gimp_dnd_viewable_source_unset (GTK_WIDGET (tree_view->view),
                                              view->container->children_type))
            {
              gtk_drag_source_unset (GTK_WIDGET (tree_view->view));
            }

          g_signal_handlers_disconnect_by_func (tree_view->view,
                                                gimp_container_tree_view_button_press,
                                                tree_view);
        }
    }
  else if (container)
    {
      if (gimp_dnd_drag_source_set_by_type (GTK_WIDGET (tree_view->view),
                                            GDK_BUTTON1_MASK | GDK_BUTTON2_MASK,
                                            container->children_type,
                                            GDK_ACTION_COPY))
        {
          gimp_dnd_viewable_source_set (GTK_WIDGET (tree_view->view),
                                        container->children_type,
                                        gimp_container_tree_view_drag_viewable,
                                        tree_view);
        }

      /*  connect button_press_event after DND so we can keep the list from
       *  selecting the item on button2
       */
      g_signal_connect (tree_view->view, "button_press_event",
                        G_CALLBACK (gimp_container_tree_view_button_press),
                        tree_view);
    }

  GIMP_CONTAINER_VIEW_CLASS (parent_class)->set_container (view, container);

  if (view->container)
    {
      GimpViewableClass *viewable_class;

      viewable_class = g_type_class_ref (container->children_type);

      tree_view->name_changed_handler_id =
        gimp_container_add_handler (view->container,
                                    viewable_class->name_changed_signal,
                                    G_CALLBACK (gimp_container_tree_view_name_changed),
                                    tree_view);

      g_type_class_unref (viewable_class);
    }
}

static gpointer
gimp_container_tree_view_insert_item (GimpContainerView *view,
				      GimpViewable      *viewable,
				      gint               index)
{
  GimpContainerTreeView *tree_view;
  GtkTreeIter           *iter;

  tree_view = GIMP_CONTAINER_TREE_VIEW (view);

  iter = g_new0 (GtkTreeIter, 1);

  if (index == -1)
    gtk_list_store_append (GTK_LIST_STORE (tree_view->model), iter);
  else
    gtk_list_store_insert (GTK_LIST_STORE (tree_view->model), iter, index);

  gimp_container_tree_view_set (tree_view, iter, viewable);

  return (gpointer) iter;
}

static void
gimp_container_tree_view_remove_item (GimpContainerView *view,
				      GimpViewable      *viewable,
				      gpointer           insert_data)
{
  GimpContainerTreeView *tree_view;
  GtkTreeIter           *iter;

  tree_view = GIMP_CONTAINER_TREE_VIEW (view);

  iter = (GtkTreeIter *) insert_data;

  if (iter)
    gtk_list_store_remove (GTK_LIST_STORE (tree_view->model), iter);
}

static void
gimp_container_tree_view_reorder_item (GimpContainerView *view,
				       GimpViewable      *viewable,
				       gint               new_index,
				       gpointer           insert_data)
{
  GimpContainerTreeView *tree_view;
  GtkTreeIter           *iter;

  tree_view = GIMP_CONTAINER_TREE_VIEW (view);

  iter = (GtkTreeIter *) insert_data;

  if (iter)
    {
      GtkTreePath *path;
      GtkTreeIter  selected_iter;
      gboolean     selected;

      selected = gtk_tree_selection_get_selected (tree_view->selection,
                                                  NULL, &selected_iter);

      if (selected)
        {
          GimpPreviewRenderer *renderer;

          gtk_tree_model_get (tree_view->model, &selected_iter,
                              COLUMN_RENDERER, &renderer,
                              -1);

          if (renderer->viewable != viewable)
            selected = FALSE;

          g_object_unref (renderer);
        }

      if (new_index == -1 || new_index == view->container->num_children - 1)
        {
          gtk_list_store_move_before (GTK_LIST_STORE (tree_view->model),
                                      iter, NULL);
        }
      else if (new_index == 0)
        {
          gtk_list_store_move_after (GTK_LIST_STORE (tree_view->model),
                                     iter, NULL);
        }
      else
        {
          GtkTreeIter place_iter;
          gint        old_index;

          path = gtk_tree_model_get_path (tree_view->model, iter);
          old_index = gtk_tree_path_get_indices (path)[0];
          gtk_tree_path_free (path);

          if (new_index != old_index)
            {
              path = gtk_tree_path_new_from_indices (new_index, -1);
              gtk_tree_model_get_iter (tree_view->model, &place_iter, path);
              gtk_tree_path_free (path);

              if (new_index > old_index)
                gtk_list_store_move_after (GTK_LIST_STORE (tree_view->model),
                                           iter, &place_iter);
              else
                gtk_list_store_move_before (GTK_LIST_STORE (tree_view->model),
                                            iter, &place_iter);
            }
        }

      if (selected)
        {
          path = gtk_tree_model_get_path (tree_view->model, iter);
          gtk_tree_view_scroll_to_cell (tree_view->view, path,
                                        NULL, FALSE, 0.0, 0.0);
          gtk_tree_path_free (path);
        }
    }
}

static void
gimp_container_tree_view_select_item (GimpContainerView *view,
				      GimpViewable      *viewable,
				      gpointer           insert_data)
{
  GimpContainerTreeView *tree_view;
  GtkTreeIter           *iter;

  tree_view = GIMP_CONTAINER_TREE_VIEW (view);

  iter = (GtkTreeIter *) insert_data;

  if (iter)
    {
      GtkTreePath *path;
      GtkTreeIter  selected_iter;

      if (gtk_tree_selection_get_selected (tree_view->selection, NULL,
                                           &selected_iter))
        {
          GimpPreviewRenderer *renderer;
          gboolean             equal;

          gtk_tree_model_get (tree_view->model, &selected_iter,
                              COLUMN_RENDERER, &renderer,
                              -1);

          equal = (renderer->viewable == viewable);

          g_object_unref (renderer);

          if (equal)
            return;
        }

      path = gtk_tree_model_get_path (tree_view->model, iter);

      g_signal_handlers_block_by_func (tree_view->selection,
				       gimp_container_tree_view_selection_changed,
				       tree_view);

#ifdef __GNUC__
#warning FIXME: remove this hack as soon as #108956 is fixed.
#endif
      if (tree_view->main_column->editable_widget)
        gtk_cell_editable_remove_widget (tree_view->main_column->editable_widget);

      gtk_tree_view_set_cursor (tree_view->view, path, NULL, FALSE);

      g_signal_handlers_unblock_by_func (tree_view->selection,
					 gimp_container_tree_view_selection_changed,
					 tree_view);

      gtk_tree_view_scroll_to_cell (tree_view->view, path,
                                    NULL, FALSE, 0.0, 0.0);

      gtk_tree_path_free (path);
    }
  else
    {
      gtk_tree_selection_unselect_all (tree_view->selection);
    }
}

static void
gimp_container_tree_view_clear_items (GimpContainerView *view)
{
  GimpContainerTreeView *tree_view;

  tree_view = GIMP_CONTAINER_TREE_VIEW (view);

  gtk_list_store_clear (GTK_LIST_STORE (tree_view->model));

  GIMP_CONTAINER_VIEW_CLASS (parent_class)->clear_items (view);
}

static void
gimp_container_tree_view_set_preview_size (GimpContainerView *view)
{
  GimpContainerTreeView *tree_view;
  GtkWidget             *tree_widget;
  GtkTreeIter            iter;
  gboolean               iter_valid;
  GList                 *list;

  tree_view = GIMP_CONTAINER_TREE_VIEW (view);

  tree_widget = GTK_WIDGET (tree_view->view);

  for (list = tree_view->toggle_cells; list; list = g_list_next (list))
    {
      gchar       *stock_id;
      GtkIconSize  icon_size;

      g_object_get (list->data, "stock-id", &stock_id, NULL);

      if (stock_id)
        {
          icon_size = gimp_get_icon_size (tree_widget,
                                          stock_id,
                                          GTK_ICON_SIZE_BUTTON,
                                          view->preview_size -
                                          2 * tree_widget->style->xthickness,
                                          view->preview_size -
                                          2 * tree_widget->style->ythickness);

          g_object_set (list->data, "stock-size", icon_size, NULL);

          g_free (stock_id);
        }
    }

  for (iter_valid = gtk_tree_model_get_iter_first (tree_view->model, &iter);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (tree_view->model, &iter))
    {
      GimpPreviewRenderer *renderer;

      gtk_tree_model_get (tree_view->model, &iter,
                          COLUMN_RENDERER, &renderer,
                          -1);

      gimp_preview_renderer_set_size (renderer, view->preview_size,
                                      tree_view->preview_border_width);

      g_object_unref (renderer);
    }

  gtk_tree_view_columns_autosize (tree_view->view);
}


/*  callbacks  */

static void
gimp_container_tree_view_selection_changed (GtkTreeSelection      *selection,
                                            GimpContainerTreeView *tree_view)
{
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      GimpPreviewRenderer *renderer;

      gtk_tree_model_get (tree_view->model, &iter,
                          COLUMN_RENDERER, &renderer,
                          -1);

      gimp_container_view_item_selected (GIMP_CONTAINER_VIEW (tree_view),
                                         renderer->viewable);
      g_object_unref (renderer);
    }
}

static GtkCellRenderer *
gimp_container_tree_view_find_click_cell (GList             *cells,
                                          GtkTreeViewColumn *column,
                                          GdkRectangle      *column_area,
                                          gint               tree_x,
                                          gint               tree_y)
{
  GtkCellRenderer *renderer;
  GList           *list;

  for (list = cells; list; list = g_list_next (list))
    {
      gint start_pos;
      gint width;

      renderer = (GtkCellRenderer *) list->data;

      if (renderer->visible &&
          gtk_tree_view_column_cell_get_position (column, renderer,
                                                  &start_pos, &width)       &&
          width > 0                                                         &&
          column_area->x + start_pos + renderer->xpad             <= tree_x &&
          column_area->x + start_pos + renderer->xpad + width - 1 >= tree_x)
        {
#if 0
          g_print ("click on cell at %d (%d width)\n", start_pos, width);
#endif

          return renderer;
        }
    }

  return NULL;
}

static gboolean
gimp_container_tree_view_button_press (GtkWidget             *widget,
                                       GdkEventButton        *bevent,
                                       GimpContainerTreeView *tree_view)
{
  GimpContainerView *container_view;
  GtkTreeViewColumn *column;
  GtkTreePath       *path;

  container_view = GIMP_CONTAINER_VIEW (tree_view);

  tree_view->dnd_viewable = NULL;

  if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget),
                                     bevent->x,
                                     bevent->y,
                                     &path, &column, NULL, NULL))
    {
      GimpPreviewRenderer      *renderer;
      GimpCellRendererToggle   *toggled_cell = NULL;
      GimpCellRendererViewable *clicked_cell = NULL;
      GtkCellRenderer          *edit_cell    = NULL;
      GdkRectangle              column_area;
      gint                      tree_x;
      gint                      tree_y;
      GtkTreeIter               iter;

      gtk_tree_model_get_iter (tree_view->model, &iter, path);

      gtk_tree_model_get (tree_view->model, &iter,
                          COLUMN_RENDERER, &renderer,
                          -1);

      tree_view->dnd_viewable = renderer->viewable;

      gtk_tree_view_widget_to_tree_coords (tree_view->view,
                                           bevent->x, bevent->y,
                                           &tree_x, &tree_y);
      gtk_tree_view_get_background_area (tree_view->view, path,
                                         column, &column_area);

      gtk_tree_view_column_cell_set_cell_data (column,
                                               tree_view->model,
                                               &iter,
                                               FALSE, FALSE);

      toggled_cell = (GimpCellRendererToggle *)
        gimp_container_tree_view_find_click_cell (tree_view->toggle_cells,
                                                  column, &column_area,
                                                  tree_x, tree_y);

      if (! toggled_cell)
        clicked_cell = (GimpCellRendererViewable *)
          gimp_container_tree_view_find_click_cell (tree_view->renderer_cells,
                                                    column, &column_area,
                                                    tree_x, tree_y);

      if (! toggled_cell && ! clicked_cell)
        edit_cell =
          gimp_container_tree_view_find_click_cell (tree_view->editable_cells,
                                                    column, &column_area,
                                                    tree_x, tree_y);

      g_object_ref (tree_view);

      switch (bevent->button)
        {
        case 1:
          if (bevent->type == GDK_BUTTON_PRESS)
            {
              /*  don't select item if a toggle was clicked */
              if (! toggled_cell)
                gimp_container_view_item_selected (container_view,
                                                   renderer->viewable);

              /*  a callback invoked by selecting the item may have
               *  destroyed us, so check if the container is still there
               */
              if (container_view->container)
                {
                  gchar *path_str = NULL;

                  /*  another row may have been set by selecting  */
                  gtk_tree_view_column_cell_set_cell_data (column,
                                                           tree_view->model,
                                                           &iter,
                                                           FALSE, FALSE);

                  if (toggled_cell || clicked_cell)
                    path_str = gtk_tree_path_to_string (path);
 
                  if (toggled_cell)
                    {
                      gimp_cell_renderer_toggle_clicked (toggled_cell,
                                                         path_str,
                                                         bevent->state);
                    }
                  else if (clicked_cell)
                    {
                      gimp_cell_renderer_viewable_clicked (clicked_cell,
                                                           path_str,
                                                           bevent->state);
                    }

                  if (path_str)
                    g_free (path_str);
                }
            }
          else if (bevent->type == GDK_2BUTTON_PRESS)
            {
              if (edit_cell)
                {
#ifdef __GNUC__
#warning FIXME: remove this hack as soon as #108956 is fixed.
#endif
                  if (column->editable_widget)
                    gtk_cell_editable_remove_widget (column->editable_widget);

                  gtk_tree_view_set_cursor_on_cell (tree_view->view, path,
                                                    column, edit_cell, TRUE);
                }
              else if (! toggled_cell) /* ignore double click on toggles */
                {
                  gimp_container_view_item_activated (container_view,
                                                      renderer->viewable);
                }
            }
          break;

        case 2:
          break;

        case 3:
          gimp_container_view_item_selected (container_view,
                                             renderer->viewable);

          if (container_view->container)
            gimp_container_view_item_context (GIMP_CONTAINER_VIEW (tree_view),
                                              renderer->viewable);
          break;

        default:
          break;
        }

      g_object_unref (tree_view);

      gtk_tree_path_free (path);
      g_object_unref (renderer);
    }

  return TRUE;
}

static void
gimp_container_tree_view_renderer_update (GimpPreviewRenderer   *renderer,
                                          GimpContainerTreeView *tree_view)
{
  GimpContainerView *view;
  GtkTreeIter       *iter;

  view = GIMP_CONTAINER_VIEW (tree_view);

  iter = g_hash_table_lookup (view->hash_table, renderer->viewable);

  if (iter)
    {
      GtkTreePath *path;

      path = gtk_tree_model_get_path (tree_view->model, iter);

      gtk_tree_model_row_changed (tree_view->model, path, iter);

      gtk_tree_path_free (path);
    }
}

static void
gimp_container_tree_view_name_changed (GimpObject            *object,
                                       GimpContainerTreeView *tree_view)
{
  GimpContainerView *view;
  GtkTreeIter       *iter;

  view = GIMP_CONTAINER_VIEW (tree_view);

  iter = g_hash_table_lookup (view->hash_table, object);

  if (iter)
    {
      gchar *name;

      if (view->get_name_func)
        name = view->get_name_func (G_OBJECT (object), NULL);
      else
        name = g_strdup (gimp_object_get_name (object));

      gtk_list_store_set (GTK_LIST_STORE (tree_view->model), iter,
                          COLUMN_NAME, name,
                          -1);

      g_free (name);
    }
}

static GimpViewable *
gimp_container_tree_view_drag_viewable (GtkWidget *widget,
                                        gpointer   data)
{
  GimpContainerTreeView *tree_view;

  tree_view = GIMP_CONTAINER_TREE_VIEW (data);

  return tree_view->dnd_viewable;
}
