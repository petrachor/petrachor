set(FILES
    util.h
    endian.h
    compiler.h
    fnv.h
    data_sizes.h
    sha3.c sha3.h
)

add_library(ethash ${FILES})
set_target_properties(ethash PROPERTIES C_STANDARD 99)
