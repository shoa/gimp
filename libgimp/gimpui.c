/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
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

#include "config.h"

#include <stdlib.h>

#include <gtk/gtk.h>

#include "libgimpmodule/gimpmodule.h"

#include "gimp.h"
#include "gimpui.h"

#include "libgimpwidgets/gimpwidgets.h"
#include "libgimpwidgets/gimpwidgets-private.h"


/*  local function prototypes  */

static void  gimp_ui_help_func              (const gchar *help_id,
                                             gpointer     help_data);
static void  gimp_ensure_modules            (void);
static void  gimp_window_transient_realized (GtkWidget   *window,
                                             GdkWindow   *parent);


static gboolean gimp_ui_initialized = FALSE;


/*  public functions  */

/**
 * gimp_ui_init:
 * @prog_name: The name of the plug-in which will be passed as argv[0] to
 *             gtk_init(). It's a convention to use the name of the
 *             executable and _not_ the PDB procedure name or something.
 * @preview:   This parameter is unused and exists for historical
 *             reasons only.
 *
 * This function initializes GTK+ with gtk_init() and initializes GDK's
 * image rendering subsystem (GdkRGB) to follow the GIMP main program's
 * colormap allocation/installation policy.
 *
 * The GIMP's colormap policy can be determinded by the user with the
 * gimprc variables @min_colors and @install_cmap.
 **/
void
gimp_ui_init (const gchar *prog_name,
	      gboolean     preview)
{
  const gchar  *display_name;
  gint          argc;
  gchar       **argv;
  gchar        *themerc;
  GdkScreen    *screen;

  g_return_if_fail (prog_name != NULL);

  if (gimp_ui_initialized)
    return;

  display_name = gimp_display_name ();

  if (display_name)
    {
      const gchar *var_name = NULL;

#if defined (GDK_WINDOWING_X11)
      var_name = "DISPLAY";
#elif defined (GDK_WINDOWING_DIRECTFB) || defined (GDK_WINDOWING_FB)
      var_name = "GDK_DISPLAY";
#endif

      if (var_name)
        putenv (g_strdup_printf ("%s=%s", var_name, display_name));
    }

  argc    = 2;
  argv    = g_new (gchar *, 2);
  argv[0] = g_strdup (prog_name);
  argv[1] = g_strdup_printf ("--class=%s", gimp_wm_class ());

  gtk_init (&argc, &argv);

  themerc = gimp_personal_rc_file ("themerc");
  gtk_rc_add_default_file (themerc);
  g_free (themerc);

  gdk_rgb_set_min_colors (gimp_min_colors ());
  gdk_rgb_set_install (gimp_install_cmap ());

  screen = gdk_screen_get_default ();
  gtk_widget_set_default_colormap (gdk_screen_get_rgb_colormap (screen));

  gimp_widgets_init (gimp_ui_help_func,
                     gimp_context_get_foreground,
                     gimp_context_get_background,
                     gimp_ensure_modules);

  if (! gimp_show_tool_tips ())
    gimp_help_disable_tooltips ();

  gimp_dialogs_show_help_button (gimp_show_help_button ());

  gimp_ui_initialized = TRUE;
}

/**
 * gimp_ui_get_display_window:
 *
 * Returns the #GdkWindow of a display window. The purpose is to allow
 * to make plug-in dialogs transient to the image display as explained
 * with gdk_window_set_transient_for().
 *
 * You shouldn't have to call this function directly. Use
 * gimp_window_set_transient_for_display() instead.
 *
 * Return value: A reference to a #GdkWindow or %NULL. You should
 *               unref the window using g_object_unref() as soon as
 *               you don't need it any longer.
 *
 * Since: GIMP 2.4
 */
GdkWindow *
gimp_ui_get_display_window (guint32 gdisp_ID)
{
  GdkNativeWindow  window;

  g_return_val_if_fail (gimp_ui_initialized, NULL);

#ifndef GDK_NATIVE_WINDOW_POINTER
  window = gimp_display_get_window_handle (gdisp_ID);
  if (window)
    return gdk_window_foreign_new_for_display (gdk_display_get_default (),
                                               window);
#endif

  return NULL;
}

/**
 * gimp_window_set_transient_for_display:
 * @window:   the #GtkWindow that should become transient
 * @gdisp_ID: display ID of the image window that should become the parent
 *
 * Indicates to the window manager that @window is a transient dialog
 * associated with the GIMP image window that is identified by it's
 * display ID.  See gdk_window_set_transient_for () for more information.
 *
 * Most of the time you will want to use the convenience function
 * gimp_window_set_transient_for_default_display().
 *
 * Since: GIMP 2.4
 */
void
gimp_window_set_transient_for_display (GtkWindow *window,
                                       guint32    gdisp_ID)
{
  GdkWindow *display;

  g_return_if_fail (gimp_ui_initialized);
  g_return_if_fail (GTK_IS_WINDOW (window));

  gtk_window_set_transient_for (window, NULL);

  g_signal_handlers_disconnect_matched (window, G_SIGNAL_MATCH_FUNC,
                                        0, 0, NULL,
                                        gimp_window_transient_realized,
                                        NULL);

  display = gimp_ui_get_display_window (gdisp_ID);
  if (! display)
    return;

  if (GTK_WIDGET_REALIZED (window))
    gdk_window_set_transient_for (GTK_WIDGET (window)->window, display);

  g_signal_connect_object (window, "realize",
                           G_CALLBACK (gimp_window_transient_realized),
                           display, 0);
  g_object_unref (display);
}

/**
 * gimp_window_set_transient_for_default_display:
 * @window: the #GtkWindow that should become transient
 *
 * Indicates to the window manager that @window is a transient dialog
 * associated with the GIMP image window that the plug-in has been
 * started from. See also gimp_window_set_transient_for_display().
 *
 * Since: GIMP 2.4
 */
void
gimp_window_set_transient_for_default_display (GtkWindow *window)
{
  g_return_if_fail (gimp_ui_initialized);
  g_return_if_fail (GTK_IS_WINDOW (window));

  gimp_window_set_transient_for_display (window, gimp_default_display ());
}


/*  private functions  */

static void
gimp_ui_help_func (const gchar *help_id,
                   gpointer     help_data)
{
  gimp_help (NULL, help_id);
}

static void
gimp_ensure_modules (void)
{
  static GimpModuleDB *module_db = NULL;

  if (! module_db)
    {
      gchar *load_inhibit = gimp_get_module_load_inhibit ();
      gchar *module_path  = gimp_gimprc_query ("module-path");

      module_db = gimp_module_db_new (FALSE);

      gimp_module_db_set_load_inhibit (module_db, load_inhibit);
      gimp_module_db_load (module_db, module_path);

      g_free (module_path);
      g_free (load_inhibit);
    }
}

static void
gimp_window_transient_realized (GtkWidget *window,
                                GdkWindow *parent)
{
  if (GTK_WIDGET_REALIZED (window))
    gdk_window_set_transient_for (window->window, parent);
}
