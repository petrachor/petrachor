file(GLOB sources "*.cpp")
file(GLOB subscription "subscription/*.cpp")
file(GLOB headers "*.h")

add_library(websocket-api ${sources} ${headers} ${subscription})

target_include_directories(websocket-api PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(websocket-api jsoncpp_lib_static ethereum ethcore devcore web3jsonrpc)

add_subdirectory(test
        )