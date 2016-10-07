set(CMAKE_MIN_REQUIRED 3.6)
cmake_minimum_required(VERSION ${CMAKE_MIN_REQUIRED})
project(Vesta)

set(INCLUDE_ROOT ${CMAKE_SOURCE_DIR}/include)
set(SRC_ROOT ${CMAKE_SOURCE_DIR}/src)
set(LIB_ROOT ${SRC_ROOT}/lib)
set(EXT_ROOT ${CMAKE_SOURCE_DIR}/ext)

macro(vesta_use_lib LIBNAME)
	include_directories(${INCLUDE_ROOT}/${LIBNAME})
	add_subdirectory(${LIB_ROOT}/${LIBNAME})
	set(LINK_LIBS ${LINK_LIBS} ${LIBNAME})
endmacro(vesta_use_lib)

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}) 
add_subdirectory(${SRC_ROOT})


