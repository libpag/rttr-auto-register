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

#include "clangraii/clangIndex.h"
#include "clangraii/clangString.h"
#include "clangraii/translationUnit.h"
#include <clang-c/Index.h>
#include <cstring>
#include <filesystem>
#include <optional>
#include <set>
#include <unordered_map>

namespace Register {

struct RTTRMarkClassInfo {
  std::string className;
  std::string path;
  std::vector<std::string> properties;
  std::vector<std::string> methods;
};

struct RTTRMarkEnumInfo {
  std::string enumName;
  std::string path;
  std::vector<std::string> elements;
};

struct ForwardDeclInfo {
  bool isFoundDef = false;
  std::string defFilePath;
};

// Token 结构体
struct Token {
  CXTokenKind kind;
  std::string spelling;
  CXCursor cursor;

  bool isIdentifier(const std::string &id) const {
    return kind == CXToken_Identifier && spelling == id;
  }

  bool isCommentOrPunctuation() const {
    return kind == CXToken_Comment || kind == CXToken_Punctuation;
  }
};

std::shared_ptr<TranslationUnit>
GetTranslationUnit(ClangIndex &index, const std::string &filepath,
                   const std::vector<const char *> &args);

std::string GetFullQualifiedName(CXCursor cursor);

std::optional<CXCursor> FindNextMember(const std::vector<Token> &tokens,
                                       size_t startIndex, CXCursorKind kind);

std::vector<std::string> ProcessProperty(CXCursor cursor,
                                         const std::string &property_macro,
                                         CXCursorKind kind);

std::vector<Token> GenerateTokenList(TranslationUnit &tu);

void ParseRttrMarkClass(const std::vector<Token> &token_list,
                        const std::vector<std::string> &registerMacros,
                        std::vector<RTTRMarkClassInfo> &classInfos,
                        std::vector<RTTRMarkEnumInfo> &enumInfos);

void GenerateCPPCode(const std::vector<RTTRMarkClassInfo> &classInfos,
                     const std::vector<RTTRMarkEnumInfo> &enumInfos,
                     const std::string &outputFile,
                     const std::vector<std::string> &relativePaths);

void GetHeaderFiles(const std::filesystem::path &dir,
                    std::vector<std::string> &files);

void GetForwardDecl(
    TranslationUnit &tu,
    std::unordered_map<std::string, Register::ForwardDeclInfo> &map);

void GetClassDefHeaderFileList(
    TranslationUnit &tu, std::unordered_map<std::string, std::string> &DefList);
void GetForwardDeclDefPath(
    const std::unordered_map<std::string, std::string> &DefList,
    std::vector<std::string> &path);
CXType getUnderlyingType(CXType type);
bool typeIsDefined(CXType type);
void printDiagnostics(CXTranslationUnit translationUnit);
} // namespace Register
