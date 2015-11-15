#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>
#include <cairo/Ector_Cairo.h>

#include "ector_private.h"
#include "ector_cairo_private.h"

static cairo_pattern_t *(*cairo_pattern_create_linear)(double x0, double y0,
                                                       double x1, double y1) = NULL;
static void (*cairo_set_source)(cairo_t *cr, cairo_pattern_t *source) = NULL;
static void (*cairo_fill)(cairo_t *cr) = NULL;
static void (*cairo_rectangle)(cairo_t *cr,
                               double x, double y,
                               double width, double height) = NULL;
static void (*cairo_pattern_destroy)(cairo_pattern_t *pattern) = NULL;

static void (*cairo_pattern_set_extend)(cairo_pattern_t *pattern, cairo_extend_t extend) = NULL;

typedef struct _Ector_Renderer_Cairo_Gradient_Linear_Data Ector_Renderer_Cairo_Gradient_Linear_Data;
struct _Ector_Renderer_Cairo_Gradient_Linear_Data
{
   Ector_Cairo_Surface_Data *parent;
};

static cairo_pattern_t *
_ector_renderer_cairo_gradient_linear_prepare(Eo *obj EINA_UNUSED,
                                              Ector_Renderer_Generic_Gradient_Linear_Data *gld,
                                              Ector_Renderer_Generic_Gradient_Data *gd,
                                              unsigned int mul_col)
{
   cairo_pattern_t *pat;

   pat = cairo_pattern_create_linear(gld->start.x, gld->start.y,
                                     gld->end.x, gld->end.y);
   if (!pat) return NULL;
   _ector_renderer_cairo_gradient_prepare(pat, gd, mul_col);

   cairo_pattern_set_extend(pat, _ector_cairo_extent_get(gd->s));

   return pat;
}

static Eina_Bool
_ector_renderer_cairo_gradient_linear_ector_renderer_generic_base_prepare(Eo *obj,
                                                                          Ector_Renderer_Cairo_Gradient_Linear_Data *pd)
{
   eo_do_super(obj, ECTOR_RENDERER_CAIRO_GRADIENT_LINEAR_CLASS, ector_renderer_prepare());

   if (!pd->parent)
     {
        Ector_Renderer_Generic_Base_Data *base;

        base = eo_data_scope_get(obj, ECTOR_RENDERER_GENERIC_BASE_CLASS);
        pd->parent = eo_data_xref(base->surface, ECTOR_CAIRO_SURFACE_CLASS, obj);
     }

   return EINA_FALSE;
}

static Eina_Bool
_ector_renderer_cairo_gradient_linear_ector_renderer_generic_base_draw(Eo *obj,
                                                                       Ector_Renderer_Cairo_Gradient_Linear_Data *pd,
                                                                       Ector_Rop op, Eina_Array *clips, unsigned int mul_col)
{
   Ector_Renderer_Generic_Gradient_Linear_Data *gld;
   Ector_Renderer_Generic_Gradient_Data *gd;
   cairo_pattern_t *pat;

   // FIXME: don't ignore clipping !
   gld = eo_data_scope_get(obj, ECTOR_RENDERER_GENERIC_GRADIENT_LINEAR_MIXIN);
   gd = eo_data_scope_get(obj, ECTOR_RENDERER_GENERIC_GRADIENT_MIXIN);
   if (!gd || !gld) return EINA_FALSE;

   pat = _ector_renderer_cairo_gradient_linear_prepare(obj, gld, gd, mul_col);
   if (!pat) return EINA_FALSE;

   eo_do_super(obj, ECTOR_RENDERER_CAIRO_GRADIENT_LINEAR_CLASS, ector_renderer_draw(op, clips, mul_col));

   cairo_rectangle(pd->parent->cairo, gld->start.x, gld->start.y,
                   gld->end.x - gld->start.x,
                   gld->end.y - gld->start.y);
   cairo_set_source(pd->parent->cairo, pat);
   cairo_fill(pd->parent->cairo);
   cairo_pattern_destroy(pat);

   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_cairo_gradient_linear_ector_renderer_cairo_base_fill(Eo *obj,
                                                                     Ector_Renderer_Cairo_Gradient_Linear_Data *pd,
                                                                     unsigned int mul_col)
{
   Ector_Renderer_Generic_Gradient_Linear_Data *gld;
   Ector_Renderer_Generic_Gradient_Data *gd;
   cairo_pattern_t *pat;

   gld = eo_data_scope_get(obj, ECTOR_RENDERER_GENERIC_GRADIENT_LINEAR_MIXIN);
   gd = eo_data_scope_get(obj, ECTOR_RENDERER_GENERIC_GRADIENT_MIXIN);
   if (!gd || !gld) return EINA_FALSE;

   pat = _ector_renderer_cairo_gradient_linear_prepare(obj, gld, gd, mul_col);
   if (!pat) return EINA_FALSE;

   cairo_set_source(pd->parent->cairo, pat);
   cairo_pattern_destroy(pat);

   return EINA_TRUE;
}

static void
_ector_renderer_cairo_gradient_linear_ector_renderer_generic_base_bounds_get(Eo *obj,
                                                                             Ector_Renderer_Cairo_Gradient_Linear_Data *pd EINA_UNUSED,
                                                                             Eina_Rectangle *r)
{
   Ector_Renderer_Generic_Gradient_Linear_Data *gld;
   Ector_Renderer_Cairo_Base_Data *bd;

   gld = eo_data_scope_get(obj, ECTOR_RENDERER_GENERIC_GRADIENT_RADIAL_MIXIN);
   bd = eo_data_scope_get(obj, ECTOR_RENDERER_CAIRO_BASE_CLASS);
   EINA_RECTANGLE_SET(r,
                      bd->generic->origin.x + gld->start.x,
                      bd->generic->origin.y + gld->start.y,
                      gld->end.x - gld->start.x,
                      gld->end.y - gld->start.x);
}

static Eo_Base *
_ector_renderer_cairo_gradient_linear_eo_base_finalize(Eo *obj, Ector_Renderer_Cairo_Gradient_Linear_Data *pd EINA_UNUSED)
{
   Ector_Renderer_Generic_Base_Data *base;

   eo_do_super(obj, ECTOR_RENDERER_CAIRO_GRADIENT_LINEAR_CLASS, obj = eo_finalize());
   if (!obj) return NULL;

   base = eo_data_scope_get(obj, ECTOR_RENDERER_GENERIC_BASE_CLASS);

   USE(base, cairo_set_source, NULL);
   USE(base, cairo_pattern_destroy, NULL);
   USE(base, cairo_rectangle, NULL);
   USE(base, cairo_fill, NULL);
   USE(base, cairo_pattern_destroy, NULL);
   USE(base, cairo_set_source, NULL);
   USE(base, cairo_pattern_set_extend, NULL);
   USE(base, cairo_pattern_create_linear, NULL);
   USE(base, cairo_pattern_add_color_stop_rgba, NULL);

   return obj;
}

static void
_ector_renderer_cairo_gradient_linear_eo_base_destructor(Eo *obj,
                                                         Ector_Renderer_Cairo_Gradient_Linear_Data *pd)
{
   Ector_Renderer_Generic_Base_Data *base;

   base = eo_data_scope_get(obj, ECTOR_RENDERER_GENERIC_BASE_CLASS);
   eo_data_xunref(base->surface, pd->parent, obj);

   eo_do_super(obj, ECTOR_RENDERER_CAIRO_GRADIENT_LINEAR_CLASS, eo_destructor());
}

static void
_ector_renderer_cairo_gradient_linear_efl_gfx_gradient_base_stop_set(Eo *obj,
                                                                     Ector_Renderer_Cairo_Gradient_Linear_Data *pd EINA_UNUSED,
                                                                     const Efl_Gfx_Gradient_Stop *colors,
                                                                     unsigned int length)
{
   eo_do_super(obj, ECTOR_RENDERER_CAIRO_GRADIENT_LINEAR_CLASS,
               efl_gfx_gradient_stop_set(colors, length));
}

static unsigned int
_ector_renderer_cairo_gradient_linear_ector_renderer_generic_base_crc_get(Eo *obj, Ector_Renderer_Cairo_Gradient_Linear_Data *pd EINA_UNUSED)
{
   Ector_Renderer_Generic_Gradient_Linear_Data *gld;
   Ector_Renderer_Generic_Gradient_Data *gd;
   unsigned int crc;

   eo_do_super(obj, ECTOR_RENDERER_CAIRO_GRADIENT_LINEAR_CLASS,
               crc = ector_renderer_crc_get());

   gld = eo_data_scope_get(obj, ECTOR_RENDERER_GENERIC_GRADIENT_LINEAR_MIXIN);
   gd = eo_data_scope_get(obj, ECTOR_RENDERER_GENERIC_GRADIENT_MIXIN);
   if (!gd || !gld) return crc;

   crc = eina_crc((void*) gd->s, sizeof (Efl_Gfx_Gradient_Spread), crc, EINA_FALSE);
   if (gd->colors_count)
     crc = eina_crc((void*) gd->colors, sizeof (Efl_Gfx_Gradient_Stop) * gd->colors_count, crc, EINA_FALSE);
   crc = eina_crc((void*) gld, sizeof (Ector_Renderer_Generic_Gradient_Linear_Data), crc, EINA_FALSE);

   return crc;
}

#include "ector_renderer_cairo_gradient_linear.eo.c"
