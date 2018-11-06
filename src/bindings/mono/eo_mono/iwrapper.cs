#pragma warning disable 1591

using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

using static eina.NativeCustomExportFunctions;

namespace efl { namespace eo {

public class Globals {
    [DllImport(efl.Libs.Eo)] public static extern void efl_object_init();
    [DllImport(efl.Libs.Eo)] public static extern void efl_object_shutdown();
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        _efl_add_internal_start([MarshalAs(UnmanagedType.LPStr)] String file, int line,
                                IntPtr klass, IntPtr parent, byte is_ref, byte is_fallback);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        _efl_add_end(IntPtr eo, byte is_ref, byte is_fallback);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_ref(IntPtr eo);
    [DllImport(efl.Libs.CustomExports)] public static extern void
        efl_unref(IntPtr eo);
    [DllImport(efl.Libs.Eo)] public static extern int
        efl_ref_count(IntPtr eo);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2, IntPtr base3);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2, IntPtr base3, IntPtr base4);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2, IntPtr base3, IntPtr base4, IntPtr base5);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2, IntPtr base3, IntPtr base4, IntPtr base5, IntPtr base6);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2, IntPtr base3, IntPtr base4, IntPtr base5, IntPtr base6, IntPtr base7);
    [DllImport(efl.Libs.Eo)] public static extern byte efl_class_functions_set(IntPtr klass_id, IntPtr object_ops, IntPtr class_ops);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr efl_data_scope_get(IntPtr obj, IntPtr klass);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr efl_super(IntPtr obj, IntPtr klass);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr efl_class_get(IntPtr obj);
#if WIN32
    public static IntPtr RTLD_DEFAULT = new IntPtr(1);
#else
    public static IntPtr RTLD_DEFAULT = new IntPtr(0);
#endif
    [DllImport(efl.Libs.Evil)] public static extern IntPtr dlerror();
    [DllImport(efl.Libs.Evil)] public static extern IntPtr dlsym
       (IntPtr handle, [MarshalAs(UnmanagedType.LPStr)] String name);

   [DllImport(efl.Libs.Eo)] public static extern bool efl_event_callback_priority_add(
              System.IntPtr obj,
              IntPtr desc,
              short priority,
              efl.Event_Cb cb,
              System.IntPtr data);
   [DllImport(efl.Libs.Eo)] public static extern bool efl_event_callback_del(
              System.IntPtr obj,
              IntPtr desc,
              efl.Event_Cb cb,
              System.IntPtr data);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_object_legacy_only_event_description_get([MarshalAs(UnmanagedType.LPStr)] String name);

    public const int RTLD_NOW = 2;

    public delegate byte class_initializer(IntPtr klass);
    
    public static IntPtr register_class(class_initializer initializer, String class_name, IntPtr base_klass)
    {
        ClassDescription description;
        description.version = 2; // EO_VERSION
        description.name = class_name;
        description.class_type = 0; // REGULAR
        description.data_size = (UIntPtr)8;
        description.class_initializer = IntPtr.Zero;
        description.class_constructor = IntPtr.Zero;
        description.class_destructor = IntPtr.Zero;

        if(initializer != null)
            description.class_initializer = Marshal.GetFunctionPointerForDelegate(initializer);

        IntPtr description_ptr = eina.MemoryNative.Alloc(Marshal.SizeOf(description));
        Marshal.StructureToPtr(description, description_ptr, false);
      
        eina.Log.Debug("Going to register!");
        IntPtr klass = efl.eo.Globals.efl_class_new(description_ptr, base_klass, IntPtr.Zero);
        if(klass == IntPtr.Zero)
            eina.Log.Error("klass was not registered");
        else
            eina.Log.Debug("Registered class successfully");
        return klass;
    }
    public static IntPtr instantiate_start(IntPtr klass, efl.Object parent)
    {
        eina.Log.Debug($"Instantiating from klass 0x{klass.ToInt64():x}");
        System.IntPtr parent_ptr = System.IntPtr.Zero;
        if(parent != null)
            parent_ptr = parent.raw_handle;

        System.IntPtr eo = efl.eo.Globals._efl_add_internal_start("file", 0, klass, parent_ptr, 1, 0);
        Console.WriteLine($"Eo instance right after internal_start 0x{eo.ToInt64():x} with refcount {efl.eo.Globals.efl_ref_count(eo)}");
        Console.WriteLine($"Parent was 0x{parent_ptr.ToInt64()}");
        return eo;
    }

    public static IntPtr instantiate_end(IntPtr eo) {
        eina.Log.Debug("calling efl_add_internal_end");
        eo = efl.eo.Globals._efl_add_end(eo, 1, 0);
        eina.Log.Debug($"efl_add_end returned eo 0x{eo.ToInt64():x}");
        return eo;
    }
    public static void data_set(efl.eo.IWrapper obj)
    {
      eina.Log.Debug($"Calling data_scope_get with obj {obj.raw_handle.ToInt64():x} and klass {obj.raw_klass.ToInt64():x}");
      IntPtr pd = efl.eo.Globals.efl_data_scope_get(obj.raw_handle, obj.raw_klass);
      {
          GCHandle gch = GCHandle.Alloc(obj);
          EolianPD epd;
          epd.pointer = GCHandle.ToIntPtr(gch);
          Marshal.StructureToPtr(epd, pd, false);
      }
    }
    public static efl.eo.IWrapper data_get(IntPtr pd)
    {
        EolianPD epd = (EolianPD)Marshal.PtrToStructure(pd, typeof(EolianPD));
        if(epd.pointer != IntPtr.Zero)
        {
            GCHandle gch = GCHandle.FromIntPtr(epd.pointer);
            return (efl.eo.IWrapper)gch.Target;
        }
        else
            return null;
    }

    public static IntPtr cached_string_to_intptr(Dictionary<String, IntPtr> dict, String str)
    {
        IntPtr ptr = IntPtr.Zero;
        if (!dict.TryGetValue(str, out ptr))
        {
            ptr = eina.StringConversion.ManagedStringToNativeUtf8Alloc(str);
            dict[str] = ptr;
        }

        return ptr;
    }

    public static IntPtr cached_stringshare_to_intptr(Dictionary<String, IntPtr> dict, String str)
    {
        IntPtr ptr = IntPtr.Zero;
        if (!dict.TryGetValue(str, out ptr))
        {
            ptr = eina.Stringshare.eina_stringshare_add(str);
            dict[str] = ptr;
        }

        return ptr;
    }

    public static void free_dict_values(Dictionary<String, IntPtr> dict)
    {
        foreach(IntPtr ptr in dict.Values)
        {
            eina.MemoryNative.Free(ptr);
        }
    }

    public static void free_stringshare_values(Dictionary<String, IntPtr> dict)
    {
        foreach(IntPtr ptr in dict.Values)
        {
            eina.Stringshare.eina_stringshare_del(ptr);
        }
    }

    public static void free_gchandle(IntPtr ptr)
    {
        GCHandle handle = GCHandle.FromIntPtr(ptr);
        handle.Free();
    }

    public static System.Threading.Tasks.Task<eina.Value> WrapAsync(eina.Future future, CancellationToken token)
    {
        // Creates a task that will wait for SetResult for completion.
        // TaskCompletionSource is used to create tasks for 'external' Task sources.
        var tcs = new System.Threading.Tasks.TaskCompletionSource<eina.Value>();

        // Flag to be passed to the cancell callback
        bool fulfilled = false;

        future.Then((eina.Value received) => {
                lock (future)
                {
                    // Convert an failed Future to a failed Task.
                    if (received.GetValueType() == eina.ValueType.Error)
                    {
                        eina.Error err;
                        received.Get(out err);
                        if (err == eina.Error.ECANCELED)
                            tcs.SetCanceled();
                        else
                            tcs.TrySetException(new efl.FutureException(received));
                    }
                    else
                    {
                        // Will mark the returned task below as completed.
                        tcs.SetResult(received);
                    }
                    fulfilled = true;
                    return received;
                }
        });
        // Callback to be called when the token is cancelled.
        token.Register(() => {
                lock (future)
                {
                    // Will trigger the Then callback above with an eina.Error
                    if (!fulfilled)
                        future.Cancel();
                }
        });

        return tcs.Task;
    }
} // Globals

public static class Config
{
    public static void Init()
    {
        Globals.efl_object_init();
    }

    public static void Shutdown()
    {
        Globals.efl_object_shutdown();
    }
}

public interface IWrapper
{
    IntPtr raw_handle
    {
        get;
    }
    IntPtr raw_klass
    {
        get;
    }
}

public interface IOwnershipTag
{
}

public class OwnTag : IOwnershipTag
{
}

public class NonOwnTag : IOwnershipTag
{
}

public class MarshalTest<T, U> : ICustomMarshaler
    where U : IOwnershipTag
{
    public static ICustomMarshaler GetInstance(string cookie)
    {
        eina.Log.Debug("MarshalTest.GetInstace cookie " + cookie);
        return new MarshalTest<T, U>();
    }
    public void CleanUpManagedData(object ManagedObj)
    {
        //eina.Log.Warning("MarshalTest.CleanUpManagedData not implemented");
        //throw new NotImplementedException();
    }

    public void CleanUpNativeData(IntPtr pNativeData)
    {
        //eina.Log.Warning("MarshalTest.CleanUpNativeData not implemented");
        //throw new NotImplementedException();
    }

    public int GetNativeDataSize()
    {
        eina.Log.Debug("MarshalTest.GetNativeDataSize");
        return 0;
        //return 8;
    }

    public IntPtr MarshalManagedToNative(object ManagedObj)
    {
        eina.Log.Debug("MarshalTest.MarshallManagedToNative");
        var r = ((IWrapper)ManagedObj).raw_handle;
        if (typeof(U) == typeof(OwnTag))
            efl.eo.Globals.efl_ref(r);
        return r;
    }

    public object MarshalNativeToManaged(IntPtr pNativeData)
    {
        eina.Log.Debug("MarshalTest.MarshalNativeToManaged");
        if (typeof(U) != typeof(OwnTag))
            efl.eo.Globals.efl_ref(pNativeData);
        return Activator.CreateInstance(typeof(T), new System.Object[] {pNativeData});
//        return null;
    }
}

public class StringPassOwnershipMarshaler : ICustomMarshaler {
    public object MarshalNativeToManaged(IntPtr pNativeData) {
        var ret = eina.StringConversion.NativeUtf8ToManagedString(pNativeData);
        eina.MemoryNative.Free(pNativeData);
        return ret;
    }

    public IntPtr MarshalManagedToNative(object managedObj) {
        return eina.MemoryNative.StrDup((string)managedObj);
    }

    public void CleanUpNativeData(IntPtr pNativeData) {
        // No need to cleanup. C will take care of it.
    }

    public void CleanUpManagedData(object managedObj) {
    }

    public int GetNativeDataSize() {
        return -1;
    }

    public static ICustomMarshaler GetInstance(string cookie) {
        if (marshaler == null) {
            marshaler = new StringPassOwnershipMarshaler();
        }
        return marshaler;
    }
    static private StringPassOwnershipMarshaler marshaler;
}

public class StringKeepOwnershipMarshaler: ICustomMarshaler {
    public object MarshalNativeToManaged(IntPtr pNativeData) {
        return eina.StringConversion.NativeUtf8ToManagedString(pNativeData);
    }

    public IntPtr MarshalManagedToNative(object managedObj) {
        return eina.StringConversion.ManagedStringToNativeUtf8Alloc((string)managedObj);
    }

    public void CleanUpNativeData(IntPtr pNativeData) {
        // No need to free. The Native side will keep the ownership.
    }

    public void CleanUpManagedData(object managedObj) {
    }

    public int GetNativeDataSize() {
        return -1;
    }

    public static ICustomMarshaler GetInstance(string cookie) {
        if (marshaler == null) {
            marshaler = new StringKeepOwnershipMarshaler();
        }
        return marshaler;
    }
    static private StringKeepOwnershipMarshaler marshaler;
}

public class StringsharePassOwnershipMarshaler : ICustomMarshaler {
    public object MarshalNativeToManaged(IntPtr pNativeData) {
        var ret = eina.StringConversion.NativeUtf8ToManagedString(pNativeData);
        eina.Stringshare.eina_stringshare_del(pNativeData);
        return ret;
    }

    public IntPtr MarshalManagedToNative(object managedObj) {
        return eina.Stringshare.eina_stringshare_add((string)managedObj);
    }

    public void CleanUpNativeData(IntPtr pNativeData) {
        // No need to free as it's for own() parameters.
    }

    public void CleanUpManagedData(object managedObj) {
    }

    public int GetNativeDataSize() {
        return -1;
    }

    public static ICustomMarshaler GetInstance(string cookie) {
        if (marshaler == null) {
            marshaler = new StringsharePassOwnershipMarshaler();
        }
        return marshaler;
    }
    static private StringsharePassOwnershipMarshaler marshaler;
}

public class StringshareKeepOwnershipMarshaler : ICustomMarshaler {
    public object MarshalNativeToManaged(IntPtr pNativeData) {
        return eina.StringConversion.NativeUtf8ToManagedString(pNativeData);
    }

    public IntPtr MarshalManagedToNative(object managedObj) {
        return eina.Stringshare.eina_stringshare_add((string)managedObj);
    }

    public void CleanUpNativeData(IntPtr pNativeData) {
        // No need to free, as the native side will keep ownership.
    }

    public void CleanUpManagedData(object managedObj) {
    }

    public int GetNativeDataSize() {
        return -1;
    }

    public static ICustomMarshaler GetInstance(string cookie) {
        if (marshaler == null) {
            marshaler = new StringshareKeepOwnershipMarshaler();
        }
        return marshaler;
    }
    static private StringshareKeepOwnershipMarshaler marshaler;
}

public class StrbufPassOwnershipMarshaler : ICustomMarshaler {
    public object MarshalNativeToManaged(IntPtr pNativeData) {
        return new eina.Strbuf(pNativeData, eina.Ownership.Managed);
    }

    public IntPtr MarshalManagedToNative(object managedObj) {
        eina.Strbuf buf = managedObj as eina.Strbuf;
        buf.ReleaseOwnership();
        return buf.Handle;
    }

    public void CleanUpNativeData(IntPtr pNativeData) {
        // No need to cleanup. C will take care of it.
    }

    public void CleanUpManagedData(object managedObj) {
    }

    public int GetNativeDataSize() {
        return -1;
    }

    public static ICustomMarshaler GetInstance(string cookie) {
        if (marshaler == null) {
            marshaler = new StrbufPassOwnershipMarshaler();
        }
        return marshaler;
    }
    static private StrbufPassOwnershipMarshaler marshaler;
}

public class StrbufKeepOwnershipMarshaler: ICustomMarshaler {
    public object MarshalNativeToManaged(IntPtr pNativeData) {
        return new eina.Strbuf(pNativeData, eina.Ownership.Unmanaged);
    }

    public IntPtr MarshalManagedToNative(object managedObj) {
        eina.Strbuf buf = managedObj as eina.Strbuf;
        return buf.Handle;
    }

    public void CleanUpNativeData(IntPtr pNativeData) {
        // No need to free. The Native side will keep the ownership.
    }

    public void CleanUpManagedData(object managedObj) {
    }

    public int GetNativeDataSize() {
        return -1;
    }

    public static ICustomMarshaler GetInstance(string cookie) {
        if (marshaler == null) {
            marshaler = new StrbufKeepOwnershipMarshaler();
        }
        return marshaler;
    }
    static private StrbufKeepOwnershipMarshaler marshaler;
}



} // namespace eo

/// <summary>General exception for errors inside the binding.</summary>
public class EflException : Exception
{
    /// <summary>Create a new EflException with the given message.</summary>
    public EflException(string message) : base(message)
    {
    }
}

/// <summary>Exception to be raised when a Task fails due to a failed eina.Future.</summary>
public class FutureException : EflException
{
    /// <summary>The error code returned by the failed eina.Future.</summary>
    public eina.Error Error { get; private set; }

    /// <summary>Construct a new exception from the eina.Error stored in the given eina.Value.</summary>
    public FutureException(eina.Value value) : base("Future failed.")
    {
        if (value.GetValueType() != eina.ValueType.Error)
            throw new ArgumentException("FutureException must receive an eina.Value with eina.Error.");
        eina.Error err;
        value.Get(out err);
        Error = err;
    }
}

} // namespace efl
