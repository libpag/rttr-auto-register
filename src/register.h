/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making tgfx available.
//
//  Copyright (C) 2024 THL A29 Limited, a Tencent company. All rights reserved.
//
//  Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
//  in compliance with the License. You may obtain a copy of the License at
//
//      https://opensource.org/licenses/BSD-3-Clause
//
//  unless required by applicable law or agreed to in writing, software distributed under the
//  license is distributed on an "as is" basis, without warranties or conditions of any kind,
//  either express or implied. see the license for the specific language governing permissions
//  and limitations under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <filesystem>
#include <clang-c/Index.h>
#include "clangraii/translationUnit.h"
#include "clangraii/clangIndex.h"
#include "clangraii/clangString.h"

namespace Register {

    struct RTTRMarkClassInfo {
        std::string className;
        std::string path;
        std::vector<std::string> properties;
        std::vector<std::string> readOnlyProperties;
    };

    struct RTTRMarkEnumInfo {
        std::string enumName;
        std::string path;
        std::vector<std::string> elements;
    };

    // Token 结构体
    struct Token {
        CXTokenKind kind;
        std::string spelling;
        CXCursor cursor;

        bool isIdentifier(const std::string& id) const {
            return kind == CXToken_Identifier && spelling == id;
        }

        bool isCommentOrPunctuation() const {
            return kind == CXToken_Comment || kind == CXToken_Punctuation;
        }
    };

    std::shared_ptr<TranslationUnit> GetTranslationUnit(ClangIndex& index, const std::string& filepath,
        const std::vector<const char*>& args);

    std::string GetFullQualifiedName(CXCursor cursor);

    std::optional<CXCursor> FindNextMember(const std::vector<Token>& tokens, size_t startIndex);

    std::vector<std::string> ProcessProperty(CXCursor cursor, const std::string& property_macro);

    std::vector<RTTRMarkClassInfo> ParseRttrMarkClass(TranslationUnit& tu, const std::string& mark_macro,
        const std::string& property_macro, const std::string& readonly_property_macro);

    std::vector<RTTRMarkEnumInfo> ParseRttrMarkEnum(TranslationUnit& tu, const std::string& mark_macro);

    void GenerateCPPCode(const std::vector<RTTRMarkClassInfo>& classInfos, const std::vector<RTTRMarkEnumInfo>& enumInfos,
        const std::string& outputFile, const std::vector<std::string>& relativePaths);

    void GetHeaderFiles(const std::filesystem::path& dir, std::vector<std::string>& files);

    std::vector<std::string> splitBySemicolon(const std::string& str);
}
