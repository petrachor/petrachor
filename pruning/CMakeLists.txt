file(GLOB sources "*.cpp")
file(GLOB subscription "subscription/*.cpp")
file(GLOB headers "*.h")

add_library(pruning ${sources} ${headers})

#target_include_directories(pruning PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(pruning jsoncpp_lib_static ethereum ethcore devcore)

add_subdirectory(test)
