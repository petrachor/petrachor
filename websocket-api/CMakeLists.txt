file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

add_library(websocket-api ${sources} ${headers})

# This "local" include path will be propagated to all other libs.
target_include_directories(websocket-api PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(websocket-api jsoncpp_lib_static ethereum ethcore devcore)

add_subdirectory(test
        )