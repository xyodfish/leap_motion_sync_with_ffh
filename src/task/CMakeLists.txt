
if (UNIX)    
    find_package(Threads REQUIRED)    
endif (UNIX)


include_directories(
        ${INC_DIR}/third_party
        ${INC_DIR}/third_party/leapMotion/v2
        ${INC_DIR}/third_party/spdlog
        ${INC_DIR}/third_party/yaml-cpp
        ${COMMON}
        ${UDP}
        ${FFH}
)

link_directories(
        ${LIB_DIR}
        ${LIB_THIRD_PARTY_DIR}
        ${LIB_THIRD_PARTY_DIR}/leapMotion/v2
        ${LIB_THIRD_PARTY_DIR}
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_library(LeapMotionV2Wrapper SHARED leapMotionV2Wrapper.cpp leapMotionV2WrapperImpl.cpp)
target_link_libraries(LeapMotionV2Wrapper LeapC
 spdlog yaml-cpp )

add_library(LpV2SyncFfh SHARED lpV2SyncFfh.cpp )
target_link_libraries(LpV2SyncFfh FfhCtrl LeapMotionV2Wrapper)

add_executable(test_lpv2_demo test_lpv2_demo.cpp)
target_link_libraries(test_lpv2_demo
LpV2SyncFfh)

