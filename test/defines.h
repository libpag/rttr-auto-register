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

// Pre-defined macro
#if !defined(RTTR_AUTO_REGISTER_CLASS)
#define RTTR_AUTO_REGISTER_CLASS
#endif

// Pre-defined macro
#if !defined(RTTR_SKIP_REGISTER_PROPERTY)
#define RTTR_SKIP_REGISTER_PROPERTY
#endif

#if !defined(RTTR_REGISTER_FUNCTION_AS_PROPERTY)
#define RTTR_REGISTER_FUNCTION_AS_PROPERTY(propertyName, function)
#endif

// Customized macros
#if !defined(RTTR_TEST_MACRO)
#define RTTR_TEST_MACRO
#endif