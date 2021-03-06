#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"
#include "efl_ui_test_item_container_common.h"

Eo *position_manager;

static Eo* win;

static Eina_Array *arr_obj;
static Eina_Inarray *arr_size;
static Eina_Accessor inner_size_acc;
static Eina_Accessor *size_acc;
static void
item_container_setup()
{
   win = win_add();
}

static void
item_container_teardown()
{
   win = NULL;
}

static Eina_Bool
_get_at(Eina_Accessor *it EINA_UNUSED, unsigned int idx, void **data)
{
   Eina_Size2D *result_ptr = (void*)data;
   Eina_Size2D *inner_result;

   if (!eina_accessor_data_get(size_acc, idx, (void*)&inner_result))
     return EINA_FALSE;
   *result_ptr = *inner_result;
   return EINA_TRUE;
}

static void
_free_cb(Eina_Accessor *it EINA_UNUSED)
{
   eina_accessor_free(size_acc);
}

static Eina_Bool
_lock_cb(Eina_Accessor *it EINA_UNUSED)
{
   return eina_accessor_lock(size_acc);
}

static Eina_Accessor*
_clone_cb(Eina_Accessor *it EINA_UNUSED)
{
   return eina_accessor_clone(size_acc);
}


static void
_initial_setup(void)
{
   arr_obj = eina_array_new(10);
   arr_size = eina_inarray_new(sizeof(Eina_Size2D), 10);
   size_acc = eina_inarray_accessor_new(arr_size);

   inner_size_acc.version = EINA_ACCESSOR_VERSION;
   EINA_MAGIC_SET(&inner_size_acc, EINA_MAGIC_ACCESSOR);
   inner_size_acc.get_at = _get_at;
   inner_size_acc.free = _free_cb;
   inner_size_acc.lock = _lock_cb;
   inner_size_acc.clone = _clone_cb;

   efl_ui_position_manager_entity_data_access_set(position_manager,
      eina_array_accessor_new(arr_obj),
      &inner_size_acc, 0);
}

static int
_add_item(Eo *obj, Eina_Size2D size)
{
   int idx = eina_array_count(arr_obj);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_array_count(arr_obj) == eina_inarray_count(arr_size), -1);

   eina_array_push(arr_obj, (void*)0x1); //wtf
   eina_array_data_set(arr_obj, idx, obj);
   eina_inarray_push(arr_size, &size);


   efl_ui_position_manager_entity_item_added(position_manager, idx, obj);

   return idx;
}

static void
_update_item(int index, Eo *obj, Eina_Size2D size)
{
   Eina_Size2D *s;

   eina_array_data_set(arr_obj, index, obj);
   s = eina_inarray_nth(arr_size, index);
   *s = size;
}

static void
_ticker(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   efl_loop_quit(efl_main_loop_get(), EINA_VALUE_EMPTY);
}

static void
_iterate_a_few(void)
{
   efl_add(EFL_LOOP_TIMER_CLASS, efl_main_loop_get(),
      efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _ticker, NULL),
      efl_loop_timer_interval_set(efl_added, 0.1));
   efl_loop_begin(efl_main_loop_get());
}

EFL_START_TEST(no_crash1)
{
   _initial_setup();

   //try to resize the viewport while we have no item
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 200, 200));
   _iterate_a_few();
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 0, 0));
   _iterate_a_few();
   _add_item(efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, win), EINA_SIZE2D(20, 20));
   _iterate_a_few();
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 200, 200));
   _iterate_a_few();
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 0, 0));
   _iterate_a_few();
}
EFL_END_TEST

EFL_START_TEST(no_crash2)
{
   _initial_setup();

   //try to resize the viewport while we have no item
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 200, 200));
   _iterate_a_few();
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 0, 0));
   _iterate_a_few();
   _add_item(NULL, EINA_SIZE2D(20, 20));
   _iterate_a_few();
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 0, 0));
   _iterate_a_few();
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 200, 200));
   _iterate_a_few();
   _update_item(0, efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, win), EINA_SIZE2D(20, 20));
   _iterate_a_few();
}
EFL_END_TEST

static void
_content_size_cb(void *data, const Efl_Event *ev)
{
   Eina_Size2D *size = data;
   *size = *((Eina_Size2D*)ev->info);
}

EFL_START_TEST(viewport_newsize_event_result)
{
   Eina_Size2D size = EINA_SIZE2D(-2, -2), min_size = EINA_SIZE2D(-2, -2);
   efl_event_callback_add(position_manager,
      EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_SIZE_CHANGED, _content_size_cb, &size);
   efl_event_callback_add(position_manager,
      EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_MIN_SIZE_CHANGED, _content_size_cb, &min_size);
   _initial_setup();
   _add_item(efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, win), EINA_SIZE2D(20, 20));
   _add_item(efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, win), EINA_SIZE2D(20, 30));

   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(0, 0, 200, 200));
   _iterate_a_few();

   ck_assert_int_ne(size.w, -2);
   ck_assert_int_ne(size.h, -2);
   ck_assert_int_ne(min_size.w, -2);
   ck_assert_int_ne(min_size.h, -2);
}
EFL_END_TEST

void efl_ui_test_position_manager_common_add(TCase *tc)
{
   tcase_add_checked_fixture(tc, item_container_setup, item_container_teardown);
   tcase_add_test(tc, no_crash1);
   tcase_add_test(tc, no_crash2);
   tcase_add_test(tc, viewport_newsize_event_result);
}
