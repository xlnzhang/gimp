/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * GimpGroupLayer
 * Copyright (C) 2009  Michael Natterer <mitch@gimp.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <string.h>

#include <gegl.h>

#include "core-types.h"

#include "gimpgrouplayer.h"
#include "gimpimage.h"
#include "gimpdrawablestack.h"

#include "gimp-intl.h"


static void            gimp_group_layer_finalize     (GObject      *object);

static gint64          gimp_group_layer_get_memsize  (GimpObject   *object,
                                                      gint64       *gui_size);

static GimpContainer * gimp_group_layer_get_children (GimpViewable *viewable);

static GimpItem      * gimp_group_layer_duplicate    (GimpItem     *item,
                                                      GType         new_type);


G_DEFINE_TYPE (GimpGroupLayer, gimp_group_layer, GIMP_TYPE_LAYER)

#define parent_class gimp_group_layer_parent_class


static void
gimp_group_layer_class_init (GimpGroupLayerClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  GimpObjectClass   *gimp_object_class = GIMP_OBJECT_CLASS (klass);
  GimpViewableClass *viewable_class    = GIMP_VIEWABLE_CLASS (klass);
  GimpItemClass     *item_class        = GIMP_ITEM_CLASS (klass);

  object_class->finalize           = gimp_group_layer_finalize;

  gimp_object_class->get_memsize   = gimp_group_layer_get_memsize;

  viewable_class->default_stock_id = "gtk-directory";
  viewable_class->get_children     = gimp_group_layer_get_children;

  item_class->duplicate            = gimp_group_layer_duplicate;

  item_class->default_name         = _("Group Layer");
  item_class->rename_desc          = _("Rename Group Layer");
  item_class->translate_desc       = _("Move Group Layer");
  item_class->scale_desc           = _("Scale Group Layer");
  item_class->resize_desc          = _("Resize Group Layer");
  item_class->flip_desc            = _("Flip Group Layer");
  item_class->rotate_desc          = _("Rotate Group Layer");
  item_class->transform_desc       = _("Transform Group Layer");
}

static void
gimp_group_layer_init (GimpGroupLayer *layer)
{
  layer->children = gimp_drawable_stack_new (GIMP_TYPE_LAYER);
}

static void
gimp_group_layer_finalize (GObject *object)
{
  GimpGroupLayer *layer = GIMP_GROUP_LAYER (object);

  if (layer->children)
    {
      g_object_unref (layer->children);
      layer->children = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
gimp_group_layer_get_memsize (GimpObject *object,
                              gint64     *gui_size)
{
  GimpGroupLayer *layer   = GIMP_GROUP_LAYER (object);
  gint64          memsize = 0;

  memsize += gimp_object_get_memsize (GIMP_OBJECT (layer->children), gui_size);

  return memsize + GIMP_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static GimpContainer *
gimp_group_layer_get_children (GimpViewable *viewable)
{
  GimpGroupLayer *layer = GIMP_GROUP_LAYER (viewable);

  return layer->children;
}

static GimpItem *
gimp_group_layer_duplicate (GimpItem *item,
                            GType     new_type)
{
  GimpItem *new_item;

  g_return_val_if_fail (g_type_is_a (new_type, GIMP_TYPE_GROUP_LAYER), NULL);

  new_item = GIMP_ITEM_CLASS (parent_class)->duplicate (item, new_type);

  if (GIMP_IS_GROUP_LAYER (new_item))
    {
      GimpGroupLayer *layer     = GIMP_GROUP_LAYER (item);
      GimpGroupLayer *new_layer = GIMP_GROUP_LAYER (new_item);
      GList          *list;
      gint            position;

      for (list = GIMP_LIST (layer->children)->list, position = 0;
           list;
           list = g_list_next (list))
        {
          GimpItem *child = list->data;
          GimpItem *new_child;

          new_child = gimp_item_duplicate (child, G_TYPE_FROM_INSTANCE (child));

          gimp_container_insert (new_layer->children,
                                 GIMP_OBJECT (new_child),
                                 position);
        }
    }

  return new_item;
}

GimpLayer *
gimp_group_layer_new (GimpImage *image)
{
  GimpGroupLayer *layer;

  g_return_val_if_fail (GIMP_IS_IMAGE (image), NULL);

  layer = g_object_new (GIMP_TYPE_GROUP_LAYER, NULL);

  gimp_drawable_configure (GIMP_DRAWABLE (layer),
                           image,
                           0, 0, 1, 1,
                           gimp_image_base_type_with_alpha (image),
                           NULL);

  return GIMP_LAYER (layer);
}