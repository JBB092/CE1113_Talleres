include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(calculadora main.c)
target_link_libraries(calculadora operaciones)

install(TARGETS calculadora DESTINATION bin)