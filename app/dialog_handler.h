/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 * Copyright (C) 1999 Andy Thomas
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
#ifndef __DIALOG_HANDLER_H_
#define __DIALOG_HANDLER_H_

#ifndef __DIALOG_HANDLER_C_
/* Bypass these declarations when compiling the file with the definitions,
 * because of the G_MODULE_EXPORT attribute in it.
 */
void dialog_register          (GtkWidget *dialog);
void dialog_unregister        (GtkWidget *dialog);
#endif

void dialog_register_toolbox  (GtkWidget *dialog);
void dialog_register_fileload (GtkWidget *dialog);

void dialog_toggle            (void);
void dialog_idle_all          (void);
void dialog_unidle_all        (void);
void dialog_show_toolbox      (void);

#endif  /* __DIALOG_HANDLER_H_ */
