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
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <string.h>

#include "gimp.h"
#include "gimpui.h"

#define MENU_THUMBNAIL_WIDTH   24
#define MENU_THUMBNAIL_HEIGHT  24

/* Copy data from temp_PDB call */
struct _GimpBrushData 
{
  gboolean           busy;
  gchar             *bname;
  gdouble            opacity;
  gint               spacing;
  gint               paint_mode;
  gint               width;
  gint               height;
  gchar             *brush_mask_data;
  GRunBrushCallback  callback;
  gboolean           closing;
  gpointer           data;
};

typedef struct _GimpBrushData GimpBrushData;

/* Copy data from temp_PDB call */
struct _GimpPatternData 
{
  gboolean             busy;
  gchar               *pname;
  gint                 width;
  gint                 height;
  gint                 bytes;
  gchar               *pattern_mask_data;
  GRunPatternCallback  callback;
  gboolean             closing;
  gpointer             data;
};

typedef struct _GimpPatternData GimpPatternData;

/* Copy data from temp_PDB call */
struct _GimpGradientData 
{
  gboolean              busy;
  gchar                *gname;
  gint                  width;
  gdouble              *gradient_data;
  GRunGradientCallback  callback;
  gboolean              closing;
  gpointer              data;
};

typedef struct _GimpGradientData GimpGradientData;

static void     gimp_menu_callback      (GtkWidget        *widget,
					 gint32           *id);
static void     do_brush_callback       (GimpBrushData       *bdata);
static gint     idle_test_brush         (GimpBrushData       *bdata);
static gint     idle_test_pattern       (GimpPatternData     *pdata);
static gint     idle_test_gradient      (GimpGradientData    *gdata);
static void     temp_brush_invoker      (gchar            *name,
					 gint              nparams,
					 GParam           *param,
					 gint             *nreturn_vals,
					 GParam          **return_vals);
static gboolean input_callback	        (GIOChannel       *channel,
					 GIOCondition      condition,
					 gpointer          data);
static void     gimp_setup_callbacks    (void);
static gchar*   gen_temp_plugin_name    (void);
static void     fill_preview_with_thumb (GtkWidget        *widget,
					 gint32            drawable_ID,
					 gint              width,
					 gint              height);
/* From gimp.c */
void gimp_run_temp (void);

static GHashTable *gbrush_ht = NULL;
static GHashTable *gpattern_ht = NULL;
static GHashTable *ggradient_ht = NULL;
static GimpBrushData *active_brush_pdb = NULL;
static GimpPatternData *active_pattern_pdb = NULL;
static GimpGradientData *active_gradient_pdb = NULL;

GtkWidget*
gimp_image_menu_new (GimpConstraintFunc constraint,
		     GimpMenuCallback   callback,
		     gpointer           data,
		     gint32             active_image)
{
  GtkWidget *menu;
  GtkWidget *menuitem;
  char *filename;
  char *label;
  gint32 *images;
  int nimages;
  int i, k;

  menu = gtk_menu_new ();
  gtk_object_set_user_data (GTK_OBJECT (menu), (gpointer) callback);
  gtk_object_set_data (GTK_OBJECT (menu), "gimp_callback_data", data);

  images = gimp_query_images (&nimages);
  for (i = 0, k = 0; i < nimages; i++)
    if (!constraint || (* constraint) (images[i], -1, data))
      {
	filename = gimp_image_get_filename (images[i]);
	label = g_new (char, strlen (filename) + 16);
	sprintf (label, "%s-%d", g_basename (filename), images[i]);
	g_free (filename);

	menuitem = gtk_menu_item_new_with_label (label);
	gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			    (GtkSignalFunc) gimp_menu_callback,
			    &images[i]);
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);

	g_free (label);

	if (images[i] == active_image)
	  gtk_menu_set_active (GTK_MENU (menu), k);

	k += 1;
      }

  if (k == 0)
    {
      menuitem = gtk_menu_item_new_with_label ("none");
      gtk_widget_set_sensitive (menuitem, FALSE);
      gtk_menu_append (GTK_MENU (menu), menuitem);
      gtk_widget_show (menuitem);
    }

  if (images)
    {
      if (active_image == -1)
	active_image = images[0];
      (* callback) (active_image, data);
    }

  return menu;
}



GtkWidget*
gimp_layer_menu_new (GimpConstraintFunc constraint,
		     GimpMenuCallback   callback,
		     gpointer           data,
		     gint32             active_layer)
{
  GtkWidget *menu;
  GtkWidget *menuitem;
  char *name;
  char *image_label;
  char *label;
  gint32 *images;
  gint32 *layers;
  gint32 layer;
  int nimages;
  int nlayers;
  int i, j, k;

  menu = gtk_menu_new ();
  gtk_object_set_user_data (GTK_OBJECT (menu), (gpointer) callback);
  gtk_object_set_data (GTK_OBJECT (menu), "gimp_callback_data", data);

  layer = -1;

  images = gimp_query_images (&nimages);
  for (i = 0, k = 0; i < nimages; i++)
    if (!constraint || (* constraint) (images[i], -1, data))
      {
	name = gimp_image_get_filename (images[i]);
	image_label = g_new (char, strlen (name) + 16);
	sprintf (image_label, "%s-%d", g_basename (name), images[i]);
	g_free (name);

	layers = gimp_image_get_layers (images[i], &nlayers);
	for (j = 0; j < nlayers; j++)
	  if (!constraint || (* constraint) (images[i], layers[j], data))
	    {
	      GtkWidget *hbox;
	      GtkWidget *vbox;
	      GtkWidget *wcolor_box;
	      GtkWidget *wlabel;

	      name = gimp_layer_get_name (layers[j]);
	      label = g_new (char, strlen (image_label) + strlen (name) + 2);
	      sprintf (label, "%s/%s", image_label, name);
	      g_free (name);

	      menuitem = gtk_menu_item_new();
	      gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
				  (GtkSignalFunc) gimp_menu_callback,
				  &layers[j]);

	      hbox = gtk_hbox_new(FALSE, 0);
	      gtk_container_add(GTK_CONTAINER(menuitem), hbox);
	      gtk_widget_show(hbox);

	      vbox = gtk_vbox_new(FALSE, 0);
	      gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	      gtk_widget_show(vbox);
	      
	      wcolor_box = gtk_preview_new(GTK_PREVIEW_COLOR);
	      gtk_preview_set_dither (GTK_PREVIEW (wcolor_box), GDK_RGB_DITHER_MAX);

	      fill_preview_with_thumb(wcolor_box,
				      layers[j],
				      MENU_THUMBNAIL_WIDTH,
				      MENU_THUMBNAIL_HEIGHT);

	      gtk_widget_set_usize( GTK_WIDGET (wcolor_box) , 
				    MENU_THUMBNAIL_WIDTH , 
				    MENU_THUMBNAIL_HEIGHT);

	      gtk_container_add(GTK_CONTAINER(vbox), wcolor_box);
	      gtk_widget_show(wcolor_box);

	      wlabel = gtk_label_new(label);
	      gtk_misc_set_alignment(GTK_MISC(wlabel), 0.0, 0.5);
	      gtk_box_pack_start(GTK_BOX(hbox), wlabel, TRUE, TRUE, 4);
	      gtk_widget_show(wlabel);

	      gtk_menu_append (GTK_MENU (menu), menuitem);
	      gtk_widget_show (menuitem);

	      g_free (label);

	      if (layers[j] == active_layer)
		{
		  layer = active_layer;
		  gtk_menu_set_active (GTK_MENU (menu), k);
		}
	      else if (layer == -1)
		layer = layers[j];

	      k += 1;
	    }

	g_free (image_label);
      }
  g_free (images);

  if (k == 0)
    {
      menuitem = gtk_menu_item_new_with_label ("none");
      gtk_widget_set_sensitive (menuitem, FALSE);
      gtk_menu_append (GTK_MENU (menu), menuitem);
      gtk_widget_show (menuitem);
    }

  if (layer != -1)
    (* callback) (layer, data);

  return menu;
}

GtkWidget*
gimp_channel_menu_new (GimpConstraintFunc constraint,
		       GimpMenuCallback   callback,
		       gpointer           data,
		       gint32             active_channel)
{
  GtkWidget *menu;
  GtkWidget *menuitem;
  char *name;
  char *image_label;
  char *label;
  gint32 *images;
  gint32 *channels;
  gint32 channel;
  int nimages;
  int nchannels;
  int i, j, k;

  menu = gtk_menu_new ();
  gtk_object_set_user_data (GTK_OBJECT (menu), (gpointer) callback);
  gtk_object_set_data (GTK_OBJECT (menu), "gimp_callback_data", data);

  channel = -1;

  images = gimp_query_images (&nimages);
  for (i = 0, k = 0; i < nimages; i++)
    if (!constraint || (* constraint) (images[i], -1, data))
      {
	name = gimp_image_get_filename (images[i]);
	image_label = g_new (char, strlen (name) + 16);
	sprintf (image_label, "%s-%d", g_basename (name), images[i]);
	g_free (name);

	channels = gimp_image_get_channels (images[i], &nchannels);
	for (j = 0; j < nchannels; j++)
	  if (!constraint || (* constraint) (images[i], channels[j], data))
	    {
	      GtkWidget *hbox;
	      GtkWidget *vbox;
	      GtkWidget *wcolor_box;
	      GtkWidget *wlabel;

	      name = gimp_channel_get_name (channels[j]);
	      label = g_new (char, strlen (image_label) + strlen (name) + 2);
	      sprintf (label, "%s/%s", image_label, name);
	      g_free (name);

	      menuitem = gtk_menu_item_new();
	      gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
				  (GtkSignalFunc) gimp_menu_callback,
				  &channels[j]);
	      
	      hbox = gtk_hbox_new(FALSE, 0);
	      gtk_container_add(GTK_CONTAINER(menuitem), hbox);
	      gtk_widget_show(hbox);

	      vbox = gtk_vbox_new(FALSE, 0);
	      gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	      gtk_widget_show(vbox);
	      
	      wcolor_box = gtk_preview_new(GTK_PREVIEW_COLOR);
	      gtk_preview_set_dither (GTK_PREVIEW (wcolor_box), GDK_RGB_DITHER_MAX);

 	      fill_preview_with_thumb(wcolor_box, 
 				      channels[j], 
 				      MENU_THUMBNAIL_WIDTH, 
 				      MENU_THUMBNAIL_HEIGHT); 

	      gtk_widget_set_usize( GTK_WIDGET (wcolor_box) , 
				    MENU_THUMBNAIL_WIDTH , 
				    MENU_THUMBNAIL_HEIGHT);

	      gtk_container_add(GTK_CONTAINER(vbox), wcolor_box);
	      gtk_widget_show(wcolor_box);

	      wlabel = gtk_label_new(label);
	      gtk_misc_set_alignment(GTK_MISC(wlabel), 0.0, 0.5);
	      gtk_box_pack_start(GTK_BOX(hbox), wlabel, TRUE, TRUE, 4);
	      gtk_widget_show(wlabel);

	      gtk_menu_append (GTK_MENU (menu), menuitem);
	      gtk_widget_show (menuitem);

	      g_free (label);

	      if (channels[j] == active_channel)
		{
		  channel = active_channel;
		  gtk_menu_set_active (GTK_MENU (menu), k);
		}
	      else if (channel == -1)
		channel = channels[j];

	      k += 1;
	    }

	g_free (image_label);
      }
  g_free (images);

  if (k == 0)
    {
      menuitem = gtk_menu_item_new_with_label ("none");
      gtk_widget_set_sensitive (menuitem, FALSE);
      gtk_menu_append (GTK_MENU (menu), menuitem);
      gtk_widget_show (menuitem);
    }

  if (channel != -1)
    (* callback) (channel, data);

  return menu;
}

GtkWidget*
gimp_drawable_menu_new (GimpConstraintFunc constraint,
			GimpMenuCallback   callback,
			gpointer           data,
			gint32             active_drawable)
{
  GtkWidget *menu;
  GtkWidget *menuitem;
  gchar  *name;
  gchar  *image_label;
  gchar  *label;
  gint32 *images;
  gint32 *layers;
  gint32 *channels;
  gint32  drawable;
  gint    nimages;
  gint    nlayers;
  gint    nchannels;
  gint    i, j, k;

  menu = gtk_menu_new ();
  gtk_object_set_user_data (GTK_OBJECT (menu), (gpointer) callback);
  gtk_object_set_data (GTK_OBJECT (menu), "gimp_callback_data", data);

  drawable = -1;

  images = gimp_query_images (&nimages);

  for (i = 0, k = 0; i < nimages; i++)
    if (!constraint || (* constraint) (images[i], -1, data))
      {
	name = gimp_image_get_filename (images[i]);
	image_label = g_new (char, strlen (name) + 16);
	sprintf (image_label, "%s-%d", g_basename (name), images[i]);
	g_free (name);

	layers = gimp_image_get_layers (images[i], &nlayers);

	for (j = 0; j < nlayers; j++)
	  if (!constraint || (* constraint) (images[i], layers[j], data))
	    {
	      GtkWidget *hbox;
	      GtkWidget *vbox;
	      GtkWidget *wcolor_box;
	      GtkWidget *wlabel;

	      name = gimp_layer_get_name (layers[j]);
	      label = g_new (gchar, strlen (image_label) + strlen (name) + 2);
	      sprintf (label, "%s/%s", image_label, name);
	      g_free (name);

	      menuitem = gtk_menu_item_new ();
	      gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
				  (GtkSignalFunc) gimp_menu_callback,
				  &layers[j]);

	      hbox = gtk_hbox_new (FALSE, 0);
	      gtk_container_add (GTK_CONTAINER(menuitem), hbox);
	      gtk_widget_show (hbox);

	      vbox = gtk_vbox_new (FALSE, 0);
	      gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
	      gtk_widget_show (vbox);
	      
	      wcolor_box = gtk_preview_new (GTK_PREVIEW_COLOR);
	      gtk_preview_set_dither (GTK_PREVIEW (wcolor_box), GDK_RGB_DITHER_MAX);

	      fill_preview_with_thumb (wcolor_box,
				       layers[j],
				       MENU_THUMBNAIL_WIDTH,
				       MENU_THUMBNAIL_HEIGHT);

	      gtk_widget_set_usize (GTK_WIDGET (wcolor_box), 
				    MENU_THUMBNAIL_WIDTH , 
				    MENU_THUMBNAIL_HEIGHT);

	      gtk_container_add (GTK_CONTAINER(vbox), wcolor_box);
	      gtk_widget_show (wcolor_box);

	      wlabel = gtk_label_new (label);
	      gtk_misc_set_alignment (GTK_MISC (wlabel), 0.0, 0.5);
	      gtk_box_pack_start (GTK_BOX (hbox), wlabel, TRUE, TRUE, 4);
	      gtk_widget_show (wlabel);

	      gtk_menu_append (GTK_MENU (menu), menuitem);
	      gtk_widget_show (menuitem);

	      g_free (label);

	      if (layers[j] == active_drawable)
		{
		  drawable = active_drawable;
		  gtk_menu_set_active (GTK_MENU (menu), k);
		}
	      else if (drawable == -1)
		drawable = layers[j];

	      k += 1;
	    }

	channels = gimp_image_get_channels (images[i], &nchannels);

	for (j = 0; j < nchannels; j++)
	  if (!constraint || (* constraint) (images[i], channels[j], data))
	    {
	      GtkWidget *hbox;
	      GtkWidget *vbox;
	      GtkWidget *wcolor_box;
	      GtkWidget *wlabel;

	      name = gimp_channel_get_name (channels[j]);
	      label = g_new (char, strlen (image_label) + strlen (name) + 2);
	      sprintf (label, "%s/%s", image_label, name);
	      g_free (name);

	      menuitem = gtk_menu_item_new();
	      gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
				  (GtkSignalFunc) gimp_menu_callback,
				  &channels[j]);

	      hbox = gtk_hbox_new (FALSE, 0);
	      gtk_container_add (GTK_CONTAINER (menuitem), hbox);
	      gtk_widget_show (hbox);

	      vbox = gtk_vbox_new (FALSE, 0);
	      gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
	      gtk_widget_show (vbox);
	      
	      wcolor_box = gtk_preview_new (GTK_PREVIEW_COLOR);
	      gtk_preview_set_dither (GTK_PREVIEW (wcolor_box), GDK_RGB_DITHER_MAX);

 	      fill_preview_with_thumb (wcolor_box, 
				       channels[j], 
				       MENU_THUMBNAIL_WIDTH, 
				       MENU_THUMBNAIL_HEIGHT); 

	      gtk_widget_set_usize (GTK_WIDGET (wcolor_box), 
				    MENU_THUMBNAIL_WIDTH , 
				    MENU_THUMBNAIL_HEIGHT);
	      
	      gtk_container_add (GTK_CONTAINER (vbox), wcolor_box);
	      gtk_widget_show (wcolor_box);

	      wlabel = gtk_label_new (label);
	      gtk_misc_set_alignment (GTK_MISC (wlabel), 0.0, 0.5);
	      gtk_box_pack_start (GTK_BOX (hbox), wlabel, TRUE, TRUE, 4);
	      gtk_widget_show (wlabel);

	      gtk_menu_append (GTK_MENU (menu), menuitem);
	      gtk_widget_show (menuitem);

	      g_free (label);

	      if (channels[j] == active_drawable)
		{
		  drawable = active_drawable;
		  gtk_menu_set_active (GTK_MENU (menu), k);
		}
	      else if (drawable == -1)
		drawable = channels[j];

	      k += 1;
	    }

	g_free (image_label);
      }
  g_free (images);

  if (k == 0)
    {
      menuitem = gtk_menu_item_new_with_label ("none");
      gtk_widget_set_sensitive (menuitem, FALSE);
      gtk_menu_append (GTK_MENU (menu), menuitem);
      gtk_widget_show (menuitem);
    }

  if (drawable != -1)
    (* callback) (drawable, data);

  return menu;
}


static void
gimp_menu_callback (GtkWidget *widget,
		    gint32    *id)
{
  GimpMenuCallback callback;
  gpointer         callback_data;

  callback = (GimpMenuCallback) gtk_object_get_user_data (GTK_OBJECT (widget->parent));
  callback_data = gtk_object_get_data (GTK_OBJECT (widget->parent), "gimp_callback_data");

  (* callback) (*id, callback_data);
}

static void
fill_preview_with_thumb (GtkWidget *widget,
			 gint32     drawable_ID,
			 gint       width,
			 gint       height)
{
  guchar  *drawable_data;
  gint     bpp;
  gint     x, y;
  guchar  *src;
  gdouble  r, g, b, a;
  gdouble  c0, c1;
  guchar  *p0, *p1, *even, *odd;

  bpp = 0; /* Only returned */
  
  drawable_data = 
    gimp_drawable_get_thumbnail_data (drawable_ID, &width, &height, &bpp);

  gtk_preview_size (GTK_PREVIEW (widget), width, height);

  even = g_malloc (width * 3);
  odd  = g_malloc (width * 3);
  src  = drawable_data;

  for (y = 0; y < height; y++)
    {
      p0 = even;
      p1 = odd;
      
      for (x = 0; x < width; x++) 
	{
	  if(bpp == 4)
	    {
	      r =  ((gdouble)src[x*4+0])/255.0;
	      g = ((gdouble)src[x*4+1])/255.0;
	      b = ((gdouble)src[x*4+2])/255.0;
	      a = ((gdouble)src[x*4+3])/255.0;
	    }
	  else if(bpp == 3)
	    {
	      r =  ((gdouble)src[x*3+0])/255.0;
	      g = ((gdouble)src[x*3+1])/255.0;
	      b = ((gdouble)src[x*3+2])/255.0;
	      a = 1.0;
	    }
	  else
	    {
	      r = ((gdouble)src[x*bpp+0])/255.0;
	      g = b = r;
	      if(bpp == 2)
		a = ((gdouble)src[x*bpp+1])/255.0;
	      else
		a = 1.0;
	    }
	
	  if ((x / GIMP_CHECK_SIZE_SM) & 1) 
	    {
	      c0 = GIMP_CHECK_LIGHT;
	      c1 = GIMP_CHECK_DARK;
	    } 
	  else 
	    {
	      c0 = GIMP_CHECK_DARK;
	      c1 = GIMP_CHECK_LIGHT;
	    }
	
	  *p0++ = (c0 + (r - c0) * a) * 255.0;
	  *p0++ = (c0 + (g - c0) * a) * 255.0;
	  *p0++ = (c0 + (b - c0) * a) * 255.0;
	  
	  *p1++ = (c1 + (r - c1) * a) * 255.0;
	  *p1++ = (c1 + (g - c1) * a) * 255.0;
	  *p1++ = (c1 + (b - c1) * a) * 255.0;  
	}
      
      if ((y / GIMP_CHECK_SIZE_SM) & 1)
	gtk_preview_draw_row (GTK_PREVIEW (widget), (guchar *)odd, 0, y, width);
      else
	gtk_preview_draw_row (GTK_PREVIEW (widget), (guchar *)even, 0, y, width);

      src += width * bpp;
    }

  g_free (even);
  g_free (odd);
}


/* These functions allow the callback PDB work with gtk
 * ALT.
 * Note that currently PDB calls in libgimp are completely deterministic.
 * There is always one call followed by a reply.
 * If we are in the main gdk wait routine we can cannot get a reply
 * to a wire message, only the request for a new PDB proc to be run.
 * we will restrict this to a temp PDB function we have registered.
 */

static void 
do_brush_callback (GimpBrushData *bdata)
{
  if (!bdata->busy)
    return;

  if (bdata->callback)
    bdata->callback (bdata->bname,
		     bdata->opacity,
		     bdata->spacing,
		     bdata->paint_mode,
		     bdata->width,
		     bdata->height,
		     bdata->brush_mask_data,
		     bdata->closing,
		     bdata->data);

  if (bdata->bname)
    g_free (bdata->bname);  
  
  if (bdata->brush_mask_data)
    g_free (bdata->brush_mask_data); 

  bdata->busy = FALSE;
  bdata->bname = NULL;
  bdata->brush_mask_data = NULL;
}

static void 
do_pattern_callback (GimpPatternData *pdata)
{
  if (!pdata->busy)
    return;

  if (pdata->callback)
    pdata->callback (pdata->pname,
		     pdata->width,
		     pdata->height,
		     pdata->bytes,
		     pdata->pattern_mask_data,
		     pdata->closing,
		     pdata->data);
  
  if (pdata->pname)
    g_free (pdata->pname);  
  
  if (pdata->pattern_mask_data)
    g_free (pdata->pattern_mask_data); 

  pdata->busy = FALSE;
  pdata->pname = NULL;
  pdata->pattern_mask_data = NULL;
}

static void 
do_gradient_callback (GimpGradientData *gdata)
{
  if (!gdata->busy)
    return;

  if (gdata->callback)
    gdata->callback (gdata->gname,
		     gdata->width,
		     gdata->gradient_data,
		     gdata->closing,
		     gdata->data);

  if (gdata->gname)
    g_free (gdata->gname);  
  
  if (gdata->gradient_data)
    g_free (gdata->gradient_data); 

  gdata->busy = FALSE;
  gdata->gname = NULL;
  gdata->gradient_data = NULL;
}

static gint
idle_test_brush (GimpBrushData *bdata)
{
  do_brush_callback (bdata);

  return FALSE;
}


static gint
idle_test_pattern (GimpPatternData *pdata)
{
  do_pattern_callback (pdata);

  return FALSE;
}

static gint
idle_test_gradient (GimpGradientData *gdata)
{
  do_gradient_callback (gdata);

  return FALSE;
}

static void
temp_brush_invoker(gchar    *name,
		   gint      nparams,
		   GParam   *param,
		   gint     *nreturn_vals,
		   GParam **return_vals)
{
  static GParam values[1];
  GStatusType status = STATUS_SUCCESS;
  GimpBrushData *bdata;

  bdata = (GimpBrushData *) g_hash_table_lookup (gbrush_ht, name);

  if (!bdata)
    g_warning("Can't find internal brush data");
  else
    if(!bdata->busy)
      {
	bdata->bname           = g_strdup (param[0].data.d_string);
	bdata->opacity         = (gdouble)param[1].data.d_float;
	bdata->spacing         = param[2].data.d_int32;
	bdata->paint_mode      = param[3].data.d_int32;
	bdata->width           = param[4].data.d_int32;
	bdata->height          = param[5].data.d_int32;
	bdata->brush_mask_data = g_malloc(param[6].data.d_int32);
	g_memmove (bdata->brush_mask_data, 
		   param[7].data.d_int8array, param[6].data.d_int32); 
	bdata->closing         = param[8].data.d_int32;
	active_brush_pdb       = bdata;
	bdata->busy            = TRUE;
	
	gtk_idle_add ((GtkFunction)idle_test_brush, active_brush_pdb);
      }

  *nreturn_vals = 1;
  *return_vals = values;
  
  values[0].type = PARAM_STATUS;
  values[0].data.d_status = status;
}

static void
temp_pattern_invoker(gchar    *name,
		     gint      nparams,
		     GParam   *param,
		     gint     *nreturn_vals,
		     GParam  **return_vals)
{
  static GParam values[1];
  GStatusType status = STATUS_SUCCESS;
  GimpPatternData *pdata;

  pdata = (GimpPatternData *)g_hash_table_lookup (gpattern_ht, name);

  if (!pdata)
    g_warning ("Can't find internal pattern data");
  else
    if (!pdata->busy)
      {
	pdata->pname             = g_strdup(param[0].data.d_string);
	pdata->width             = param[1].data.d_int32;
	pdata->height            = param[2].data.d_int32;
	pdata->bytes             = param[3].data.d_int32;
	pdata->pattern_mask_data = g_malloc(param[4].data.d_int32);
	g_memmove (pdata->pattern_mask_data,
		   param[5].data.d_int8array, param[4].data.d_int32);
	pdata->closing           = param[6].data.d_int32;
	active_pattern_pdb       = pdata;
	pdata->busy              = TRUE;

	gtk_idle_add ((GtkFunction)idle_test_pattern, active_pattern_pdb);
      }

  *nreturn_vals = 1;
  *return_vals = values;
  
  values[0].type = PARAM_STATUS;
  values[0].data.d_status = status;
}

static void
temp_gradient_invoker(gchar    *name,
		      gint      nparams,
		      GParam   *param,
		      gint     *nreturn_vals,
		      GParam  **return_vals)
{
  static GParam values[1];
  GStatusType status = STATUS_SUCCESS;
  GimpGradientData *gdata;

  gdata = (GimpGradientData *) g_hash_table_lookup (ggradient_ht, name);

  if (!gdata)
      g_warning("Can't find internal gradient data");
  else
    {
      if (!gdata->busy)
	{
	  gint i;
	  gdouble *pv;
	  gdouble *values;
	  
	  gdata->gname         = g_strdup (param[0].data.d_string);
	  gdata->width         = param[1].data.d_int32;
	  gdata->gradient_data = g_new (gdouble, param[1].data.d_int32);

	  values = param[2].data.d_floatarray;
	  pv = gdata->gradient_data;

	  for (i = 0; i < gdata->width; i++)
	    gdata->gradient_data[i] = param[2].data.d_floatarray[i];

	  gdata->closing      = param[3].data.d_int32;
	  active_gradient_pdb = gdata;
	  gdata->busy         = TRUE;
	  gtk_idle_add ((GtkFunction)idle_test_gradient, active_gradient_pdb);
	}
    }

  *nreturn_vals = 1;
  *return_vals = values;
  
  values[0].type = PARAM_STATUS;
  values[0].data.d_status = status;
}

static gboolean
input_callback (GIOChannel  *channel,
		GIOCondition condition,
		gpointer     data)
{
  /* We have some data in the wire - read it */
  /* The below will only ever run a single proc */
  gimp_run_temp ();

  return TRUE;
}

static void
gimp_setup_callbacks (void)
{
  static gboolean first_time = TRUE;

  if (first_time)
    {
      /* Tie into the gdk input function only once */
      g_io_add_watch (_readchannel, G_IO_IN | G_IO_PRI, input_callback, NULL);

      /* This needed on Win32 */
      gimp_request_wakeups ();

      first_time = FALSE;
    }
}

static gchar *
gen_temp_plugin_name (void)
{
  GParam *return_vals;
  gint   nreturn_vals;
  gchar *result;

  return_vals = gimp_run_procedure ("gimp_temp_PDB_name",
				    &nreturn_vals,
				    PARAM_END);

  result = "temp_name_gen_failed";
  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    result = g_strdup (return_vals[1].data.d_string);

  gimp_destroy_params (return_vals, nreturn_vals);

  return result;
}

/* Can only be used in conjuction with gdk since we need to tie into the input 
 * selection mech.
 */
gpointer
gimp_interactive_selection_brush(gchar             *dialogname, 
				 gchar             *brush_name,
				 gdouble            opacity,
				 gint               spacing,
				 gint               paint_mode,
				 GRunBrushCallback  callback,
				 gpointer           data)
{
  static GParamDef args[] =
  {
    { PARAM_STRING,    "str",           "String"},
    { PARAM_FLOAT,     "opacity",       "Opacity"},
    { PARAM_INT32,     "spacing",       "Spacing"},
    { PARAM_INT32,     "paint mode",    "Paint mode"},
    { PARAM_INT32,     "mask width",    "Brush width"},
    { PARAM_INT32,     "mask height"    "Brush heigth"},
    { PARAM_INT32,     "mask len",      "Length of brush mask data"},
    { PARAM_INT8ARRAY, "mask data",     "The brush mask data"},
    { PARAM_INT32,     "dialog status", "Registers if the dialog was closing "
                                        "[0 = No, 1 = Yes]"},
  };
  static GParamDef *return_vals = NULL;
  static int nargs = sizeof (args) / sizeof (args[0]);
  static int nreturn_vals = 0;
  gint bnreturn_vals;
  GParam *pdbreturn_vals;
  gchar *pdbname = gen_temp_plugin_name ();
  GimpBrushData *bdata;

  bdata = g_new0 (GimpBrushData, 1);

  gimp_install_temp_proc (pdbname,
			  "Temp PDB for interactive popups",
			  "More here later",
			  "Andy Thomas",
			  "Andy Thomas",
			  "1997",
			  NULL,
			  "RGB*, GRAY*",
			  PROC_TEMPORARY,
			  nargs, nreturn_vals,
			  args, return_vals,
			  temp_brush_invoker);

  pdbreturn_vals =
    gimp_run_procedure("gimp_brushes_popup",
		       &bnreturn_vals,
		       PARAM_STRING, pdbname,
		       PARAM_STRING, dialogname,
		       PARAM_STRING, brush_name,
		       PARAM_FLOAT,  opacity,
		       PARAM_INT32,  spacing,
		       PARAM_INT32,  paint_mode,
		       PARAM_END);

/*   if (pdbreturn_vals[0].data.d_status != STATUS_SUCCESS) */
/*     { */
/*       printf("ret failed = 0x%x\n",bnreturn_vals); */
/*     } */
/*   else */
/*       printf("worked = 0x%x\n",bnreturn_vals); */

  gimp_setup_callbacks(); /* New function to allow callbacks to be watched */

  gimp_destroy_params (pdbreturn_vals,bnreturn_vals);

  /*   g_free(pdbname); */

  /* Now add to hash table so we can find it again */
  if (gbrush_ht == NULL)
    gbrush_ht = g_hash_table_new (g_str_hash,
				  g_str_equal);
  
  bdata->callback = callback;
  bdata->data = data;
  g_hash_table_insert (gbrush_ht, pdbname, bdata);

  return pdbname;
}


gchar *
gimp_brushes_get_brush_data (gchar    *bname,
			     gdouble  *opacity,
			     gint     *spacing,
			     gint     *paint_mode,
			     gint     *width,
			     gint     *height,
			     gchar   **mask_data)
{
  GParam *return_vals;
  gint nreturn_vals;
  gchar *ret_name = NULL;

  return_vals = gimp_run_procedure ("gimp_brushes_get_brush_data",
				    &nreturn_vals,
				    PARAM_STRING, bname,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    {
      ret_name     = g_strdup (return_vals[1].data.d_string);
      *opacity     = return_vals[2].data.d_float;
      *spacing     = return_vals[3].data.d_int32;
      *paint_mode  = return_vals[4].data.d_int32;
      *width       = return_vals[5].data.d_int32;
      *height      = return_vals[6].data.d_int32;
      *mask_data   = g_new (gchar, return_vals[7].data.d_int32);
      g_memmove (*mask_data, 
		 return_vals[8].data.d_int32array, return_vals[7].data.d_int32);
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return ret_name;
}

gint 
gimp_brush_close_popup (gpointer popup_pnt)
{
  GParam *return_vals;
  gint nreturn_vals;
  gint retval;

  return_vals = gimp_run_procedure ("gimp_brushes_close_popup",
				    &nreturn_vals,
				    PARAM_STRING, popup_pnt,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);

  retval = (return_vals[0].data.d_status == STATUS_SUCCESS);

  return retval;
}

gint 
gimp_brush_set_popup(gpointer  popup_pnt, 
		     gchar    *pname,
		     gdouble   opacity,
		     gint      spacing,
		     gint      paint_mode)
{
  GParam *return_vals;
  gint nreturn_vals;
  gint retval;

  return_vals = gimp_run_procedure ("gimp_brushes_set_popup",
				    &nreturn_vals,
				    PARAM_STRING, popup_pnt,
				    PARAM_STRING, pname,
				    PARAM_FLOAT,  opacity,
				    PARAM_INT32,  spacing,
				    PARAM_INT32,  paint_mode,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);

  retval = (return_vals[0].data.d_status == STATUS_SUCCESS);

  return retval;
}



gpointer
gimp_interactive_selection_pattern (gchar               *dialogname, 
				    gchar               *pattern_name,
				    GRunPatternCallback  callback,
				    gpointer             data)
{
  static GParamDef args[] =
  {
    { PARAM_STRING,   "str",           "String"},
    { PARAM_INT32,    "mask width",    "Pattern width"},
    { PARAM_INT32,    "mask height",   "Pattern heigth"},
    { PARAM_INT32,    "mask bpp",      "Pattern bytes per pixel"},
    { PARAM_INT32,    "mask len",      "Length of pattern mask data"},
    { PARAM_INT8ARRAY,"mask data",     "The pattern mask data"},
    { PARAM_INT32,    "dialog status", "Registers if the dialog was closing "
                                       "[0 = No, 1 = Yes]"},
  };
  static GParamDef *return_vals = NULL;
  static int nargs = sizeof (args) / sizeof (args[0]);
  static int nreturn_vals = 0;
  gint bnreturn_vals;
  GParam *pdbreturn_vals;
  gchar *pdbname = gen_temp_plugin_name ();
  GimpPatternData *pdata;

  pdata = g_new0 (GimpPatternData, 1);

  gimp_install_temp_proc (pdbname,
			  "Temp PDB for interactive popups",
			  "More here later",
			  "Andy Thomas",
			  "Andy Thomas",
			  "1997",
			  NULL,
			  "RGB*, GRAY*",
			  PROC_TEMPORARY,
			  nargs, nreturn_vals,
			  args, return_vals,
			  temp_pattern_invoker);

  pdbreturn_vals =
    gimp_run_procedure("gimp_patterns_popup",
		       &bnreturn_vals,
		       PARAM_STRING,pdbname,
		       PARAM_STRING,dialogname,
		       PARAM_STRING,pattern_name,/*name*/
		       PARAM_END);

  gimp_setup_callbacks (); /* New function to allow callbacks to be watched */

  gimp_destroy_params (pdbreturn_vals, bnreturn_vals);

  /* Now add to hash table so we can find it again */
  if (gpattern_ht == NULL)
    gpattern_ht = g_hash_table_new (g_str_hash,
				    g_str_equal);

  pdata->callback = callback;
  pdata->data = data;
  g_hash_table_insert (gpattern_ht, pdbname,pdata);

  return pdbname;
}


gchar *
gimp_pattern_get_pattern_data (gchar   *pname,
			       gint    *width,
			       gint    *height,
			       gint    *bytes,
			       gchar  **mask_data)
{
  GParam *return_vals;
  gint nreturn_vals;
  gchar *ret_name = NULL;

  return_vals = gimp_run_procedure ("gimp_patterns_get_pattern_data",
				    &nreturn_vals,
				    PARAM_STRING, pname,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    {
      ret_name   = g_strdup(return_vals[1].data.d_string);
      *width     = return_vals[2].data.d_int32;
      *height    = return_vals[3].data.d_int32;
      *bytes     = return_vals[4].data.d_int32;
      *mask_data = g_new (gchar,return_vals[5].data.d_int32);
      g_memmove (*mask_data, 
		 return_vals[6].data.d_int32array, return_vals[5].data.d_int32);
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return ret_name;
}

gint 
gimp_pattern_close_popup (gpointer popup_pnt)
{
  GParam *return_vals;
  gint nreturn_vals;
  gint retval;

  return_vals = gimp_run_procedure ("gimp_patterns_close_popup",
				    &nreturn_vals,
				    PARAM_STRING, popup_pnt,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);

  retval = (return_vals[0].data.d_status == STATUS_SUCCESS);

  return retval;
}

gint 
gimp_pattern_set_popup (gpointer  popup_pnt, 
		        gchar    *pname)
{
  GParam *return_vals;
  gint nreturn_vals;
  gint retval;

  return_vals = gimp_run_procedure ("gimp_patterns_set_popup",
				    &nreturn_vals,
				    PARAM_STRING, popup_pnt,
				    PARAM_STRING, pname,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);

  retval = (return_vals[0].data.d_status == STATUS_SUCCESS);

  return retval;
}

gpointer
gimp_interactive_selection_gradient (gchar                 *dialogname, 
				     gchar                 *gradient_name,
				     gint                  sample_sz,
				     GRunGradientCallback  callback,
				     gpointer              data)
{
  static GParamDef args[] =
  {
    { PARAM_STRING,    "str",           "String"},
    { PARAM_INT32,     "grad width",    "Gradient width"},
    { PARAM_FLOATARRAY,"grad data",     "The gradient mask data"},
    { PARAM_INT32,     "dialog status", "Registers if the dialog was closing "
                                        "[0 = No, 1 = Yes]"},
  };
  static GParamDef *return_vals = NULL;
  static int nargs = sizeof (args) / sizeof (args[0]);
  static int nreturn_vals = 0;
  gint bnreturn_vals;
  GParam *pdbreturn_vals;
  gchar *pdbname = gen_temp_plugin_name();
  GimpGradientData *gdata;

  gdata = g_new0 (GimpGradientData, 1);

  gimp_install_temp_proc (pdbname,
			  "Temp PDB for interactive popups",
			  "More here later",
			  "Andy Thomas",
			  "Andy Thomas",
			  "1997",
			  NULL,
			  "RGB*, GRAY*",
			  PROC_TEMPORARY,
			  nargs, nreturn_vals,
			  args, return_vals,
			  temp_gradient_invoker);

  pdbreturn_vals =
    gimp_run_procedure ("gimp_gradients_popup",
			&bnreturn_vals,
			PARAM_STRING, pdbname,
			PARAM_STRING, dialogname,
			PARAM_STRING, gradient_name, /*name*/
			PARAM_INT32,  sample_sz,     /* size of sample to be returned */ 
			PARAM_END);

  gimp_setup_callbacks (); /* New function to allow callbacks to be watched */

  gimp_destroy_params (pdbreturn_vals,bnreturn_vals);

  /* Now add to hash table so we can find it again */
  if (ggradient_ht == NULL)
    ggradient_ht = g_hash_table_new (g_str_hash,
				     g_str_equal);

  gdata->callback = callback;
  gdata->data = data;
  g_hash_table_insert (ggradient_ht, pdbname,gdata);

  return pdbname;
}

gchar *
gimp_gradient_get_gradient_data (gchar     *gname,
				 gint      *width,
				 gint       sample_sz,
				 gdouble  **grad_data)
{
  GParam *return_vals;
  gint nreturn_vals;
  gchar *ret_name = NULL;

  return_vals = gimp_run_procedure ("gimp_gradients_get_gradient_data",
				    &nreturn_vals,
				    PARAM_STRING, gname,
				    PARAM_INT32,  sample_sz,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    {
      gint i;

      ret_name = g_strdup (return_vals[1].data.d_string);
      *width = return_vals[2].data.d_int32;
      *grad_data = g_new (gdouble, *width);

      for (i = 0; i < *width; i++)
	(*grad_data)[i] = return_vals[3].data.d_floatarray[i];
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return ret_name;
}


gint 
gimp_gradient_close_popup (gpointer popup_pnt)
{
  GParam *return_vals;
  gint nreturn_vals;
  gint retval;

  return_vals = gimp_run_procedure ("gimp_gradients_close_popup",
				    &nreturn_vals,
				    PARAM_STRING, popup_pnt,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);

  retval = (return_vals[0].data.d_status == STATUS_SUCCESS);

  return retval;
}

gint 
gimp_gradient_set_popup (gpointer  popup_pnt, 
			 gchar    *gname)
{
  GParam *return_vals;
  gint nreturn_vals;
  gint retval;

  return_vals = gimp_run_procedure ("gimp_gradients_set_popup",
				    &nreturn_vals,
				    PARAM_STRING, popup_pnt,
				    PARAM_STRING, gname,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);

  retval = (return_vals[0].data.d_status == STATUS_SUCCESS);

  return retval;
}



