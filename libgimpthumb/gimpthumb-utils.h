/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * Thumbnail handling according to the Thumbnail Managing Standard.
 * http://triq.net/~pearl/thumbnail-spec/
 *
 * Copyright (C) 2001-2003  Sven Neumann <sven@gimp.org>
 *                          Michael Natterer <mitch@gimp.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GIMP_THUMB_UTILS_H__
#define __GIMP_THUMB_UTILS_H__

G_BEGIN_DECLS


gboolean            gimp_thumb_init             (const gchar    *creator,
                                                 const gchar    *thumb_basedir);

const gchar       * gimp_thumb_get_thumb_dir    (GimpThumbSize   size);
gboolean            gimp_thumb_ensure_thumb_dir (GimpThumbSize   size,
                                                 GError        **error);

gchar             * gimp_thumb_name_from_uri    (const gchar    *uri,
                                                 GimpThumbSize   size);
gchar             * gimp_thumb_find_thumb       (const gchar    *uri,
                                                 GimpThumbSize  *size);

GimpThumbFileType   gimp_thumb_file_test        (const gchar    *filename,
                                                 gint64         *mtime,
                                                 gint64         *size,
                                                 gint           *err_no);


G_END_DECLS

#endif /* __GIMP_THUMB_UTILS_H__ */
