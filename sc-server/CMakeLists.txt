set (SC_SERVER_DIR "${SC_MACHINE_ROOT}/sc-server")

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.hpp")

if (${WIN32})
  set (BOOST_LIBS_LIST )
else()
   set (BOOST_LIBS_LIST
       boost_program_options)
endif (${WIN32})

  
add_executable(sc-server ${SOURCES} ${HEADERS})
include_directories(${SC_MEMORY_SRC} ${GLIB2_INCLUDE_DIRS})
target_link_libraries(sc-server sc-memory sc-memory-cpp ${BOOST_LIBS_LIST})

add_dependencies(sc-server
  sc-kpm
  sc-memory
  sc-memory-cpp)
