
add_compile_options(-Wl,-E)

aux_source_directory(. DIR_V7JS_SRCS)
add_library(v7js ${DIR_V7JS_SRCS})