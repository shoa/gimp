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

#ifndef __MENUS_H__
#define __MENUS_H__


#define MENU_SEPARATOR(path) \
        { { (path), NULL, NULL, 0, "<Separator>" }, NULL, NULL, NULL }

#define MENU_BRANCH(path) \
        { { (path), NULL, NULL, 0, "<Branch>" }, NULL, NULL, NULL }


extern GimpMenuFactory *global_menu_factory;


void   menus_init                   (Gimp            *gimp);
void   menus_exit                   (Gimp            *gimp);
void   menus_restore                (Gimp            *gimp);
void   menus_save                   (Gimp            *gimp);
void   menus_clear                  (Gimp            *gimp);

void   menus_open_recent_add        (GimpUIManager   *manager,
                                     const gchar     *ui_path);

void   menus_last_opened_add        (GimpItemFactory *item_factory);
void   menus_filters_subdirs_to_top (GtkMenu         *menu);


#endif /* __MENUS_H__ */
