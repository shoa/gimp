/* The GIMP -- an image manipulation program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * Utitility functions for GimpConfig.
 * Copyright (C) 2001  Sven Neumann <sven@gimp.org>
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

#include <string.h>

#include <glib-object.h>

#include "libgimpbase/gimpenv.h"

#include "gimpconfig-utils.h"


/**
 * gimp_config_diff:
 * @a: a #GObject
 * @b: another #GObject of the same type as @a
 * @flags: a mask of GParamFlags
 * 
 * Compares all properties of @a and @b that have all @flags set. If
 * @flags is 0, all properties are compared.
 * 
 * Return value: a GList of differing GParamSpecs.
 **/
GList *
gimp_config_diff (GObject      *a,
                  GObject      *b,
                  GParamFlags   flags)
{
  GParamSpec **param_specs;
  guint        n_param_specs;
  gint         i;
  GList       *list = NULL;

  g_return_val_if_fail (G_IS_OBJECT (a), FALSE);
  g_return_val_if_fail (G_IS_OBJECT (b), FALSE);
  g_return_val_if_fail (G_TYPE_FROM_INSTANCE (a) == G_TYPE_FROM_INSTANCE (b),
                        FALSE);

  param_specs = g_object_class_list_properties (G_OBJECT_GET_CLASS (a),
                                                &n_param_specs);

  for (i = 0; i < n_param_specs; i++)
    {
      if (! flags || ((param_specs[i]->flags & flags) == flags))
        {
          GValue a_value = { 0, };
          GValue b_value = { 0, };

          g_value_init (&a_value, param_specs[i]->value_type);
          g_value_init (&b_value, param_specs[i]->value_type);

          g_object_get_property (a, param_specs[i]->name, &a_value);
          g_object_get_property (b, param_specs[i]->name, &b_value);

          if (g_param_values_cmp (param_specs[i], &a_value, &b_value))
            list = g_list_prepend (list, param_specs[i]);

          g_value_unset (&a_value);
          g_value_unset (&b_value);
        }
    }

  g_free (param_specs);

  return g_list_reverse (list);
}


static void
gimp_config_connect_notify (GObject    *src,
                            GParamSpec *param_spec,
                            GObject    *dest)
{
  GValue value = { 0, };

  g_value_init (&value, param_spec->value_type);

  g_object_get_property (src,  param_spec->name, &value);
  g_object_set_property (dest, param_spec->name, &value);

  g_value_unset (&value);
}

/**
 * gimp_config_connect:
 * @src: a #GObject
 * @dest: another #GObject of the same type as @src
 * 
 * Connects @dest with @src so that all property changes of @src are
 * applied to @dest using a "notify" handler.
 **/
void
gimp_config_connect (GObject *src,
                     GObject *dest)
{
  g_return_if_fail (G_IS_OBJECT (src));
  g_return_if_fail (G_IS_OBJECT (dest));
  g_return_if_fail (G_TYPE_FROM_INSTANCE (src) == G_TYPE_FROM_INSTANCE (dest));

  g_signal_connect_object (src, "notify",
                           G_CALLBACK (gimp_config_connect_notify),
                           dest, 0);
}

/**
 * gimp_config_disconnect:
 * @src: a #GObject
 * @dest: another #GObject of the same type as @src
 * 
 * Removes a connection between @dest and @src that was previously set
 * up using gimp_config_connect().
 **/
void
gimp_config_disconnect (GObject *src,
                        GObject *dest)
{
  g_return_if_fail (G_IS_OBJECT (src));
  g_return_if_fail (G_IS_OBJECT (dest));

  g_signal_handlers_disconnect_by_func (src,
                                        G_CALLBACK (gimp_config_connect_notify),
                                        dest);
}

/**
 * gimp_config_copy_properties:
 * @src: a #GObject
 * @dest: another #GObject of the same type as @src
 * 
 * Retrieves all read and writeable property settings from @src and
 * applies the values to @dest.
 **/
void
gimp_config_copy_properties (GObject *src,
                             GObject *dest)
{
  GObjectClass  *klass;
  GParamSpec   **property_specs;
  guint          n_property_specs;
  guint          i;

  g_return_if_fail (G_IS_OBJECT (src));
  g_return_if_fail (G_IS_OBJECT (dest));
  g_return_if_fail (G_TYPE_FROM_INSTANCE (src) == G_TYPE_FROM_INSTANCE (dest));

  klass = G_OBJECT_GET_CLASS (src);

  property_specs = g_object_class_list_properties (klass, &n_property_specs);

  if (!property_specs)
    return;

  for (i = 0; i < n_property_specs; i++)
    {
      GParamSpec *prop_spec;
 
      prop_spec = property_specs[i];

      if (prop_spec->flags & G_PARAM_READABLE &&
	  prop_spec->flags & G_PARAM_WRITABLE)
	{
	  GValue value = { 0, };

	  g_value_init (&value, prop_spec->value_type);
      
	  g_object_get_property (src,  prop_spec->name, &value);
	  g_object_set_property (dest, prop_spec->name, &value);
	  
	  g_value_unset (&value);
	}
    }

  g_free (property_specs);
}

/**
 * gimp_config_reset_properties:
 * @object: a #GObject
 * 
 * Resets all writable properties of @object to the default values as
 * defined in their #GParamSpec.
 **/
void
gimp_config_reset_properties (GObject *object)
{
  GObjectClass  *klass;
  GParamSpec   **property_specs;
  guint          n_property_specs;
  guint          i;

  g_return_if_fail (G_IS_OBJECT (object));
  
  klass = G_OBJECT_GET_CLASS (object);

  property_specs = g_object_class_list_properties (klass, &n_property_specs);

  if (!property_specs)
    return;

  for (i = 0; i < n_property_specs; i++)
    {
      GParamSpec *prop_spec;
 
      prop_spec = property_specs[i];

      if ((prop_spec->flags & G_PARAM_WRITABLE) &&
          ! G_IS_PARAM_SPEC_OBJECT (prop_spec))
	{
	  GValue value = { 0, };

	  g_value_init (&value, prop_spec->value_type);
      
	  g_param_value_set_default (prop_spec, &value);
	  g_object_set_property (object, prop_spec->name, &value);
	  
	  g_value_unset (&value);
	}
    }

  g_free (property_specs);
}


/*
 * GimpConfig path utilities
 */

gchar *
gimp_config_build_data_path (const gchar *name)
{
  return g_strconcat ("${gimp_dir}", G_DIR_SEPARATOR_S, name,
                      G_SEARCHPATH_SEPARATOR_S,
                      "${gimp_data_dir}", G_DIR_SEPARATOR_S, name,
                      NULL);
}

gchar * 
gimp_config_build_plug_in_path (const gchar *name)
{
  return g_strconcat ("${gimp_dir}", G_DIR_SEPARATOR_S, name,
                      G_SEARCHPATH_SEPARATOR_S,
                      "${gimp_plug_in_dir}", G_DIR_SEPARATOR_S, name,
                      NULL);
}
