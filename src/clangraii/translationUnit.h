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
#include <vector>

namespace Register {
class TranslationUnit {
public:
  TranslationUnit(CXIndex index, const std::string &filename,
                  const std::vector<const char *> &args) {

    tu_ = clang_parseTranslationUnit(index, filename.c_str(), args.data(),
                                     static_cast<int>(args.size()), nullptr, 0,
                                     CXTranslationUnit_None);
  }

  ~TranslationUnit() {
    if (tu_)
      clang_disposeTranslationUnit(tu_);
  }

  operator CXTranslationUnit() const { return tu_; }
  explicit operator bool() const { return tu_ != nullptr; }

private:
  CXTranslationUnit tu_ = nullptr;
};
} // namespace Register
