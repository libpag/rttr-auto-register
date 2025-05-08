// Auto-generated code
#pragma once
#include "example.h"
#include <iostream>
#include <rttr/registration.h>
using namespace rttr;

RTTR_REGISTRATION {
  using namespace rttr;

  std::cout << "Rttr registed!" << std::endl;

  registration::class_<base>("base");

  registration::class_<derived>("derived").property_readonly(
      "derivedvalue", &derived::derivedvalue);

  registration::class_<Outer::Inner::MyClass>("MyClass")
      .property_readonly("value", &Outer::Inner::MyClass::value)
      .property_readonly("value1", &Outer::Inner::MyClass::value1)
      .property_readonly("value2", &Outer::Inner::MyClass::value2)
      .property_readonly("value3", &Outer::Inner::MyClass::value3)
      .property_readonly("readOnlyvalue", &Outer::Inner::MyClass::readOnlyvalue)
      .property_readonly("readOnlyvalue1",
                         &Outer::Inner::MyClass::readOnlyvalue1)
      .property_readonly("readOnlyvalue2",
                         &Outer::Inner::MyClass::readOnlyvalue2)
      .property_readonly("readOnlyvalue3",
                         &Outer::Inner::MyClass::readOnlyvalue3)
      .property_readonly("value4", &Outer::Inner::MyClass::value4)
      .property_readonly("shareptr", &Outer::Inner::MyClass::shareptr)
      .method("hhh_function", &Outer::Inner::MyClass::hhh_functionRTTR);

  registration::class_<GlobalClass>("GlobalClass");

  registration::class_<Outer::Container>("Container");

  registration::class_<OuterStruct::InnerClass>("InnerClass")
      .method("hhh_function", &OuterStruct::InnerClass::hhh_functionRTTR);

  registration::class_<test::testStruct>("testStruct")
      .property_readonly("c", &test::testStruct::c)
      .property_readonly("d", &test::testStruct::d)
      .property_readonly("a", &test::testStruct::a)
      .property_readonly("b", &test::testStruct::b);

  registration::class_<test1::testStruct1>("testStruct1")
      .property_readonly("c", &test1::testStruct1::c)
      .property_readonly("d", &test1::testStruct1::d)
      .property_readonly("a", &test1::testStruct1::a)
      .property_readonly("b", &test1::testStruct1::b)
      .property_readonly("f", &test1::testStruct1::f);

  registration::enumeration<Outer::Inner::MyEnum>("MyEnum")(
      value("VALUE1", Outer::Inner::MyEnum::VALUE1),
      value("VALUE2", Outer::Inner::MyEnum::VALUE2),
      value("VALUE3", Outer::Inner::MyEnum::VALUE3));

  registration::enumeration<Outer::Inner::MyEnum1>("MyEnum1")(
      value("VALUE1", Outer::Inner::MyEnum1::VALUE1),
      value("VALUE2", Outer::Inner::MyEnum1::VALUE2),
      value("VALUE3", Outer::Inner::MyEnum1::VALUE3));
}
