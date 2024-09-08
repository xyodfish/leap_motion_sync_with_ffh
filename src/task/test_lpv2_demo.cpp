#include "lpV2SyncFfh.h"

using namespace ar::Hardware::LeapMotion;

int main() {
    LpV2SyncFFH lpTask("./leap_motion_demo_config.yaml");

    if (lpTask.tryConnect() == AR_RETURN_VALUE::SUCCESS) {
        lpTask.start();
    }

    return 0;
}
