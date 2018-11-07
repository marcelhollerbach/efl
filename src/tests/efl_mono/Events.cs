using System;

using static Dummy.TestObject; // For the event args

namespace TestSuite
{

class TestEoEvents
{
    public bool called = false;
    public bool correct_sender = false;
    public Efl.Loop loop { get; set; }
    protected void callback(object sender, EventArgs e) {
        called = true;
        Efl.Object obj = sender as Efl.Object;
        if (obj != null)
        {
            obj.SetName("loop_called");
            correct_sender = true;
        }

        Eina.Value v = new Eina.Value(Eina.ValueType.Int32);
        v.Set(0);
        loop.Quit(v);
    }
    protected void another_callback(object sender, EventArgs e) { }

    public static void idle_event()
    {
        Efl.Loop loop = new Efl.Loop();
        loop.SetName("loop");
        TestEoEvents listener = new TestEoEvents();
        listener.loop = loop;
        loop.IdleEvt += listener.callback;

        Test.Assert(!listener.called);
        Test.Assert(!listener.correct_sender);
        Test.AssertEquals("loop", loop.GetName());
        loop.Begin();
        Test.Assert(listener.called);
        Test.Assert(listener.correct_sender);
        Test.AssertEquals("loop_called", loop.GetName());
    }

    public static void event_with_string_payload()
    {
        var obj = new Dummy.TestObject();
        string received_string = null;

        obj.EvtWithStringEvt += (object sender, EvtWithStringEvt_Args e) => {
            received_string = e.arg;
        };

        obj.EmitEventWithString("Some args");

        Test.AssertEquals("Some args", received_string);
    }

    public static void event_with_int_payload()
    {
        var obj = new Dummy.TestObject();
        int received_int= 0;

        obj.EvtWithIntEvt += (object sender, EvtWithIntEvt_Args e) => {
            received_int = e.arg;
        };

        obj.EmitEventWithInt(-1984);

        Test.AssertEquals(-1984, received_int);
    }

    public static void event_with_bool_payload()
    {
        var obj = new Dummy.TestObject();
        bool received_bool = false;

        obj.EvtWithBoolEvt += (object sender, EvtWithBoolEvt_Args e) => {
            received_bool = e.arg;
        };

        obj.EmitEventWithBool(true);

        Test.AssertEquals(true, received_bool);

        obj.EmitEventWithBool(false);

        Test.AssertEquals(false, received_bool);
    }

    public static void event_with_uint_payload()
    {
        var obj = new Dummy.TestObject();
        uint received_uint = 0;
        obj.EvtWithUintEvt += (object sender, EvtWithUintEvt_Args e) => {
            received_uint = e.arg;
        };

        obj.EmitEventWithUint(0xbeef);

        Test.AssertEquals<uint>(0xbeef, received_uint);
    }

    public static void event_with_object_payload()
    {
        var obj = new Dummy.TestObject();
        Dummy.TestObject received_obj = null;

        obj.EvtWithObjEvt += (object sender, EvtWithObjEvt_Args e) => {
            received_obj = e.arg;
        };

        var sent_obj = new Dummy.TestObject();

        obj.EmitEventWithObj(sent_obj);

        Test.AssertEquals(sent_obj, received_obj);
    }

    public static void event_with_error_payload()
    {
        var obj = new Dummy.TestObject();
        Eina.Error received_error = 0;

        obj.EvtWithErrorEvt += (object sender, EvtWithErrorEvt_Args e) => {
            received_error = e.arg;
        };

        Eina.Error sent_error = -2001;

        obj.EmitEventWithError(sent_error);

        Test.AssertEquals(sent_error, received_error);
    }

    public static void event_with_struct_payload()
    {
        var obj = new Dummy.TestObject();
        Dummy.StructSimple received_struct = default(Dummy.StructSimple);

        obj.EvtWithStructEvt += (object sender, EvtWithStructEvt_Args e) => {
            received_struct = e.arg;
        };

        Dummy.StructSimple sent_struct = default(Dummy.StructSimple);
        sent_struct.Fstring = "Struct Event";

        obj.EmitEventWithStruct(sent_struct);

        Test.AssertEquals(sent_struct.Fstring, received_struct.Fstring);
    }

    public static void event_in_init_callback()
    {
        int received = 0;
        int sent = 42;
        var obj = new Dummy.TestObject(null, (Dummy.TestObject t) => {
            t.EvtWithIntEvt += (object sender, EvtWithIntEvt_Args e) => {
                received = e.arg;
            };
        });

        obj.EmitEventWithInt(sent);

        Test.AssertEquals(sent, received);
    }
}

class TestEventAddRemove
{
    public static void test_add_remove_event()
    {
        var obj = new Dummy.TestObject();
        bool called = true;

        EventHandler<Dummy.TestObject.EvtWithIntEvt_Args> evtCb = (object sender, EvtWithIntEvt_Args e) => {
            called = true;
        };

        obj.EvtWithIntEvt += evtCb;
        obj.EmitEventWithInt(42);
        Test.Assert(called);

        called = false;
        obj.EvtWithIntEvt -= evtCb;
        obj.EmitEventWithInt(42);
        Test.Assert(!called);



    }
}
}
