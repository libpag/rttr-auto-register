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

#include "register.h"
#include "clangraii/clangDiagnostic.h"
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>

namespace Register {

static std::set<std::string> undefinedTypeList;
std::shared_ptr<TranslationUnit>
GetTranslationUnit(ClangIndex &index, const std::string &filepath,
                   const std::vector<const char *> &args) {
  std::shared_ptr<TranslationUnit> tu =
      std::make_shared<TranslationUnit>(index, filepath, args);
  if (!(*tu)) {
    std::cerr << "Failed to parse file: " << filepath << std::endl;
    return nullptr;
  }
  // printDiagnostics(*tu);
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
  if (kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl ||
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

std::optional<CXCursor> FindNextMember(const std::vector<Token> &tokens,
                                       size_t startIndex, CXCursorKind kind) {
  for (size_t i = startIndex + 1; i < tokens.size(); ++i) {
    const Token &token = tokens[i];

    if (token.isCommentOrPunctuation()) {
      continue;
    }

    CXCursor cursor = token.cursor;
    while (!clang_Cursor_isNull(cursor)) {
      if (clang_getCursorKind(cursor) == kind) {
        return cursor;
      }
      cursor = clang_getCursorSemanticParent(cursor);
    }
  }
  return std::nullopt;
}

std::vector<std::string> ProcessProperty(CXCursor cursor,
                                         const std::string &property_macro,
                                         CXCursorKind kind) {
  std::vector<std::string> result;

  // 获取token范围
  CXToken *tokens = nullptr;
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

    token_list.push_back(
        {clang_getTokenKind(token), ClangString(spelling).str(), token_cursor});
  }
  clang_disposeTokens(tu, tokens, num_tokens);

  // 处理token
  for (size_t i = 0; i < token_list.size(); ++i) {
    const Token &token = token_list[i];

    // 查找属性宏
    if (token.isIdentifier(property_macro) && i + 1 < token_list.size()) {
      // 查找下一个成员变量
      auto member_cursor = FindNextMember(token_list, i, kind);

      if (member_cursor) {
        // 获取成员名称
        CXString member_name = clang_getCursorSpelling(*member_cursor);
        result.push_back(ClangString(member_name).str());
      }
    }
  }

  return result;
}

bool isUnCopiedType(CXType type) {
  ClangString typeStr(clang_getTypeSpelling(type));
  std::string str = typeStr.str();
  // std::cout << "type: " << str << std::endl;

  // 检查类型名中是否包含"std::unique_ptr"或"unique_ptr"
  bool isUniquePtr = str.find("unique") != std::string::npos ||
                     str.find("Unique") != std::string::npos;
  bool isMutex = str.find("mutex") != std::string::npos;
  bool isAtomic = str.find("atomic") != std::string::npos;
  bool isUnNamed = str.find("unname") != std::string::npos;
  return isUniquePtr || isMutex || isAtomic || isUnNamed;
}

CXChildVisitResult visitor(CXCursor cursor, CXCursor parent,
                           CXClientData client_data) {
  if (clang_getCursorKind(cursor) == CXCursor_StructDecl) {
    ClangString cursorName(clang_getCursorSpelling(cursor));

    // 检查是否有pag_api宏
    CXSourceRange range = clang_getCursorExtent(cursor);
    CXToken *tokens;
    unsigned numTokens;
    clang_tokenize(clang_Cursor_getTranslationUnit(cursor), range, &tokens,
                   &numTokens);

    bool hasPagApi = false;
    for (unsigned i = 0; i < numTokens; ++i) {
      if (clang_getTokenKind(tokens[i]) == CXToken_Identifier) {
        ClangString tokenText(clang_getTokenSpelling(
            clang_Cursor_getTranslationUnit(cursor), tokens[i]));
        if (tokenText.str() == "pag_api") {
          hasPagApi = true;
          break;
        }
      }
    }
    clang_disposeTokens(clang_Cursor_getTranslationUnit(cursor), tokens,
                        numTokens);

    if (hasPagApi) {
      std::cout << "Found pag_api marked struct: " << cursorName.str()
                << std::endl;
      std::string qualified_name = GetFullQualifiedName(cursor);
      std::cout << "qualified_name: " << qualified_name << std::endl;

      // 遍历成员
      clang_visitChildren(
          cursor,
          [](CXCursor c, CXCursor parent, CXClientData data) {
            if (clang_getCursorKind(c) == CXCursor_FieldDecl) {
              CX_CXXAccessSpecifier access = clang_getCXXAccessSpecifier(c);
              if (access == CX_CXXPrivate) {
                CXType memberType = clang_getCursorType(c);
                if (!isUnCopiedType(memberType)) {
                  ClangString memberName(clang_getCursorSpelling(c));
                  ClangString typeName(clang_getTypeSpelling(memberType));
                  std::cout << "  Member: " << memberName.str()
                            << " (type: " << typeName.str() << ")" << std::endl;
                }
              }
            }
            return CXChildVisit_Continue;
          },
          nullptr);
    }
  }
  return CXChildVisit_Continue;
}

std::vector<Token> GenerateTokenList(TranslationUnit &tu) {
  // 获取翻译单元的游标
  CXCursor tu_cursor = clang_getTranslationUnitCursor(tu);

  // 获取token范围
  CXToken *tokens = nullptr;
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

    token_list.push_back(
        {clang_getTokenKind(token), ClangString(spelling).str(), token_cursor});
  }
  clang_disposeTokens(tu, tokens, num_tokens);
  return token_list;
}

void ParseRttrMarkClass(const std::vector<Token> &token_list,
                        const std::vector<std::string> &registerMacros,
                        std::vector<RTTRMarkClassInfo> &classInfos,
                        std::vector<RTTRMarkEnumInfo> &enumInfos) {
  // 处理token
  for (size_t i = 0; i < token_list.size(); ++i) {
    const Token &token = token_list[i];
    bool identifierMatch = false;
    for (const auto &macro : registerMacros) {
      if (token.spelling == macro) {
        identifierMatch = true;
        break;
      }
    }
    if (identifierMatch && (i + 1 < token_list.size())) {
      CXCursor cursor = token_list[i + 1].cursor;
      CXCursorKind kind = clang_getCursorKind(cursor);
      if (kind == CXCursor_StructDecl || kind == CXCursor_ClassDecl) {
        //获取类名
        ClangString name(clang_getCursorSpelling(cursor));
        // 获取完整限定名
        std::string qualified_name = GetFullQualifiedName(cursor);

        struct Elements {
          std::vector<std::string> properties;
          std::vector<std::string> methods;
        } elements;

        // 遍历成员
        clang_visitChildren(
            cursor,
            [](CXCursor c, CXCursor parent, CXClientData data) {
              auto elements = static_cast<Elements *>(data);
              if (clang_getCursorKind(c) == CXCursor_FieldDecl) {
                CXType memberType = clang_getCursorType(c);
                if (!isUnCopiedType(memberType)) {
                  ClangString memberName(clang_getCursorSpelling(c));
                  elements->properties.push_back(memberName);
                }
              }
              if (clang_getCursorKind(c) == CXCursor_CXXMethod) {
                ClangString name(clang_getCursorSpelling(c));
                std::string methName = name.str();
                // std::cout << "method: " << methName << std::endl;
                if (methName.find("RTTRAUTOMARK") != std::string::npos) {
                  elements->methods.push_back(methName);
                }
              }
              return CXChildVisit_Continue;
            },
            &elements);

        classInfos.push_back({name, qualified_name,
                              std::move(elements.properties),
                              std::move(elements.methods)});
      } else if (kind == CXCursor_EnumDecl) {
        // 获取枚举名
        ClangString name(clang_getCursorSpelling(cursor));
        // 获取完整限定名
        std::string qualified_name = GetFullQualifiedName(cursor);
        // 收集枚举常量
        std::vector<std::string> elements;
        // 遍历枚举的子节点
        clang_visitChildren(
            cursor,
            [](CXCursor c, CXCursor parent, CXClientData client_data) {
              if (clang_getCursorKind(c) == CXCursor_EnumConstantDecl) {
                auto elements =
                    static_cast<std::vector<std::string> *>(client_data);
                ClangString spelling(clang_getCursorSpelling(c));
                elements->push_back(spelling);
              }
              return CXChildVisit_Continue;
            },
            &elements);

        enumInfos.push_back({name, qualified_name, std::move(elements)});
      }
    }
  }
}

std::string removeRttrSuffix(std::string str) {
  const std::string suffix = "RTTRAUTOMARK";
  if (str.size() >= suffix.size() &&
      str.substr(str.size() - suffix.size()) == suffix) {
    return str.substr(0, str.size() - suffix.size());
  }
  return str;
}

void GenerateCPPCode(const std::vector<RTTRMarkClassInfo> &classInfos,
                     const std::vector<RTTRMarkEnumInfo> &enumInfos,
                     const std::string &outputFile,
                     const std::vector<std::string> &relativePaths) {
  std::ofstream f(outputFile);
  if (!f.is_open()) {
    std::cerr << "Error: Could not open file " << outputFile << " for writing"
              << std::endl;
    return;
  }

  // 写入文件头
  f << "// Auto-generated code\n";
  f << "#pragma once\n";
  f << "#include <rttr/registration.h>\n";
  f << "#include <iostream>\n";
  for (const std::string &relativePath : relativePaths) {
    f << "#include \"" << relativePath << "\"\n";
  }
  f << "using namespace rttr;\n\n";
  f << "\n\n\n";
  f << "RTTR_REGISTRATION\n";
  f << "{\n";
  f << "  using namespace rttr;\n\n";
  f << "  std::cout << \"Rttr registed!\" << std::endl;\n\n";

  // 生成类注册代码
  for (const auto &info : classInfos) {
    f << "  registration::class_<" << info.path << ">(\"" << info.className
      << "\")";

    // 处理普通属性
    for (const auto &prop : info.properties) {
      f << "\n      .property_readonly(\"" << prop << "\", &" << info.path
        << "::" << prop << ")";
    }

    // 处理方法
    for (const auto &method : info.methods) {
      f << "\n      .method(\"" << removeRttrSuffix(method) << "\", &"
        << info.path << "::" << method << ")";
    }

    f << ";\n\n";
  }

  // 生成枚举注册代码
  for (const auto &info : enumInfos) {
    f << "  registration::enumeration<" << info.path << ">(\"" << info.enumName
      << "\")";
    f << "\n     (";

    // 处理枚举值
    for (size_t i = 0; i < info.elements.size(); ++i) {
      const auto &elem = info.elements[i];
      if (i == info.elements.size() - 1) {
        f << "\n            value(\"" << elem << "\", " << info.path
          << "::" << elem << ")";
      } else {
        f << "\n            value(\"" << elem << "\", " << info.path
          << "::" << elem << "),";
      }
    }

    f << "\n     )";
    f << ";\n\n";
  }

  f << "}\n";
  f.close();
}

void GetHeaderFiles(const std::filesystem::path &dir,
                    std::vector<std::string> &files) {
  for (const auto &entry : std::filesystem::recursive_directory_iterator(dir)) {
    if (entry.is_regular_file() && entry.path().extension() == ".h") {
      files.push_back(entry.path().string());
    }
  }
}

std::vector<std::string> splitBySemicolon(const std::string &str) {
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
void GetForwardDecl(
    TranslationUnit &tu,
    std::unordered_map<std::string, Register::ForwardDeclInfo> &map) {
  if (!tu) {
    return;
  }

  // 获取翻译单元的游标
  CXCursor tu_cursor = clang_getTranslationUnitCursor(tu);
  clang_visitChildren(
      tu_cursor,
      [](CXCursor cursor, CXCursor parent, CXClientData client_data) {
        auto map = static_cast<
            std::unordered_map<std::string, Register::ForwardDeclInfo> *>(
            client_data);
        CXCursorKind kind = clang_getCursorKind(cursor);
        // 判断是否为类或结构体的前向声明
        if (kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl) {
          // 检查定义是否为空（判断是否为前向声明）
          if (!clang_isCursorDefinition(cursor)) {
            ClangString cursorName(clang_getCursorSpelling(cursor));
            if (map->find(cursorName.str()) == map->end()) {
              map->insert({cursorName.str(), {}});
            }
            std::cout << "forward class: " << cursorName.str() << std::endl;
          }
        }
        return CXChildVisit_Recurse;
      },
      &map);
}

void GetClassDefHeaderFileList(
    TranslationUnit &tu,
    std::unordered_map<std::string, std::string> &DefList) {
  if (!tu) {
    return;
  }
  // 获取翻译单元的游标
  CXCursor tu_cursor = clang_getTranslationUnitCursor(tu);
  clang_visitChildren(
      tu_cursor,
      [](CXCursor cursor, CXCursor parent, CXClientData client_data) {
        auto DefList =
            static_cast<std::unordered_map<std::string, std::string> *>(
                client_data);
        CXCursorKind kind = clang_getCursorKind(cursor);
        // 判断是否为类或结构体的前向声明
        if (kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl) {
          // 检查定义是否为空（判断是否为前向声明）
          if (clang_isCursorDefinition(cursor)) {
            std::string qualified_name = GetFullQualifiedName(cursor);
            if (DefList->find(qualified_name) == DefList->end()) {
              CXFile file;
              unsigned line, column, offset;
              CXSourceLocation location = clang_getCursorLocation(cursor);
              clang_getFileLocation(location, &file, &line, &column, &offset);
              ClangString filepath(clang_getFileName(file));
              DefList->at(qualified_name) = filepath.str();
            }
          }
        }
        return CXChildVisit_Recurse;
      },
      &DefList);
}

CXType getUnderlyingType(CXType type) {
  if (clang_Type_getNumTemplateArguments(type) > 0) {
    // 常见容器和智能指针检查
    CXString typeName = clang_getTypeSpelling(type);
    const char *name = clang_getCString(typeName);

    // 检查是否是标准库容器或智能指针
    if (strstr(name, "std::vector") || strstr(name, "std::list") ||
        strstr(name, "std::shared_ptr") || strstr(name, "std::unique_ptr") ||
        strstr(name, "std::weak_ptr") || strstr(name, "std::optional")) {

      // 获取第一个模板参数
      CXType templateArg = clang_Type_getTemplateArgumentAsType(type, 0);
      if (templateArg.kind != CXType_Invalid) {
        clang_disposeString(typeName);
        return getUnderlyingType(templateArg); // 递归处理嵌套情况
      }
    }
    clang_disposeString(typeName);
  }

  if (type.kind == CXType_Pointer || type.kind == CXType_LValueReference ||
      type.kind == CXType_RValueReference) {
    CXType pointee = clang_getPointeeType(type);
    if (pointee.kind != CXType_Invalid) {
      return getUnderlyingType(pointee); // 递归处理
    }
  }
  return clang_Type_getNamedType(type);
}
bool typeIsDefined(CXType type) {
  if (type.kind == CXType_Record) {
    auto del_cursor = clang_getTypeDeclaration(type);
    auto def_cursor = clang_getCursorDefinition(del_cursor);
    if (clang_Cursor_isNull(def_cursor)) {
      return false;
    }
  }
  return true;
}
void printDiagnostics(CXTranslationUnit translationUnit) {
  int nbDiag = clang_getNumDiagnostics(translationUnit);
  printf("There are %i diagnostics:\n", nbDiag);

  bool foundError = false;
  for (unsigned int currentDiag = 0; currentDiag < nbDiag; ++currentDiag) {
    CXDiagnostic diagnotic = clang_getDiagnostic(translationUnit, currentDiag);
    CXString errorString = clang_formatDiagnostic(
        diagnotic, clang_defaultDiagnosticDisplayOptions());
    std::string tmp{clang_getCString(errorString)};
    clang_disposeString(errorString);
    if (tmp.find("error:") != std::string::npos) {
      foundError = true;
    }
    std::cerr << tmp << std::endl;
  }
  if (foundError) {
    std::cerr << "Please resolve these issues and try again." << std::endl;
    exit(-1);
  }
}
} // namespace Register
