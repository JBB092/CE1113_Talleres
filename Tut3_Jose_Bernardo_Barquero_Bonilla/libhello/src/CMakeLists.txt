include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(saludo main.c)
target_link_libraries(saludo hello)

install(TARGETS saludo DESTINATION bin)