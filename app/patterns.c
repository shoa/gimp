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

#include <sys/types.h>

#include <gtk/gtk.h>

#include "apptypes.h"

#include "datafiles.h"
#include "gimpcontext.h"
#include "gimpdatalist.h"
#include "gimppattern.h"
#include "gimprc.h"
#include "patterns.h"
#include "pattern_select.h"
#include "temp_buf.h"

#include "libgimp/gimpintl.h"


/*  local function prototypes  */
static void   patterns_load_pattern (const gchar *filename);


/*  global variables  */
GimpList *global_pattern_list = NULL;


/*  static variables  */
static GimpPattern *standard_pattern = NULL;


/*  public functions  */

void
patterns_init (gboolean no_data)
{
  if (global_pattern_list)
    patterns_free ();
  else
    global_pattern_list = GIMP_LIST (gimp_data_list_new (GIMP_TYPE_PATTERN));

  if (pattern_path != NULL && !no_data)
    {
      pattern_select_freeze_all ();

      datafiles_read_directories (pattern_path, patterns_load_pattern, 0);

      pattern_select_thaw_all ();
    }

  gimp_context_refresh_patterns ();
}

void
patterns_free (void)
{
  if (! global_pattern_list)
    return;

  pattern_select_freeze_all ();

  while (global_pattern_list->list)
    {
      gimp_container_remove (GIMP_CONTAINER (global_pattern_list),
			     GIMP_OBJECT (global_pattern_list->list->data));
    }

  pattern_select_thaw_all ();
}

GimpPattern *
patterns_get_standard_pattern (void)
{
  if (! standard_pattern)
    {
      guchar *data;
      gint    row, col;

      standard_pattern = GIMP_PATTERN (gtk_type_new (GIMP_TYPE_PATTERN));

      gimp_object_set_name (GIMP_OBJECT (standard_pattern), "Standard");

      standard_pattern->filename = NULL;
      standard_pattern->mask     = temp_buf_new (32, 32, 3, 0, 0, NULL);

      data = temp_buf_data (standard_pattern->mask);

      for (row = 0; row < standard_pattern->mask->height; row++)
	for (col = 0; col < standard_pattern->mask->width; col++)
	  {
	    memset (data, (col % 2) && (row % 2) ? 255 : 0, 3);
	    data += 3;
	  }

      /*  set ref_count to 2 --> never swap the standard pattern  */
      gtk_object_ref (GTK_OBJECT (standard_pattern));
      gtk_object_ref (GTK_OBJECT (standard_pattern));
      gtk_object_sink (GTK_OBJECT (standard_pattern));
    }

  return standard_pattern;
}

static void
patterns_load_pattern (const gchar *filename)
{
  GimpPattern   *pattern;

  g_return_if_fail (filename != NULL);

  pattern = gimp_pattern_load (filename);

  if (! pattern)
    g_message (_("Warning: Failed to load pattern\n\"%s\""), filename);

  if (pattern != NULL)
    gimp_container_add (GIMP_CONTAINER (global_pattern_list),
                        GIMP_OBJECT (pattern));
}
