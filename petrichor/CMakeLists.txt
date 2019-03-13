set(
    sources
    AccountManager.cpp AccountManager.h
    main.cpp
    MinerAux.h
)

add_executable(petrichor ${sources})
target_link_libraries(
    petrichor
    PRIVATE ethereum ethashseal evm web3jsonrpc webthree devcore
)

install(TARGETS petrichor DESTINATION bin)
