/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 THL A29 Limited, a Tencent company. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not
//  use this file except in compliance with the License. You may obtain a copy
//  of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  unless required by applicable law or agreed to in writing, software
//  distributed under the license is distributed on an "as is" basis, without
//  warranties or conditions of any kind, either express or implied. see the
//  license for the specific language governing permissions and limitations
//  under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "./third/rttrGeneratorMacro.h"
#include <memory>
using namespace std;
namespace Outer {
namespace Inner {
enum class REGISTER_ENUM MyEnum { VALUE1, VALUE2, VALUE3 };
}
} // namespace Outer

namespace Outer {
namespace Inner {
enum REGISTER_ENUM MyEnum1 { VALUE1, VALUE2, VALUE3 };
}
} // namespace Outer

class REGISTER_CLASS base {};

class REGISTER_CLASS derived : public base {

  int derivedvalue;
};

namespace Outer {
namespace Inner {
class REGISTER_CLASS MyClass {

  int value;
  int value1;

  int value2;
  int value3;
  // comment test
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
} // namespace Inner
} // namespace Outer

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
} // namespace Outer

struct OuterStruct {
  class REGISTER_CLASS InnerClass {
    void func();

  private:
    MAKE_FUNCTION(hhh)
  };
};

struct test {
  struct REGISTER_CLASS testStruct {
  public:
    float c;
    float d;

  private:
    int a;
    int b;
    void print();
  };
};

struct test1 {
  class REGISTER_CLASS testStruct1 {
  public:
    float c;
    float d;

  private:
    int a;
    int b;
    float *f;
    void print();
  };
};