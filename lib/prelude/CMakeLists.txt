add_library(prelude INTERFACE)
target_link_libraries(prelude INTERFACE macro)
target_include_directories(prelude INTERFACE ${SDL_BITS_INCLUDE_DIR}/prelude)
