/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * GimpImagefilePreview Widget
 * Copyright (C) 2001 Michael Natterer <mitch@gimp.org>
 *                    Sven Neumann <sven@gimp.org>
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

#ifndef __GIMP_IMAGEFILE_PREVIEW_H__
#define __GIMP_IMAGEFILE_PREVIEW_H__


#include "gimppreview.h"


G_BEGIN_DECLS

#define GIMP_TYPE_IMAGEFILE_PREVIEW            (gimp_imagefile_preview_get_type ())
#define GIMP_IMAGEFILE_PREVIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_IMAGEFILE_PREVIEW, GimpImagefilePreview))
#define GIMP_IMAGEFILE_PREVIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_IMAGEFILE_PREVIEW, GimpImagefilePreviewClass))
#define GIMP_IS_IMAGEFILE_PREVIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (obj, GIMP_TYPE_IMAGEFILE_PREVIEW))
#define GIMP_IS_IMAGEFILE_PREVIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_IMAGEFILE_PREVIEW))
#define GIMP_IMAGEFILE_PREVIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GIMP_TYPE_IMAGEFILE_PREVIEW, GimpImagefilePreviewClass))


typedef struct _GimpImagefilePreviewClass  GimpImagefilePreviewClass;

struct _GimpImagefilePreview
{
  GimpPreview  parent_instance;
};

struct _GimpImagefilePreviewClass
{
  GimpPreviewClass  parent_class;
};


GType   gimp_imagefile_preview_get_type (void) G_GNUC_CONST;


G_END_DECLS

#endif /* __GIMP_IMAGEFILE_PREVIEW_H__ */
