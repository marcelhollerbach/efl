#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "elm_suite.h"
#include <Elementary.h>

typedef struct _Simple_Test_Widget
{
   Evas_Object* (*constructor)(Evas_Object *win);
   const char *name;
} Simple_Test_Widget;

static Evas_Object*
_custom_clock(Evas_Object *win)
{
  Evas_Object *o = elm_clock_add(win);
  elm_clock_edit_set(o, EINA_TRUE);
  return o;
}

static Evas_Object*
_custom_gengrid(Evas_Object *win)
{
  Evas_Object *o = elm_gengrid_add(win);
  elm_gengrid_item_append(o, NULL, NULL, NULL, NULL);
  return o;
}

static Evas_Object*
_custom_genlist(Evas_Object *win)
{
  Evas_Object *o = elm_genlist_add(win);
  elm_genlist_item_append(o, NULL, NULL, NULL, 0, NULL, NULL);
  return o;
}

static Evas_Object*
_custom_list(Evas_Object *win)
{
  Evas_Object *o = elm_list_add(win);
  elm_list_item_append(o, "test", NULL, NULL, NULL, NULL);
  return o;
}


static const Simple_Test_Widget simple_widgets[] = {
  {elm_button_add, "button"},
  {elm_check_add, "check"},
  {elm_radio_add, "radio"},
  {elm_diskselector_add, "diskselector"},
  {elm_entry_add, "entry"},
  {elm_flipselector_add, "flipselector"},
  {elm_video_add, "video"},
  {elm_spinner_add, "spinner"},
  {elm_multibuttonentry_add, "mbe"},
  {elm_fileselector_add, "fileselector"},
  {elm_fileselector_button_add, "fileselector_button"},
  {elm_fileselector_entry_add, "fileselector_entry"},
  {_custom_clock, "clock"},
  {elm_toolbar_add, "toolbar"},
  {elm_gengrid_add, "gengrid1"},
  {elm_genlist_add, "genlist1"},
  {_custom_gengrid, "gengrid2"},
  {_custom_genlist, "genlist2"},
  {elm_list_add, "list1"},
  {_custom_list, "list2"},
  {NULL, NULL},
};

static void
_eventing_test(void *data, Evas_Object *obj, void *event_info)
{
   Eina_Bool *val = data;

   *val = EINA_TRUE;
}

EFL_START_TEST (elm_test_widget_focus_simple_widget)
{
   Evas_Object *win, *box, *resettor, *o;
   const char *file = NULL, *key = NULL;

   win = win_add(NULL, "focus test", ELM_WIN_BASIC);
   box = elm_box_add(win);
   elm_win_resize_object_add(win, box);
   evas_object_show(win);
   evas_object_show(box);

   resettor = o = elm_button_add(win);
   elm_box_pack_end(box, o);
   evas_object_show(o);
   elm_object_focus_set(o, EINA_TRUE);

   for (int i = 0; simple_widgets[i].name; ++i)
     {
        Eina_Bool flag_focused = EINA_FALSE, flag_unfocused = EINA_FALSE;

        printf("Testing %s\n", simple_widgets[i].name);

        o = simple_widgets[i].constructor(win);
        evas_object_smart_callback_add(o, "focused", _eventing_test, &flag_focused);
        evas_object_smart_callback_add(o, "unfocused", _eventing_test, &flag_unfocused);
        elm_box_pack_end(box, o);
        evas_object_show(o);

        //I have no idea why this is needed - but if this here is not done,
        // then elements that are part of a layout will NOT be shown even if
        // the window and layout is shown
        evas_object_hide(win);
        evas_object_show(win);

        elm_object_focus_set(o, EINA_TRUE);
        ck_assert_int_eq(flag_focused, EINA_TRUE);
        ck_assert_int_eq(flag_unfocused, EINA_FALSE);

        elm_object_focus_set(resettor, EINA_TRUE);
        ck_assert_int_eq(flag_focused, EINA_TRUE);
        ck_assert_int_eq(flag_unfocused, EINA_TRUE);
     }
}
EFL_END_TEST


void elm_test_widget_focus(TCase *tc)
{
   tcase_add_test(tc, elm_test_widget_focus_simple_widget);
}
