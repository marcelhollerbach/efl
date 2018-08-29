#include <lotplayer_capi.h>
#include "vg_common.h"

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_vg_loader_json_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_vg_loader_json_log_dom, __VA_ARGS__)

static int _evas_vg_loader_json_log_dom = -1;

static Eina_Bool
evas_vg_load_file_close_json(Vg_File_Data *vfd)
{
   if (!vfd) return EINA_FALSE;

   LOTPlayer *player = (LOTPlayer *) vfd->loader_data;

   ERR("close json vfd = %p, lotplayer! = %p", vfd, player);

   lotplayer_destroy(player);
   free(vfd);

   return EINA_TRUE;
}

static Eina_Bool
evas_vg_load_file_data_json(Vg_File_Data *vfd)
{
   if (!vfd) return EINA_FALSE;

   LOTPlayer *player = (LOTPlayer *) vfd->loader_data;
   double progress = 0;
   int xx = 0;

   lotplayer_set_size(player, 400, 400);

   int size = lotplayer_get_node_count(player, progress);

   ERR("data json vfd = %p, player = %p, size = %d", vfd, player, size);

   for (int i = 0; i < size; i++)
     {
        const LOTNode *p = lotplayer_get_node(player, progress, i);
#if 1
        ERR("%f[%d] node(%p)", progress, xx++, p);
        ERR("\t Flag - None(%d), Path(%d), Paint(%d), All(%d)", p->mFlag&ChangeFlagNone, p->mFlag&ChangeFlagPath, p->mFlag&ChangeFlagPaint, p->mFlag&ChangeFlagAll);
        ERR("\t BrushType - mType(%d)", p->mType);
        ERR("\t FillRule - mFillRule(%d)", p->mFillRule);
        ERR("\t PathData - ptPtr(%p), ptCount(%d), elmPtr(%p), elmCount(%d)", p->mPath.ptPtr, p->mPath.ptCount, p->mPath.elmPtr, p->mPath.elmCount);
        ERR("\t Color - r(%d) g(%d) b(%d) a(%d)", p->mColor.r, p->mColor.g, p->mColor.b, p->mColor.a);
        ERR("\t Stroke - enable(%d), width(%d), cap(%d), join(%d), meterLimit(%d), dashArray(%p), dashArraySize(%d)", p->mStroke.enable, p->mStroke.width, p->mStroke.cap, p->mStroke.join, p->mStroke.meterLimit, p->mStroke.dashArray, p->mStroke.dashArraySize);
        ERR("\t Gradient - type(%d), start(%f %f), end(%f %f), center(%f %f), focal(%f %f), center_radius(%f) focal_radius(%f)", p->mGradient.type, p->mGradient.start.x, p->mGradient.start.y, p->mGradient.end.x, p->mGradient.end.y, p->mGradient.center.x, p->mGradient.center.y, p->mGradient.focal.x, p->mGradient.focal.y, p->mGradient.cradius, p->mGradient.fradius);
        ERR("\n");
#endif
#if 0
        Efl_VG* shape = evas_vg_shape_add(root);
        const float *data = p->mPath.ptPtr;

        for (int i = 0; i < p->mPath.elmCount; i++)
          {
             switch (p->mPath.elmPtr[i])
               {
                case 0: //moveTo
                   evas_vg_shape_append_move_to(shape, data[0], data[1]);
                   data += 2;
                   break;
                case 1:
                   evas_vg_shape_append_line_to(shape, data[0], data[1]);
                   data += 2;
                   break;
                case 2:
                   evas_vg_shape_append_cubic_to(shape, data[0], data[1], data[2], data[3], data[4], data[5]);
                   data += 6;
                   break;
                case 3:
                   evas_vg_shape_append_close(shape);
                   break;
                default:
                   printf("eh?\n");

               }
          }

        if (p->mStroke.enable)
          {
             evas_vg_shape_stroke_width_set(shape, p->mStroke.width);
          }

        if (p->mType == BrushSolid)
          {
             int r = p->mColor.r * (p->mColor.a/255);
             int g = p->mColor.g * (p->mColor.a/255);
             int b = p->mColor.g * (p->mColor.a/255);
             int a = p->mColor.a;
             if (p->mStroke.enable)
               evas_vg_shape_stroke_color_set(shape, r, g, b, a);
             else
               evas_vg_node_color_set(shape, r, g, b, a);
          }
     }
#endif
     }

   return EINA_TRUE;
}

static Vg_File_Data*
evas_vg_load_file_open_json(const char *file,
                            const char *key EINA_UNUSED,
                            int *error EINA_UNUSED)
{
   Vg_File_Data *vfd = calloc(1, sizeof(Vg_File_Data));
   if (!vfd) return NULL;

   LOTPlayer *player = lotplayer_create();
   if (!player)
     {
        ERR("Failed to create LOTPlayer");
        free(vfd);
        return NULL;
     }

   int ret = lotplayer_set_file(player, file);
   if (LOT_PLAYER_ERROR_NONE != ret)
     {
        ERR("Failed to lotplayer_set_file(), result = %d", ret);
        free(vfd);
        lotplayer_destroy(player);
        return NULL;
     }

   vfd->loader_data = (void *) player;

   ERR("open json vfd(%p) lotplayer! = %p, file = %s", vfd, player, file);

   return vfd;
}

static Evas_Vg_Load_Func evas_vg_load_json_func =
{
   evas_vg_load_file_open_json,
   evas_vg_load_file_close_json,
   evas_vg_load_file_data_json
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_vg_load_json_func);
   _evas_vg_loader_json_log_dom = eina_log_domain_register
     ("vg-load-json", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_vg_loader_json_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   if (_evas_vg_loader_json_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_vg_loader_json_log_dom);
        _evas_vg_loader_json_log_dom = -1;
     }
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "json",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_VG_LOADER, vg_loader, json);

#ifndef EVAS_STATIC_BUILD_VG_JSON
EVAS_EINA_MODULE_DEFINE(vg_loader, json);
#endif

