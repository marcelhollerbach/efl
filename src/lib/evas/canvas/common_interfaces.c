#include "evas_common_private.h"
#include "evas_private.h"

#include "Eo.h"

Eina_Hash* signals_hash_table = NULL;

EAPI const Eo_Event_Description _ZOOM_START_EVENT =
   EO_EVENT_DESCRIPTION("zoom,start", "");

EAPI const Eo_Event_Description _ZOOM_STOP_EVENT =
   EO_EVENT_DESCRIPTION("zoom,stop", "");

EAPI const Eo_Event_Description _ZOOM_CHANGE_EVENT =
   EO_EVENT_DESCRIPTION("zoom,changed", "");

EAPI const Eo_Event_Description _SELECTED_EVENT =
   EO_EVENT_DESCRIPTION("selected", "");

EAPI const Eo_Event_Description _UNSELECTED_EVENT =
   EO_EVENT_DESCRIPTION("unselected", "");

EAPI const Eo_Event_Description _SELECTION_PASTE_EVENT =
   EO_EVENT_DESCRIPTION("selection,paste", "");

EAPI const Eo_Event_Description _SELECTION_COPY_EVENT =
   EO_EVENT_DESCRIPTION("selection,copy", "");

EAPI const Eo_Event_Description _SELECTION_CUT_EVENT =
   EO_EVENT_DESCRIPTION("selection,cut", "");

EAPI const Eo_Event_Description _SELECTION_START_EVENT =
   EO_EVENT_DESCRIPTION("selection,start", "");

EAPI const Eo_Event_Description _SELECTION_CHANGED_EVENT =
   EO_EVENT_DESCRIPTION("selection,changed", "");

EAPI const Eo_Event_Description _SELECTION_CLEARED_EVENT =
   EO_EVENT_DESCRIPTION("selection,cleared", "");


static const Eo_Event_Description *_zoomable_events_desc[] = {
     EVAS_SMART_ZOOM_START_EVENT,
     EVAS_SMART_ZOOM_STOP_EVENT,
     EVAS_SMART_ZOOM_CHANGE_EVENT,
     NULL
};

static const Eo_Event_Description *_selectable_events_desc[] = {
     EVAS_SMART_SELECTED_EVENT,
     EVAS_SMART_UNSELECTED_EVENT,
     EVAS_SMART_SELECTION_PASTE_EVENT,
     EVAS_SMART_SELECTION_COPY_EVENT,
     EVAS_SMART_SELECTION_CUT_EVENT,
     EVAS_SMART_SELECTION_START_EVENT,
     EVAS_SMART_SELECTION_CHANGED_EVENT,
     EVAS_SMART_SELECTION_CLEARED_EVENT,
     NULL
};

static const Eo_Event_Description *_evas_clickable_interface_event_desc[]; 
static const Eo_Event_Description *_evas_draggable_interface_event_desc[];
static const Eo_Event_Description *_evas_scrollable_interface_event_desc[];

#define ADD_SIGNAL(name, event) eina_hash_add(signals_hash_table, name, event)
#define ADD_INTERFACE_SIGNALS(events_desc) \
   do \
   { \
      int i = 0; \
      while (events_desc[i]) { \
           _Evas_Event_Description *desc = calloc(1, sizeof(*desc)); \
           desc->eo_desc = (Eo_Event_Description *)events_desc[i];                           \
           desc->is_desc_allocated = EINA_FALSE;                     \
           ADD_SIGNAL(events_desc[i]->name, desc);                   \
           i++; \
      } \
   } \
   while (0);

static void
_signal_interface_del(void *data)
{
   _Evas_Event_Description *desc = data;
   if (desc->is_desc_allocated) free(desc->eo_desc);
   free(desc);
}

static void
_evas_signal_interface_class_constructor(Eo_Class *klass EINA_UNUSED)
{
   signals_hash_table = eina_hash_string_superfast_new(_signal_interface_del);
   ADD_INTERFACE_SIGNALS(_evas_draggable_interface_event_desc);
   ADD_INTERFACE_SIGNALS(_evas_scrollable_interface_event_desc);
   ADD_INTERFACE_SIGNALS(_zoomable_events_desc);
   ADD_INTERFACE_SIGNALS(_selectable_events_desc);
   ADD_INTERFACE_SIGNALS(_evas_draggable_interface_event_desc);
}
#undef ADD_INTERFACE_SIGNALS
#undef ADD_SIGNAL

static void
_evas_signal_interface_class_destructor(Eo_Class *klass EINA_UNUSED)
{
   eina_hash_free(signals_hash_table);
}


static const Eo_Class_Description zoomable_interface_desc = {
     EO_VERSION,
     "Evas_Zoomable_Interface",
     EO_CLASS_TYPE_INTERFACE,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     _zoomable_events_desc,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(evas_smart_zoomable_interface_get, &zoomable_interface_desc, EVAS_SIGNAL_INTERFACE_CLASS, NULL);

static const Eo_Class_Description selectable_interface_desc = {
     EO_VERSION,
     "Evas_Selectable_Interface",
     EO_CLASS_TYPE_INTERFACE,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     _selectable_events_desc,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(evas_smart_selectable_interface_get, &selectable_interface_desc, EVAS_SIGNAL_INTERFACE_CLASS, NULL);

#include "canvas/evas_signal_interface.eo.c"
#include "canvas/evas_draggable_interface.eo.c"
#include "canvas/evas_clickable_interface.eo.c"
#include "canvas/evas_scrollable_interface.eo.c"