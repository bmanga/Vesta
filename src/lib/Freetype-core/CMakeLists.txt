cmake_minimum_required(VERSION ${CMAKE_MIN_REQUIRED})
project (Freetype-core)
file (GLOB CXX_SRC "*.cpp")
file (GLOB C_SRC "*.c")
include_directories(${INCLUDE_ROOT}/Freetype-core)	

file (GLOB CXX_HDR "${INCLUDE_ROOT}/Freetype-core/*.h")

#include openGL
include_directories(${EXT_ROOT}/glew/include)


set(LINK_LIBS ${LINK_LIBS} ${EXT_ROOT}/glew/lib/Windows/x64/glew32.lib)
set(LINK_LIBS ${LINK_LIBS} opengl32.lib)
include_directories(${FREETYPE_PATH}/include)
set(LINK_LIBS ${LINK_LIBS} ${FREETYPE_PATH}/objs/vc2010/x64/freetype26MT.lib)
add_library(Freetype-core ${CXX_SRC} ${C_SRC} ${CXX_HDR})
target_link_libraries(Freetype-core ${LINK_LIBS})