/**
 * @brief Add a new elm_code widget to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @see elm_code_widget_code_set
 *
 * @ingroup Data
 */
EAPI Evas_Object *elm_code_widget_add(Evas_Object *parent, Elm_Code *code);
EAPI void elm_code_widget_alpha_set(Elm_Code_Widget *widget, int alpha);

#include "elm_code_widget.eo.legacy.h"
