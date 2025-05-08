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

#include "CLI11.hpp"
#include "register.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

void GenerateHeaderFiles(std::vector<std::string> searchPaths,
                         std::vector<std::string> &headFiles) {
  for (const auto &searchPath : searchPaths) {
    Register::GetHeaderFiles(searchPath, headFiles);
  }
}
void SetArgs(std::vector<std::string> includePaths,
             std::vector<const char *> &args) {
  static std::vector<std::string> tempStrings;
  tempStrings.clear();
  for (auto &token : includePaths) {
    tempStrings.emplace_back(std::string("-I") + token);
    args.push_back(tempStrings.back().c_str());
  }
}
int main(int argc, char **argv) {
  CLI::App app{"RTTR AUTO REGISTER"};

  std::vector<std::string> searchPaths;
  app.add_option("-s, --search", searchPaths, "Find path list")
      ->required()
      ->check(CLI::ExistingDirectory)
      ->take_all();

  std::string outputFile;
  app.add_option("-o,--output", outputFile, "Output file")->required();

  std::vector<std::string> includePaths;
  app.add_option("-i,--include", includePaths, "Include path list")
      ->check(CLI::ExistingDirectory)
      ->take_all();

  std::vector<std::string> registerMacros;
  app.add_option("-m,--macro", registerMacros, "Register macro list")
      ->take_all();

  CLI11_PARSE(app, argc, argv);

  registerMacros.emplace_back("RTTR_AUTO_REGISTER");
  std::sort(registerMacros.begin(), registerMacros.end());
  auto last = std::unique(registerMacros.begin(), registerMacros.end());
  registerMacros.erase(last, registerMacros.end());

  // 获取输出文件路径
  fs::path output_file = fs::absolute(outputFile);

  std::vector<std::string> headFiles;
  GenerateHeaderFiles(searchPaths, headFiles);

  try {
    // 初始化 Clang
    Register::ClangIndex index;
    if (!index) {
      std::cerr << "Failed to create Clang index\n";
      return 1;
    }

    std::vector<const char *> args = {"-DTGFX_ENABLE_PROFILING", "-x", "c++",
                                      "-std=c++17"};
    SetArgs(includePaths, args);

    std::vector<Register::RTTRMarkClassInfo> class_result;
    std::vector<Register::RTTRMarkEnumInfo> enum_result;
    std::vector<std::string> parse_file_rel_path;
    std::unordered_map<std::string, std::string> def_header_file;

    for (const auto& header_file : headFiles) {
      std::cout << "Process file : " << header_file << std::endl;
      auto tu = Register::GetTranslationUnit(index, header_file, args);

      if (!tu) {
        std::cerr << "Failed to parse translation unit\n";
        return 1;
      }
      // GetClassDefHeaderFileList(*tu, def_header_file);

      auto tokenList = Register::GenerateTokenList(*tu);
      std::vector<Register::RTTRMarkClassInfo> current_class_result;
      std::vector<Register::RTTRMarkEnumInfo> current_enum_result;

      Register::ParseRttrMarkClass(tokenList, registerMacros,
                                   current_class_result, current_enum_result);

      if (!current_class_result.empty() || !current_enum_result.empty()) {
        // 计算相对路径
        fs::path relative_path =
            fs::relative(header_file, output_file.parent_path());
        parse_file_rel_path.push_back(relative_path);
        class_result.insert(class_result.end(), current_class_result.begin(),
                            current_class_result.end());
        enum_result.insert(enum_result.end(), current_enum_result.begin(),
                           current_enum_result.end());
      }
    }

    // 生成代码
    Register::GenerateCPPCode(class_result, enum_result, output_file.string(),
                              parse_file_rel_path);

    // 输出成功信息
    std::cout << "Generated code written to " << output_file << "\n";
    std::cout << "Processing completed.\n";
    std::cout
        << "Please check the generated code for any errors or warnings.\n";
    std::cout << "You can now include this file in your project.\n";
    std::cout << "Thank you for using the RTTR auto-registration tool.\n";

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}