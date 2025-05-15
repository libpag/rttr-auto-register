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

cmake_minimum_required(VERSION 3.13)
project(RttrAutoRegister)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif ()

if (MSVC)
    add_compile_options("/utf-8")
endif (MSVC)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DDEBUG)
else()
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-ffunction-sections -fdata-sections)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-dead_strip")
    endif()
endif ()

set(RTTR_INCLUDES vendor/libclang/include)
set(RTTR_LIBRARIES "")

if (NOT CMAKE_PREFIX_PATH)
    # set your own llvm install path
    set(CMAKE_PREFIX_PATH "")
endif()

set(RTTR_SOURCE_FILES 
    src/main.cpp
    src/register.h
    src/register.cpp
    src/CLI11.hpp
    src/clangraii/clangString.h
    src/clangraii/translationUnit.h
    src/clangraii/clangIndex.h
    src/clangraii/clangDiagnostic.h
)

list(APPEND RTTR_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libclang/include")

add_executable(RttrAutoRegister ${RTTR_SOURCE_FILES})

if(APPLE)
    find_library(COREFOUNDATION CoreFoundation)
    list(APPEND RTTR_LIBRARIES "${COREFOUNDATION}")
    find_library(SECURITY Security)
    list(APPEND RTTR_LIBRARIES "${SECURITY}")
    find_library(ZLIB z)
    list(APPEND RTTR_LIBRARIES "${ZLIB}")
    execute_process(COMMAND bash -c "unzip ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libclang/mac/lib/libclang.a.zip -d ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libclang/mac/lib")
    file(GLOB CLANG_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libclang/mac/lib/libclang.a")
    list(APPEND RTTR_LIBRARIES ${CLANG_LIBRARIES})

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND strip "$<TARGET_FILE:${PROJECT_NAME}>"
    COMMENT "Strip debug symbols done on final binary.")
elseif (WIN32)
    file(GLOB CLANG_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libclang/win/lib/libclang.lib")
    list(APPEND RTTR_LIBRARIES ${CLANG_LIBRARIES})
    add_custom_command(TARGET RttrAutoRegister POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libclang/win/lib/libclang.dll
            $<TARGET_FILE_DIR:RttrAutoRegister>/libclang.dll
            COMMENT "正在复制文件..."
            VERBATIM
    )
endif()

target_include_directories(RttrAutoRegister PRIVATE ${RTTR_INCLUDES})
target_link_libraries(RttrAutoRegister PRIVATE ${RTTR_LIBRARIES})