/* The GIMP -- an image manipulation program
 * Copyright (C) 1995-2003 Spencer Kimball and Peter Mattis
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

/* NOTE: This file is autogenerated by pdbgen.pl. */

#include "config.h"


#include <gtk/gtk.h>

#include "libgimpbase/gimpbasetypes.h"

#include "pdb-types.h"
#include "display/display-types.h"
#include "procedural_db.h"

#include "core/gimp.h"
#include "core/gimpcontainer.h"
#include "core/gimpimage.h"
#include "display/gimpdisplay-foreach.h"
#include "display/gimpdisplay.h"

static ProcRecord display_new_proc;
static ProcRecord display_delete_proc;
static ProcRecord displays_flush_proc;
static ProcRecord displays_reconnect_proc;

void
register_display_procs (Gimp *gimp)
{
  procedural_db_register (gimp, &display_new_proc);
  procedural_db_register (gimp, &display_delete_proc);
  procedural_db_register (gimp, &displays_flush_proc);
  procedural_db_register (gimp, &displays_reconnect_proc);
}

static Argument *
display_new_invoker (Gimp     *gimp,
                     Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpImage *gimage;
  GimpDisplay *display = NULL;

  gimage = gimp_image_get_by_ID (gimp, args[0].value.pdb_int);
  if (! GIMP_IS_IMAGE (gimage))
    success = FALSE;

  if (success)
    {
      display = (GimpDisplay *) gimp_create_display (gimp, gimage, 1.0);

      success = (display != NULL);

      /* the first display takes ownership of the image */
      if (success && gimage->disp_count == 1)
	g_object_unref (gimage);
    }

  return_args = procedural_db_return_args (&display_new_proc, success);

  if (success)
    return_args[1].value.pdb_int = gimp_display_get_ID (display);

  return return_args;
}

static ProcArg display_new_inargs[] =
{
  {
    GIMP_PDB_IMAGE,
    "image",
    "The image"
  }
};

static ProcArg display_new_outargs[] =
{
  {
    GIMP_PDB_DISPLAY,
    "display",
    "The new display"
  }
};

static ProcRecord display_new_proc =
{
  "gimp_display_new",
  "Create a new display for the specified image.",
  "Creates a new display for the specified image. If the image already has a display, another is added. Multiple displays are handled transparently by the GIMP. The newly created display is returned and can be subsequently destroyed with a call to 'gimp-display-delete'. This procedure only makes sense for use with the GIMP UI.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  1,
  display_new_inargs,
  1,
  display_new_outargs,
  { { display_new_invoker } }
};

static Argument *
display_delete_invoker (Gimp     *gimp,
                        Argument *args)
{
  gboolean success = TRUE;
  GimpDisplay *display;

  display = gimp_display_get_by_ID (gimp, args[0].value.pdb_int);
  if (! GIMP_IS_DISPLAY (display))
    success = FALSE;

  if (success)
    gimp_display_delete (display);

  return procedural_db_return_args (&display_delete_proc, success);
}

static ProcArg display_delete_inargs[] =
{
  {
    GIMP_PDB_DISPLAY,
    "display",
    "The display to delete"
  }
};

static ProcRecord display_delete_proc =
{
  "gimp_display_delete",
  "Delete the specified display.",
  "This procedure removes the specified display. If this is the last remaining display for the underlying image, then the image is deleted also.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  1,
  display_delete_inargs,
  0,
  NULL,
  { { display_delete_invoker } }
};

static Argument *
displays_flush_invoker (Gimp     *gimp,
                        Argument *args)
{
  gimp_container_foreach (gimp->images, (GFunc) gimp_image_flush, NULL);
  return procedural_db_return_args (&displays_flush_proc, TRUE);
}

static ProcRecord displays_flush_proc =
{
  "gimp_displays_flush",
  "Flush all internal changes to the user interface",
  "This procedure takes no arguments and returns nothing except a success status. Its purpose is to flush all pending updates of image manipulations to the user interface. It should be called whenever appropriate.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  0,
  NULL,
  0,
  NULL,
  { { displays_flush_invoker } }
};

static Argument *
displays_reconnect_invoker (Gimp     *gimp,
                            Argument *args)
{
  gboolean success = TRUE;
  GimpImage *old_image;
  GimpImage *new_image;

  old_image = gimp_image_get_by_ID (gimp, args[0].value.pdb_int);
  if (! GIMP_IS_IMAGE (old_image))
    success = FALSE;

  new_image = gimp_image_get_by_ID (gimp, args[1].value.pdb_int);
  if (! GIMP_IS_IMAGE (new_image))
    success = FALSE;

  if (success)
    gimp_displays_reconnect (gimp, old_image, new_image);

  return procedural_db_return_args (&displays_reconnect_proc, success);
}

static ProcArg displays_reconnect_inargs[] =
{
  {
    GIMP_PDB_IMAGE,
    "old_image",
    "The old image (should have at least one display)"
  },
  {
    GIMP_PDB_IMAGE,
    "new_image",
    "The new image (must not have a display)"
  }
};

static ProcRecord displays_reconnect_proc =
{
  "gimp_displays_reconnect",
  "Reconnect displays from one image to another image.",
  "This procedure connects all displays of the old_image to the new_image. If the new_image already has a display the reconnect is not performed and the procedure returns without success. You should rarely need to use this function.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  2,
  displays_reconnect_inargs,
  0,
  NULL,
  { { displays_reconnect_invoker } }
};
