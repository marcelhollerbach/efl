typedef Eo Elm_Animation_View;

typedef enum
{
   ELM_ANIMATION_VIEW_STATE_NOT_READY, /*< Animation view is not ready yet to play. i.e. it didn't have file set. @since 1.22 */
   ELM_ANIMATION_VIEW_STATE_PLAY, /*< Animation view is on playing animation. @since 1.22 */
   ELM_ANIMATION_VIEW_STATE_PLAY_BACK, /*< Animation view is on playing rewind animation. @since 1.22 */
   ELM_ANIMATION_VIEW_STATE_PAUSE, /*< Animation view is paused. To continue animation, please resume animation. @since 1.22 */
   ELM_ANIMATION_VIEW_STATE_STOP /*< After animation is finished or stopped by request. @since 1.22 */
} Elm_Animation_View_State;

/**
 * Add a new animation view to the parent's canvas
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Elm_Animation_View
 *
 * @since 1.22
 */
EAPI Elm_Animation_View     *elm_animation_view_add(Evas_Object *parent);

/**
 *
 * Set the source file from where an animation view must fetch the real
 * vector data (it may be an Eet file, besides pure image ones).
 *
 * If the file supports multiple data stored in it (as Eet files do),
 * you can specify the key to be used as the index of the vector in
 * this file.
 *
 * @param[in] file The vector file path.
 * @param[in] key The vector key in @p file (if its an Eet one), or @c
NULL, otherwise.
 *
 * @ingroup Elm_Animation_View
 *
 * @since 1.22
*/
EAPI Eina_Bool        elm_animation_view_file_set(Evas_Object *obj, const char *file, const char *key);

/**
 * @brief Get the animation view state as a while
 *
 * @return The current animation view state
 *
 * @ingroup Elm_Animation_View
 *
 * @since 1.22
 */
EAPI Elm_Animation_View_State elm_animation_view_state_get(const Elm_Animation_View *obj);

#include "elm_animation_view.eo.legacy.h"
