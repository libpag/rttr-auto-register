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
endif ()

# configures the local installation path of the llvm library.
if (NOT CMAKE_PREFIX_PATH)
    if (NOT EXISTS ${PROJECT_SOURCE_DIR}/ClANGCMAKE.cfg)
        file(WRITE ${PROJECT_SOURCE_DIR}/ClANGCMAKE.cfg
                "set(CMAKE_PREFIX_PATH /opt/homebrew/opt/llvm/lib/cmake/clang)  #put your own clang path here")
    endif ()
    include("./ClANGCMAKE.cfg")
endif ()

find_package(Clang REQUIRED CONFIG)
message(STATUS "Found Clang ${CLANG_VERSION}")
message(STATUS "Clang include dir: ${CLANG_INCLUDE_DIRS}")

add_executable(RttrAutoRegister
        src/main.cpp
        src/register.h
        src/register.cpp
        src/clangraii/clangString.h
        src/clangraii/translationUnit.h
        src/clangraii/clangIndex.h
        src/clangraii/clangDiagnostic.h
)

target_include_directories(RttrAutoRegister PRIVATE
    ${CLANG_INCLUDE_DIRS}
)

target_link_libraries(RttrAutoRegister PRIVATE
    libclang
)

if(APPLE)
    find_library(COREFOUNDATION CoreFoundation)
    find_library(SECURITY Security)

    target_link_libraries(RttrAutoRegister PRIVATE
        ${COREFOUNDATION}
        ${SECURITY}
    )
    
    set_target_properties(RttrAutoRegister PROPERTIES
        INSTALL_RPATH "@loader_path/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )

endif()

if(WIN32)
    target_compile_definitions(clang_analyzer PRIVATE
        _CRT_SECURE_NO_WARNINGS
    )
endif()