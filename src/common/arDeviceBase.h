#ifndef __AR_DEVICE_BASE_H__
#define __AR_DEVICE_BASE_H__

#include "generalReturnValue.h"
#include "taskScheduler.h"

namespace ar::Hardware
{
    class DeviceBase : public Runnable
    {
      public:
        virtual AR_RETURN_VALUE connect()                              = 0;
        virtual AR_RETURN_VALUE disconnect()                           = 0;
        virtual AR_RETURN_VALUE initialize(const std::string &config)  = 0;
        virtual AR_RETURN_VALUE parseConfig(const std::string &config) = 0;

      protected:
        bool inConnection_ = false;
    };
} // namespace ar::Hardware
#endif