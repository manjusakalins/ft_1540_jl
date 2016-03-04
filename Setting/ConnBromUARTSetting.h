#ifndef CONNBROMUARTSETTING_H
#define CONNBROMUARTSETTING_H

#include "ConnSetting.h"
#include "../Arg/BootRomArg.h"

namespace APCore
{
class ConnBromUARTSetting : public ConnSetting
{
public:
    ConnBromUARTSetting(const std::string& port);
    ConnBromUARTSetting(const XML::Node &node);

    virtual Connection *CreateConnection(
            APKey key, HW_StorageType_E stor, bool pwr_key_reset);

    virtual void LoadXML(const XML::Node &node);
    virtual void SaveXML(XML::Node &node) const;

    void set_cb_dl_da_init(
            CALLBACK_WRITE_BUF_PROGRESS_INIT cb_dl_da_init)
    {
        cb_download_da_init_ = cb_dl_da_init;
    }

    void set_cb_dl_da(
            CALLBACK_WRITE_BUF_PROGRESS cb_dl_da)
    {
        cb_download_da_ = cb_dl_da;
    }

    void set_baudrate(unsigned int baudrate)
    {
        m_baudrate = baudrate;
    }

    unsigned int get_baudrate()
    {
        return m_baudrate;
    }

private:
    std::string com_port_;
    unsigned int m_baudrate;
    CALLBACK_WRITE_BUF_PROGRESS_INIT cb_download_da_init_;
    CALLBACK_WRITE_BUF_PROGRESS      cb_download_da_;
};

}

#endif // CONNBROMUARTSETTING_H
