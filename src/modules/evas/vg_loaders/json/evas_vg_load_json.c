#include <lottieanimation_capi.h>
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

   Lottie_Animation *lot_anim = (Lottie_Animation *) vfd->loader_data;
   lottie_animation_destroy(lot_anim);
   if (vfd->anim_data) free(vfd->anim_data);
   free(vfd);

   return EINA_TRUE;
}

static Eina_Bool
evas_vg_load_file_data_json(Vg_File_Data *vfd)
{
   return vg_common_json_create_vg_node(vfd);
}

static Vg_File_Data*
evas_vg_load_file_open_json(const char *file,
                            const char *key EINA_UNUSED,
                            int *error EINA_UNUSED)
{
   Vg_File_Data *vfd = calloc(1, sizeof(Vg_File_Data));
   if (!vfd) return NULL;

   Lottie_Animation *lot_anim = lottie_animation_from_file(file);
   if (!lot_anim)
     {
        ERR("Failed lottie_animation_from_file");
        goto err;
     }

   unsigned int frame_cnt = lottie_animation_get_totalframe(lot_anim);

   //Support animation
   if (frame_cnt > 1)
     {
        vfd->anim_data = calloc(1, sizeof(Vg_File_Anim_Data));
        if (!vfd->anim_data) goto err;
        vfd->anim_data->duration = lottie_animation_get_duration(lot_anim);
        vfd->anim_data->frame_cnt = frame_cnt;
     }

   vfd->loader_data = (void *) lot_anim;

   return vfd;

err:
   if (vfd)
     {
        if (vfd->anim_data) free(vfd->anim_data);
        free(vfd);
     }
   if (lot_anim) lottie_animation_destroy(lot_anim);

   return NULL;
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
EVAS_EINA_MODULE_DEFINE(vg_loader, json)
#endif

