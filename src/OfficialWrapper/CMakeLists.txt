include_directories(
        ${INC_DIR}/third_party/leapMotion/v1
        ${INC_DIR}/third_party/leapMotion/v2
)

link_directories(
        ${LIB_DIR}
        ${LIB_THIRD_PARTY_DIR}
        ${LIB_THIRD_PARTY_DIR}/leapMotion/v2
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_library(LeapC++ SHARED LeapC++.cpp LeapImplementationC++.cpp)
target_link_libraries(LeapC++ LeapC)