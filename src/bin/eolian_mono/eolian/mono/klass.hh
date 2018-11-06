#ifndef EOLIAN_MONO_CLASS_DEFINITION_HPP
#define EOLIAN_MONO_CLASS_DEFINITION_HPP

#include "grammar/integral.hpp"
#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/attribute_conditional.hpp"
#include "blacklist.hh"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "type.hh"
#include "name_helpers.hh"
#include "async_function_definition.hh"
#include "function_definition.hh"
#include "function_registration.hh"
#include "function_declaration.hh"
#include "documentation.hh"
#include "part_definition.hh"
#include "events.hh"
#include "grammar/string.hpp"
#include "grammar/attribute_replace.hpp"
#include "grammar/integral.hpp"
#include "grammar/case.hpp"
#include "using_decl.hh"
#include "utils.hh"

#include <string>
#include <algorithm>

namespace eolian_mono {

template <typename OutputIterator, typename Context>
static bool generate_static_cast_method(OutputIterator sink, grammar::attributes::klass_def const& cls, Context const &context)
{
   return as_generator(
       scope_tab << "///<summary>Casts obj into an instance of this type.</summary>\n"
       << scope_tab << "public static " << name_helpers::klass_concrete_name(cls) << " static_cast(efl.Object obj)\n"
       << scope_tab << "{\n"
       << scope_tab << scope_tab << "if (obj == null)\n"
       << scope_tab << scope_tab << scope_tab << "throw new System.ArgumentNullException(\"obj\");\n"
       << scope_tab << scope_tab << "return new " << name_helpers::klass_concrete_name(cls) << "(obj.raw_handle);\n"
       << scope_tab << "}\n"
       ).generate(sink, nullptr, context);
}

template <typename OutputIterator, typename Context>
static bool generate_equals_method(OutputIterator sink, Context const &context)
{
   return as_generator(
       scope_tab << "///<summary>Verifies if the given object is equal to this one.</summary>\n" 
       << scope_tab << "public override bool Equals(object obj)\n"
       << scope_tab << "{\n"
       << scope_tab << scope_tab << "var other = obj as efl.Object;\n"
       << scope_tab << scope_tab << "if (other == null)\n"
       << scope_tab << scope_tab << scope_tab << "return false;\n"
       << scope_tab << scope_tab << "return this.raw_handle == other.raw_handle;\n"
       << scope_tab << "}\n"
       << scope_tab << "///<summary>Gets the hash code for this object based on the native pointer it points to.</summary>\n"
       << scope_tab << "public override int GetHashCode()\n"
       << scope_tab << "{\n"
       << scope_tab << scope_tab << "return this.raw_handle.ToInt32();\n"
       << scope_tab << "}\n"
       << scope_tab << "///<summary>Turns the native pointer into a string representation.</summary>\n"
       << scope_tab << "public override String ToString()\n"
       << scope_tab << "{\n"
       << scope_tab << scope_tab << "return $\"{this.GetType().Name}@[{this.raw_handle.ToInt32():x}]\";\n"
       << scope_tab << "}\n"
      ).generate(sink, nullptr, context);
}

/* Get the actual number of functions of a class, checking for blacklisted ones */
static std::size_t
get_inheritable_function_count(grammar::attributes::klass_def const& cls)
{
   auto methods = cls.get_all_methods();
   return std::count_if(methods.cbegin(), methods.cend(), [](grammar::attributes::function_def const& func)
     {
        return !blacklist::is_function_blacklisted(func.c_name) && !func.is_static;
     });
}

template<typename Context>
static bool
is_inherit_context(Context const& context)
{
   return context_find_tag<class_context>(context).current_wrapper_kind == class_context::inherit;
}

struct klass
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     EINA_CXX_DOM_LOG_DBG(eolian_mono::domain) << "klass_generator: " << cls.eolian_name << std::endl;
     std::string suffix, class_type;
     switch(cls.type)
       {
       case attributes::class_type::regular:
         class_type = "class";
         suffix = "CLASS";
         break;
       case attributes::class_type::abstract_:
         class_type = "class";
         suffix = "CLASS";
         break;
       case attributes::class_type::mixin:
         class_type = "interface";
         suffix = "MIXIN";
         break;
       case attributes::class_type::interface_:
         class_type = "interface";
         suffix = "INTERFACE";
         break;
       }

     if (!name_helpers::open_namespaces(sink, cls.namespaces, context))
       return false;

     auto methods = cls.get_all_methods();

     // Interface class
     if(class_type == "interface")
     {
     auto iface_cxt = context_add_tag(class_context{class_context::interface}, context);

     if(!as_generator(documentation).generate(sink, cls, iface_cxt))
       return false;

     if(!as_generator
        (
         "public " /*<< class_type*/ "interface" /*<<*/ " " << string << " : "
         )
        .generate(sink, name_helpers::klass_interface_name(cls), iface_cxt))
       return false;
     for(auto first = std::begin(cls.immediate_inherits)
           , last = std::end(cls.immediate_inherits); first != last; ++first)
       {
         if(first->type != attributes::class_type::regular
            && first->type != attributes::class_type::abstract_)
           if(!as_generator("\n" << scope_tab << string << " ,").generate(sink, name_helpers::klass_full_interface_name(*first), iface_cxt))
             return false;
       }

     if(!as_generator("\n" << scope_tab << "efl.eo.IWrapper, IDisposable").generate(sink, attributes::unused, iface_cxt))
       return false;

     if(!as_generator("\n{\n").generate(sink, attributes::unused, iface_cxt))
       return false;

     if(!as_generator(*(scope_tab << function_declaration)).generate(sink, cls.functions, iface_cxt))
       return false;

     if(!as_generator(*(scope_tab << async_function_declaration)).generate(sink, cls.functions, iface_cxt))
       return false;

     if(!as_generator(*(event_declaration)).generate(sink, cls.events, iface_cxt))
       return false;

     for (auto &&p : cls.parts)
       if (!as_generator(
              documentation(1)
              << name_helpers::klass_full_concrete_or_interface_name(p.klass) << " " << utils::capitalize(p.name) << "{ get;}\n"
            ).generate(sink, p, iface_cxt))
         return false;

     // End of interface declaration
     if(!as_generator("}\n").generate(sink, attributes::unused, iface_cxt)) return false;
     }

     // Concrete class for interfaces, mixins, etc.
     if(class_type != "class")
       {
         auto concrete_cxt = context_add_tag(class_context{class_context::concrete}, context);
         auto concrete_name = name_helpers::klass_concrete_name(cls);
         auto interface_name = name_helpers::klass_interface_name(cls);


         auto inherit_name = name_helpers::klass_concrete_name(cls);
         auto native_inherit_name = name_helpers::klass_native_inherit_name(cls);
         /* bool root = !helpers::has_regular_ancestor(cls); */

         std::vector<attributes::klass_name> inherit_classes;
         std::set<attributes::klass_name, attributes::compare_klass_name_by_name> inherit_interfaces;
         std::copy_if(cls.immediate_inherits.begin(), cls.immediate_inherits.end()
                      , std::back_inserter(inherit_classes)
                      , [] (attributes::klass_name const& klass)
                      { switch (klass.type) { case attributes::class_type::regular:
                                              case attributes::class_type::abstract_: return true;
                                              default: return false; }; });

        /* inherit_interfaces = helpers::non_implemented_interfaces(cls); */
        inherit_interfaces = helpers::interface_inherits(cls);
        /* std::copy_if(cls.immediate_inherits.begin(), cls.immediate_inherits.end() */
        /*              , std::back_inserter(inherit_interfaces) */
        /*              , [] (attributes::klass_name const& klass) */
        /*              { switch (klass.type) { case attributes::class_type::regular: return false; */
        /*                                      default: return true; }; }); */


         if(!as_generator
            (
             documentation
             << "sealed public class " << concrete_name << " : " << "\n"
             << (klass_full_concrete_or_interface_name % ",") << "\n"
             << (inherit_classes.size() > 0 ? ", " : "" ) << interface_name << "\n"
             << scope_tab << *(", " << name_helpers::klass_full_concrete_or_interface_name) << "\n"
             /* << (inherit_interfaces.size() > 0 ? ", " : "") << (klass_full_concrete_or_interface_name % ",") << "\n" */
             << "{\n"
             << scope_tab << "System.IntPtr handle;\n"
             << scope_tab << "///<summary>Pointer to the native instance.</summary>\n"
             << scope_tab << "public System.IntPtr raw_handle {\n"
             << scope_tab << scope_tab << "get { return handle; }\n"
             << scope_tab << "}\n"
             << scope_tab << "///<summary>Pointer to the native class description.</summary>\n"
             << scope_tab << "public System.IntPtr raw_klass {\n"
             << scope_tab << scope_tab << "get { return efl.eo.Globals.efl_class_get(handle); }\n"
             << scope_tab << "}\n"
             << scope_tab << "///<summary>Returns the pointer to the underlying Eo class object. Used internally on class methods.</summary>\n"
             << scope_tab << "[System.Runtime.InteropServices.DllImport(" << context_find_tag<library_context>(concrete_cxt).actual_library_name(cls.filename)
             << ")] public static extern System.IntPtr\n"
             << scope_tab << scope_tab << name_helpers::klass_get_name(cls) << "();\n"
             << scope_tab << "///<summary>Constructs an instance from a native pointer.</summary>\n"
             << scope_tab << "public " << concrete_name << "(System.IntPtr raw)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "handle = raw;\n"
             << scope_tab << scope_tab << "register_event_proxies();\n"
             << scope_tab << "}\n"
             << scope_tab << "///<summary>Destructor.</summary>\n"
             << scope_tab << "~" << concrete_name << "()\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "Dispose(false);\n"
             << scope_tab << "}\n"
             << scope_tab << "///<summary>Releases the underlying native instance.</summary>\n"
             << scope_tab << "internal void Dispose(bool disposing)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "if (handle != System.IntPtr.Zero) {\n"
             << scope_tab << scope_tab << scope_tab << "efl.eo.Globals.efl_unref(handle);\n"
             << scope_tab << scope_tab << scope_tab << "handle = System.IntPtr.Zero;\n"
             << scope_tab << scope_tab << "}\n"
             << scope_tab << "}\n"
             << scope_tab << "///<summary>Releases the underlying native instance.</summary>\n"
             << scope_tab << "public void Dispose()\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "Dispose(true);\n"
             << scope_tab << scope_tab << "GC.SuppressFinalize(this);\n"
             << scope_tab << "}\n"
            )
            .generate(sink, std::make_tuple(cls, inherit_classes, inherit_interfaces), concrete_cxt))
           return false;

         if (!generate_static_cast_method(sink, cls, concrete_cxt))
           return false;

         if (!generate_equals_method(sink, concrete_cxt))
           return false;

         if (!generate_events(sink, cls, concrete_cxt))
             return false;

         if (!generate_events_registration(sink, cls, concrete_cxt))
             return false;

         // Parts
         if(!as_generator(*(part_definition))
            .generate(sink, cls.get_all_parts(), concrete_cxt)) return false;

         // Concrete function definitions
         if(!as_generator(*(function_definition))
            .generate(sink, methods, concrete_cxt)) return false;

         // Async wrappers
         if(!as_generator(*(async_function_definition)).generate(sink, methods, concrete_cxt))
           return false;

         if(!as_generator(*(event_argument_wrapper)).generate(sink, cls.events, context))
           return false;

         if(!as_generator("}\n").generate(sink, attributes::unused, concrete_cxt)) return false;
       }

     // Inheritable class
     if(class_type == "class")
       {
        auto inherit_cxt = context_add_tag(class_context{class_context::inherit}, context);
        /* bool cls_has_string_return = has_string_return(cls); */
        /* bool cls_has_stringshare_return = has_stringshare_return(cls); */

        auto inherit_name = name_helpers::klass_concrete_name(cls);
        auto native_inherit_name = name_helpers::klass_native_inherit_name(cls);
        bool root = !helpers::has_regular_ancestor(cls);

        std::vector<attributes::klass_name> inherit_classes;
        std::set<attributes::klass_name, attributes::compare_klass_name_by_name> inherit_interfaces;
        std::copy_if(cls.immediate_inherits.begin(), cls.immediate_inherits.end()
                     , std::back_inserter(inherit_classes)
                     , [] (attributes::klass_name const& klass)
                     { switch (klass.type) { case attributes::class_type::regular:
                                             case attributes::class_type::abstract_:
                                                return true;
                                             default: return false; }; });

        inherit_interfaces = helpers::non_implemented_interfaces(cls);
        /* std::copy_if(cls.immediate_inherits.begin(), cls.immediate_inherits.end() */
        /*              , std::back_inserter(inherit_interfaces) */
        /*              , [] (attributes::klass_name const& klass) */
        /*              { switch (klass.type) { case attributes::class_type::regular: return false; */
        /*                                      default: return true; }; }); */

        // Class header
        if(!as_generator
            (
             documentation
             << "public " << class_type << " " << inherit_name << " : "
             /* << (cls.immediate_inherits.empty() ? "" : " : ") */
             << (klass_full_concrete_or_interface_name % ",") // classes
             << (inherit_classes.empty() ? "" : ",")
             << " efl.eo.IWrapper" << (root ? ", IDisposable" : "")
             << (inherit_interfaces.empty() ? "" : ",")
             << (klass_full_concrete_or_interface_name % ",") // interfaces
             << "\n{\n"
             )
           .generate(sink, std::make_tuple(cls, inherit_classes, inherit_interfaces), inherit_cxt))
           return false;


         // Class body
         if(!generate_fields(sink, cls, inherit_cxt))
           return false;

         if (!generate_constructors(sink, cls, inherit_cxt))
           return false;

         if (!generate_dispose_methods(sink, cls, inherit_cxt))
           return false;

         if (!generate_static_cast_method(sink, cls, inherit_cxt))
           return false;
        
         if (!generate_equals_method(sink, inherit_cxt))
           return false;

         if (!generate_events(sink, cls, inherit_cxt))
             return false;

         if (!generate_events_registration(sink, cls, inherit_cxt))
             return false;

         // Parts
         if(!as_generator(*(part_definition))
            .generate(sink, cls.get_all_parts(), inherit_cxt)) return false;

         // Inherit function definitions
         if(!as_generator(*(function_definition(true)))
            .generate(sink, methods, inherit_cxt)) return false;

         // Async wrappers
         if(!as_generator(*(async_function_definition(true))).generate(sink, methods, inherit_cxt))
           return false;

         if(!as_generator(*(event_argument_wrapper)).generate(sink, cls.events, context))
           return false;
         
         if(!as_generator("}\n").generate(sink, attributes::unused, inherit_cxt)) return false;
       }

     std::size_t function_count = get_inheritable_function_count(cls);

     int function_registration_index = 0;
     auto index_generator = [&function_registration_index]
       {
         return function_registration_index++;
       };
     
     // Native Inherit class
     if(class_type == "class")
       {
         auto inative_cxt = context_add_tag(class_context{class_context::inherit_native}, context);
         auto native_inherit_name = name_helpers::klass_native_inherit_name(cls);
         if(!as_generator
            (
             "internal " << class_type << " " << native_inherit_name << " {\n"
             << scope_tab << "public static byte class_initializer(IntPtr klass)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "Efl_Op_Description[] descs = new Efl_Op_Description[" << grammar::int_ << "];\n"
            )
            .generate(sink, function_count, inative_cxt))
           return false;

         // Native wrapper registration
         if(!as_generator(*(function_registration(index_generator, cls)))
            .generate(sink, methods, inative_cxt)) return false;

         if(!as_generator
            (   scope_tab << scope_tab << "IntPtr descs_ptr = Marshal.AllocHGlobal(Marshal.SizeOf(descs[0])*" << function_count << ");\n"
             << scope_tab << scope_tab << "IntPtr ptr = descs_ptr;\n"
             << scope_tab << scope_tab << "for(int i = 0; i != " << function_count << "; ++i)\n"
             << scope_tab << scope_tab << "{\n"
             << scope_tab << scope_tab << scope_tab << "Marshal.StructureToPtr(descs[i], ptr, false);\n"
             << scope_tab << scope_tab << scope_tab << "ptr = IntPtr.Add(ptr, Marshal.SizeOf(descs[0]));\n"
             << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << "Efl_Object_Ops ops;\n"
             << scope_tab << scope_tab << "ops.descs = descs_ptr;\n"
             << scope_tab << scope_tab << "ops.count = (UIntPtr)" << function_count << ";\n"
             << scope_tab << scope_tab << "IntPtr ops_ptr = Marshal.AllocHGlobal(Marshal.SizeOf(ops));\n"
             << scope_tab << scope_tab << "Marshal.StructureToPtr(ops, ops_ptr, false);\n"
             << scope_tab << scope_tab << "efl.eo.Globals.efl_class_functions_set(klass, ops_ptr, IntPtr.Zero);\n"
            ).generate(sink, attributes::unused, inative_cxt)) return false;


         if(!as_generator(scope_tab << scope_tab << "return 1;\n"
                          << scope_tab << "}\n")
            .generate(sink, attributes::unused, inative_cxt)) return false;
         //
         // Native method definitions
         if(!as_generator(*(native_function_definition(cls)))
            .generate(sink, methods, inative_cxt)) return false;

         if(!as_generator("}\n").generate(sink, attributes::unused, inative_cxt)) return false;
       }

     if(!name_helpers::close_namespaces(sink, cls.namespaces, context))
       return false;

     return true;
   }

   template <typename OutputIterator, typename Context>
   bool generate_fields(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     if (helpers::has_regular_ancestor(cls))
       return true;

     // TODO Maybe we should avoid defining these dicts for every instance.
     // To avoid the 'new' warning, maybe we could prefix the field with the class name
     bool cls_has_string_return = has_string_return(cls) || 1;
     bool cls_has_stringshare_return = has_stringshare_return(cls) || 1;

     return as_generator(
                scope_tab << "protected System.IntPtr handle;\n"
                << scope_tab << (cls_has_string_return ? ("public Dictionary<String, IntPtr> cached_strings = new Dictionary<String, IntPtr>();") : "") << "\n"
                << scope_tab << (cls_has_stringshare_return ? ("public Dictionary<String, IntPtr> cached_stringshares = new Dictionary<String, IntPtr>();") : "") << "\n"
                << scope_tab << "///<summary>Pointer to the native instance.</summary>\n"
                << scope_tab << "public System.IntPtr raw_handle {\n"
                << scope_tab << scope_tab << "get { return handle; }\n"
                << scope_tab << "}\n"

                << scope_tab << "protected bool inherited;\n"
             ).generate(sink, attributes::unused, context);
   }

   template <typename OutputIterator, typename Context>
   bool generate_constructors(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     bool root = !helpers::has_regular_ancestor(cls);
     auto inherit_name = name_helpers::klass_concrete_name(cls);
     auto native_inherit_name = name_helpers::klass_native_inherit_name(cls);

     if(!as_generator(
             scope_tab << "///<summary>Delegate for function to be called from inside the native constructor.</summary>\n"
             << scope_tab << "public" << (root ? "" : " new") << " delegate void ConstructingMethod(" << /*interface_name*/ inherit_name << " obj);\n"
             << scope_tab << "[System.Runtime.InteropServices.DllImport(" << context_find_tag<library_context>(context).actual_library_name(cls.filename)
             << ")] public static extern System.IntPtr\n"
             << scope_tab << scope_tab << name_helpers::klass_get_name(cls) << "();\n"
            ).generate(sink, attributes::unused, context))
       return false;

     if(!as_generator(
                scope_tab << "public " << (root ? "" : "new ") <<  "static System.IntPtr klass = System.IntPtr.Zero;\n"
                << scope_tab << "private static readonly object klassAllocLock = new object();\n"
                << scope_tab << "///<summary>Pointer to the native class description.</summary>\n"
                << scope_tab << "public " << (root ? "" : "new " ) << "System.IntPtr raw_klass {\n"
                << scope_tab << scope_tab << "get { return klass; }\n"
                << scope_tab << "}\n"
            ).generate(sink, attributes::unused, context))
         return false;

     if (!root)
       {
          return as_generator(
                     scope_tab << "///<summary>Creates a new instance.</summary>\n"
                     << scope_tab << "///<param>Parent instance.</param>\n"
                     << scope_tab << "///<param>Delegate to call constructing methods that should be run inside the constructor.</param>\n"
                     // FIXME Pass the class name and class get function to this constructor.
                     << scope_tab << "public " << inherit_name << "(efl.Object parent = null, ConstructingMethod init_cb=null) : base(" << native_inherit_name << ".class_initializer, \"" << inherit_name << "\", " << name_helpers::klass_get_name(cls) <<  "(), typeof(" << inherit_name << "), parent, ref klass)\n"
                     << scope_tab << "{\n"
                     << scope_tab << scope_tab << "if (init_cb != null) {\n"
                     << scope_tab << scope_tab << scope_tab << "init_cb(this);\n"
                     << scope_tab << scope_tab << "}\n"
                     << scope_tab << scope_tab << "// Actually finish instantiating the Eo object.\n"
                     << scope_tab << scope_tab << "FinishInstantiation();\n"
                     << scope_tab << "}\n"

                     << scope_tab << "///<summary>Internal construtor to forward the wrapper initialization to the root class.\n"
                     << scope_tab << "protected " << inherit_name << "(efl.eo.Globals.class_initializer class_initializer, String klass_name, IntPtr base_klass, Type managed_type, efl.Object parent, ref IntPtr target_klass) : base(class_initializer, klass_name, base_klass, managed_type, parent, ref target_klass) {}\n"

                     << scope_tab << "///<summary>Constructs an instance from a native pointer.</summary>\n"
                     << scope_tab << "public " << inherit_name << "(System.IntPtr raw)" << (root ? "" : " : base(raw)") << "\n"
                     << scope_tab << "{\n"
                     << scope_tab << scope_tab << (root ? "handle = raw;\n" : "")
                     << scope_tab << scope_tab << "register_event_proxies();\n"
                     << scope_tab << "}\n"
                  ).generate(sink, attributes::unused, context);
       }

     // Detailed constructors go only in root classes.
     return as_generator(
             scope_tab << "///<summary>Creates a new instance.</summary>\n"
             << scope_tab << "///<param>Parent instance.</param>\n"
             << scope_tab << "///<param>Delegate to call constructing methods that should be run inside the constructor.</param>\n"
             << scope_tab << "public " << inherit_name << "(efl.Object parent = null, ConstructingMethod init_cb=null) : this(" << native_inherit_name << ".class_initializer, \"" << inherit_name << "\", " << name_helpers::klass_get_name(cls) << "(), typeof(" << inherit_name << "), parent, ref klass)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "if (init_cb != null) {\n"
             << scope_tab << scope_tab << scope_tab << "init_cb(this);\n"
             << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << "// Actually finish instantiating the Eo object.\n"
             << scope_tab << scope_tab << "FinishInstantiation();\n"
             << scope_tab << "}\n"
             /* << scope_tab << "protected " << inherit_name << "() {}\n" */

             << scope_tab << "protected " << inherit_name << "(efl.eo.Globals.class_initializer class_initializer, String klass_name, IntPtr base_klass, Type managed_type, efl.Object parent, ref IntPtr target_klass)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "inherited = this.GetType() != managed_type;\n"
             << scope_tab << scope_tab << "IntPtr actual_klass = base_klass;\n"
             << scope_tab << scope_tab << "if (inherited) {\n"
             << scope_tab << scope_tab << scope_tab << "if (target_klass == System.IntPtr.Zero) {\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << "lock (klassAllocLock) {\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "if (target_klass == System.IntPtr.Zero) {\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "target_klass = efl.eo.Globals.register_class(class_initializer, klass_name, base_klass);\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "if (target_klass == System.IntPtr.Zero) {\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "throw new System.InvalidOperationException(\"Failed to initialize class '" << inherit_name << "'\");\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << scope_tab << "actual_klass = target_klass;\n"
             << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << "handle = efl.eo.Globals.instantiate_start(actual_klass, parent);\n"
             << scope_tab << scope_tab << "register_event_proxies();\n"
             << scope_tab << "}\n"

             << scope_tab << "protected void FinishInstantiation()\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "if (inherited) {\n"
             << scope_tab << scope_tab << scope_tab << "efl.eo.Globals.data_set(this);\n"
             << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << "handle = efl.eo.Globals.instantiate_end(handle);\n"
             << scope_tab << scope_tab << "eina.Error.RaiseIfOccurred();\n"
             << scope_tab << "}\n"

             << scope_tab << "///<summary>Constructs an instance from a native pointer.</summary>\n"
             << scope_tab << "public " << inherit_name << "(System.IntPtr raw)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "handle = raw;\n"
             << scope_tab << scope_tab << "register_event_proxies();\n"
             << scope_tab << "}\n"
             ).generate(sink, attributes::unused, context);
   }


   template <typename OutputIterator, typename Context>
   bool generate_dispose_methods(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     std::string name = join_namespaces(cls.namespaces, '.') + cls.eolian_name;
     if (helpers::has_regular_ancestor(cls))
       return true;

     bool cls_has_string_return = has_string_return(cls);
     bool cls_has_stringshare_return = has_stringshare_return(cls);

     auto inherit_name = name_helpers::klass_concrete_name(cls);

     return as_generator(

             scope_tab << "///<summary>Destructor.</summary>\n"
             << scope_tab << "~" << inherit_name << "()\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "Dispose(false);\n"
             << scope_tab << "}\n"

             << scope_tab << "///<summary>Releases the underlying native instance.</summary>\n"
             << scope_tab << "protected virtual void Dispose(bool disposing)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "if (handle != System.IntPtr.Zero) {\n"
             << scope_tab << scope_tab << scope_tab << "//FIXME C# is complaining that the class doen't have the Layout\n"
             << scope_tab << scope_tab << scope_tab << "//\n"
             << scope_tab << scope_tab << scope_tab << "efl.eo.Globals.efl_unref(handle);\n"
             << scope_tab << scope_tab << scope_tab << "handle = System.IntPtr.Zero;\n"
             << scope_tab << scope_tab << "}\n"
             << scope_tab << "}\n"

             << scope_tab << "///<summary>Releases the underlying native instance.</summary>\n"
             << scope_tab << "public void Dispose()\n"
             << scope_tab << "{\n"
             << scope_tab << (cls_has_string_return ? "efl.eo.Globals.free_dict_values(cached_strings);" : "") << "\n"
             << scope_tab << (cls_has_stringshare_return ? "efl.eo.Globals.free_stringshare_values(cached_stringshares);" : "") << "\n"
             << scope_tab << scope_tab << "Dispose(true);\n"
             << scope_tab << scope_tab << "GC.SuppressFinalize(this);\n"
             << scope_tab << "}\n"
             ).generate(sink, attributes::unused, context);
   }

   template <typename OutputIterator, typename Context>
   bool generate_events_registration(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     bool root = !helpers::has_regular_ancestor(cls);
     std::string virtual_modifier = " ";

     if (is_inherit_context(context))
         virtual_modifier = root ? "virtual " : "override ";
     // Event proxy registration
     if (!as_generator(
            scope_tab << "protected " << virtual_modifier << "void register_event_proxies()\n"
            << scope_tab << "{\n"
         )
         .generate(sink, NULL, context))
         return false;

     // Generate event registrations here

     if (!root)
       if (!as_generator(scope_tab << scope_tab << "base.register_event_proxies();\n").generate(sink, NULL, context))
         return false;

     // Assigning the delegates
     if (!as_generator(*(event_registration())).generate(sink, cls.events, context))
       return false;

     for (auto&& c : helpers::non_implemented_interfaces(cls))
       {
          // Only non-regular types (which declare events through interfaces) need to register them.
          if (c.type == attributes::class_type::regular)
            continue;

          attributes::klass_def klass(get_klass(c, cls.unit), cls.unit);

          if (!as_generator(*(event_registration(&klass, true))).generate(sink, klass.events, context))
             return false;
       }

     if (!as_generator(
            scope_tab << "}\n"
                 ).generate(sink, NULL, context))
         return false;

     return true;
   }

   template <typename OutputIterator, typename Context>
   bool generate_events(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {

     if (!has_events(cls))
         return true;

     std::string visibility = is_inherit_context(context) ? "protected" : "private";

     if (!helpers::has_regular_ancestor(cls))
       {
         if (!as_generator(scope_tab << "protected readonly object eventLock = new object();\n"
                 << scope_tab << "protected Dictionary<string, int> event_cb_count = new Dictionary<string, int>();\n")
                 .generate(sink, NULL, context))
             return false;

     // Callback registration functions
     if (!as_generator(
            scope_tab << "protected bool add_cpp_event_handler(string key, efl.Event_Cb evt_delegate) {\n"
            << scope_tab << scope_tab << "int event_count = 0;\n"
            << scope_tab << scope_tab << "if (!event_cb_count.TryGetValue(key, out event_count))\n"
            << scope_tab << scope_tab << scope_tab << "event_cb_count[key] = event_count;\n"
            << scope_tab << scope_tab << "if (event_count == 0) {\n"

            << scope_tab << scope_tab << scope_tab << "IntPtr desc = efl.Event_Description.GetNative(key);\n"
            << scope_tab << scope_tab << scope_tab << "if (desc == IntPtr.Zero) {\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "eina.Log.Error($\"Failed to get native event {key}\");\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "return false;\n"
            << scope_tab << scope_tab << scope_tab << "}\n"

            << scope_tab << scope_tab << scope_tab << "bool result = efl.eo.Globals.efl_event_callback_priority_add(handle, desc, 0, evt_delegate, System.IntPtr.Zero);\n"
            << scope_tab << scope_tab << scope_tab << "if (!result) {\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "eina.Log.Error($\"Failed to add event proxy for event {key}\");\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "return false;\n"
            << scope_tab << scope_tab << scope_tab << "}\n"
            << scope_tab << scope_tab << scope_tab << "eina.Error.RaiseIfOccurred();\n"
            << scope_tab << scope_tab << "} \n"
            << scope_tab << scope_tab << "event_cb_count[key]++;\n"
            << scope_tab << scope_tab << "return true;\n"
            << scope_tab << "}\n"
            << scope_tab << "protected bool remove_cpp_event_handler(string key, efl.Event_Cb evt_delegate) {\n"
            << scope_tab << scope_tab << "int event_count = 0;\n"
            << scope_tab << scope_tab << "if (!event_cb_count.TryGetValue(key, out event_count))\n"
            << scope_tab << scope_tab << scope_tab << "event_cb_count[key] = event_count;\n"
            << scope_tab << scope_tab << "if (event_count == 1) {\n"

            << scope_tab << scope_tab << scope_tab << "IntPtr desc = efl.Event_Description.GetNative(key);\n"
            << scope_tab << scope_tab << scope_tab << "if (desc == IntPtr.Zero) {\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "eina.Log.Error($\"Failed to get native event {key}\");\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "return false;\n"
            << scope_tab << scope_tab << scope_tab << "}\n"

            << scope_tab << scope_tab << scope_tab << "bool result = efl.eo.Globals.efl_event_callback_del(handle, desc, evt_delegate, System.IntPtr.Zero);\n"
            << scope_tab << scope_tab << scope_tab << "if (!result) {\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "eina.Log.Error($\"Failed to remove event proxy for event {key}\");\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "return false;\n"
            << scope_tab << scope_tab << scope_tab << "}\n"
            << scope_tab << scope_tab << scope_tab << "eina.Error.RaiseIfOccurred();\n"
            << scope_tab << scope_tab << "} else if (event_count == 0) {\n"
            << scope_tab << scope_tab << scope_tab << "eina.Log.Error($\"Trying to remove proxy for event {key} when there is nothing registered.\");\n"
            << scope_tab << scope_tab << scope_tab << "return false;\n"
            << scope_tab << scope_tab << "} \n"
            << scope_tab << scope_tab << "event_cb_count[key]--;\n"
            << scope_tab << scope_tab << "return true;\n"
            << scope_tab << "}\n"
            )
             .generate(sink, NULL, context))
         return false;
       }

     // Self events
     if (!as_generator(*(event_definition(cls))).generate(sink, cls.events, context))
       return false;

     // Inherited events

     // For now, as mixins can inherit from regular classes, we can't filter out inherited events.
     auto inherits = helpers::non_implemented_interfaces(cls);
     for (auto&& c : inherits)
       {
          attributes::klass_def klass(get_klass(c, cls.unit), cls.unit);
          if (!as_generator(*(event_definition(klass, true))).generate(sink, klass.events, context))
            return false;
       }
     return true;
   }
};

struct klass const klass = {};

}

namespace efl { namespace eolian { namespace grammar {
  
template <>
struct is_eager_generator<struct ::eolian_mono::klass> : std::true_type {};
template <>
struct is_generator<struct ::eolian_mono::klass> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<struct ::eolian_mono::klass> : std::integral_constant<int, 1> {};
}
      
} } }

#endif
