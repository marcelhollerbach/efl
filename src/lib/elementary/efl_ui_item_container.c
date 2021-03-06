#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED
#define EFL_UI_SCROLL_MANAGER_PROTECTED
#define EFL_UI_SCROLLBAR_PROTECTED
#define EFL_UI_WIDGET_FOCUS_MANAGER_PROTECTED

#include <Efl_Ui.h>
#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

#define MY_CLASS      EFL_UI_ITEM_CONTAINER_CLASS

#define MY_DATA_GET(obj, pd) \
  Efl_Ui_Item_Container_Data *pd = efl_data_scope_get(obj, MY_CLASS);

typedef struct {
   Efl_Ui_Scroll_Manager *smanager;
   Efl_Ui_Pan *pan;
   Eina_List *selected;
   Eina_List *items;
   Efl_Ui_Select_Mode mode;
   Efl_Ui_Layout_Orientation dir;
   Eina_Size2D content_min_size;
   Efl_Ui_Position_Manager_Entity *pos_man;
   struct {
      Eina_Accessor pass_on;
      unsigned int last_index;
      const Eina_List *current;
   } obj_accessor;
   struct {
      Eina_Bool w;
      Eina_Bool h;
   } match_content;
   Eina_Accessor size_accessor;
   Efl_Gfx_Entity *sizer;
} Efl_Ui_Item_Container_Data;

static Eina_Bool register_item(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Item *item);
static Eina_Bool unregister_item(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Item *item);

static void
flush_min_size(Eo *obj, Efl_Ui_Item_Container_Data *pd)
{
   Eina_Size2D tmp = pd->content_min_size;

   if (!pd->match_content.w)
     tmp.w = -1;

   if (!pd->match_content.h)
     tmp.h = -1;

   efl_gfx_hint_size_min_set(obj, tmp);
}

static int
clamp_index(Efl_Ui_Item_Container_Data *pd, int index)
{
   if (index < ((int)eina_list_count(pd->items)) * -1)
     return -1;
   else if (index > (int)eina_list_count(pd->items) - 1)
     return 1;
   return 0;
}

static int
index_adjust(Efl_Ui_Item_Container_Data *pd, int index)
{
   int c = eina_list_count(pd->items);
   if (index < c * -1)
     return 0;
   else if (index > c - 1)
     return c - 1;
   else if (index < 0)
     return index + c;
   return index;
}

static Eina_Bool
_obj_accessor_get_at(Eina_Accessor *accessor, unsigned int idx, void **data)
{
   ptrdiff_t offset = offsetof(Efl_Ui_Item_Container_Data, obj_accessor);
   Efl_Ui_Item_Container_Data *pd = (void*)accessor - offset;
   const Eina_List *over;
   unsigned int middle;
   unsigned int i;

   if (idx >= eina_list_count(pd->items))
     return EINA_FALSE;

   if (pd->obj_accessor.last_index == idx)
     over = pd->obj_accessor.current;
   else if (idx > pd->obj_accessor.last_index)
     {
        /* After current position. */
        middle = ((eina_list_count(pd->items) - pd->obj_accessor.last_index))/2;

        if (idx > middle)
          /* Go backward from the end. */
          for (i = eina_list_count(pd->items) - 1,
               over = eina_list_last(pd->items);
               i > idx && over;
               --i, over = eina_list_prev(over))
            ;
        else
          /* Go forward from current. */
          for (i = pd->obj_accessor.last_index, over = pd->obj_accessor.current;
               i < idx && over;
               ++i, over = eina_list_next(over))
            ;
     }
   else
     {
        /* Before current position. */
        middle = pd->obj_accessor.last_index/2;

        if (idx > middle)
          /* Go backward from current. */
          for (i = pd->obj_accessor.last_index, over = pd->obj_accessor.current;
               i > idx && over;
               --i, over = eina_list_prev(over))
            ;
        else
          /* Go forward from start. */
          for (i = 0, over = pd->items;
               i < idx && over;
               ++i, over = eina_list_next(over))
            ;
     }

   if (!over)
     return EINA_FALSE;

   pd->obj_accessor.last_index = idx;
   pd->obj_accessor.current = over;

   *data = eina_list_data_get(over);
   return EINA_TRUE;
}

static Eina_Accessor*
_obj_clone(Eina_Accessor *accessor)
{
   ptrdiff_t offset = offsetof(Efl_Ui_Item_Container_Data, obj_accessor);
   Efl_Ui_Item_Container_Data *pd = (void*)accessor - offset;

   return eina_list_accessor_new(pd->items);
}

static Eina_List *
_null_container(Eina_Accessor *accessor EINA_UNUSED)
{
   ERR("Not allowed to get a container!");
   return NULL;
}

static void
_free(Eina_Accessor *accessor EINA_UNUSED)
{
   ERR("Freeing this accessor is not supported");
}

static void
_obj_accessor_init(Eina_Accessor *accessor)
{
   //this is the accessor for accessing the items
   //we have to workarround here the problem that
   //no accessor can be created for a not yet created list.
   accessor->version = EINA_ACCESSOR_VERSION;
   accessor->get_at = FUNC_ACCESSOR_GET_AT(_obj_accessor_get_at);
   accessor->clone = FUNC_ACCESSOR_CLONE(_obj_clone);
   accessor->get_container = FUNC_ACCESSOR_GET_CONTAINER(_null_container);
   accessor->free = FUNC_ACCESSOR_FREE(_free);
   EINA_MAGIC_SET(accessor, EINA_MAGIC_ACCESSOR);
}

static Eina_Bool
_size_accessor_get_at(Eina_Accessor *accessor, unsigned int idx, void **data)
{
   Eina_Size2D *size = (Eina_Size2D*)data;
   ptrdiff_t offset = offsetof(Efl_Ui_Item_Container_Data, size_accessor);
   Efl_Ui_Item_Container_Data *pd = (void*)accessor - offset;

   if (idx > eina_list_count(pd->items))
     return EINA_FALSE;

   Eo *subobj = eina_list_nth(pd->items, idx);

   *size = efl_gfx_hint_size_combined_min_get(subobj);

   return EINA_TRUE;
}

static Eina_Accessor*
_size_clone(Eina_Accessor *accessor EINA_UNUSED)
{
   return NULL;
}

static void
_size_accessor_init(Eina_Accessor *accessor)
{
   accessor->version = EINA_ACCESSOR_VERSION;
   accessor->get_at = FUNC_ACCESSOR_GET_AT(_size_accessor_get_at);
   accessor->clone = FUNC_ACCESSOR_CLONE(_size_clone);
   accessor->get_container = FUNC_ACCESSOR_GET_CONTAINER(_null_container);
   accessor->free = FUNC_ACCESSOR_FREE(_free);
   EINA_MAGIC_SET(accessor, EINA_MAGIC_ACCESSOR);
}

static void
_pan_viewport_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   MY_DATA_GET(data, pd);
   Eina_Rect rect = efl_ui_scrollable_viewport_geometry_get(data);

   efl_ui_position_manager_entity_viewport_set(pd->pos_man, rect);
}

static void
_pan_position_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   MY_DATA_GET(data, pd);
   Eina_Position2D pos = efl_ui_pan_position_get(pd->pan);
   Eina_Position2D max = efl_ui_pan_position_max_get(pd->pan);
   Eina_Vector2 rpos = {0.0, 0.0};

   if (max.x > 0.0)
     rpos.x = (double)pos.x/(double)max.x;
   if (max.y > 0.0)
     rpos.y = (double)pos.y/(double)max.y;

   efl_ui_position_manager_entity_scroll_position_set(pd->pos_man, rpos.x, rpos.y);
}

EFL_CALLBACKS_ARRAY_DEFINE(pan_events_cb,
  {EFL_UI_PAN_EVENT_PAN_POSITION_CHANGED, _pan_position_changed_cb},
  {EFL_UI_PAN_EVENT_PAN_VIEWPORT_CHANGED, _pan_viewport_changed_cb},
)

static void
_item_scroll_internal(Eo *obj EINA_UNUSED,
                      Efl_Ui_Item_Container_Data *pd,
                      Efl_Ui_Item *item,
                      double align,
                      Eina_Bool anim)
{
   Eina_Rect ipos, view;
   Eina_Position2D vpos;

   if (!pd->smanager) return;

   ipos = efl_ui_position_manager_entity_position_single_item(pd->pos_man, eina_list_data_idx(pd->items, item));
   view = efl_ui_scrollable_viewport_geometry_get(pd->smanager);
   vpos = efl_ui_scrollable_content_pos_get(pd->smanager);

   ipos.x = ipos.x + vpos.x - view.x;
   ipos.y = ipos.y + vpos.y - view.y;

   //FIXME scrollable needs some sort of align, the docs do not even garantee to completly move in the element
   efl_ui_scrollable_scroll(pd->smanager, ipos, anim);
}

EOLIAN static void
_efl_ui_item_container_item_scroll(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Item *item, Eina_Bool animation)
{
   _item_scroll_internal(obj, pd, item, -1.0, animation);
}

EOLIAN static void
_efl_ui_item_container_item_scroll_align(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Item *item, double align, Eina_Bool animation)
{
   _item_scroll_internal(obj, pd, item, align, animation);
}

EOLIAN static Efl_Ui_Item*
_efl_ui_item_container_last_selected_item_get(const Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd)
{
   return eina_list_last_data_get(pd->selected);
}

EOLIAN static Eina_Iterator*
_efl_ui_item_container_selected_items_get(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd)
{
   return eina_list_iterator_new(pd->selected);
}

EOLIAN static Efl_Object*
_efl_ui_item_container_efl_object_constructor(Eo *obj, Efl_Ui_Item_Container_Data *pd EINA_UNUSED)
{
   Eo *o;

   pd->dir = EFL_UI_LAYOUT_ORIENTATION_VERTICAL;

   _obj_accessor_init(&pd->obj_accessor.pass_on);
   _size_accessor_init(&pd->size_accessor);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "item_container");

   o = efl_constructor(efl_super(obj, MY_CLASS));

   pd->sizer = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas_object_evas_get(obj));
   efl_gfx_color_set(pd->sizer, 0, 0, 0, 0);

   pd->pan = efl_add(EFL_UI_PAN_CLASS, obj);
   efl_content_set(pd->pan, pd->sizer);
   efl_event_callback_array_add(pd->pan, pan_events_cb(), obj);

   pd->smanager = efl_add(EFL_UI_SCROLL_MANAGER_CLASS, obj);
   efl_composite_attach(obj, pd->smanager);
   efl_ui_mirrored_set(pd->smanager, efl_ui_mirrored_get(obj));
   efl_ui_scroll_manager_pan_set(pd->smanager, pd->pan);

   efl_ui_scroll_connector_bind(obj, pd->smanager);

   return o;
}

EOLIAN static Efl_Object*
_efl_ui_item_container_efl_object_finalize(Eo *obj, Efl_Ui_Item_Container_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->pos_man, NULL);

   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Error
_efl_ui_item_container_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Item_Container_Data *pd)
{
   Eina_Error res;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_ERROR_GENERIC);
   res = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (res == EFL_UI_THEME_APPLY_ERROR_GENERIC) return res;
   efl_ui_mirrored_set(pd->smanager, efl_ui_mirrored_get(obj));
   efl_content_set(efl_part(wd->resize_obj, "efl.content"), pd->pan);

   return res;
}

EOLIAN static void
_efl_ui_item_container_efl_object_destructor(Eo *obj, Efl_Ui_Item_Container_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

static void
deselect_all(Efl_Ui_Item_Container_Data *pd)
{
   while(pd->selected)
     {
        Eo *item = eina_list_data_get(pd->selected);
        efl_ui_item_selected_set(item, EINA_FALSE);
        EINA_SAFETY_ON_TRUE_RETURN(eina_list_data_get(pd->selected) == item);
     }
}

EOLIAN static void
_efl_ui_item_container_efl_object_invalidate(Eo *obj, Efl_Ui_Item_Container_Data *pd EINA_UNUSED)
{
   efl_ui_item_container_position_manager_set(obj, NULL);

   deselect_all(pd);

   while(pd->items)
     efl_del(pd->items->data);

   efl_invalidate(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Iterator*
_efl_ui_item_container_efl_container_content_iterate(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd)
{
   return eina_list_iterator_new(pd->items);
}

EOLIAN static int
_efl_ui_item_container_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd)
{
   return eina_list_count(pd->items);
}

EOLIAN static void
_efl_ui_item_container_efl_ui_layout_orientable_orientation_set(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Layout_Orientation dir)
{
   if (pd->dir == dir) return;

   pd->dir = dir;
   if (pd->pos_man)
     efl_ui_layout_orientation_set(pd->pos_man, dir);
}

EOLIAN static Efl_Ui_Layout_Orientation
_efl_ui_item_container_efl_ui_layout_orientable_orientation_get(const Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd)
{
   return pd->dir;
}

EOLIAN static void
_efl_ui_item_container_efl_ui_scrollable_interactive_match_content_set(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd, Eina_Bool w, Eina_Bool h)
{
   if (pd->match_content.w == w && pd->match_content.h == h)
     return;

   pd->match_content.w = w;
   pd->match_content.h = h;

   efl_ui_scrollable_match_content_set(pd->smanager, w, h);
   flush_min_size(obj, pd);
}

EOLIAN static void
_efl_ui_item_container_efl_ui_multi_selectable_select_mode_set(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Select_Mode mode)
{
   pd->mode = mode;
   if ((mode == EFL_UI_SELECT_MODE_SINGLE_ALWAYS || mode == EFL_UI_SELECT_MODE_SINGLE) &&
       eina_list_count(pd->selected) > 0)
     {
        Efl_Ui_Item *last = eina_list_last_data_get(pd->selected);

        pd->selected = eina_list_remove_list(pd->selected, eina_list_last(pd->selected));
        deselect_all(pd);
        pd->selected = eina_list_append(pd->selected, last);
     }
   else if (mode == EFL_UI_SELECT_MODE_NONE && pd->selected)
     {
        deselect_all(pd);
     }
}

EOLIAN static Efl_Ui_Select_Mode
_efl_ui_item_container_efl_ui_multi_selectable_select_mode_get(const Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd)
{
   return pd->mode;
}

static void
_selected_cb(void *data, const Efl_Event *ev)
{
   Eo *obj = data;
   MY_DATA_GET(obj, pd);

   if (pd->mode == EFL_UI_SELECT_MODE_SINGLE_ALWAYS || pd->mode == EFL_UI_SELECT_MODE_SINGLE)
     {
        //we might get the situation that the item is already in the list and selected again, so just free the list, it will be rebuild below
        if (eina_list_data_get(pd->selected) == ev->object)
          {
            pd->selected = eina_list_free(pd->selected);
          }
        else
          {
             deselect_all(pd);
          }

     }
   else if (pd->mode == EFL_UI_SELECT_MODE_NONE)
     {
        ERR("Selection while mode is NONE, uncaught state!");
        return;
     }
   pd->selected = eina_list_append(pd->selected, ev->object);
   efl_event_callback_call(obj, EFL_UI_EVENT_ITEM_SELECTED, ev->object);
}

static void
_unselected_cb(void *data, const Efl_Event *ev)
{
   Eo *obj = data;
   MY_DATA_GET(obj, pd);

   pd->selected = eina_list_remove(pd->selected, ev->object);
   efl_event_callback_call(obj, EFL_UI_EVENT_ITEM_UNSELECTED, ev->object);
}

static void
_invalidate_cb(void *data, const Efl_Event *ev)
{
   Eo *obj = data;
   MY_DATA_GET(obj, pd);

   unregister_item(obj, pd, ev->object);
}

static void
_hints_changed_cb(void *data, const Efl_Event *ev)
{
   Eo *obj = data;
   MY_DATA_GET(obj, pd);
   int idx = eina_list_data_idx(pd->items, ev->object);

   efl_ui_position_manager_entity_item_size_changed(pd->pos_man, idx, idx);
}

static void
_redirect_cb(void *data, const Efl_Event *ev)
{
   Eo *obj = data;

#define REDIRECT_EVT(item_evt, item) \
   if (item_evt == ev->desc) efl_event_callback_call(obj, item, ev->object);
   REDIRECT_EVT(EFL_UI_EVENT_PRESSED, EFL_UI_EVENT_ITEM_PRESSED);
   REDIRECT_EVT(EFL_UI_EVENT_UNPRESSED, EFL_UI_EVENT_ITEM_UNPRESSED);
   REDIRECT_EVT(EFL_UI_EVENT_LONGPRESSED, EFL_UI_EVENT_ITEM_LONGPRESSED);
   REDIRECT_EVT(EFL_UI_EVENT_CLICKED_ANY, EFL_UI_EVENT_ITEM_CLICKED_ANY);
   REDIRECT_EVT(EFL_UI_EVENT_CLICKED, EFL_UI_EVENT_ITEM_CLICKED);
#undef REDIRECT_EVT
}

EFL_CALLBACKS_ARRAY_DEFINE(active_item,
  {EFL_GFX_ENTITY_EVENT_HINTS_CHANGED, _hints_changed_cb},
  {EFL_UI_EVENT_ITEM_SELECTED, _selected_cb},
  {EFL_UI_EVENT_ITEM_UNSELECTED, _unselected_cb},
  {EFL_UI_EVENT_PRESSED, _redirect_cb},
  {EFL_UI_EVENT_UNPRESSED, _redirect_cb},
  {EFL_UI_EVENT_LONGPRESSED, _redirect_cb},
  {EFL_UI_EVENT_CLICKED, _redirect_cb},
  {EFL_UI_EVENT_CLICKED_ANY, _redirect_cb},
  {EFL_EVENT_INVALIDATE, _invalidate_cb},
)

static Eina_Bool
register_item(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Item *item)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(item, EFL_UI_ITEM_CLASS), EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!!eina_list_data_find(pd->items, item), EINA_FALSE);

   if (!efl_ui_widget_sub_object_add(obj, item))
     return EINA_FALSE;

   efl_ui_item_container_set(item, obj);
   efl_canvas_group_member_add(pd->pan, item);
   efl_event_callback_array_add(item, active_item(), obj);
   efl_ui_mirrored_set(item, efl_ui_mirrored_get(obj));

   return EINA_TRUE;
}

static Eina_Bool
unregister_item(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Item *item)
{
   Eina_List *elem = eina_list_data_find_list(pd->items, item);
   if (!elem)
     {
        ERR("Item %p is not part of this widget", item);
        return EINA_FALSE;
     }

   if (!efl_ui_widget_sub_object_del(obj, item))
     return EINA_FALSE;

   unsigned int id = eina_list_data_idx(pd->items, item);
   if (pd->obj_accessor.last_index == id)
     {
        if (eina_list_next(elem))
          {
             pd->obj_accessor.current = eina_list_next(elem);
          }
        else if (eina_list_prev(elem))
          {
             pd->obj_accessor.last_index = id-1;
             pd->obj_accessor.current = eina_list_prev(elem);
          }
        else
          {
             //everything >= length is invalid, and we need that.
             pd->obj_accessor.last_index = eina_list_count(pd->items);
             pd->obj_accessor.current = NULL;
          }

     }

   pd->items = eina_list_remove(pd->items, item);
   pd->selected = eina_list_remove(pd->selected, item);
   efl_event_callback_array_del(item, active_item(), obj);
   efl_ui_position_manager_entity_item_removed(pd->pos_man, id, item);

   return EINA_TRUE;
}

static void
update_pos_man(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd, Efl_Gfx_Entity *subobj)
{
   int id = eina_list_data_idx(pd->items, subobj);
   if (id == 0)
     {
        pd->obj_accessor.last_index = id;
        pd->obj_accessor.current = pd->items;
     }
   efl_ui_position_manager_entity_item_added(pd->pos_man, id, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_pack_clear(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd)
{
   while(pd->items)
     {
        efl_del(pd->items->data);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_unpack_all(Eo *obj, Efl_Ui_Item_Container_Data *pd)
{
   while(pd->items)
     {
        if (!unregister_item(obj, pd, pd->items->data))
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

EOLIAN static Efl_Gfx_Entity*
_efl_ui_item_container_efl_pack_linear_pack_unpack_at(Eo *obj, Efl_Ui_Item_Container_Data *pd, int index)
{
   Efl_Ui_Item *it = eina_list_nth(pd->items, index_adjust(pd, index));

   EINA_SAFETY_ON_NULL_RETURN_VAL(it, NULL);

   if (!unregister_item(obj, pd, it))
     return NULL;

   return it;
}

EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_unpack(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Gfx_Entity *subobj)
{
   return unregister_item(obj, pd, subobj);
}


EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_pack(Eo *obj, Efl_Ui_Item_Container_Data *pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   return efl_pack_end(obj, subobj);
}


EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_linear_pack_end(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (!register_item(obj, pd, subobj))
     return EINA_FALSE;
   pd->items = eina_list_append(pd->items, subobj);
   update_pos_man(obj, pd, subobj);
   return EINA_TRUE;
}


EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_linear_pack_begin(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (!register_item(obj, pd, subobj))
     return EINA_FALSE;
   pd->items = eina_list_prepend(pd->items, subobj);
   update_pos_man(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_linear_pack_before(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   Eina_List *subobj_list = eina_list_data_find_list(pd->items, existing);
   EINA_SAFETY_ON_NULL_RETURN_VAL(subobj_list, EINA_FALSE);

   if (!register_item(obj, pd, subobj))
     return EINA_FALSE;
   pd->items = eina_list_prepend_relative_list(pd->items, subobj, subobj_list);
   update_pos_man(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_linear_pack_after(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   Eina_List *subobj_list = eina_list_data_find_list(pd->items, existing);
   EINA_SAFETY_ON_NULL_RETURN_VAL(subobj_list, EINA_FALSE);

   if (!register_item(obj, pd, subobj))
     return EINA_FALSE;
   pd->items = eina_list_append_relative_list(pd->items, subobj, subobj_list);
   update_pos_man(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_linear_pack_at(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Gfx_Entity *subobj, int index)
{
   Eina_List *subobj_list;
   int clamp;

   clamp = clamp_index(pd, index);
   index = index_adjust(pd, index);
   subobj_list = eina_list_nth_list(pd->items, index);
   EINA_SAFETY_ON_NULL_RETURN_VAL(subobj_list, EINA_FALSE);
   if (!register_item(obj, pd, subobj))
     return EINA_FALSE;
   if (clamp == 0)
     pd->items = eina_list_prepend_relative_list(pd->items, subobj, subobj_list);
   else if (clamp == 1)
     pd->items = eina_list_append(pd->items, subobj);
   else
     pd->items = eina_list_prepend(pd->items, subobj);
   update_pos_man(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static int
_efl_ui_item_container_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd, const Efl_Gfx_Entity *subobj)
{
   return eina_list_data_idx(pd->items, (void*)subobj);
}

EOLIAN static Efl_Gfx_Entity*
_efl_ui_item_container_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd, int index)
{
   return eina_list_nth(pd->items, index_adjust(pd, index));
}

static void
_pos_content_size_changed_cb(void *data, const Efl_Event *ev)
{
   Eina_Size2D *size = ev->info;
   MY_DATA_GET(data, pd);

   efl_gfx_entity_size_set(pd->sizer, *size);
}

static void
_pos_content_min_size_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eina_Size2D *size = ev->info;
   MY_DATA_GET(data, pd);

   pd->content_min_size = *size;

   flush_min_size(data, pd);
}

EFL_CALLBACKS_ARRAY_DEFINE(pos_manager_cbs,
  {EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_SIZE_CHANGED, _pos_content_size_changed_cb},
  {EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_MIN_SIZE_CHANGED, _pos_content_min_size_changed_cb},
)

EOLIAN static void
_efl_ui_item_container_position_manager_set(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Position_Manager_Entity *layouter)
{
   if (layouter)
     EINA_SAFETY_ON_FALSE_RETURN(efl_isa(layouter, EFL_UI_POSITION_MANAGER_ENTITY_INTERFACE));

   if (pd->pos_man)
     {
        efl_event_callback_array_del(pd->pos_man, pos_manager_cbs(), obj);
        efl_ui_position_manager_entity_data_access_set(pd->pos_man, NULL, NULL, 0);
        efl_del(pd->pos_man);
     }
   pd->pos_man = layouter;
   if (pd->pos_man)
     {
        efl_parent_set(pd->pos_man, obj);
        efl_event_callback_array_add(pd->pos_man, pos_manager_cbs(), obj);
        efl_ui_position_manager_entity_data_access_set(pd->pos_man, &pd->obj_accessor.pass_on, &pd->size_accessor, eina_list_count(pd->items));
        efl_ui_position_manager_entity_viewport_set(pd->pos_man, efl_ui_scrollable_viewport_geometry_get(obj));
        efl_ui_layout_orientation_set(pd->pos_man, pd->dir);
     }
}

EOLIAN static Efl_Ui_Position_Manager_Entity*
_efl_ui_item_container_position_manager_get(const Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd)
{
  return pd->pos_man;
}

EOLIAN static Efl_Ui_Focus_Manager*
_efl_ui_item_container_efl_ui_widget_focus_manager_focus_manager_create(Eo *obj, Efl_Ui_Item_Container_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *root)
{
   return efl_add(EFL_UI_FOCUS_MANAGER_CALC_CLASS, obj,
    efl_ui_focus_manager_root_set(efl_added, root));
}

EOLIAN static Eina_Bool
_efl_ui_item_container_efl_ui_widget_focus_state_apply(Eo *obj, Efl_Ui_Item_Container_Data *pd EINA_UNUSED, Efl_Ui_Widget_Focus_State current_state, Efl_Ui_Widget_Focus_State *configured_state, Efl_Ui_Widget *redirect EINA_UNUSED)
{
   return efl_ui_widget_focus_state_apply(efl_super(obj, MY_CLASS), current_state, configured_state, obj);
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_item_container_efl_ui_focus_manager_move(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Focus_Direction direction)
{
   Eo *new_obj, *focus;
   Eina_Size2D step;

   new_obj = efl_ui_focus_manager_move(efl_super(obj, MY_CLASS), direction);
   focus = efl_ui_focus_manager_focus_get(obj);
   step = efl_gfx_hint_size_min_get(focus);
   if (!new_obj)
     {
        Eina_Rect pos = efl_gfx_entity_geometry_get(focus);
        Eina_Rect view = efl_ui_scrollable_viewport_geometry_get(pd->smanager);
        Eina_Position2D vpos = efl_ui_scrollable_content_pos_get(pd->smanager);

        pos.x = pos.x + vpos.x - view.x;
        pos.y = pos.y + vpos.y - view.y;
        Eina_Position2D max = efl_ui_pan_position_max_get(pd->pan);

        if (direction == EFL_UI_FOCUS_DIRECTION_RIGHT)
          {
             if (pos.x < max.x)
               {
                  pos.x = MIN(max.x, pos.x + step.w);
                  efl_ui_scrollable_scroll(obj, pos, EINA_TRUE);
                  new_obj = focus;
               }
          }
        else if (direction == EFL_UI_FOCUS_DIRECTION_LEFT)
          {
             if (pos.x > 0)
               {
                  pos.x = MAX(0, pos.x - step.w);
                  efl_ui_scrollable_scroll(obj, pos, EINA_TRUE);
                  new_obj = focus;
               }
          }
        else if (direction == EFL_UI_FOCUS_DIRECTION_UP)
          {
             if (pos.y > 0)
               {
                  pos.y = MAX(0, pos.y - step.h);
                  efl_ui_scrollable_scroll(obj, pos, EINA_TRUE);
                  new_obj = focus;
               }
          }
        else if (direction == EFL_UI_FOCUS_DIRECTION_DOWN)
          {
             if (pos.y < max.y)
               {
                  pos.y = MAX(0, pos.y + step.h);
                  efl_ui_scrollable_scroll(obj, pos, EINA_TRUE);
                  new_obj = focus;
               }
          }
     }
   else
     {
        _item_scroll_internal(obj, pd, new_obj, .0, EINA_TRUE);
     }

   return new_obj;
}

#include "efl_ui_item_container.eo.c"
