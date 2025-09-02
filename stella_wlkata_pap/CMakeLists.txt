cmake_minimum_required(VERSION 2.8.3)
project(stella_wlkata)

find_package(catkin REQUIRED
	roscpp
    serial
)

include_directories(include ${catkin_INCLUDE_DIRS} ${SDFormat_INCLUDE_DIRS})

catkin_package(
	DEPENDS
    roscpp
	serial
  CATKIN_DEPENDS
  INCLUDE_DIRS
  LIBRARIES
)

find_package(roslaunch)

add_executable(stella_wlkata_node src/stella_wlkata_node.cpp)
add_executable(stella_wlkata_shm_node src/stella_wlkata_shm_node)
target_link_libraries(stella_wlkata_node ${catkin_LIBRARIES})
target_link_libraries(stella_wlkata_shm_node ${catkin_LIBRARIES})

foreach(dir config launch meshes urdf)
	install(DIRECTORY ${dir}/
		DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/${dir})
endforeach(dir)

