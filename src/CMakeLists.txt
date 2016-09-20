
file (GLOB CXX_SRC "*.cpp")

vesta_use_lib(Freetype-core)
include_directories(${INCLUDE_ROOT})
include_directories(${EXT_ROOT}/glew/include)

find_package(Qt5Widgets)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
#Qt related stuff
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)

qt5_wrap_cpp(VESTA_HDR_MOC ${INCLUDE_ROOT}/vesta.h)
qt5_wrap_ui(VESTA_UI_MOC vesta.ui)

qt5_wrap_cpp(TEXTWINDOW_HDR_MOC ${INCLUDE_ROOT}/TextWindow.h)

#set(CMAKE_PREFIX_PATH, ${QT_PATH}/lib/cmake/Qt5Widgets)
set(LINK_LIBS ${LINK_LIBS} ${EXT_ROOT}/glew/lib/Windows/x64/glew32.lib)
set(LINK_LIBS ${LINK_LIBS} opengl32.lib)

file (GLOB CXX_HDR "${INCLUDE_ROOT}/*.h")

set(LINK_LIBS ${LINK_LIBS} Qt5::Widgets)
add_executable(Vesta ${CXX_SRC} ${VESTA_HDR_MOC} ${VESTA_UI_MOC} ${TEXTWINDOW_HDR_MOC} ${CXX_HDR})
target_link_libraries(Vesta ${LINK_LIBS})