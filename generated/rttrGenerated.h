// Auto-generated code
#pragma once
#include <rttr/registration.h>
#include <iostream>
#include "../test/example.h"
#include "../test/example1.h"

RTTR_REGISTRATION
{
	using namespace rttr;

	std::cout << "Rttr registed!" << std::endl;

	registration::class_<RttrAutoRegisterTestClass1>("RttrAutoRegisterTestClass1")
		.property("a", &RttrAutoRegisterTestClass1::a)
		.property("b", &RttrAutoRegisterTestClass1::b)
		.property("c", &RttrAutoRegisterTestClass1::c);

	registration::class_<RttrAutoRegisterTestClass2>("RttrAutoRegisterTestClass2")
		.property("a", &RttrAutoRegisterTestClass2::a)
		.property("b", &RttrAutoRegisterTestClass2::b)
		.property("c", &RttrAutoRegisterTestClass2::c);

	registration::class_<RttrAutoRegisterTestClass3>("RttrAutoRegisterTestClass3")
		.property("a", &RttrAutoRegisterTestClass3::a);

	registration::class_<RttrAutoRegisterTestClass4>("RttrAutoRegisterTestClass4")
		.property("a", &RttrAutoRegisterTestClass4::a);

	registration::class_<RttrAutoRegisterTestStruct>("RttrAutoRegisterTestStruct")
		.property("a", &RttrAutoRegisterTestStruct::a)
		.property("b", &RttrAutoRegisterTestStruct::b);

	registration::class_<RttrAutoRegisterTestClass5>("RttrAutoRegisterTestClass5")
		.property_readonly("a", &RttrAutoRegisterTestClass5::getA)
		.property_readonly("b", &RttrAutoRegisterTestClass5::getB)
		.property_readonly("c", &RttrAutoRegisterTestClass5::getC);

	registration::enumeration<RttrAutoRegisterTestEnum>("RttrAutoRegisterTestEnum")
     (
		value("TestEnum1", RttrAutoRegisterTestEnum::TestEnum1),
		value("TestEnum2", RttrAutoRegisterTestEnum::TestEnum2),
		value("TestEnum3", RttrAutoRegisterTestEnum::TestEnum3),
		value("TestEnum4", RttrAutoRegisterTestEnum::TestEnum4)
     );

}
