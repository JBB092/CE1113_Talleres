include_directories(${CMAKE_SOURCE_DIR}/include)

set(LIB_NAME hello)
set(INC_FILE ${CMAKE_SOURCE_DIR}/include/say.h)

add_library(${LIB_NAME} SHARED say.c ${INC_FILE})

install(TARGETS ${LIB_NAME} DESTINATION lib)
install(FILES ${INC_FILE} DESTINATION include)