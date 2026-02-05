## Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

cmake_minimum_required(VERSION 3.14)

# 1. Thiết lập Project và Kiểu Build
project(my_standalone_plugin)
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Debug Release RelWithDebInfo MinSizeRel")

# 2. Cấu hình C++ Standard theo chuẩn NX
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#find_package(pugixml REQUIRED)

# 3. Định nghĩa Export Macro cho Linux/Windows
if(WIN32)
    set(API_EXPORT_MACRO "__declspec(dllexport)")
else()
    set(API_EXPORT_MACRO "__attribute__((visibility(\"default\")))")
    # Thiết lập rpath để ưu tiên tìm thư viện trong cùng folder plugin
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--disable-new-dtags")
endif()

# 4. TRỎ VỀ SDK (Đây là phần quan trọng nhất khi tách ra ngoài)
# Bạn có thể truyền đường dẫn này qua command line: -DnxSdkDir=/path/to/sdk
set(nxSdkDir "" CACHE PATH "Path to the unzipped Server Plugin SDK.")

if(nxSdkDir STREQUAL "")
    # Giả sử bạn để thư mục sdk cùng cấp với folder plugin hiện tại
    set(nxSdkDir "${CMAKE_CURRENT_LIST_DIR}/../server_plugin_sdk")
endif()

if(NOT EXISTS "${nxSdkDir}/nx_kit/CMakeLists.txt")
    message(FATAL_ERROR "Could not find SDK at: ${nxSdkDir}. Please set -DnxSdkDir.")
endif()

# 5. Build nx_kit (Thư viện tiện ích của NX)
set(nxKitLibraryType "STATIC" CACHE STRING "" FORCE)
add_subdirectory("${nxSdkDir}/nx_kit" "${CMAKE_CURRENT_BINARY_DIR}/nx_kit")

# 6. Build nx_sdk (Thư viện Interface)
set(SDK_SRC_DIR "${nxSdkDir}/src")
file(GLOB_RECURSE SDK_SRC CONFIGURE_DEPENDS "${SDK_SRC_DIR}/*")

add_library(nx_sdk STATIC ${SDK_SRC} "src/nx/vms_server_plugins/analytics/sample/common.cpp" "src/nx/vms_server_plugins/analytics/sample/common.h" "src/nx/vms_server_plugins/analytics/sample/device_agent.cpp" "src/nx/vms_server_plugins/analytics/sample/device_agent.h" "src/nx/vms_server_plugins/analytics/sample/engine.cpp" "src/nx/vms_server_plugins/analytics/sample/engine.h" "src/nx/vms_server_plugins/analytics/sample/face_search_plugin_ini.cpp" "src/nx/vms_server_plugins/analytics/sample/face_search_plugin_ini.h" "src/nx/vms_server_plugins/analytics/sample/httplib.h" "src/nx/vms_server_plugins/analytics/sample/integration.cpp" "src/nx/vms_server_plugins/analytics/sample/integration.h" "src/nx/vms_server_plugins/analytics/sample/manifest_device_agent_EN.h" "src/nx/vms_server_plugins/analytics/sample/manifest_device_agent_VI.h" "src/nx/vms_server_plugins/analytics/sample/manifest_integration_EN.h" "src/nx/vms_server_plugins/analytics/sample/manifest_integration_VI.h" "src/nx/vms_server_plugins/analytics/sample/pugixml.cpp")
target_include_directories(nx_sdk PUBLIC "${SDK_SRC_DIR}")
target_link_libraries(nx_sdk PUBLIC nx_kit)
target_compile_definitions(nx_sdk PUBLIC NX_PLUGIN_API=${API_EXPORT_MACRO})

# 7. BUILD PLUGIN CỦA BẠN (Dựa trên cấu trúc sample_analytics_plugin)

# Gom tất cả file trong src (engine, device_agent, v.v.)
set(plugin_src_dir "${CMAKE_CURRENT_LIST_DIR}/src")
file(GLOB_RECURSE plugin_src_files CONFIGURE_DEPENDS "${plugin_src_dir}/*")

# Tạo thư viện tĩnh Integration (Nơi chứa logic AI/Database sau này)
add_library(my_plugin_integration STATIC ${plugin_src_files} "src/nx/vms_server_plugins/analytics/sample/common.cpp" "src/nx/vms_server_plugins/analytics/sample/common.h" "src/nx/vms_server_plugins/analytics/sample/device_agent.cpp" "src/nx/vms_server_plugins/analytics/sample/device_agent.h" "src/nx/vms_server_plugins/analytics/sample/engine.cpp" "src/nx/vms_server_plugins/analytics/sample/engine.h" "src/nx/vms_server_plugins/analytics/sample/face_search_plugin_ini.cpp" "src/nx/vms_server_plugins/analytics/sample/face_search_plugin_ini.h" "src/nx/vms_server_plugins/analytics/sample/httplib.h" "src/nx/vms_server_plugins/analytics/sample/integration.cpp" "src/nx/vms_server_plugins/analytics/sample/integration.h" "src/nx/vms_server_plugins/analytics/sample/manifest_device_agent_EN.h" "src/nx/vms_server_plugins/analytics/sample/manifest_device_agent_VI.h" "src/nx/vms_server_plugins/analytics/sample/manifest_integration_EN.h" "src/nx/vms_server_plugins/analytics/sample/manifest_integration_VI.h" "src/nx/vms_server_plugins/analytics/sample/pugixml.cpp")
target_include_directories(my_plugin_integration PUBLIC "${plugin_src_dir}")
target_link_libraries(my_plugin_integration PUBLIC nx_sdk)

# Tạo file .so cuối cùng (Chỉ chứa plugin.cpp và link với Integration)
add_library(my_plugin SHARED "plugin/plugin.cpp" "src/nx/vms_server_plugins/analytics/sample/common.cpp" "src/nx/vms_server_plugins/analytics/sample/common.h" "src/nx/vms_server_plugins/analytics/sample/device_agent.cpp" "src/nx/vms_server_plugins/analytics/sample/device_agent.h" "src/nx/vms_server_plugins/analytics/sample/engine.cpp" "src/nx/vms_server_plugins/analytics/sample/engine.h" "src/nx/vms_server_plugins/analytics/sample/face_search_plugin_ini.cpp" "src/nx/vms_server_plugins/analytics/sample/face_search_plugin_ini.h" "src/nx/vms_server_plugins/analytics/sample/httplib.h" "src/nx/vms_server_plugins/analytics/sample/integration.cpp" "src/nx/vms_server_plugins/analytics/sample/integration.h" "src/nx/vms_server_plugins/analytics/sample/manifest_device_agent_EN.h" "src/nx/vms_server_plugins/analytics/sample/manifest_device_agent_VI.h" "src/nx/vms_server_plugins/analytics/sample/manifest_integration_EN.h" "src/nx/vms_server_plugins/analytics/sample/manifest_integration_VI.h" "src/nx/vms_server_plugins/analytics/sample/pugixml.cpp")
target_link_libraries(my_plugin PRIVATE my_plugin_integration)

# Đặt tên file đầu ra cho giống chuẩn (ví dụ: my_plugin.so)
set_target_properties(my_plugin PROPERTIES PREFIX "" OUTPUT_NAME "lightjsc_face_search_plugin")