cmake_minimum_required(VERSION 3.20)

set(LIB_NAME app)

file(GLOB sources
  ${CMAKE_CURRENT_LIST_DIR}/audio/*.c
  ${CMAKE_CURRENT_LIST_DIR}/files/*.c
  ${CMAKE_CURRENT_LIST_DIR}/graphics/*.c
)

set(includes
  ${CMAKE_CURRENT_LIST_DIR}/audio
  ${CMAKE_CURRENT_LIST_DIR}/files
  ${CMAKE_CURRENT_LIST_DIR}/graphics
)

add_library(${LIB_NAME} INTERFACE)

target_sources(${LIB_NAME} INTERFACE ${sources} ${headers})
target_include_directories(${LIB_NAME} INTERFACE ${includes})

target_compile_definitions(${LIB_NAME} INTERFACE
)
