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
/////////////////////////////////////////////////////////////////////////////////////////////////s

#pragma once

#include "defines.h"

enum class RTTR_AUTO_REGISTER_CLASS RttrAutoRegisterTestEnum {
  TestEnum1,
  TestEnum2,
  TestEnum3,
  TestEnum4
};

struct RTTR_AUTO_REGISTER_CLASS RttrAutoRegisterTestStruct {
  int a;
  int b;
  int RTTR_SKIP_REGISTER_PROPERTY c;
};

class RTTR_AUTO_REGISTER_CLASS RttrAutoRegisterTestClass5 {
 public:
  int getA();
  int getB();
  int getC();
  RTTR_REGISTER_FUNCTION_AS_PROPERTY("a", getA)
  RTTR_REGISTER_FUNCTION_AS_PROPERTY("b", getB)
  RTTR_REGISTER_FUNCTION_AS_PROPERTY("c", getC)

 private:
  int a;
  int b;
  int c;
};