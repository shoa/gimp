/* The GIMP -- an image manipulation program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#ifndef __GIMP_SCAN_CONVERT_H__
#define __GIMP_SCAN_CONVERT_H__


typedef struct _GimpScanConvert GimpScanConvert;


/* Create a new scan conversion context.  Set "antialias" to 1 for no
 * supersampling, or the amount to supersample by otherwise.
 */
GimpScanConvert * gimp_scan_convert_new        (guint            width,
                                                guint            height,
                                                guint            antialias);

void              gimp_scan_convert_free       (GimpScanConvert *scan_converter);

/* Add "npoints" from "pointlist" to the polygon currently being
 * described by "scan_converter".
 */
void              gimp_scan_convert_add_points (GimpScanConvert *scan_converter,
                                                guint            n_points,
                                                GimpVector2     *points,
                                                gboolean         new_polygon);


/* Scan convert the polygon described by the list of points passed to
 * scan_convert_add_points, and return a channel with a bits set if
 * they fall within the polygon defined.  The polygon is filled
 * according to the even-odd rule.  The polygon is closed by
 * joining the final point to the initial point.
 */
GimpChannel     * gimp_scan_convert_to_channel (GimpScanConvert *scan_converter,
                                                GimpImage       *gimage);


#endif /* __GIMP_SCAN_CONVERT_H__ */
