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

add_library(watchpanel_lib
  libs/watchpanel/src/model.cc
  libs/watchpanel/src/graphics.cc
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
  PUBLIC PkgConfig::PUGIXML PkgConfig::CURL
  PRIVATE pthread
)

add_library(bdf_font_lib
  libs/bdf-font/src/bdf_font.cc
)

target_include_directories(bdf_font_lib PUBLIC
  libs/bdf-font/include
)

add_library(render_svg_lib
  libs/render-svg/src/svg_canvas.cc
)

target_include_directories(render_svg_lib PUBLIC
  libs/render-svg/include
  libs/render-svg/include/watchpanel
)

target_link_libraries(render_svg_lib PUBLIC watchpanel_lib PkgConfig::PUGIXML)

add_library(led_canvas_lib
  libs/led-canvas/src/led_canvas.cc
)

target_include_directories(led_canvas_lib PUBLIC
  libs/led-canvas/include
)

target_link_libraries(led_canvas_lib PUBLIC watchpanel_lib bdf_font_lib)

add_library(terminal_canvas_lib
  libs/terminal-canvas/src/terminal_canvas.cc
)

target_include_directories(terminal_canvas_lib PUBLIC
  libs/terminal-canvas/include
)

target_link_libraries(terminal_canvas_lib PUBLIC watchpanel_lib bdf_font_lib)

add_executable(page-renderer
  apps/page-renderer/src/main.cc
)
target_link_libraries(page-renderer PRIVATE render_svg_lib)

add_executable(clock-led
  apps/clock-led/src/watchy_main.cc
)
target_link_libraries(clock-led PRIVATE pthread m)

add_executable(panel-runtime
  apps/panel-runtime/src/main.cc
)
target_link_libraries(panel-runtime PRIVATE render_svg_lib pthread)

add_executable(simulator
  apps/simulator/main.cc
)
target_link_libraries(simulator PRIVATE terminal_canvas_lib)

install(TARGETS page-renderer clock-led panel-runtime simulator DESTINATION bin)
