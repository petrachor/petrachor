set(
    sources
    AccountManager.cpp AccountManager.h
    main.cpp
    MinerAux.h
)

add_executable(petrachor ${sources})
target_link_libraries(
    petrachor
    PRIVATE ethereum ethashseal evm web3jsonrpc webthree devcore
)

install(TARGETS petrachor DESTINATION bin)
