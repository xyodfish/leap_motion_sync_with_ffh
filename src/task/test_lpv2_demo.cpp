#include "helper_functions.h"
#include "lpV2SyncFfh.h"
#include "spdlog/spdlog.h"

using namespace ar::Hardware::LeapMotion;

int main() {
    LpV2SyncFFH lpTask("./leap_motion_demo_config.yaml");

    if (lpTask.tryConnect() == AR_RETURN_VALUE::SUCCESS) {
        lpTask.start();
    }

    return 0;
}
