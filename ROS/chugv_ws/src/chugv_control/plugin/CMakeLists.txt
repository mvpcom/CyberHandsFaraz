cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

include (FindPkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(GAZEBO gazebo)
endif()
include_directories(${GAZEBO_INCLUDE_DIRS})
link_directories(${GAZEBO_LIBRARY_DIRS})

add_library(rosplugin SHARED robot_0_plugin.cc)
set_target_properties(rosplugin PROPERTIES COMPILE_FLAGS "${roscpp_CFLAGS_OTHER}")
set_target_properties(rosplugin PROPERTIES LINK_FLAGS "${roscpp_LDFLAGS_OTHER}")
target_link_libraries(rosplugin ${roscpp_LIBRARIES}   ${GAZEBO_LIBRARIES}
  ${Boost_LIBRARIES}
  ${PROTUBUF_LIBRARIES})
install (TARGETS rosplugin DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/gazebo_plugins/)
