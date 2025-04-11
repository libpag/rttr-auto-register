#pragma once

#include <rttrGeneratorMacro.h>

struct foo{
    REGISTER_CLASS()
    REGISTER_PROPERTY
    int bar;
    REGISTER_PROPERTY_READONLY
    int* bar_pointer;
};