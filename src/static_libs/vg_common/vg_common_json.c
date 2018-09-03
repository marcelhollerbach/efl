#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "vg_common.h"
#include <Evas.h>

Eina_Bool
vg_common_json_create_vg_node(Vg_File_Data *vfd)
{
#ifdef BUILD_VG_LOADER_JSON
   LOTPlayer *player = (LOTPlayer *) vfd->loader_data;
   if (!player) return EINA_FALSE;

   //Root node
   if (vfd->root) efl_unref(vfd->root);
   vfd->root = efl_add_ref(EFL_CANVAS_VG_CONTAINER_CLASS, NULL);
   Efl_VG *root = vfd->root;
   if (!root) return EINA_FALSE;

   lotplayer_set_size(player, vfd->view_box.w, vfd->view_box.h);

   float progress = ((float) vfd->anim_data->frame_num / (float) vfd->anim_data->frame_cnt);
   int size = lotplayer_get_node_count(player, progress);

   //ERR("data json vfd = %p, player = %p, size = %d, root(%p) viewbox(%d %d %d %d) progress(%f)", vfd, player, size, root, vfd->view_box.x, vfd->view_box.y, vfd->view_box.w, vfd->view_box.h, progress);

   //Construct vg tree
   for (int i = 0; i < size; i++)
     {
        const LOTNode *p = lotplayer_get_node(player, progress, i);
        if (!p) continue;

#ifdef PRINT_LOTTIE_INFO
        ERR("%f[%d] node(%p)", progress, i, p);
        ERR("\t Flag - None(%d), Path(%d), Paint(%d), All(%d)", p->mFlag&ChangeFlagNone, p->mFlag&ChangeFlagPath, p->mFlag&ChangeFlagPaint, p->mFlag&ChangeFlagAll);
        ERR("\t BrushType - mType(%d)", p->mType);
        ERR("\t FillRule - mFillRule(%d)", p->mFillRule);
        ERR("\t PathData - ptPtr(%p), ptCount(%d), elmPtr(%p), elmCount(%d)", p->mPath.ptPtr, p->mPath.ptCount, p->mPath.elmPtr, p->mPath.elmCount);
        ERR("\t Color - r(%d) g(%d) b(%d) a(%d)", p->mColor.r, p->mColor.g, p->mColor.b, p->mColor.a);
        ERR("\t Stroke - enable(%d), width(%d), cap(%d), join(%d), meterLimit(%d), dashArray(%p), dashArraySize(%d)", p->mStroke.enable, p->mStroke.width, p->mStroke.cap, p->mStroke.join, p->mStroke.meterLimit, p->mStroke.dashArray, p->mStroke.dashArraySize);
        ERR("\t Gradient - type(%d), start(%f %f), end(%f %f), center(%f %f), focal(%f %f), center_radius(%f) focal_radius(%f)", p->mGradient.type, p->mGradient.start.x, p->mGradient.start.y, p->mGradient.end.x, p->mGradient.end.y, p->mGradient.center.x, p->mGradient.center.y, p->mGradient.focal.x, p->mGradient.focal.y, p->mGradient.cradius, p->mGradient.fradius);
        ERR("\n");
#endif

        Efl_VG* shape = evas_vg_shape_add(root);
        if (!shape) continue;

        const float *data = p->mPath.ptPtr;
        if (!data) continue;

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
                   ERR("What Path Type? = %d", p->mPath.elmPtr[i]);
               }
          }

        if (p->mStroke.enable)
          evas_vg_shape_stroke_width_set(shape, p->mStroke.width);

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
#else
   return EINA_FALSE;
#endif

   return EINA_TRUE;
}
