#ifndef __FFH_CTRL_H__
#define __FFH_CTRL_H__

#include "generalReturnValue.h"
#include "udpClient.h"
#include "udp_hand_types.hpp"

class FfhCtrl {
   public:
    explicit FfhCtrl(const int& _index, const std::string& _ip, const int& _port);
    explicit FfhCtrl(const FfhUdpData& data);
    FfhCtrl() = default;
    ~FfhCtrl();

    /**
     * @brief 
     * 
     * @return AR_RETURN_VALUE 
     */
    AR_RETURN_VALUE _connect();

    /**
     * @brief 断连
     * 
     * @return AR_RETURN_VALUE 
     */
    AR_RETURN_VALUE _disconnect();

    /**
     * @brief 
     * 
     * @return int 
     */
    int send_hand_cmd();

    /**
     * @brief 
     * 
     * @return int 
     */
    int recieve_hand_data();

    /**
     * @brief Get the hand data object
     * 
     * @return udp_hand_data 
     */
    udp_hand_data get_hand_data();

    void set_hand_cmd(udp_hand_cmd cmd) { handCmd_ = cmd; }

    int send_hand_cmd(udp_hand_cmd cmd);

    int getId() const;

   private:
    std::shared_ptr<UdpClient> udp_;
    udp_hand_cmd handCmd_;
    udp_hand_data handData_;

    int index_;

    bool inConnection_{false};

    void dataProcess();
};

#endif