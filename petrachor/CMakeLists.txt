set(
    sources
    AccountManager.cpp AccountManager.h
    main.cpp
    MinerAux.h
)

add_executable(petrachor ${sources})
target_link_libraries(
    petrachor
    PRIVATE ethereum aleth-interpreter ethash::keccak ethashseal evm cryptopp-static devcrypto web3jsonrpc webthree devcore
        websocket-api pruning
)

install(TARGETS petrachor DESTINATION bin)
