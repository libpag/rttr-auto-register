#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include "register.h"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    std::cout << "argc: " << argc << std::endl;
    //检查命令行参数
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <header_file_dir> <output_file> [include_dirs]\n";
        std::cerr << "Example: " << argv[0] << " /path/to /path/to/output.cpp /path/include /path1/include\n";
        return 1;
    }

    // 获取并验证输入文件
    fs::path header_file_dir = fs::absolute(argv[1]);
    if (!fs::exists(header_file_dir)) {
        std::cerr << "Error: Header file dir '" << header_file_dir << "' does not exist\n";
        return 1;
    }

    // 获取输出文件路径
    fs::path output_file = fs::absolute(argv[2]);

    std::vector<std::string> tokens;
    for(int i = 3; i < argc; i++) {
      tokens.push_back(argv[i]);
    }



    std::vector<std::string> headFiles;
    Register::GetHeaderFiles(header_file_dir, headFiles);

    try {
        // 初始化 Clang
        Register::ClangIndex index;
        if (!index) {
            std::cerr << "Failed to create Clang index\n";
            return 1;
        }

        // 解析文件
        std::vector<const char*> args = {
            "-x",
            "c++",
            "-std=c++17"
        };

        for(auto& token : tokens) {
          token = (std::string("-I") + token);
          std::cout << "Include instruction: " << std::endl;
          std::cout << token << std::endl;
          args.push_back(token.c_str());
        }

        std::vector<Register::RTTRMarkClassInfo> class_result;
        std::vector<Register::RTTRMarkEnumInfo> enum_result;
        std::vector<std::string> parse_file_rel_path;

        for(const std::string& header_file : headFiles) {
            auto tu = Register::GetTranslationUnit(index, header_file, args);

            if (!tu) {
                std::cerr << "Failed to parse translation unit\n";
                return 1;
            }

            // 解析类和枚举
            auto current_class_result = ParseRttrMarkClass(*tu, "REGISTER_CLASS",
                "REGISTER_PROPERTY", "REGISTER_PROPERTY_READONLY");
            auto current_enum_result = ParseRttrMarkEnum(*tu, "REGISTER_ENUM");

            if(!current_class_result.empty() || !current_enum_result.empty()) {
                // 计算相对路径
                fs::path relative_path = fs::relative(header_file, output_file.parent_path());
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
        std::cout << "Please check the generated code for any errors or warnings.\n";
        std::cout << "You can now include this file in your project.\n";
        std::cout << "Thank you for using the RTTR auto-registration tool.\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}