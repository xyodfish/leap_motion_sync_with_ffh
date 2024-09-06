include_directories(
    ${COMMON}
    ${UDP}
    ${INC_DIR}/third_party
)

link_directories(
        ${LIB_DIR}
        ${LIB_THIRD_PARTY_DIR}
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

add_library(FfhCtrl SHARED ffhCtrl.cpp udp_hand_types.cpp)
target_link_libraries(FfhCtrl UdpClient)
