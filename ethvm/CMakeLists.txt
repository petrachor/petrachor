add_executable(ethvm main.cpp)

target_link_libraries(ethvm PRIVATE ethereum evm ethash::ethash ethashseal devcore)

install(TARGETS ethvm DESTINATION bin OPTIONAL)
