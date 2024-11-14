cmake_minimum_required(VERSION 3.10)

project(psyz)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

add_library(psyz STATIC
    src/platform/log.c
    src/platform/common.c
    src/platform/sdl2_gl.c
    src/platform/plat_unix.c
    src/psyz/libapi.c
    src/psyz/libetc.c
    src/psyz/libgpu.c
    ../decomp/src/libcard/card.c
    ../decomp/src/libcard/init.c
    ../decomp/src/libgpu/sys.c
    ../decomp/src/libgpu/ext.c
    ../decomp/src/libgpu/font.c
    ../decomp/src/libgpu/otag.c
    ../decomp/src/libgpu/prim.c
    ../decomp/src/libgpu/tmd.c
    src/psyz/libgte.c
    src/psyz/libgs.c
    src/psyz/libcd.c
    src/psyz/libcard.c
    src/psyz/libspu.c
    src/psyz/libsnd.c
    src/psyz/libsn.c
    src/psyz/libpress.c
)
target_compile_definitions(psyz PUBLIC __psyz)
target_include_directories(psyz PUBLIC include)
target_link_libraries(psyz PUBLIC SDL2::SDL2 -lGL)
