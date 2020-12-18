# Find Zephyr. This also loads Zephyr's build system.
cmake_minimum_required(VERSION 3.13.1)

set(BOARD core840)

find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

# if(CONFIG_PINMUX)
#   zephyr_library()
#   zephyr_library_sources(src/pinmux.c)
#   zephyr_library_include_directories(${ZEPHYR_BASE}/drivers)
# endif()

project(rgbwatch)

set(APP_SRC
  src/main.c
  src/rgb_led.c
  src/import/disp.c
  src/import/clock.c
  src/import/accel.c
  src/import/menu.c
  src/import/screen.c
  src/pinmux.c
  src/board.c
  )

set(APP_INCLUDE
  include
  include/import
  )

target_sources(app PRIVATE ${APP_SRC})
target_include_directories(app PRIVATE ${APP_INCLUDE})
