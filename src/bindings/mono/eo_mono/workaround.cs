#pragma warning disable 1591

using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

///<summary>Eo class description, passed to efl_class_new.</summary>
[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)]
public struct ClassDescription
{
    ///<summary>Current Eo version.</summary>
    public uint version;
    ///<summary>Name of the class.</summary>
    [MarshalAs(UnmanagedType.LPStr)] public String name;
    ///<summary>Class type.</summary>
    public int class_type;
    ///<summary>Size of data (private + protected + public) per instance.</summary>
    public UIntPtr data_size;
    ///<summary>Initializer for the class.</summary>
    public IntPtr class_initializer;
    ///<summary>Constructor of the class.</summary>
    public IntPtr class_constructor;
    ///<summary>Destructor of the class.</summary>
    public IntPtr class_destructor;
}

///<summary>Description of an Eo API operation.</summary>
[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)]
public struct Efl_Op_Description
{
    ///<summary>The EAPI function offering this op. (String with the name of the function on Windows)</summary>
    public IntPtr api_func;
    ///<summary>The static function to be called for this op</summary>
    public IntPtr func;
}

///<summary>List of operations on a given Object.</summary>
[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)]
public struct Efl_Object_Ops
{
    ///<summary>The op descriptions array of size count.</summary>
    public IntPtr descs;
    ///<summary>Number of op descriptions.</summary>
    public UIntPtr count;
};

[StructLayout(LayoutKind.Sequential)]
public struct EolianPD
{
    public IntPtr pointer;
}

#pragma warning disable 0169
public struct EvasObjectBoxLayout
{
    IntPtr o;
    IntPtr priv;
    IntPtr user_data;
};
[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)]
public struct EvasObjectBoxData
{
}
public delegate void EinaFreeCb(IntPtr data);
public struct EvasObjectBoxOption {
    IntPtr obj;
    [MarshalAsAttribute(UnmanagedType.U1)] bool max_reached;
    [MarshalAsAttribute(UnmanagedType.U1)] bool min_reached;
    Evas.Coord alloc_size;
};
#pragma warning restore 0169

namespace Efl {

[StructLayout(LayoutKind.Sequential)]
public struct EventDescription {
    public IntPtr Name;
    [MarshalAs(UnmanagedType.U1)] public bool Unfreezable;
    [MarshalAs(UnmanagedType.U1)] public bool Legacy_is;
    [MarshalAs(UnmanagedType.U1)] public bool Restart;

    private static Dictionary<string, IntPtr> descriptions = new Dictionary<string, IntPtr>();

    public EventDescription(string name)
    {
        this.Name = GetNative(name);
        this.Unfreezable = false;
        this.Legacy_is = false;
        this.Restart = false;
    }

    public static IntPtr GetNative(string name)
    {
        if (!descriptions.ContainsKey(name))
        {
            IntPtr data = Efl.Eo.Globals.dlsym(Efl.Eo.Globals.RTLD_DEFAULT, name);

            if (data == IntPtr.Zero) {
                string error = Eina.StringConversion.NativeUtf8ToManagedString(Efl.Eo.Globals.dlerror());
                throw new Exception(error);
            }
            descriptions.Add(name, data);
        }
        return descriptions[name];
    }
};


public delegate void EventCb(System.IntPtr data, ref Event_StructInternal evt);
#pragma warning disable 0169
public struct Dbg_Info {
    IntPtr name;
    IntPtr value;
};
#pragma warning restore 0169

[StructLayout(LayoutKind.Sequential)]
public struct TextCursorCursor {
    IntPtr obj;
    UIntPtr pos; // UIntPtr to automatically change size_t between 32/64
    IntPtr node;
    [MarshalAsAttribute(UnmanagedType.U1)]bool changed;
}

[StructLayout(LayoutKind.Sequential)]
public struct TextAnnotateAnnotation {
    IntPtr list;
    IntPtr obj;
    IntPtr start_node;
    IntPtr end_node;
    [MarshalAsAttribute(UnmanagedType.U1)]bool is_item;
}

public delegate void SignalCb(IntPtr data, IntPtr obj, IntPtr emission, IntPtr source);

namespace Access {

public struct ActionData {
    public IntPtr name;
    public IntPtr action;
    public IntPtr param;
    public IntPtr func;
}

public struct StateSet {
    private ulong val;

    public static implicit operator StateSet(ulong x)
    {
        return new StateSet{val=x};
    }
    public static implicit operator ulong(StateSet x)
    {
        return x.val;
    }
}

} // namespace Access

} // namespace Efl

namespace Evas { namespace Font {

}


// C# does not allow typedefs, so we use these implicit conversions.
public struct ModifierMask {
    private ulong mask;

    public static implicit operator ModifierMask(ulong x)
    {
        return new ModifierMask{mask=x};
    }
    public static implicit operator ulong(ModifierMask x)
    {
        return x.mask;
    }
}

public struct Coord {
    int val;

    public Coord(int value) { val = value; }
    static public implicit operator Coord(int val) {
        return new Coord(val);
    }
    static public implicit operator int(Coord coord) {
        return coord.val;
    }
}


}

public struct EflFontSize {
    int val;

    public EflFontSize(int value) { val = value; }
    static public implicit operator EflFontSize(int val) {
        return new EflFontSize(val);
    }
    static public implicit operator int(EflFontSize coord) {
        return coord.val;
    }
}

namespace eina {

[StructLayout(LayoutKind.Sequential)]
public struct Rectangle {
    public int x;
    public int y;
    public int w;
    public int h;
}

}

namespace Evas {

/* Copied from Evas_Legacy.h */
public enum TextStyleType
{
   ///<summary> plain, standard text.</summary>
   Plain = 0,
   ///<summary> text with shadow underneath.</summary>
   Shadow,
   ///<summary> text with an outline.</summary>
   Outline,
   ///<summary> text with a soft outline.</summary>
   SoftOutline,
   ///<summary> text with a glow effect.</summary>
   Glow,
   ///<summary> text with both outline and shadow effects.</summary>
   OutlineShadow,
   ///<summary> text with (far) shadow underneath.</summary>
   FarShadow,
   ///<summary> text with outline and soft shadow effects combined.</summary>
   OutlineSoftShadow,
   ///<summary> text with (soft) shadow underneath.</summary>
   SoftShadow,
   ///<summary> text with (far soft) shadow underneath.</summary>
   FarSoftShadow,

   // Shadow direction modifiers
   ///<summary> shadow growing to bottom right.</summary>
   ShadowDirectionBottomRight = 0 /* 0 >> 4 */,
  ///<summary> shadow growing to the bottom.</summary>
   ShadowDirectionBottom= 16 /* 1 >> 4 */,
   ///<summary> shadow growing to bottom left.</summary>
   ShadowDirectionBottomLeft = 32 /* 2 >> 4 */,
   ///<summary> shadow growing to the left.</summary>
   ShadowDirectionLeft = 48 /* 3 >> 4 */,
   ///<summary> shadow growing to top left.</summary>
   ShadowDirectionTopLeft = 64 /* 4 >> 4 */,
   ///<summary> shadow growing to the top.</summary>
   ShadowDirectionTop = 80 /* 5 >> 4 */,
   ///<summary> shadow growing to top right.</summary>
   ShadowDirectionTopRight = 96 /* 6 >> 4 */,
   ///<summary> shadow growing to the right.</summary>
   ShadowDirectionRight = 112 /* 7 >> 4 */
};

// Copied from Evas_Common.h
//
//
//

public enum CallbackType
{
  ///<summary> Mouse In Event.</summary>
  EVAS_CALLBACK_MOUSE_IN = 0,
  ///<summary> Mouse Out Event.</summary>
  EVAS_CALLBACK_MOUSE_OUT,
  ///<summary> Mouse Button Down Event.</summary>
  EVAS_CALLBACK_MOUSE_DOWN,
  ///<summary> Mouse Button Up Event.</summary>
  EVAS_CALLBACK_MOUSE_UP,
  ///<summary> Mouse Move Event.</summary>
  EVAS_CALLBACK_MOUSE_MOVE,
  ///<summary> Mouse Wheel Event.</summary>
  EVAS_CALLBACK_MOUSE_WHEEL,
  ///<summary> Multi-touch Down Event.</summary>
  EVAS_CALLBACK_MULTI_DOWN,
  ///<summary> Multi-touch Up Event.</summary>
  EVAS_CALLBACK_MULTI_UP,
  ///<summary> Multi-touch Move Event.</summary>
  EVAS_CALLBACK_MULTI_MOVE,
  ///<summary> Object Being Freed (Called after Del).</summary>
  EVAS_CALLBACK_FREE,
  ///<summary> Key Press Event.</summary>
  EVAS_CALLBACK_KEY_DOWN,
  ///<summary> Key Release Event.</summary>
  EVAS_CALLBACK_KEY_UP,
  ///<summary> Focus In Event.</summary>
  EVAS_CALLBACK_FOCUS_IN,
  ///<summary> Focus Out Event.</summary>
  EVAS_CALLBACK_FOCUS_OUT,
  ///<summary> Show Event.</summary>
  EVAS_CALLBACK_SHOW,
  ///<summary> Hide Event.</summary>
  EVAS_CALLBACK_HIDE,
  ///<summary> Move Event.</summary>
  EVAS_CALLBACK_MOVE,
  ///<summary> Resize Event.</summary>
  EVAS_CALLBACK_RESIZE,
  ///<summary> Restack Event.</summary>
  EVAS_CALLBACK_RESTACK,
  ///<summary> Object Being Deleted (called before Free).</summary>
  EVAS_CALLBACK_DEL,
  ///<summary> Events go on/off hold.</summary>
  EVAS_CALLBACK_HOLD,
  ///<summary> Size hints changed event.</summary>
  EVAS_CALLBACK_CHANGED_SIZE_HINTS,
  ///<summary> Image has been preloaded.</summary>
  EVAS_CALLBACK_IMAGE_PRELOADED,
  ///<summary> Canvas got focus as a whole.</summary>
  EVAS_CALLBACK_CANVAS_FOCUS_IN,
  ///<summary> Canvas lost focus as a whole.</summary>
  EVAS_CALLBACK_CANVAS_FOCUS_OUT,
  ///<summary>Called after render update regions have been calculated,
  /// but only if update regions exist.</summary>
  EVAS_CALLBACK_RENDER_FLUSH_PRE,
  ///<summary>Called after render update regions have
  /// been sent to the display server, but only
  /// if update regions existed for the most recent frame.</summary>
  EVAS_CALLBACK_RENDER_FLUSH_POST,
  ///<summary> Canvas object got focus.</summary>
  EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
  ///<summary> Canvas object lost focus.</summary>
  EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT,
  ///<summary> Image data has been unloaded (by some mechanism in Evas that throw out original image data).</summary>
  EVAS_CALLBACK_IMAGE_UNLOADED,
  ///<summary> Called just before rendering starts on the canvas target. @since 1.2.</summary>
  EVAS_CALLBACK_RENDER_PRE,
  ///<summary> Called just after rendering stops on the canvas target. @since 1.2.</summary>
  EVAS_CALLBACK_RENDER_POST,
  ///<summary> Image size is changed. @since 1.8.</summary>
  EVAS_CALLBACK_IMAGE_RESIZE,
  ///<summary> Devices added, removed or changed on canvas. @since 1.8.</summary>
  EVAS_CALLBACK_DEVICE_CHANGED,
  ///<summary> Input device changed value on some axis. @since 1.13.</summary>
  EVAS_CALLBACK_AXIS_UPDATE,
  ///<summary> Canvas viewport resized. @since 1.15.</summary>
  EVAS_CALLBACK_CANVAS_VIEWPORT_RESIZE,
  ///<summary>Sentinel value to indicate last enum field during iteration.</summary>
  EVAS_CALLBACK_LAST
};

}

namespace Elm {

namespace atspi {
public struct StateSet {
    private ulong val;

    public static implicit operator StateSet(ulong x)
    {
        return new StateSet{val=x};
    }
    public static implicit operator ulong(StateSet x)
    {
        return x.val;
    }
}

[StructLayout(LayoutKind.Sequential)]
public struct RelationSet
{
    public IntPtr pointer; // list<ptr(elm.atspi.Relation)>
}
} // namespace atspi

namespace gengrid { namespace item {
[StructLayout(LayoutKind.Sequential)]
public struct Class
{
    int version;
    uint refcount;
    [MarshalAsAttribute(UnmanagedType.U1)]bool delete_me;
    IntPtr item_style;
    IntPtr decorate_item_style;
    IntPtr decorate_all_item_style;

    // Delegates inside Elm_Gen_Item_Class_Functions
    IntPtr text_get;
    IntPtr content_get;
    IntPtr state_get;
    IntPtr del;
    IntPtr filter_get;
    IntPtr reusable_content_get;
}
} // namespace Item
} // namespace Genlist

} // namespace Elm

// Global delegates
public delegate IntPtr list_data_get_func_type(IntPtr l);
public delegate void region_hook_func_type(IntPtr data, IntPtr obj);
public delegate void slider_freefunc_type(IntPtr data);
public delegate void slider_func_type(double val);

public delegate int Eina_Compare_Cb(IntPtr a, IntPtr b);
public delegate void ElmInterfaceScrollableCb(IntPtr obj, IntPtr data);
public delegate void ElmInterfaceScrollableMinLimitCb(IntPtr obj,
                                                     [MarshalAsAttribute(UnmanagedType.U1)]bool w,
                                                     [MarshalAsAttribute(UnmanagedType.U1)]bool h);
public delegate void ElmInterfaceScrollableResizeCb(IntPtr obj, Evas.Coord w, Evas.Coord h);
public delegate void ElmEntryItemProviderCb(IntPtr data, IntPtr obj, IntPtr item);
public delegate void ElmEntryFilterCb(IntPtr data, IntPtr obj, IntPtr text);
[return: MarshalAsAttribute(UnmanagedType.U1)]
public delegate bool ElmMultibuttonentryItemFilterCb(IntPtr obj, IntPtr item_label, IntPtr item_data, IntPtr data);
public delegate IntPtr ElmMultibuttonentryFormatCb(int count, IntPtr data);
[return: MarshalAsAttribute(UnmanagedType.U1)]
public delegate bool ElmFileselectorFilterFunc(IntPtr path, [MarshalAsAttribute(UnmanagedType.U1)]bool dir, IntPtr data);
public delegate void EvasSmartCb(IntPtr data, IntPtr obj, IntPtr event_info);
public delegate void ElmGestureEventCb(IntPtr data, IntPtr event_info);
public delegate void ElmObjectItemSignalCb(IntPtr data, IntPtr item, IntPtr emission, IntPtr source);
public delegate void ElmTooltipItemContentCb(IntPtr data, IntPtr obj, IntPtr tooltip, IntPtr item);
public delegate void ElmSysNotifySendCb(IntPtr data, uint id);
public delegate IntPtr ElmCalendarFormatCb(IntPtr format);

namespace edje {

public delegate void SignalCb(IntPtr data, IntPtr obj, IntPtr emission, IntPtr source);
public delegate void MarkupFilterCb(IntPtr data, IntPtr obj, IntPtr part, IntPtr text );
public delegate void ItemProviderCb(IntPtr data, IntPtr obj, IntPtr part, IntPtr item);
// Message_Handler_Cb is now legacy

namespace text {
public delegate void FilterCb(IntPtr data, IntPtr obj, IntPtr part, int _type, IntPtr text);
public delegate void ChangeCb(IntPtr data, IntPtr obj, IntPtr part);
}


} // namespace edje

public enum ElmCodeStatusType {
   ELM_CODE_STATUS_TYPE_DEFAULT = 0,
   ELM_CODE_STATUS_TYPE_CURRENT,
   ELM_CODE_STATUS_TYPE_IGNORED,
   ELM_CODE_STATUS_TYPE_NOTE,
   ELM_CODE_STATUS_TYPE_WARNING,
   ELM_CODE_STATUS_TYPE_ERROR,
   ELM_CODE_STATUS_TYPE_FATAL,

   ELM_CODE_STATUS_TYPE_ADDED,
   ELM_CODE_STATUS_TYPE_REMOVED,
   ELM_CODE_STATUS_TYPE_CHANGED,

   ELM_CODE_STATUS_TYPE_PASSED,
   ELM_CODE_STATUS_TYPE_FAILED,

   ELM_CODE_STATUS_TYPE_TODO,

   ELM_CODE_STATUS_TYPE_COUNT
};


[StructLayout(LayoutKind.Sequential)]
public struct Elm_Atspi_Action
{
    IntPtr name;
    IntPtr action;
    IntPtr param;
    IntPtr func;
}

[StructLayout(LayoutKind.Sequential)]
public struct Tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
}

public delegate IntPtr Efl_Ui_Calendar_Format_Cb(Tm stime); // struct tm as argument
