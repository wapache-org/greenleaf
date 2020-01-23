
add_compile_options(-Wl,-E)

aux_source_directory(. DIR_LUA_SRCS)
add_library(lua ${DIR_LUA_SRCS})