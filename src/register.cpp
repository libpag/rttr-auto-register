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

#include "register.h"
#include "clangraii/clangDiagnostic.h"
#include <iostream>
#include <optional>
#include <fstream>
#include <sstream>

namespace Register {
    std::shared_ptr<TranslationUnit> GetTranslationUnit(ClangIndex &index, const std::string &filepath,
        const std::vector<const char *> &args) {
        std::shared_ptr<TranslationUnit> tu = std::make_shared<TranslationUnit>(index, filepath, args);
        if(!(*tu)) {
            std::cerr << "Failed to parse file: " << filepath << std::endl;
            return nullptr;
        }

        return tu;
    }

    std::string GetFullQualifiedName(CXCursor cursor) {
        std::vector<std::string> parts;

        auto add_cursor_name = [&parts](CXCursor c) {
            ClangString spelling(clang_getCursorSpelling(c));
            std::string name = spelling.str();
            if (!name.empty()) {
                parts.push_back(name);
            }
        };

        // 处理当前cursor
        CXCursorKind kind = clang_getCursorKind(cursor);
        if (kind == CXCursor_ClassDecl ||
            kind == CXCursor_StructDecl ||
            kind == CXCursor_EnumDecl) {
            add_cursor_name(cursor);
            }

        // 处理父级
        CXCursor parent = clang_getCursorSemanticParent(cursor);
        while (!clang_Cursor_isNull(parent)) {
            CXCursorKind parent_kind = clang_getCursorKind(parent);

            if (parent_kind == CXCursor_Namespace ||
                parent_kind == CXCursor_ClassDecl ||
                parent_kind == CXCursor_StructDecl) {

                ClangString parent_spelling(clang_getCursorSpelling(parent));
                std::string name = parent_spelling.str();
                if (!name.empty()) {
                    parts.insert(parts.begin(), name);
                }
                }

            parent = clang_getCursorSemanticParent(parent);
        }

        // 拼接结果
        if (parts.empty()) {
            return "";
        }

        std::string result;
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i != 0) {
                result += "::";
            }
            result += parts[i];
        }

        return result;
    }

    std::optional<CXCursor> FindNextMember(const std::vector<Token> &tokens, size_t startIndex) {
        for (size_t i = startIndex + 1; i < tokens.size(); ++i) {
            const Token& token = tokens[i];

            if (token.isCommentOrPunctuation()) {
                continue;
            }

            CXCursor cursor = token.cursor;
            while (!clang_Cursor_isNull(cursor)) {
                if (clang_getCursorKind(cursor) == CXCursor_FieldDecl) {
                    return cursor;
                }
                cursor = clang_getCursorSemanticParent(cursor);
            }
        }
        return std::nullopt;
    }

    std::vector<std::string> ProcessProperty(CXCursor cursor, const std::string &property_macro) {
        std::vector<std::string> result;

        // 获取token范围
        CXToken* tokens = nullptr;
        unsigned num_tokens = 0;
        CXSourceRange range = clang_getCursorExtent(cursor);
        CXTranslationUnit tu = clang_Cursor_getTranslationUnit(cursor);
        clang_tokenize(tu, range, &tokens, &num_tokens);

        // 转换为Token结构体
        std::vector<Token> token_list;
        for (unsigned i = 0; i < num_tokens; ++i) {
            CXToken token = tokens[i];
            CXString spelling = clang_getTokenSpelling(tu, token);
            CXCursor token_cursor;
            clang_annotateTokens(tu, &token, 1, &token_cursor);

            token_list.push_back({
                clang_getTokenKind(token),
                ClangString(spelling).str(),
                token_cursor
            });
        }
        clang_disposeTokens(tu, tokens, num_tokens);

        // 处理token
        for (size_t i = 0; i < token_list.size(); ++i) {
            const Token& token = token_list[i];

            // 查找属性宏
            if (token.isIdentifier(property_macro) && i + 1 < token_list.size()) {
                // 查找下一个成员变量
                auto member_cursor = FindNextMember(token_list, i);

                if (member_cursor) {
                    // 获取成员名称
                    CXString member_name = clang_getCursorSpelling(*member_cursor);
                    result.push_back(ClangString(member_name).str());
                }
            }
        }

        return result;
    }

    std::vector<RTTRMarkClassInfo> ParseRttrMarkClass(TranslationUnit& tu, const std::string &mark_macro,
        const std::string &property_macro, const std::string &readonly_property_macro) {

        std::vector<RTTRMarkClassInfo> result;

        if (!tu) {
            return result;
        }

        // 获取翻译单元的游标
        CXCursor tu_cursor = clang_getTranslationUnitCursor(tu);

        // 获取token范围
        CXToken* tokens = nullptr;
        unsigned num_tokens = 0;
        CXSourceRange range = clang_getCursorExtent(tu_cursor);
        clang_tokenize(tu, range, &tokens, &num_tokens);

        // 转换为Token结构体
        std::vector<Token> token_list;
        for (unsigned i = 0; i < num_tokens; ++i) {
            CXToken token = tokens[i];
            CXString spelling = clang_getTokenSpelling(tu, token);
            CXCursor token_cursor;
            clang_annotateTokens(tu, &token, 1, &token_cursor);

            token_list.push_back({
                clang_getTokenKind(token),
                ClangString(spelling).str(),
                token_cursor
            });
        }
        clang_disposeTokens(tu, tokens, num_tokens);

        // 处理token
        for (size_t i = 0; i < token_list.size(); ++i) {
            const Token& token = token_list[i];

            try {
                if (token.isIdentifier(mark_macro)) {
                    CXCursor cursor = token.cursor;

                    // 向上查找类/结构体声明
                    while (!clang_Cursor_isNull(cursor)) {
                        CXCursorKind kind = clang_getCursorKind(cursor);

                        if (kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl) {
                            // 获取类名
                            ClangString name(clang_getCursorSpelling(cursor));

                            // 获取完整限定名
                            std::string qualified_name = GetFullQualifiedName(cursor);

                            // 处理属性
                            auto properties = ProcessProperty(cursor, property_macro);
                            auto readOnlyProperties = ProcessProperty(cursor, readonly_property_macro);

                            // 添加到结果
                            result.push_back({
                                name.str(),
                                qualified_name,
                                properties,
                                readOnlyProperties
                            });

                            break;
                        }

                        // 继续向上查找
                        cursor = clang_getCursorSemanticParent(cursor);
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Error processing token: " << e.what() << std::endl;
            }
        }
        return result;
    }

    std::vector<RTTRMarkEnumInfo> ParseRttrMarkEnum(TranslationUnit& tu, const std::string &mark_macro) {
        std::vector<RTTRMarkEnumInfo> result;

        if (!tu) {
            return result;
        }

        // 获取翻译单元的游标
        CXCursor tu_cursor = clang_getTranslationUnitCursor(tu);

        // 获取token范围
        CXToken* tokens = nullptr;
        unsigned num_tokens = 0;
        CXSourceRange range = clang_getCursorExtent(tu_cursor);
        clang_tokenize(tu, range, &tokens, &num_tokens);

        // 转换为Token结构体
        std::vector<Token> token_list;
        for (unsigned i = 0; i < num_tokens; ++i) {
            CXToken token = tokens[i];
            CXString spelling = clang_getTokenSpelling(tu, token);
            CXCursor token_cursor;
            clang_annotateTokens(tu, &token, 1, &token_cursor);

            token_list.push_back({
                clang_getTokenKind(token),
                ClangString(spelling).str(),
                token_cursor
            });
        }
        clang_disposeTokens(tu, tokens, num_tokens);

        // 处理token
        for (size_t i = 0; i < token_list.size(); ++i) {
            const Token& token = token_list[i];

            try {
                if (token.kind == CXToken_Identifier && token.spelling == mark_macro) {
                    CXCursor cursor = token.cursor;

                    // 向上查找枚举声明
                    while (!clang_Cursor_isNull(cursor)) {
                        CXCursorKind kind = clang_getCursorKind(cursor);

                        if (kind == CXCursor_EnumDecl) {
                            // 获取枚举名
                            ClangString name(clang_getCursorSpelling(cursor));

                            // 获取完整限定名
                            std::string qualified_name = GetFullQualifiedName(cursor);

                            // 收集枚举常量
                            std::vector<std::string> elements;

                            // 遍历枚举的子节点
                            clang_visitChildren(cursor,
                                [](CXCursor c, CXCursor parent, CXClientData client_data) {
                                    if (clang_getCursorKind(c) == CXCursor_EnumConstantDecl) {
                                        auto elements = static_cast<std::vector<std::string>*>(client_data);
                                        CXString spelling = clang_getCursorSpelling(c);
                                        elements->push_back(ClangString(spelling).str());
                                    }
                                    return CXChildVisit_Continue;
                                },
                                &elements);

                            // 添加到结果
                            result.push_back({
                                name.str(),
                                qualified_name,
                                elements
                            });

                            break;
                        }

                        // 继续向上查找
                        cursor = clang_getCursorSemanticParent(cursor);
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Error processing token: " << e.what() << std::endl;
            }
        }

        return result;
    }

    void GenerateCPPCode(const std::vector<RTTRMarkClassInfo> &classInfos,
        const std::vector<RTTRMarkEnumInfo> &enumInfos, const std::string &outputFile,
        const std::vector<std::string>& relativePaths) {
        std::ofstream f(outputFile);
        if (!f.is_open()) {
            std::cerr << "Error: Could not open file " << outputFile << " for writing" << std::endl;
            return;
        }

        // 写入文件头
        f << "// Auto-generated code\n";
        f << "#include <rttr/registration.h>\n";
        for(const std::string& relativePath : relativePaths) {
            f << "#include \"" << relativePath << "\"\n";
        }
        f << "using namespace rttr;\n\n";
        f << "\n\n\n";
        f << "RTTR_REGISTRATION\n";
        f << "{\n";
        f << "  using namespace rttr;\n\n";

        // 生成类注册代码
        for (const auto& info : classInfos) {
            f << "  registration::class_<" << info.path << ">(\"" << info.className << "\")";

            // 处理普通属性
            for (const auto& prop : info.properties) {
                f << "\n      .property(\"" << prop << "\", &" << info.path << "::" << prop << ")";
            }

            // 处理只读属性
            for (const auto& prop : info.readOnlyProperties) {
                f << "\n      .property_readonly(\"" << prop << "\", &" << info.path << "::" << prop << ")";
            }

            f << ";\n\n";
        }

        // 生成枚举注册代码
        for (const auto& info : enumInfos) {
            f << "  registration::enumeration<" << info.path << ">(\"" << info.enumName << "\")";
            f << "\n     (";

            // 处理枚举值
            for (size_t i = 0; i < info.elements.size(); ++i) {
                const auto& elem = info.elements[i];
                if (i == info.elements.size() - 1) {
                    f << "\n            value(\"" << elem << "\", " << info.path << "::" << elem << ")";
                } else {
                    f << "\n            value(\"" << elem << "\", " << info.path << "::" << elem << "),";
                }
            }

            f << "\n     )";
            f << ";\n\n";
        }

        f << "}\n";
        f.close();

        std::cout << "Successfully generated RTTR registration code to " << outputFile << std::endl;
    }

    void GetHeaderFiles(const std::filesystem::path &dir, std::vector<std::string> &files) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".h") {
                files.push_back(entry.path().string());
            }
        }
    }

    std::vector<std::string> splitBySemicolon(const std::string& str) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;
        while (std::getline(ss, token, ';')) {
          if (!token.empty()) { // 过滤空字符串
            tokens.push_back(token);
          }
        }
        return tokens;
    }
}
