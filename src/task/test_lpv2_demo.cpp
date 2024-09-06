#include "helper_functions.h"
#include "lpV2SyncFfh.h"
#include "spdlog/spdlog.h"

using namespace ar::Hardware::LeapMotion;

int main() {
    std::string config = getParentPath(3, std::string(__FILE__)) + "/config/leap_motion_demo_config.yaml";
    LpV2SyncFFH lpTask(config);

    if (lpTask.tryConnect() == AR_RETURN_VALUE::SUCCESS) {
        lpTask.start();
    }

    return 0;
}
