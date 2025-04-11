// Auto-generated code
#include <rttr/registration>
#include "example.h"
#include "example1.h"
using namespace rttr;




RTTR_REGISTRATION
{
  using namespace rttr;

  registration::class_<base>("base");

  registration::class_<derived>("derived")
      .property("derivedvalue", &derived::derivedvalue);

  registration::class_<Outer::Inner::MyClass>("MyClass")
      .property("value", &Outer::Inner::MyClass::value)
      .property("value1", &Outer::Inner::MyClass::value1)
      .property("value2", &Outer::Inner::MyClass::value2)
      .property("value3", &Outer::Inner::MyClass::value3)
      .property("value4", &Outer::Inner::MyClass::value4)
      .property("shareptr", &Outer::Inner::MyClass::shareptr)
      .property_readonly("readOnlyvalue", &Outer::Inner::MyClass::readOnlyvalue)
      .property_readonly("readOnlyvalue1", &Outer::Inner::MyClass::readOnlyvalue1)
      .property_readonly("readOnlyvalue2", &Outer::Inner::MyClass::readOnlyvalue2)
      .property_readonly("readOnlyvalue3", &Outer::Inner::MyClass::readOnlyvalue3);

  registration::class_<GlobalClass>("GlobalClass");

  registration::class_<Outer::Container::NestedClass>("NestedClass");

  registration::class_<OuterStruct::InnerClass>("InnerClass");

  registration::class_<foo>("foo")
      .property("bar", &foo::bar)
      .property_readonly("bar_pointer", &foo::bar_pointer);

  registration::enumeration<Outer::Inner::MyEnum>("MyEnum")
     (
            value("VALUE1", Outer::Inner::MyEnum::VALUE1),
            value("VALUE2", Outer::Inner::MyEnum::VALUE2),
            value("VALUE3", Outer::Inner::MyEnum::VALUE3)
     );

}
