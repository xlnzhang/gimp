/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpoperationreplacemode.c
 * Copyright (C) 2008 Michael Natterer <mitch@gimp.org>
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

#include <gegl-plugin.h>

#include "operations-types.h"

#include "gimpoperationreplacemode.h"


static void     gimp_operation_replace_mode_prepare      (GeglOperation       *operation);
static gboolean gimp_operation_replace_mode_process      (GeglOperation       *operation,
                                                          void                *in_buf,
                                                          void                *aux_buf,
                                                          void                *aux2_buf,
                                                          void                *out_buf,
                                                          glong                samples,
                                                          const GeglRectangle *roi,
                                                          gint                 level);


G_DEFINE_TYPE (GimpOperationReplaceMode, gimp_operation_replace_mode,
               GIMP_TYPE_OPERATION_POINT_LAYER_MODE)


static void
gimp_operation_replace_mode_class_init (GimpOperationReplaceModeClass *klass)
{
  GeglOperationClass               *operation_class;
  GeglOperationPointComposer3Class *point_class;

  operation_class = GEGL_OPERATION_CLASS (klass);
  point_class     = GEGL_OPERATION_POINT_COMPOSER3_CLASS (klass);

  gegl_operation_class_set_keys (operation_class,
                                 "name",        "gimp:replace-mode",
                                 "description", "GIMP replace mode operation",
                                 NULL);

  operation_class->prepare = gimp_operation_replace_mode_prepare;
  point_class->process     = gimp_operation_replace_mode_process;
}

static void
gimp_operation_replace_mode_init (GimpOperationReplaceMode *self)
{
}

static void
gimp_operation_replace_mode_prepare (GeglOperation *operation)
{
  const Babl *format = babl_format ("RGBA float");

  gegl_operation_set_format (operation, "input",  format);
  gegl_operation_set_format (operation, "aux",    format);
  gegl_operation_set_format (operation, "aux2",   babl_format ("Y float"));
  gegl_operation_set_format (operation, "output", format);
}

static gboolean
gimp_operation_replace_mode_process (GeglOperation       *operation,
                                     void                *in_buf,
                                     void                *aux_buf,
                                     void                *aux2_buf,
                                     void                *out_buf,
                                     glong                samples,
                                     const GeglRectangle *roi,
                                     gint                 level)
{
  GimpOperationPointLayerMode *point   = GIMP_OPERATION_POINT_LAYER_MODE (operation);
  gfloat                       opacity = point->opacity;
  gfloat                      *in      = in_buf;
  gfloat                      *layer   = aux_buf;
  gfloat                      *mask    = aux2_buf;
  gfloat                      *out     = out_buf;

  while (samples--)
    {
      gint   b;
      gfloat new_alpha;

      if (mask)
        new_alpha = (layer[ALPHA] - in[ALPHA]) * (*mask) * opacity + in[ALPHA];
      else
        new_alpha = (layer[ALPHA] - in[ALPHA]) * opacity + in[ALPHA];

      if (new_alpha)
        {
          gfloat ratio;

          if (mask)
            ratio = *mask * opacity * layer[ALPHA] / new_alpha;
          else
            ratio = opacity * layer[ALPHA] / new_alpha;

          for (b = RED; b < ALPHA; b++)
            {
              gfloat t;

              if (layer[b] > in[b])
                {
                  t = (layer[b] - in[b]) * ratio;
                  out[b] = in[b] + t;
                }
              else
                {
                  t = (in[b] - layer[b]) * ratio;
                  out[b] = in[b] - t;
                }
            }
        }
      else
        {
          for (b = RED; b < ALPHA; b++)
            out[b] = in[b];
        }

      out[ALPHA] = new_alpha;

      in    += 4;
      layer += 4;
      out   += 4;

      if (mask)
        mask += 1;
    }

  return TRUE;
}
