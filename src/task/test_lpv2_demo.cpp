#include "helper_functions.h"
#include "lpV2SyncFfh.h"
#include "spdlog/spdlog.h"

using namespace ar::Hardware::LeapMotion;

int main()
{
    LpV2SyncFFH lpTask;
    std::string curFile = __FILE__;
    std::string confg   = getParentPath(3, curFile) + "/config/leap_motion_demo_config.yaml";

    lpTask.parseConfig(confg);
    if (lpTask.connect() != AR_RETURN_VALUE::SUCCESS)
    {
        spdlog::error("Lp v2 task connection failed");
        return 1;
    }

    lpTask.start();

    return 0;
}
