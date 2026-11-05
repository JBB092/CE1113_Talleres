include_directories(${CMAKE_SOURCE_DIR}/include)

set(INC_FILE ${CMAKE_SOURCE_DIR}/include/calculadora.h)

add_library(operaciones SHARED calculadora.c ${INC_FILE})

target_link_libraries(operaciones m)

install(TARGETS operaciones DESTINATION lib)
install(FILES ${INC_FILE} DESTINATION include)