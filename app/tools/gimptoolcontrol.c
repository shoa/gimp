/* The GIMP -- an image manipulation program
 * Copyright (C) 1995-2002 Spencer Kimball, Peter Mattis and others
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

#include "tools-types.h"

#include "gimptoolcontrol.h"


static void gimp_tool_control_class_init (GimpToolControlClass *klass);
static void gimp_tool_control_init (GimpToolControl *tool);

static GimpObjectClass *parent_class = NULL;

GType
gimp_tool_control_get_type (void)
{
  static GType tool_control_type = 0;

  if (! tool_control_type)
    {
      static const GTypeInfo tool_control_info =
      {
        sizeof (GimpToolControlClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) gimp_tool_control_class_init,
	NULL,           /* class_finalize */
	NULL,           /* class_data     */
	sizeof (GimpToolControl),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) gimp_tool_control_init,
      };

      tool_control_type = g_type_register_static (GIMP_TYPE_OBJECT,
		 			          "GimpToolControl", 
                                                 &tool_control_info, 0);
    }

  return tool_control_type;
}

static void
gimp_tool_control_class_init (GimpToolControlClass *klass)
{
  parent_class = g_type_class_peek_parent (klass);
}

static void
gimp_tool_control_init (GimpToolControl *control)
{
  control->state                  = INACTIVE;
  control->paused_count           = 0;

  control->toggled                = FALSE;

  control->scroll_lock            = FALSE;                  /*  Allow scrolling                  */
  control->auto_snap_to           = TRUE;                   /*  Snap to guides                   */
  control->preserve               = TRUE;                   /*  Preserve across drawable change  */
  control->handle_empty_image     = FALSE;                  /*  Require active drawable          */
  control->motion_mode            = GIMP_MOTION_MODE_HINT;  /*  Use MOTION_HINT compression      */

  control->cursor                 = GIMP_MOUSE_CURSOR;
  control->tool_cursor            = GIMP_TOOL_CURSOR_NONE;
  control->cursor_modifier        = GIMP_CURSOR_MODIFIER_NONE;

  control->toggle_cursor          = GIMP_MOUSE_CURSOR;
  control->toggle_tool_cursor     = GIMP_TOOL_CURSOR_NONE;
  control->toggle_cursor_modifier = GIMP_CURSOR_MODIFIER_NONE;
}

void
gimp_tool_control_pause            (GimpToolControl   *control)
{
  g_return_if_fail(GIMP_IS_TOOL_CONTROL(control));

  /* control->state = PAUSED ? */
  control->paused_count++;
}

void
gimp_tool_control_resume           (GimpToolControl   *control)
{
  g_return_if_fail(GIMP_IS_TOOL_CONTROL(control));

  control->paused_count--;
}

gboolean
gimp_tool_control_is_paused    (GimpToolControl   *control)
{
  g_return_val_if_fail(GIMP_IS_TOOL_CONTROL(control), FALSE);

  return control->paused_count > 0;
}

void 
gimp_tool_control_activate         (GimpToolControl   *control)
{
  g_return_if_fail(GIMP_IS_TOOL_CONTROL(control));

  control->state = ACTIVE; 
}

void 
gimp_tool_control_halt             (GimpToolControl   *control)
{
  g_return_if_fail(GIMP_IS_TOOL_CONTROL(control));

  control->state = INACTIVE; 
  control->paused_count = 0; 
  
}

gboolean
gimp_tool_control_is_active    (GimpToolControl   *control)
{
  g_return_val_if_fail(GIMP_IS_TOOL_CONTROL(control), FALSE);

  return control->state == ACTIVE;
}

void
gimp_tool_control_set_toggle       (GimpToolControl   *control,
                                    gboolean           toggled) 
{
  g_return_if_fail(GIMP_IS_TOOL_CONTROL(control));

  control->toggled = toggled; 
}

void
gimp_tool_control_set_handles_empty_image (GimpToolControl   *control,
                                           gboolean           handle_empty) 
{
  g_return_if_fail(GIMP_IS_TOOL_CONTROL(control));

  control->handle_empty_image = handle_empty; 
}

void
gimp_tool_control_set_snap_to      (GimpToolControl   *control,
                                    gboolean           snap_to) 
{
  g_return_if_fail(GIMP_IS_TOOL_CONTROL(control));

  control->auto_snap_to = snap_to; 
}

void
gimp_tool_control_set_motion_mode  (GimpToolControl   *control,
                                    GimpMotionMode     motion_mode)
{
  g_return_if_fail(GIMP_IS_TOOL_CONTROL(control));

  control->motion_mode = motion_mode;
}



void
gimp_tool_control_set_preserve     (GimpToolControl   *control,
                                    gboolean           preserve) 
{
  g_return_if_fail(GIMP_IS_TOOL_CONTROL(control));

  control->preserve = preserve; 
}

void
gimp_tool_control_set_cursor       (GimpToolControl   *control,
                                    GdkCursorType      cursor) 
{
  g_return_if_fail(GIMP_IS_TOOL_CONTROL(control));

  control->cursor = cursor; 
}

void
gimp_tool_control_set_tool_cursor  (GimpToolControl      *control,
                                    GimpToolCursorType    cursor) 
{
  g_return_if_fail(GIMP_IS_TOOL_CONTROL(control));

  control->tool_cursor = cursor; 
}
void
gimp_tool_control_set_cursor_modifier (GimpToolControl     *control,
                                       GimpCursorModifier   cmodifier) 
{
  g_return_if_fail(GIMP_IS_TOOL_CONTROL(control));

  control->cursor_modifier = cmodifier; 
}




void
gimp_tool_control_set_toggle_cursor       (GimpToolControl   *control,
                                           GdkCursorType      cursor) 
{
  g_return_if_fail(GIMP_IS_TOOL_CONTROL(control));

  control->toggle_cursor = cursor; 
}

void
gimp_tool_control_set_toggle_tool_cursor  (GimpToolControl      *control,
                                           GimpToolCursorType    cursor) 
{
  g_return_if_fail(GIMP_IS_TOOL_CONTROL(control));

  control->toggle_tool_cursor = cursor; 
}

void
gimp_tool_control_set_toggle_cursor_modifier (GimpToolControl     *control,
                                              GimpCursorModifier   cmodifier) 
{
  g_return_if_fail(GIMP_IS_TOOL_CONTROL(control));

  control->toggle_cursor_modifier = cmodifier; 
}


void
gimp_tool_control_set_scroll_lock            (GimpToolControl     *control,
                                              gboolean             scroll_lock)
{
  g_return_if_fail(GIMP_IS_TOOL_CONTROL(control));

  control->scroll_lock = scroll_lock; 
}

GimpMotionMode
gimp_tool_control_motion_mode            (GimpToolControl   *control)
{
  g_return_val_if_fail(GIMP_IS_TOOL_CONTROL(control), GIMP_MOTION_MODE_HINT);

  return control->motion_mode;
}

gboolean
gimp_tool_control_handles_empty_image    (GimpToolControl   *control)
{
  g_return_val_if_fail(GIMP_IS_TOOL_CONTROL(control), FALSE);

  return control->handle_empty_image;
}

gboolean
gimp_tool_control_auto_snap_to           (GimpToolControl   *control)
{
  g_return_val_if_fail(GIMP_IS_TOOL_CONTROL(control), FALSE);

  return control->auto_snap_to;
}

gboolean
gimp_tool_control_preserve               (GimpToolControl   *control)
{
  g_return_val_if_fail(GIMP_IS_TOOL_CONTROL(control), FALSE);

  return control->preserve;
}

gboolean
gimp_tool_control_scroll_lock            (GimpToolControl   *control)
{
  g_return_val_if_fail(GIMP_IS_TOOL_CONTROL(control), FALSE);

  return control->scroll_lock;
}

gboolean
gimp_tool_control_is_toggled             (GimpToolControl   *control)
{
  g_return_val_if_fail(GIMP_IS_TOOL_CONTROL(control), FALSE);

  return control->toggled;
}

GdkCursorType 
gimp_tool_control_get_cursor  (GimpToolControl   *control)
{
  g_return_val_if_fail(GIMP_IS_TOOL_CONTROL(control), FALSE);

  return control->cursor;
}

GdkCursorType 
gimp_tool_control_get_toggle_cursor  (GimpToolControl   *control)
{
  g_return_val_if_fail(GIMP_IS_TOOL_CONTROL(control), FALSE);

  return control->toggle_cursor;
}

GimpToolCursorType 
gimp_tool_control_get_tool_cursor  (GimpToolControl   *control)
{
  g_return_val_if_fail(GIMP_IS_TOOL_CONTROL(control), FALSE);

  return control->tool_cursor;
}

GimpToolCursorType 
gimp_tool_control_get_toggle_tool_cursor  (GimpToolControl   *control)
{
  g_return_val_if_fail(GIMP_IS_TOOL_CONTROL(control), FALSE);

  return control->toggle_tool_cursor;
}

GimpCursorModifier 
gimp_tool_control_get_cursor_modifier  (GimpToolControl   *control)
{
  g_return_val_if_fail(GIMP_IS_TOOL_CONTROL(control), FALSE);

  return control->cursor_modifier;
}

GimpCursorModifier 
gimp_tool_control_get_toggle_cursor_modifier  (GimpToolControl   *control)
{
  g_return_val_if_fail(GIMP_IS_TOOL_CONTROL(control), FALSE);

  return control->toggle_cursor_modifier;
}
