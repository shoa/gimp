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

#ifndef __GIMP_IMAGE_UNDO_H__
#define __GIMP_IMAGE_UNDO_H__


gboolean   gimp_image_undo             (GimpImage        *gimage);
gboolean   gimp_image_redo             (GimpImage        *gimage);

void       gimp_image_undo_free        (GimpImage        *gimage);

gboolean   gimp_image_undo_group_start (GimpImage        *gimage,
                                        GimpUndoType      type,
                                        const gchar      *name);
gboolean   gimp_image_undo_group_end   (GimpImage        *gimage);

GimpUndo * gimp_image_undo_push        (GimpImage        *gimage,
                                        GType             undo_gtype,
                                        gint64            size,
                                        gsize             struct_size,
                                        GimpUndoType      type,
                                        const gchar      *name,
                                        GimpDirtyMask     dirty_mask,
                                        GimpUndoPopFunc   pop_func,
                                        GimpUndoFreeFunc  free_func,
                                        ...);


#endif /* __GIMP_IMAGE_UNDO_H__ */
