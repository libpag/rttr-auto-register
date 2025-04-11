#pragma once

#include <memory>
#include <rttrGeneratorMacro.h>
namespace Outer{
    namespace Inner {
        enum class MyEnum {
            REGISTER_ENUM
            VALUE1,
            VALUE2,
            VALUE3
        };
    }
}

class base{
    REGISTER_CLASS()
};

class derived : public base{
    REGISTER_CLASS(base)

    REGISTER_PROPERTY
    int derivedvalue;

};


namespace Outer {
    namespace Inner {
        class MyClass {
            REGISTER_CLASS()

            REGISTER_PROPERTY
            int value;
            REGISTER_PROPERTY
            int value1;
            REGISTER_PROPERTY
      

            int value2;
            REGISTER_PROPERTY
            int value3;
            REGISTER_PROPERTY_READONLY
            //comment test
            int readOnlyvalue;
            REGISTER_PROPERTY_READONLY
            /*comment test*/
            int readOnlyvalue1;
            REGISTER_PROPERTY_READONLY
            int readOnlyvalue2;
            REGISTER_PROPERTY_READONLY
            int readOnlyvalue3;
            REGISTER_PROPERTY
            int value4;
            REGISTER_PROPERTY 
            std::shared_ptr<int> shareptr;
        };
    }
}

class GlobalClass {
    REGISTER_CLASS();
    void func();
};

namespace Outer {
    class Container {
    public:
        class NestedClass {
            REGISTER_CLASS(NestedClass)
            int value;
        };
    };
}

struct OuterStruct {
    class InnerClass {
        REGISTER_CLASS()
        void func();
    };
};