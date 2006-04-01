
/* Generated data (by gimp-mkenums) */

#include "config.h"
#include <glib-object.h>
#include "libgimpbase/gimpbase.h"
#include "plug-in-enums.h"
#include "gimp-intl.h"

/* enumerations from "./plug-in-enums.h" */
GType
gimp_run_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { GIMP_RUN_INTERACTIVE, "GIMP_RUN_INTERACTIVE", "interactive" },
    { GIMP_RUN_NONINTERACTIVE, "GIMP_RUN_NONINTERACTIVE", "noninteractive" },
    { GIMP_RUN_WITH_LAST_VALS, "GIMP_RUN_WITH_LAST_VALS", "with-last-vals" },
    { 0, NULL, NULL }
  };

  static const GimpEnumDesc descs[] =
  {
    { GIMP_RUN_INTERACTIVE, "GIMP_RUN_INTERACTIVE", NULL },
    { GIMP_RUN_NONINTERACTIVE, "GIMP_RUN_NONINTERACTIVE", NULL },
    { GIMP_RUN_WITH_LAST_VALS, "GIMP_RUN_WITH_LAST_VALS", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (! type)
    {
      type = g_enum_register_static ("GimpRunMode", values);
      gimp_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
plug_in_image_type_get_type (void)
{
  static const GFlagsValue values[] =
  {
    { PLUG_IN_RGB_IMAGE, "PLUG_IN_RGB_IMAGE", "rgb-image" },
    { PLUG_IN_GRAY_IMAGE, "PLUG_IN_GRAY_IMAGE", "gray-image" },
    { PLUG_IN_INDEXED_IMAGE, "PLUG_IN_INDEXED_IMAGE", "indexed-image" },
    { PLUG_IN_RGBA_IMAGE, "PLUG_IN_RGBA_IMAGE", "rgba-image" },
    { PLUG_IN_GRAYA_IMAGE, "PLUG_IN_GRAYA_IMAGE", "graya-image" },
    { PLUG_IN_INDEXEDA_IMAGE, "PLUG_IN_INDEXEDA_IMAGE", "indexeda-image" },
    { 0, NULL, NULL }
  };

  static const GimpFlagsDesc descs[] =
  {
    { PLUG_IN_RGB_IMAGE, "PLUG_IN_RGB_IMAGE", NULL },
    { PLUG_IN_GRAY_IMAGE, "PLUG_IN_GRAY_IMAGE", NULL },
    { PLUG_IN_INDEXED_IMAGE, "PLUG_IN_INDEXED_IMAGE", NULL },
    { PLUG_IN_RGBA_IMAGE, "PLUG_IN_RGBA_IMAGE", NULL },
    { PLUG_IN_GRAYA_IMAGE, "PLUG_IN_GRAYA_IMAGE", NULL },
    { PLUG_IN_INDEXEDA_IMAGE, "PLUG_IN_INDEXEDA_IMAGE", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (! type)
    {
      type = g_flags_register_static ("PlugInImageType", values);
      gimp_flags_set_value_descriptions (type, descs);
    }

  return type;
}


/* Generated data ends here */
