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

#ifndef __GIMP_DODGE_BURN_TOOL_H__
#define __GIMP_DODGE_BURN_TOOL_H__


#include "gimppainttool.h"


#define GIMP_TYPE_DODGE_BURN_TOOL            (gimp_dodge_burn_tool_get_type ())
#define GIMP_DODGE_BURN_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_DODGE_BURN_TOOL, GimpDodgeBurnTool))
#define GIMP_IS_DODGE_BURN_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIMP_TYPE_DODGE_BURN_TOOL))
#define GIMP_DODGE_BURN_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_DODGE_BURN_TOOL, GimpDodgeBurnToolClass))
#define GIMP_IS_DODGE_BURN_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_DODGE_BURN_TOOL))


typedef struct _GimpDodgeBurnTool      GimpDodgeBurnTool;
typedef struct _GimpDodgeBurnToolClass GimpDodgeBurnToolClass;

struct _GimpDodgeBurnTool
{
  GimpPaintTool parent_instance;
};

struct _GimpDodgeBurnToolClass
{
  GimpPaintToolClass parent_class;
};


void    gimp_dodge_burn_tool_register (GimpToolRegisterCallback  callback,
                                       gpointer                  data);

GType   gimp_dodge_burn_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __GIMP_DODGEBURN_TOOL_H__  */
