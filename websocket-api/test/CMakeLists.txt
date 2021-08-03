add_executable(websockets-api-test main.cpp)
target_link_libraries(websockets-api-test PRIVATE websocket-api pthread ethcore devcore)

