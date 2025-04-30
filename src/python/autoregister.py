#  Copyright (C) 2021 THL A29 Limited, a Tencent company. All rights reserved.
#
#  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
#  except in compliance with the License. You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  unless required by applicable law or agreed to in writing, software distributed under the
#  license is distributed on an "as is" basis, without warranties or conditions of any kind,
#  either express or implied. see the license for the specific language governing permissions
#  and limitations under the license.

import clang.cindex
from clang.cindex import CursorKind, TokenKind
import sys
import os

from dataclasses import dataclass
from typing import List

@dataclass
class RTTRMARKClassInfo:
    className : str
    path: str
    properties: list[str]
    readOnlyProperties: list[str]

@dataclass
class RTTRMARKEnumInfo:
    enumName: str
    path: str
    elements: list[str]

def initClang(header_path, library_path):
    try:
        clang.cindex.Config.set_library_path(library_path)
    except Exception as e:
        print(f"Error setting Clang library path: {e}")
        return None
    
    args = [
        '-x', 'c++',
        '-std=c++11',
        '-I/usr/local/include',
        '-I/Library/Developer/CommandLineTools/usr/include/c++/v1',
        '-I/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include'
    ]
    index = clang.cindex.Index.create()

    try:
        tu = index.parse(header_path, args=args)
    except Exception as e:
        print(f"Error parsing file {header_path}: {e}")
        return None

    if not tu:
        print(f"Failed to parse file: {header_path}")
        return None
    
    return tu


def get_full_qualified_name(cursor):
    """递归获取完整的限定名，包括外层类和命名空间"""
    parts = []
    
    # 首先处理当前cursor
    if cursor.kind == CursorKind.CLASS_DECL or cursor.kind == CursorKind.STRUCT_DECL or cursor.kind == CursorKind.ENUM_DECL:
        parts.append(cursor.spelling)
    
    # 然后处理所有父级  
    parent = cursor.semantic_parent
    while parent is not None:
        if parent.kind == CursorKind.NAMESPACE:
            parts.insert(0, parent.spelling)
        elif parent.kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL):
            parts.insert(0, parent.spelling)
        parent = parent.semantic_parent
    
    return "::".join(parts) if parts else ""

def find_next_member(tokens, start_index):
    for i in range(start_index + 1, len(tokens)):
        token = tokens[i]
        if token.kind in (TokenKind.COMMENT, TokenKind.PUNCTUATION):
            continue
        cursor = token.cursor
        while cursor:
            if cursor.kind == CursorKind.FIELD_DECL:
                return cursor
            cursor = cursor.semantic_parent
    return None

def process_property(cursor, property_macro):

    result = []
    
    # 收集类内的所有token
    tokens = list(cursor.get_tokens())
    
    # 查找宏标记和对应的成员变量
    for i, token in enumerate(tokens):
        # 查找标记宏
        if (token.kind == TokenKind.IDENTIFIER and 
            token.spelling == property_macro and
            i + 1 < len(tokens)):
            
            # 获取下一个token（应该是成员变量）
            nextcursor = find_next_member(tokens, i + 1)

            if nextcursor is not None:
                # 获取完整的类名路径
                member_name = nextcursor.spelling
                result.append(member_name)
    return result

def parseRttrMarkClass(tu, mark_macro, property_macro, readonly_property_macro) -> List[RTTRMARKClassInfo]:
    
    result = []
    if not tu:
        return result
    tokens = list(tu.cursor.get_tokens())
    for i, token in enumerate(tokens):
        try:
            if token.kind == TokenKind.IDENTIFIER and token.spelling == mark_macro:
                cursor = token.cursor
                
                while cursor is not None:
                    if cursor.kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL):
                        name = cursor.spelling
                        qualified_name = get_full_qualified_name(cursor)
                        propertys = process_property(cursor, property_macro)
                        readOnlyPropertys = process_property(cursor, readonly_property_macro)
                        result.append(RTTRMARKClassInfo(className=name, path=qualified_name, properties= propertys, readOnlyProperties=readOnlyPropertys))
                        break
                    
                    cursor = cursor.semantic_parent
                    
        except Exception as e:
            print(f"Error processing token: {e}")
            continue
    
    return result

def parseRttrMarkEnum(tu, mark_macro):
    result = []
    tokens = list(tu.cursor.get_tokens())
    for i, token in enumerate(tokens):
        try:
            if token.kind == TokenKind.IDENTIFIER and token.spelling == mark_macro:
                cursor = token.cursor
                
                while cursor is not None:
                    if cursor.kind == CursorKind.ENUM_DECL:
                        name = cursor.spelling
                        qualified_name = get_full_qualified_name(cursor)
                        elements = [e.spelling for e in cursor.get_children() if e.kind == CursorKind.ENUM_CONSTANT_DECL]
                        result.append(RTTRMARKEnumInfo(enumName=name, path=qualified_name, elements=elements))
                        break
                    
                    cursor = cursor.semantic_parent
                    
        except Exception as e:
            print(f"Error processing token: {e}")
            continue
    return result

def generate_cpp_code(class_infos, enum_infos, output_file, relative_path):
    with open(output_file, 'w') as f:
        f.write("// Auto-generated code\n")
        f.write("#include <rttr/registration>\n")
        f.write(f"#include \"{relative_path}\"\n")
        f.write("using namespace rttr;\n\n")
        f.write("\n\n\n")
        f.write("RTTR_REGISTRATION\n")
        f.write("{\n")
        f.write("  using namespace rttr;\n\n")
        for info in class_infos:
            f.write(f"  registration::class_<{info.path}>(\"{info.className}\")")
            for prop in info.properties:
                f.write(f"\n      .property(\"{prop}\", &{info.path}::{prop})")
            for prop in info.readOnlyProperties:
                f.write(f"\n      .property_readonly(\"{prop}\", &{info.path}::{prop})")
            f.write(";\n\n")
        for info in enum_infos:
            f.write(f"  registration::enumeration<{info.path}>(\"{info.enumName}\")")
            f.write("\n     (")
            for index, elem in enumerate(info.elements):
                if index == len(info.elements) - 1:
                    f.write(f"\n            value(\"{elem}\", {info.path}::{elem})")
                else:
                    f.write(f"\n            value(\"{elem}\", {info.path}::{elem}),")
            f.write("\n     )")
            f.write(";\n\n")
        f.write("}\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python find_nested_classes.py <header_file> <output_file>")
        print("Example: python find_nested_classes.py /path/to/header.h /path/to/output.cpp")
        sys.exit(1)
    
    header_file = os.path.expanduser(sys.argv[1])
    output_file = os.path.expanduser(sys.argv[2])
    
    if not os.path.exists(header_file):
        print(f"Error: File '{header_file}' does not exist")
        sys.exit(1)

    tu = initClang(header_file, '/Library/Developer/CommandLineTools/usr/lib')
    classresult = parseRttrMarkClass(tu, 'REGISTER_CLASS', 'REGISTER_PROPERTY', 'REGISTER_READONLY_PROPERTY')
    enumresult = parseRttrMarkEnum(tu, 'REGISTER_ENUM')
    relative_path = os.path.relpath(header_file, os.path.dirname(output_file))
    generate_cpp_code(classresult, enumresult, output_file, relative_path)
    print(f"Generated code written to {output_file}")
    print("Processing completed.")
    print("Please check the generated code for any errors or warnings.")
    print("You can now include this file in your project.")
    print("Thank you for using the RTTR auto-registration tool.")

    