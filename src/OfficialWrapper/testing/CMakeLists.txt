
set(LEAP_CPP_TEST_SRCS
  ApiPropertyTest.cpp
  IteratorTest.cpp
)

include_directories(
        ${SRC_DIR}/OfficialWrapper
        ${INC_DIR}/third_party/leapMotion/v1
)

add_executable(LeapC++Test ${LEAP_CPP_TEST_SRCS})

target_link_libraries(LeapC++Test LeapC++ gtest)
if(UNIX)
  target_link_libraries(LeapC++Test -lpthread -lm)
endif()
target_compile_definitions(LeapC++Test PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING=1)


