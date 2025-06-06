/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making tgfx
//  available.
//
//  Copyright (C) 2024 THL A29 Limited, a Tencent company. All rights reserved.
//
//  Licensed under the BSD 3-Clause License (the "License"); you may not use
//  this file except in compliance with the License. You may obtain a copy of
//  the License at
//
//      https://opensource.org/licenses/BSD-3-Clause
//
//  unless required by applicable law or agreed to in writing, software
//  distributed under the license is distributed on an "as is" basis, without
//  warranties or conditions of any kind, either express or implied. see the
//  license for the specific language governing permissions and limitations
//  under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <clang-c/Index.h>
#include <string>

namespace Register {
class ClangString {
 public:
  explicit ClangString(CXString str) : str_(str) {
  }
  ~ClangString() {
    clang_disposeString(str_);
  }

  std::string str() const {
    return std::string(clang_getCString(str_));
  }

  operator std::string() const {
    return str();
  }

 private:
  CXString str_;
};
}  // namespace Register
