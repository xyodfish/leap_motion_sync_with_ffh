
if (UNIX)    
    find_package(Threads REQUIRED)    
endif (UNIX)


include_directories(
        ${INC_DIR}/third_party/leapMotion/v2
)

link_directories(
        ${LIB_DIR}
        ${LIB_THIRD_PARTY_DIR}
        ${LIB_THIRD_PARTY_DIR}/leapMotion/v2
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_executable(leapc_example leapc_main.c)
target_link_libraries(leapc_example LeapC)

add_library(
	libExampleConnection
	OBJECT
	ExampleConnection.c)

target_link_libraries(libExampleConnection PUBLIC LeapC)

if (UNIX)
	target_link_libraries(
		libExampleConnection
		PRIVATE
		Threads::Threads)
endif()    


add_executable(CallbackSample CallbackSample.c)
target_link_libraries(CallbackSample PUBLIC libExampleConnection)