# Find Zephyr. This also loads Zephyr's build system.
cmake_minimum_required(VERSION 3.13.1)

list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(BOARD core840)

set(b0_OVERLAY_CONFIG
  ${CMAKE_CURRENT_SOURCE_DIR}/b0.conf
  )

set(mcuboot_OVERLAY_CONFIG
  overlay-minimal-external-crypto.conf
  )

set(mcuboot_CONF_FILE
  # Need to define this var instead of using
  # overlay, because of custom key logic in
  # NCS mcuboot.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/mcuboot.conf
  )

find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

project(rgbwatch)

set(CMAKE_BUILD_TYPE Release)

set(APP_SRC
  src/main.c
  src/rgb_led.c
  src/disp.c
  src/clock.c
  src/accel.c
  src/menu.c
  src/screen.c
  src/pinmux.c
  src/board.c
  src/state.c
  src/stopwatch.c
  src/countdown.c
  src/battery.c
  src/motor.c
  src/ble.c
  src/cts.c
  src/calendar.c
  )

set(APP_INCLUDE
  include
  include
  )

target_sources(app PRIVATE ${APP_SRC})
target_include_directories(app PRIVATE ${APP_INCLUDE})
