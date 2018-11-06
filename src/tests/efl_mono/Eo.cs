using System;
using System.Linq;

namespace TestSuite
{

class TestEo
{
    private class Derived : test.Testing
    {
    }

    //
    // Test cases:
    //
    public static void return_same_object()
    {
        test.Testing testing = new test.Testing();
        test.Testing o1 = testing.ReturnObject();
        Test.Assert(o1.raw_handle != IntPtr.Zero);
        Test.Assert(o1.raw_handle == testing.raw_handle);
        test.Testing o2 = o1.ReturnObject();
        Test.Assert(o2.raw_handle != IntPtr.Zero);
        Test.Assert(o2.raw_handle == o1.raw_handle);
    }
    /* Commented out as adding the event listener seems to prevent it from being GC'd.
    public static void destructor_really_frees()
    {
       bool delEventCalled = false;
       {
           test.Testing obj = new test.Testing();
           obj.DEL += (object sender, EventArgs e) => { delEventCalled = true; };
       }

       System.GC.WaitForPendingFinalizers();
       System.GC.Collect();
       System.GC.WaitForPendingFinalizers();
       System.GC.Collect();
       System.GC.WaitForPendingFinalizers();

       Test.Assert(delEventCalled, "DEL event not called");
    } */

    public static void dispose_really_frees()
    {
       bool delEventCalled = false;
       {
           test.Testing obj = new test.Testing();
           eina.Log.Error($"Created object 0x{obj.raw_handle.ToInt64():x}");
           obj.DelEvt += (object sender, EventArgs e) => { delEventCalled = true; };
           eina.Log.Error($"Will dispose object 0x{obj.raw_handle.ToInt64():x}");
           ((IDisposable)obj).Dispose();
       }

       Test.Assert(delEventCalled, "DEL event not called");
    }

    /* Commented out as adding the event listener seems to prevent it from being GC'd.
    public static void derived_destructor_really_frees()
    {
       bool delEventCalled = false;
       {
           test.Testing obj = new Derived();
           obj.DEL += (object sender, EventArgs e) => { delEventCalled = true; };
       }

       System.GC.WaitForPendingFinalizers();
       System.GC.Collect();
       System.GC.WaitForPendingFinalizers();
       System.GC.Collect();
       System.GC.WaitForPendingFinalizers();

       Test.Assert(delEventCalled, "DEL event not called");
    }

    public static void derived_dispose_really_frees()
    {
       bool delEventCalled = false;
       {
           test.Testing obj = new Derived();
           obj.DEL += (object sender, EventArgs e) => { delEventCalled = true; };
           ((IDisposable)obj).Dispose();
       }

       Test.Assert(delEventCalled, "DEL event not called");
    }
    */
}


class MyLoop : efl.Loop
{
    public MyLoop() : base(null) { }
}

class TestEoInherit
{
    public static void instantiate_inherited()
    {
        efl.Loop loop = new MyLoop();
        Test.Assert(loop.raw_handle != System.IntPtr.Zero);
    }
}

class TestEoNames
{
    public static void name_getset()
    {
        test.Testing obj = new test.Testing();

        string name = "Dummy";
        obj.SetName(name);
        Test.AssertEquals(name, obj.GetName());
    }
}

class TestEoConstructingMethods
{
    public static void constructing_method()
    {
        bool called = false;
        string name = "Test object";
        test.Testing obj = new test.Testing(null, (test.Testing a) => {
                called = true;
                Console.WriteLine("callback: obj raw_handle: {0:x}", a.raw_handle);
                a.SetName(name);
            });

        Test.Assert(called);
        Test.AssertEquals(name, obj.GetName());
    }

    private class Derived : test.Testing
    {
        public Derived(test.Testing parent = null,
                       test.Testing.ConstructingMethod cb = null) : base(parent, cb) {
        }
    }

    public static void constructing_method_inherit()
    {
        bool called = false;
        string name = "Another test object";
        Derived obj = new Derived(null, (test.Testing a) => {
                called = true;
                a.SetComment(name);
            });

        Test.Assert(called);
        Test.AssertEquals(name, obj.GetComment());
    }
}

class TestEoParent
{
    public static void basic_parent()
    {
        test.Testing parent = new test.Testing(null);
        test.Testing child = new test.Testing(parent);

        Test.AssertEquals(parent, child.GetParent());

        test.Testing parent_retrieved = test.Testing.static_cast(child.GetParent());
        Test.AssertEquals(parent, parent_retrieved);
    }

    public static void parent_inherited_class()
    {
        test.Numberwrapper parent = new test.Numberwrapper(null);
        test.Testing child = new test.Testing(parent);

        Test.AssertEquals(parent, child.GetParent());

        test.Numberwrapper parent_retrieved = test.Numberwrapper.static_cast(child.GetParent());
        Test.AssertEquals(parent, parent_retrieved);
    }

    private class Derived : test.Testing
    {
        public Derived(test.Testing parent = null) : base (parent)
        {
        }
    }

    public static void basic_parent_managed_inherit()
    {
        test.Testing parent = new Derived(null);
        test.Testing child = new Derived(parent);

        Test.AssertEquals(parent, child.GetParent());

        test.Testing parent_from_cast = test.Testing.static_cast(child.GetParent());
        Test.AssertEquals(parent, parent_from_cast);
    }
}

class TestKlassMethods
{
    public static void basic_class_method()
    {
        int reference = 0xbeef;
        test.Testing.SetKlassProp(reference);
        Test.AssertEquals(reference, test.Testing.GetKlassProp());
    }

    public static void inherited_class_method()
    {
        int reference = 0xdead;
        test.Child.SetKlassProp(reference);
        Test.AssertEquals(reference, test.Child.GetKlassProp());
    }
}

class TestTypedefs
{
    public static void basic_typedef_test()
    {
        test.Testing obj = new test.Testing();
        test.MyInt input = 1900;
        test.MyInt receiver;

        int ret = obj.BypassTypedef(input, out receiver);

        Test.AssertEquals((test.MyInt)ret, input);
        Test.AssertEquals(receiver, input);

    }
}

class TestEoAccessors
{
    public static void basic_eo_accessors()
    {
        test.Testing obj = new test.Testing();
        eina.List<int> lst = new eina.List<int>();
        lst.Append(4);
        lst.Append(3);
        lst.Append(2);
        lst.Append(5);
        eina.Accessor<int> acc = obj.CloneAccessor(lst.GetAccessor());

        var zipped = acc.Zip(lst, (first, second) => new Tuple<int, int>(first, second));

        foreach(Tuple<int, int> pair in zipped)
        {
            Test.AssertEquals(pair.Item1, pair.Item2);
        }
    }
}

class TestEoFinalize
{
    public sealed class Inherit : efl.Object
    {
        public bool finalizeCalled = false;
        public override efl.Object FinalizeAdd()
        {
            finalizeCalled = true;
            return this;
        }
    }


    public static void finalize_call()
    {
        Inherit inherit = new Inherit();
        Test.Assert(inherit.finalizeCalled);
    }
}

}
