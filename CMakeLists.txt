cmake_minimum_required(VERSION 3.16)
project(watchy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
  list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(PUGIXML REQUIRED IMPORTED_TARGET pugixml)
pkg_check_modules(CURL REQUIRED IMPORTED_TARGET libcurl)

find_path(RAPIDJSON_INCLUDE_DIR rapidjson/document.h)
if(NOT RAPIDJSON_INCLUDE_DIR)
  message(FATAL_ERROR "rapidjson headers not found (macOS: brew install rapidjson; Ubuntu/Debian: apt install rapidjson-dev)")
endif()

add_library(bdf_font_lib
  libs/bdf-font/src/bdf_font.cc
)

target_include_directories(bdf_font_lib PUBLIC
  libs/bdf-font/include
)

add_library(image_decode_lib
  libs/image-decode/src/image_decode.cc
)

target_include_directories(image_decode_lib
  PUBLIC libs/image-decode/include
  PRIVATE libs/image-decode/third_party
)

add_library(watchpanel_lib
  libs/watchpanel/src/model.cc
  libs/watchpanel/src/graphics.cc
  libs/watchpanel/src/graphics_context.cc
  libs/watchpanel/src/raster.cc
  libs/watchpanel/src/strings.cc
  libs/watchpanel/src/update.cc
  libs/watchpanel/src/data.cc
  libs/watchpanel/src/timedata.cc
  libs/watchpanel/src/hamper.cc
  libs/watchpanel/src/watchpanel.cc
)

target_include_directories(watchpanel_lib PUBLIC
  libs/watchpanel/include
  libs/watchpanel/include/watchpanel
  ${RAPIDJSON_INCLUDE_DIR}
)

target_link_libraries(watchpanel_lib
  PUBLIC PkgConfig::PUGIXML PkgConfig::CURL bdf_font_lib image_decode_lib
  PRIVATE pthread
)

add_library(render_svg_lib
  libs/render-svg/src/svg_raster.cc
)

target_include_directories(render_svg_lib PUBLIC
  libs/render-svg/include
  libs/render-svg/include/watchpanel
)

target_link_libraries(render_svg_lib PUBLIC watchpanel_lib PkgConfig::PUGIXML)

add_library(led_raster_lib
  libs/led-raster/src/led_raster.cc
)

target_include_directories(led_raster_lib PUBLIC
  libs/led-raster/include
)

target_link_libraries(led_raster_lib PUBLIC watchpanel_lib)

add_library(terminal_raster_lib
  libs/terminal-raster/src/terminal_raster.cc
)

target_include_directories(terminal_raster_lib PUBLIC
  libs/terminal-raster/include
)

target_link_libraries(terminal_raster_lib PUBLIC watchpanel_lib)

add_executable(page-renderer
  apps/page-renderer/src/main.cc
)
target_link_libraries(page-renderer PRIVATE render_svg_lib)

option(WATCHY_ENABLE_RGB_MATRIX "Build clock-led against the real rpi-rgb-led-matrix hardware library (Raspberry Pi only)" OFF)

add_executable(clock-led
  apps/clock-led/src/watchy_main.cc
)
target_link_libraries(clock-led PRIVATE watchpanel_lib led_raster_lib pthread m)

if(WATCHY_ENABLE_RGB_MATRIX)
  set(RGB_MATRIX_ROOT "/opt/rpi-rgb-led-matrix" CACHE PATH "Path to a built rpi-rgb-led-matrix checkout")
  find_path(RGB_MATRIX_INCLUDE_DIR led-matrix.h PATHS "${RGB_MATRIX_ROOT}/include")
  find_library(RGB_MATRIX_LIBRARY NAMES rgbmatrix PATHS "${RGB_MATRIX_ROOT}/lib")
  if(NOT RGB_MATRIX_INCLUDE_DIR OR NOT RGB_MATRIX_LIBRARY)
    message(FATAL_ERROR "WATCHY_ENABLE_RGB_MATRIX is ON but rpi-rgb-led-matrix wasn't found under "
                         "${RGB_MATRIX_ROOT}. Build it first (see docs/README.md), or set "
                         "-DRGB_MATRIX_ROOT=/path/to/checkout.")
  endif()
  target_compile_definitions(clock-led PRIVATE WATCHY_RGB_MATRIX)
  target_include_directories(clock-led PRIVATE "${RGB_MATRIX_INCLUDE_DIR}")
  target_link_libraries(clock-led PRIVATE "${RGB_MATRIX_LIBRARY}" rt)
endif()

add_executable(panel-runtime
  apps/panel-runtime/src/main.cc
)
target_link_libraries(panel-runtime PRIVATE render_svg_lib pthread)

add_executable(simulator
  apps/simulator/main.cc
)
target_link_libraries(simulator PRIVATE terminal_raster_lib)

install(TARGETS page-renderer clock-led panel-runtime simulator DESTINATION bin)

enable_testing()

add_executable(bdf_font_test
  tests/bdf_font_test.cc
)
target_link_libraries(bdf_font_test PRIVATE watchpanel_lib)

add_test(NAME bdf_font_test COMMAND bdf_font_test)
set_tests_properties(bdf_font_test PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_executable(document_model_test
  tests/document_model_test.cc
)
target_link_libraries(document_model_test PRIVATE watchpanel_lib)

add_test(NAME document_model_test COMMAND document_model_test)
set_tests_properties(document_model_test PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_executable(image_decode_test
  tests/image_decode_test.cc
)
target_link_libraries(image_decode_test PRIVATE image_decode_lib)

add_test(NAME image_decode_test COMMAND image_decode_test)
set_tests_properties(image_decode_test PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_executable(weather_page_test
  tests/weather_page_test.cc
)
target_link_libraries(weather_page_test PRIVATE watchpanel_lib)

add_test(NAME weather_page_test COMMAND weather_page_test)
set_tests_properties(weather_page_test PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
