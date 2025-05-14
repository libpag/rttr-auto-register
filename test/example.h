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

#include <memory>
#include "defines.h"

class RTTR_AUTO_REGISTER_CLASS RttrAutoRegisterTestClass1 {
 public:
  int a;
  int b;
  int c;
};

class RTTR_TEST_MACRO RttrAutoRegisterTestClass2 {
 public:
  int a;
  int b;
  int c;
};

class RTTR_AUTO_REGISTER_CLASS RttrAutoRegisterTestClass3 {
public:
  int a;
  int RTTR_SKIP_REGISTER_PROPERTY b;
};

class RTTR_AUTO_REGISTER_CLASS RttrAutoRegisterTestClass4 {
 public:
  int a;
  std::shared_ptr<int> RTTR_SKIP_REGISTER_PROPERTY b = nullptr;
};