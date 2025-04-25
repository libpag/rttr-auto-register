#pragma once

#include <memory>
#include "./third/rttrGeneratorMacro.h"
using namespace std;
namespace Outer{
    namespace Inner {
        enum class REGISTER_ENUM MyEnum {
            VALUE1,
            VALUE2,
            VALUE3
        };
    }
}

namespace Outer{
    namespace Inner {
        enum REGISTER_ENUM MyEnum1 {
            VALUE1,
            VALUE2,
            VALUE3
        };
    }
}

class REGISTER_CLASS base{
};

class REGISTER_CLASS derived : public base{

    int derivedvalue;

};


namespace Outer {
    namespace Inner {
        class REGISTER_CLASS MyClass {

            int value;
            int value1;
      

            int value2;
            int value3;
            //comment test
            int readOnlyvalue;
            /*comment test*/
            int readOnlyvalue1;
            int readOnlyvalue2;
            int readOnlyvalue3;
            int value4;
            std::shared_ptr<int> shareptr;
public:
			MAKE_FUNCTION(hhh)
        };
    }
}

class REGISTER_CLASS GlobalClass {
    void func();
};

namespace Outer {
    class REGISTER_CLASS Container {
    public:
        class NestedClass {
            int value;
        };
    };
}

struct OuterStruct {
    class REGISTER_CLASS InnerClass {
        void func();
private:
MAKE_FUNCTION(hhh)
    };
};


struct test{
    struct REGISTER_CLASS testStruct{
        public:
        float c;
        float d;
        private:
        int a;
        int b;
        void print();
    };
};

struct test1{
    class REGISTER_CLASS testStruct1{
        public:
        float c;
        float d;
        private:
        int a;
        int b;
        float* f;
        void print();
    };
};