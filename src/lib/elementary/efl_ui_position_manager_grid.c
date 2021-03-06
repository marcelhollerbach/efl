#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

#define MY_CLASS      EFL_UI_POSITION_MANAGER_GRID_CLASS
#define MY_DATA_GET(obj, pd) \
  Efl_Ui_Position_Manager_Grid_Data *pd = efl_data_scope_get(obj, MY_CLASS);

typedef struct {
   Eina_Accessor *content_acc, *size_acc;
   unsigned int size;
   Eina_Rect viewport;
   Eina_Vector2 scroll_position;
   Efl_Ui_Layout_Orientation dir;
   struct {
      unsigned int start_id, end_id;
   } prev_run;
   Eina_Size2D max_min_size;
   Eina_Size2D last_viewport_size;
   Eina_Size2D prev_min_size;
   struct {
      int columns;
      int rows;
   } current_display_table;
} Efl_Ui_Position_Manager_Grid_Data;

static inline void
vis_change_segment(Efl_Ui_Position_Manager_Grid_Data *pd, int a, int b, Eina_Bool flag)
{
   for (int i = MIN(a, b); i < MAX(a, b); ++i)
     {
        Efl_Gfx_Entity *ent;

        EINA_SAFETY_ON_FALSE_RETURN(eina_accessor_data_get(pd->content_acc, i, (void**) &ent));
        if (ent && !efl_ui_focus_object_focus_get(ent))
          {
             efl_gfx_entity_visible_set(ent, flag);
          }
     }
}

static void
_reposition_content(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd)
{
   Eina_Size2D space_size;
   int relevant_space_size, relevant_viewport;
   unsigned int start_id, end_id, step;

   if (!pd->size) return;
   if (pd->max_min_size.w <= 0 || pd->max_min_size.h <= 0) return;
   if (pd->current_display_table.columns <= 0 || pd->current_display_table.rows <= 0) return;

   //space size contains the amount of space that is outside the viewport (either to the top or to the left)
   space_size.w = (MAX(pd->last_viewport_size.w - pd->viewport.w, 0))*pd->scroll_position.x;
   space_size.h = (MAX(pd->last_viewport_size.h - pd->viewport.h, 0))*pd->scroll_position.y;

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        relevant_space_size = space_size.h;
        relevant_viewport = pd->viewport.h;
        step = pd->max_min_size.h;
     }
   else
     {
        relevant_space_size = space_size.w;
        relevant_viewport = pd->viewport.w;
        step = pd->max_min_size.w;
     }
   start_id =  MIN((MAX(relevant_space_size,0) / step)*pd->current_display_table.columns, pd->size);
   end_id =  MIN((((MAX(relevant_space_size,0) + relevant_viewport + step) / step)*pd->current_display_table.columns)+1, pd->size);

   EINA_SAFETY_ON_FALSE_RETURN(start_id <= end_id);
   EINA_SAFETY_ON_FALSE_RETURN(start_id <= pd->size);

   //to performance optimize the whole widget, we are setting the objects that are outside the viewport to visibility false
   //The code below ensures that things outside the viewport are always hidden, and things inside the viewport are visible
   if (end_id < pd->prev_run.start_id || start_id > pd->prev_run.end_id)
     {
        //it is important to first make the segment visible here, and then hide the rest
        //otherwise we get a state where item_container has 0 subchildren, which triggers a lot of focus logic.
        vis_change_segment(pd, start_id, end_id, EINA_TRUE);
        vis_change_segment(pd, pd->prev_run.start_id, pd->prev_run.end_id, EINA_FALSE);
     }
   else
     {
        vis_change_segment(pd, pd->prev_run.start_id, start_id, (pd->prev_run.start_id > start_id));
        vis_change_segment(pd, pd->prev_run.end_id, end_id, (pd->prev_run.end_id < end_id));
     }

   for (unsigned int i = start_id; i < end_id; ++i)
     {
        Eina_Rect geom;
        Efl_Gfx_Entity *ent;
        geom.size = pd->max_min_size;
        geom.pos = pd->viewport.pos;

        if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
          {
             geom.x += pd->max_min_size.w*(i%pd->current_display_table.columns);
             geom.y += pd->max_min_size.h*(i/pd->current_display_table.columns);
             geom.y -= (relevant_space_size);
          }
        else
          {
             geom.x += pd->max_min_size.w*(i/pd->current_display_table.columns);
             geom.y += pd->max_min_size.h*(i%pd->current_display_table.columns);
             geom.x -= (relevant_space_size);
          }

        EINA_SAFETY_ON_FALSE_RETURN(eina_accessor_data_get(pd->content_acc, i, (void**) &ent));
        //printf(">%d (%d, %d, %d, %d) %p\n", i, geom.x, geom.y, geom.w, geom.h, ent);
        efl_gfx_entity_geometry_set(ent, geom);
     }
   pd->prev_run.start_id = start_id;
   pd->prev_run.end_id = end_id;
}

static inline void
_flush_abs_size(Eo *obj, Efl_Ui_Position_Manager_Grid_Data *pd)
{
   int minor, major;
   Eina_Size2D vp_size;

   if (!pd->size) return;
   if (pd->max_min_size.w <= 0 || pd->max_min_size.h <= 0) return;

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        major = pd->viewport.w/pd->max_min_size.w;
        pd->current_display_table.columns = major;
     }
   else
     {
        major = pd->viewport.h/pd->max_min_size.h;
        pd->current_display_table.columns = major;
     }

   if (major <= 0) return;
   minor = ceil((double)pd->size/(double)major);

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     pd->current_display_table.rows = minor;
   else
     pd->current_display_table.rows = minor;

   /*
    * calculate how much size we need with major in the given orientation.
    * The
    */
   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        vp_size.w = pd->viewport.w;
        vp_size.h = minor*pd->max_min_size.h;
     }
   else
     {
        vp_size.h = pd->viewport.h;
        vp_size.w = minor*pd->max_min_size.w;
     }
   if (vp_size.h != pd->last_viewport_size.h || vp_size.w != pd->last_viewport_size.w)
     {
        pd->last_viewport_size = vp_size;
        efl_event_callback_call(obj, EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_SIZE_CHANGED, &vp_size);
     }
}

static inline void
_update_min_size(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, int added_index)
{
   Eina_Size2D elemsize;

   EINA_SAFETY_ON_FALSE_RETURN(eina_accessor_data_get(pd->size_acc, added_index, (void*)&elemsize));
   pd->max_min_size.w = MAX(pd->max_min_size.w, elemsize.w);
   pd->max_min_size.h = MAX(pd->max_min_size.h, elemsize.h);
}

static inline void
_flush_min_size(Eo *obj, Efl_Ui_Position_Manager_Grid_Data *pd)
{
   Eina_Size2D min_size = pd->max_min_size;

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     min_size.h = -1;
   else
     min_size.w = -1;

   if (pd->prev_min_size.w != min_size.w || pd->prev_min_size.h != min_size.h)
     {
        pd->prev_min_size = min_size;
        efl_event_callback_call(obj, EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_MIN_SIZE_CHANGED, &min_size);
     }
}

EOLIAN static void
_efl_ui_position_manager_grid_efl_ui_position_manager_entity_data_access_set(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, Eina_Accessor *obj_access, Eina_Accessor *size_access, int size)
{
   pd->size_acc = size_access;
   pd->content_acc = obj_access;
   pd->size = size;
}

EOLIAN static void
_efl_ui_position_manager_grid_efl_ui_position_manager_entity_viewport_set(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, Eina_Rect viewport)
{
   pd->viewport = viewport;
   _flush_abs_size(obj, pd);
   _reposition_content(obj, pd);
}

EOLIAN static void
_efl_ui_position_manager_grid_efl_ui_position_manager_entity_scroll_position_set(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, double x, double y)
{
   pd->scroll_position.x = x;
   pd->scroll_position.y = y;
   _reposition_content(obj, pd);
}

EOLIAN static void
_efl_ui_position_manager_grid_efl_ui_position_manager_entity_item_added(Eo *obj, Efl_Ui_Position_Manager_Grid_Data *pd, int added_index, Efl_Gfx_Entity *subobj EINA_UNUSED)
{
   pd->size ++;

   efl_gfx_entity_visible_set(subobj, EINA_FALSE);
   _update_min_size(obj, pd, added_index);
   _flush_min_size(obj, pd);
   _flush_abs_size(obj, pd);
   _reposition_content(obj, pd); //FIXME we might can skip that
}

EOLIAN static void
_efl_ui_position_manager_grid_efl_ui_position_manager_entity_item_removed(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, int removed_index EINA_UNUSED, Efl_Gfx_Entity *subobj EINA_UNUSED)
{
   EINA_SAFETY_ON_FALSE_RETURN(pd->size > 0);
   pd->size --;
   pd->prev_run.start_id = MIN(pd->prev_run.start_id, pd->size);
   pd->prev_run.end_id = MIN(pd->prev_run.end_id, pd->size);
   //we ignore here that we might loose the item giving the current max min size
   _flush_abs_size(obj, pd);
   _reposition_content(obj, pd); //FIXME we might can skip that
   efl_gfx_entity_visible_set(subobj, EINA_TRUE);
}


EOLIAN static void
_efl_ui_position_manager_grid_efl_ui_position_manager_entity_item_size_changed(Eo *obj, Efl_Ui_Position_Manager_Grid_Data *pd, int start_id, int end_id)
{
   for (int i = start_id; i <= end_id; ++i)
     {
        _update_min_size(obj, pd, i);
     }

   _flush_min_size(obj, pd);
   _flush_abs_size(obj, pd);
   _reposition_content(obj, pd); //FIXME we could check if this is needed or not
}

EOLIAN static void
_efl_ui_position_manager_grid_efl_ui_layout_orientable_orientation_set(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, Efl_Ui_Layout_Orientation dir)
{
   pd->dir = dir;
   _flush_min_size(obj, pd);
   _flush_abs_size(obj, pd);
   _reposition_content(obj, pd); //FIXME we could check if this is needed or not
}


EOLIAN static Efl_Ui_Layout_Orientation
_efl_ui_position_manager_grid_efl_ui_layout_orientable_orientation_get(const Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd)
{
   return pd->dir;
}

EOLIAN static Eina_Rect
_efl_ui_position_manager_grid_efl_ui_position_manager_entity_position_single_item(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, int idx)
{
   Eina_Rect geom;
   Eina_Size2D space_size;
   unsigned int relevant_space_size;

   if (!pd->size) return EINA_RECT(0, 0, 0, 0);
   if (pd->max_min_size.w <= 0 || pd->max_min_size.h <= 0) return EINA_RECT(0, 0, 0, 0);
   if (pd->current_display_table.columns <= 0 || pd->current_display_table.rows <= 0) return EINA_RECT(0, 0, 0, 0);

   //space size contains the amount of space that is outside the viewport (either to the top or to the left)
   space_size.w = (MAX(pd->last_viewport_size.w - pd->viewport.w, 0))*pd->scroll_position.x;
   space_size.h = (MAX(pd->last_viewport_size.h - pd->viewport.h, 0))*pd->scroll_position.y;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(space_size.w >= 0 && space_size.h >= 0, EINA_RECT(0, 0, 0, 0));
   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        relevant_space_size = space_size.h;
     }
   else
     {
        relevant_space_size = space_size.w;
     }
   geom.size = pd->max_min_size;
   geom.pos = pd->viewport.pos;

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        geom.x += pd->max_min_size.w*(idx%pd->current_display_table.columns);
        geom.y += pd->max_min_size.h*(idx/pd->current_display_table.columns);
        geom.y -= (relevant_space_size);
     }
   else
     {
        geom.x += pd->max_min_size.w*(idx/pd->current_display_table.columns);
        geom.y += pd->max_min_size.h*(idx%pd->current_display_table.columns);
        geom.x -= (relevant_space_size);
     }

   return geom;
}

#include "efl_ui_position_manager_grid.eo.c"
