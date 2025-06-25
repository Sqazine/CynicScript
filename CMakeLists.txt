cmake_minimum_required(VERSION 3.10)

project(CynicScript VERSION 0.1.0 LANGUAGES C CXX)

configure_file(Version.h.in generated/Version.h @ONLY)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CYS_DEBUG_POSTFIX "_d" CACHE STRING "library or executable debug version postfix")
set(CYS_RELEASE_POSTFIX "" CACHE STRING "library or executable release version postfix")
option(CYS_STATIC_BUILD "build CynicScript library as static library" OFF)
option(CYS_BUILD_EXECUTABLE "build CynicScript executable file" ON)
option(CYS_UTF8_ENCODE "use utf8 encode" ON)

set(CMAKE_DEBUG_POSTFIX ${CYS_DEBUG_POSTFIX}) 
set(CMAKE_RELEASE_POSTFIX ${CYS_RELEASE_POSTFIX})

set(CYS_BINARY_DIR ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CYS_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CYS_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CYS_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CYS_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CYS_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CYS_BINARY_DIR})

if(MSVC OR ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    set(LIB_NAME libCynicScript)
else()
    set(LIB_NAME CynicScript)
endif()

file(GLOB CYS_HEADER_FILES "${GENERATED_DIR}/*.h" "*.h")
file(GLOB CYS_SOURCE_FILES "${GENERATED_DIR}/*.cpp" "*.cpp")

source_group("src" FILES ${CYS_HEADER_FILES} ${CYS_SOURCE_FILES})
if(CYS_STATIC_BUILD)
    add_library(${LIB_NAME} STATIC ${CYS_HEADER_FILES} ${CYS_SOURCE_FILES})
    target_compile_definitions(${LIB_NAME} PUBLIC CYS_BUILD_STATIC)
else()
    add_library(${LIB_NAME} SHARED ${CYS_HEADER_FILES} ${CYS_SOURCE_FILES})
    target_compile_definitions(${LIB_NAME} PUBLIC CYS_BUILD_DLL)
endif()
target_include_directories(${LIB_NAME} PRIVATE ${GENERATED_DIR})

if(CYS_BUILD_EXECUTABLE)
    set(EXE_NAME CynicScript)

    file(GLOB EXAMPLES "${CMAKE_SOURCE_DIR}/examples/*.cys")

    source_group("examples" FILES ${EXAMPLES})
    add_executable(${EXE_NAME} CynicScript.cc ${EXAMPLES})
    set_target_properties(${EXE_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    target_include_directories(${EXE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/libCynicScript)
    target_include_directories(${EXE_NAME} PRIVATE ${GENERATED_DIR})
    target_link_libraries(${EXE_NAME} PRIVATE ${LIB_NAME})
endif()

if(CYS_UTF8_ENCODE)
    target_compile_definitions(${LIB_NAME} PUBLIC CYS_UTF8_ENCODE)
    if(CYS_BUILD_EXECUTABLE)
        target_compile_definitions(${EXE_NAME} PUBLIC CYS_UTF8_ENCODE)
    endif()
endif()

if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    target_compile_definitions(${LIB_NAME} PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
    if(CYS_BUILD_EXECUTABLE)
        target_compile_definitions(${EXE_NAME} PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
    endif()

    if(MSVC)
        target_compile_options(${LIB_NAME} PRIVATE "/wd4251;" "/bigobj;" "/MD;")
        if(CYS_BUILD_EXECUTABLE)
            target_compile_options(${EXE_NAME} PRIVATE "/wd4251;" "/bigobj;" "/MD;")
            set_property ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_NAME})
        endif()
    endif()
else()
    target_compile_options(${LIB_NAME} PRIVATE "-fpermissive")
    if(CYS_BUILD_EXECUTABLE)
        target_compile_options(${EXE_NAME} PRIVATE "-fpermissive")
    endif()
endif()

file(COPY ${CYS_HEADER_FILES} DESTINATION ${CYS_BINARY_DIR}/inc)