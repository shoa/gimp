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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>

#ifdef G_OS_WIN32
#include <io.h>
#endif

#ifndef _O_BINARY
#define _O_BINARY 0
#endif

#include <gtk/gtk.h>

#include <stdio.h>

#include "apptypes.h"

#include "gimppattern.h"
#include "gimprc.h"
#include "patterns.h"
#include "pattern_header.h"
#include "temp_buf.h"

#include "libgimp/gimpintl.h"


static void   gimp_pattern_class_init (GimpPatternClass *klass);
static void   gimp_pattern_init       (GimpPattern      *pattern);
static void   gimp_pattern_destroy    (GtkObject        *object);


static GimpObjectClass *parent_class = NULL;


GtkType
gimp_pattern_get_type (void)
{
  static GtkType pattern_type = 0;

  if (! pattern_type)
    {
      static const GtkTypeInfo pattern_info =
      {
        "GimpPattern",
        sizeof (GimpPattern),
        sizeof (GimpPatternClass),
        (GtkClassInitFunc) gimp_pattern_class_init,
        (GtkObjectInitFunc) gimp_pattern_init,
        /* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL
      };

      pattern_type = gtk_type_unique (GIMP_TYPE_OBJECT, &pattern_info);
  }

  return pattern_type;
}

static void
gimp_pattern_class_init (GimpPatternClass *klass)
{
  GtkObjectClass *object_class;

  object_class = (GtkObjectClass *) klass;

  parent_class = gtk_type_class (GIMP_TYPE_OBJECT);
  
  object_class->destroy = gimp_pattern_destroy;
}

static void
gimp_pattern_init (GimpPattern *pattern)
{
  pattern->filename  = NULL;
  pattern->mask      = NULL;
}

static void
gimp_pattern_destroy (GtkObject *object)
{
  GimpPattern *pattern;

  pattern = GIMP_PATTERN (object);

  g_free (pattern->filename);

  if (pattern->mask)
    temp_buf_free (pattern->mask);

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GimpPattern *
gimp_pattern_load (const gchar *filename)
{
  GimpPattern   *pattern = NULL;
  gint           fd;
  PatternHeader  header;
  gint           bn_size;
  gchar         *name    = NULL;

  g_return_val_if_fail (filename != NULL, NULL);

  fd = open (filename, O_RDONLY | _O_BINARY);
  if (fd == -1)
    return NULL;

  /*  Read in the header size  */
  if (read (fd, &header, sizeof (header)) != sizeof (header))
    goto error;

  /*  rearrange the bytes in each unsigned int  */
  header.header_size  = g_ntohl (header.header_size);
  header.version      = g_ntohl (header.version);
  header.width        = g_ntohl (header.width);
  header.height       = g_ntohl (header.height);
  header.bytes        = g_ntohl (header.bytes);
  header.magic_number = g_ntohl (header.magic_number);

  /*  Check for correct file format */
  if (header.magic_number != GPATTERN_MAGIC || header.version != 1 || 
      header.header_size <= sizeof (header)) 
    {
      g_message (_("Unknown pattern format version #%d in \"%s\"."),
		 header.version, filename);
      goto error;
    }
  
  /*  Check for supported bit depths  */
  if (header.bytes != 1 && header.bytes != 3)
    {
      g_message ("Unsupported pattern depth: %d\n"
		 "in file \"%s\"\n"
		 "GIMP Patterns must be GRAY or RGB\n",
                 header.bytes, filename);
      goto error;
    }

  /*  Read in the pattern name  */
  if ((bn_size = (header.header_size - sizeof (header))))
    {
      name = g_new (gchar, bn_size);

      if ((read (fd, name, bn_size)) < bn_size)
        {
          g_message (_("Error in GIMP pattern file \"%s\"."), filename);
	  goto error;
        }
    }
  else
    {
      name = g_strdup (_("Unnamed"));
    }

  pattern = GIMP_PATTERN (gtk_type_new (GIMP_TYPE_PATTERN));
  pattern->mask = temp_buf_new (header.width, header.height, header.bytes,
                                0, 0, NULL);
  if (read (fd, temp_buf_data (pattern->mask), 
            header.width * header.height * header.bytes) <
      header.width * header.height * header.bytes)
    {
      g_message (_("GIMP pattern file appears to be truncated: \"%s\"."),
		 filename);
      goto error;
    }

  close (fd);

  GIMP_OBJECT (pattern)->name = name;

  pattern->filename = g_strdup (filename);

  /*  Swap the pattern to disk (if we're being stingy with memory) */
  if (stingy_memory_use)
    temp_buf_swap (pattern->mask);

  return pattern;

 error:
  if (pattern)
    gtk_object_unref (GTK_OBJECT (pattern));
  else if (name)
    g_free (name);

  close (fd);

  return NULL;
}

TempBuf *
gimp_pattern_get_mask (const GimpPattern *pattern)
{
  g_return_val_if_fail (pattern != NULL, NULL);
  g_return_val_if_fail (GIMP_IS_PATTERN (pattern), NULL);

  return pattern->mask;
}
