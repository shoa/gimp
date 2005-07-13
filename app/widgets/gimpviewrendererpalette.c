/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpviewrendererpalette.c
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

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>

#include "libgimpcolor/gimpcolor.h"
#include "libgimpmath/gimpmath.h"

#include "widgets-types.h"

#include "core/gimppalette.h"

#include "gimpviewrendererpalette.h"


static void   gimp_view_renderer_palette_class_init (GimpViewRendererPaletteClass *klass);
static void   gimp_view_renderer_palette_init       (GimpViewRendererPalette      *renderer);

static void   gimp_view_renderer_palette_finalize   (GObject             *object);

static void   gimp_view_renderer_palette_render     (GimpViewRenderer    *renderer,
                                                     GtkWidget           *widget);


static GimpViewRendererClass *parent_class = NULL;


GType
gimp_view_renderer_palette_get_type (void)
{
  static GType renderer_type = 0;

  if (! renderer_type)
    {
      static const GTypeInfo renderer_info =
      {
        sizeof (GimpViewRendererPaletteClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) gimp_view_renderer_palette_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (GimpViewRendererPalette),
        0,              /* n_preallocs */
        (GInstanceInitFunc) gimp_view_renderer_palette_init,
      };

      renderer_type = g_type_register_static (GIMP_TYPE_VIEW_RENDERER,
                                              "GimpViewRendererPalette",
                                              &renderer_info, 0);
    }

  return renderer_type;
}

static void
gimp_view_renderer_palette_class_init (GimpViewRendererPaletteClass *klass)
{
  GObjectClass          *object_class   = G_OBJECT_CLASS (klass);
  GimpViewRendererClass *renderer_class = GIMP_VIEW_RENDERER_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize = gimp_view_renderer_palette_finalize;

  renderer_class->render = gimp_view_renderer_palette_render;
}

static void
gimp_view_renderer_palette_init (GimpViewRendererPalette *renderer)
{
}

static void
gimp_view_renderer_palette_finalize (GObject *object)
{
  GimpViewRendererPalette *renderer = GIMP_VIEW_RENDERER_PALETTE (object);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gimp_view_renderer_palette_render (GimpViewRenderer *renderer,
                                   GtkWidget        *widget)
{
  GimpViewRendererPalette *renderpal = GIMP_VIEW_RENDERER_PALETTE (renderer);
  GimpPalette             *palette;
  guchar                  *row;
  GList                   *list;
  gint                     columns;
  gint                     rows;
  gint                     cell_size;
  gint                     x, y;

  palette = GIMP_PALETTE (renderer->viewable);

  if (! renderer->buffer)
    renderer->buffer = g_new (guchar, renderer->height * renderer->rowstride);

  memset (renderer->buffer, 255, renderer->height * renderer->rowstride);

  if (palette->n_columns > 1)
    cell_size = MAX (4, renderer->width / palette->n_columns);
  else
    cell_size = 4;

  columns = renderer->width  / cell_size;
  rows    = renderer->height / cell_size;

  list = palette->colors;

  row = g_new0 (guchar, renderer->rowstride);

  for (y = 0; y < rows && list; y++)
    {
      gint i;

      memset (row, 255, renderer->rowstride);

      for (x = 0; x < columns && list; x++)
        {
          GimpPaletteEntry *entry = list->data;

          list = g_list_next (list);

          gimp_rgb_get_uchar (&entry->color,
                              &row[x * cell_size * 3 + 0],
                              &row[x * cell_size * 3 + 1],
                              &row[x * cell_size * 3 + 2]);

          for (i = 1; i < cell_size; i++)
            {
              row[(x * cell_size + i) * 3 + 0] = row[(x * cell_size) * 3 + 0];
              row[(x * cell_size + i) * 3 + 1] = row[(x * cell_size) * 3 + 1];
              row[(x * cell_size + i) * 3 + 2] = row[(x * cell_size) * 3 + 2];
            }
        }

      for (i = 0; i < cell_size; i++)
        memcpy (renderer->buffer + (y * cell_size + i) * renderer->rowstride,
                row,
                renderer->rowstride);
    }

  g_free (row);

  renderer->needs_render = FALSE;
}
