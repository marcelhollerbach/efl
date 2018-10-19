
/**
 * This widget emits the following signals, besides the ones sent from
 * @ref Elm_Layout:
 * @li "play,start": animation is just started.
 * @li "play,repeat": animation is just repeated.
 * @li "play,done": animation is finished.
 * @li "play,pause": animation is paused.
 * @li "play,resume": animation is resumed.
 * @li "play,stop": animation is stopped.
 * In all cases, the @c event parameter of the callback will be
 * @c NULL.
*/

typedef enum
{
   ELM_ANIMATION_VIEW_STATE_NOT_READY,
   ELM_ANIMATION_VIEW_STATE_PLAY,
   ELM_ANIMATION_VIEW_STATE_PLAY_BACK,
   ELM_ANIMATION_VIEW_STATE_PAUSE,
   ELM_ANIMATION_VIEW_STATE_STOP
} Elm_Animation_View_State;

//Default
EAPI Evas_Object     *elm_animation_view_add(Evas_Object *parent);
EAPI Eina_Bool        elm_animation_view_file_set(Evas_Object *obj, const char *file, const char *key);


//Candidates
//EAPI Eina_Bool        elm_animation_view_frame_set(Evas_Object *obj, int frame);
//EAPI int              elm_animation_view_frame_count_get(const Evas_Object *obj);
//EAPI int              elm_animation_view_frame_get(const Evas_Object *obj);
//EAPI int              elm_animation_view_duration_get(const Evas_Object *obj);

#include "elm_animation_view.eo.legacy.h"
