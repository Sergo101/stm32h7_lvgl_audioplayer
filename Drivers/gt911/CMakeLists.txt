cmake_minimum_required(VERSION 3.20)

set(LIB_NAME gt911_lib)

file(GLOB sources
  # ${CMAKE_CURRENT_LIST_DIR}/Src/ctiic.c
  ${CMAKE_CURRENT_LIST_DIR}/Src/gt911.c
)

file(GLOB headers
${CMAKE_CURRENT_LIST_DIR}/Src/gt911.h
)

set(includes
  ${CMAKE_CURRENT_LIST_DIR}/Inc
)

add_library(${LIB_NAME} INTERFACE)
add_library(lib::gt911_lib ALIAS ${LIB_NAME})

target_sources(${LIB_NAME} INTERFACE ${sources} ${headers})
target_include_directories(${LIB_NAME} INTERFACE ${includes})

target_compile_definitions(${LIB_NAME} INTERFACE
)
