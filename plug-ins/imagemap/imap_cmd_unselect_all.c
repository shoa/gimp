/*
 * This is a plug-in for the GIMP.
 *
 * Generates clickable image maps.
 *
 * Copyright (C) 1998-1999 Maurits Rijk  lpeek.mrijk@consunet.nl
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
 *
 */

#include "imap_cmd_unselect.h"
#include "imap_cmd_unselect_all.h"
#include "libgimp/stdplugins-intl.h"
#include "imap_main.h"

COMMAND_PROTO(unselect_all_command);

static CommandClass_t unselect_all_command_class = {
   unselect_all_command_destruct,
   unselect_all_command_execute,
   unselect_all_command_undo,
   unselect_all_command_redo
};

typedef struct {
   Command_t parent;
   ObjectList_t *list;
   Object_t *exception;
} UnselectAllCommand_t;

Command_t* 
unselect_all_command_new(ObjectList_t *list, Object_t *exception)
{
   UnselectAllCommand_t *command = g_new(UnselectAllCommand_t, 1);
   command->list = list;
   command->exception = (exception) ? object_ref(exception) : exception;
   return command_init(&command->parent, _("Unselect All"), 
		       &unselect_all_command_class);
}

static void
unselect_all_command_destruct(Command_t *parent)
{
   UnselectAllCommand_t *command = (UnselectAllCommand_t*) parent;
   if (command->exception)
      object_unref(command->exception);
}

static void
select_one_object(Object_t *obj, gpointer data)
{
   UnselectAllCommand_t *command = (UnselectAllCommand_t*) data;
   command_add_subcommand(&command->parent, unselect_command_new(obj));
}

static CmdExecuteValue_t
unselect_all_command_execute(Command_t *parent)
{
   UnselectAllCommand_t *command = (UnselectAllCommand_t*) parent;
   gpointer id;
   CmdExecuteValue_t rvalue;

   id = object_list_add_select_cb(command->list, select_one_object,
				  command);
   if (object_list_deselect_all(command->list, command->exception)) {
      redraw_preview();		/* Fix me! */
      rvalue = CMD_APPEND;
   } else {
      rvalue = CMD_DESTRUCT;
   }
   object_list_remove_select_cb(command->list, id);
   return rvalue;
}

static void
unselect_all_command_undo(Command_t *command)
{
   redraw_preview();		/* Fix me! */
}

static void
unselect_all_command_redo(Command_t *command)
{
   redraw_preview();		/* Fix me! */
}
