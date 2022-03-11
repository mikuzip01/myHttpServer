cmake_minimum_required(VERSION 3.0.0)
project(myHttpServer VERSION 0.1.0)

include(CTest)
enable_testing()
find_package(Threads)

# add_definitions(-D__PRINT_INFO_TO_DISP__)
add_definitions(-D__LOG_INFO__)

set(HttpServer_SOURCE_FILES
        src/httpserver.cpp
        src/ThreadPool.cpp
        src/utils.cpp
        src/HttpData.cpp
        src/Responser.cpp
        src/Pages.cpp
        src/Timer.cpp
        src/Error.h
        src/AsyncLoger.cpp
        )
# add_executable(httpServer src/httpserver.cpp)
add_executable(httpServer ${HttpServer_SOURCE_FILES})
target_link_libraries(httpServer ${CMAKE_THREAD_LIBS_INIT})

add_executable(postdataecho.cgi src/cgidemo.c)

# add_executable(asyncLoger src/AsyncLoger.cpp)
# target_link_libraries(asyncLoger ${CMAKE_THREAD_LIBS_INIT})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

# 
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

include(CPack)
