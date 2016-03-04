#include "ConnBromUSBSetting.h"
#include "../XMLParser/XMLNode.h"
#include "../Host/Inc/DeviceConn.h"

#include <QtGlobal>

namespace APCore
{

ConnBromUSBSetting::ConnBromUSBSetting(USBSpeed speed, USBPower power)
    : speed_(speed), power_(power),
      cb_download_da_init_(NULL),
      cb_download_da_(NULL),
      cb_usb_stat_init_(NULL)
{
}

ConnBromUSBSetting::ConnBromUSBSetting(const XML::Node &node)
    : cb_download_da_init_(NULL),
      cb_download_da_(NULL),
      cb_usb_stat_init_(NULL)
{
    LoadXML(node);
}

Connection *ConnBromUSBSetting::CreateConnection(
        APKey key, HW_StorageType_E stor, bool pwr_key_reset)
{
    BromBootArg args;

    FlashTool_Connect_Arg &ft_conn_arg =
            (FlashTool_Connect_Arg&)(args.GetBromBootArg());

    if(cb_download_da_init_ != NULL)
    {
        args.set_cb_download_da_init(cb_download_da_init_);
    }
    if(cb_download_da_ != NULL)
    {
        args.set_cb_download_da(cb_download_da_);
    }

    int force_chg = 0;

    if (power_== AutoDetect)
        force_chg = FORCE_CHARGE_AUTO;
    else if (power_== WithoutBattery)
        force_chg = FORCE_CHARGE_ON;
    else
        force_chg = FORCE_CHARGE_OFF;

    ft_conn_arg.m_force_charge = force_chg;

    ft_conn_arg.m_reset_key =
        pwr_key_reset ? RESET_BY_PWR_KEY_ALONE : RESET_BY_PWR_HOME_KEY;

    ConnMedia *med = new ConnMediaUSB(USBHighSpeed==speed_);
#ifdef _WIN32
    if(!m_com_port.empty())
    {
        std::string::size_type index = m_com_port.find("+");
        if(index != std::string::npos)
        {
            std::string n1 = m_com_port.substr(0,index);
            std::string n2 = m_com_port.substr(index+1);
            LOGI("Specify Normal com port = %s , HighSpeed com port = %s." , n1.c_str(), n2.c_str());
            med->setFriendlyName(n1);
            med->setHighSpeedFriendlyName(n2);
        }
        else
        {
            med->setFriendlyName(m_com_port);
        }
    }
#endif

    ConnArgBox *box = new ConnArgBox(
                key, get_stop_flag(), get_timeout(),stor, ft_conn_arg);

    return new Connection(
                _SHARED_PTR<ConnMedia>(med),
                _SHARED_PTR<ConnArgBox>(box));
}

void ConnBromUSBSetting::LoadXML(const XML::Node &node)
{
    Q_ASSERT(node.GetAttribute("type") == "BromUSB");

    std::string speed_str = node.GetAttribute("high-speed");
    speed_ = speed_str=="true"?USBHighSpeed:USBFullSpeed;

    std::string power_str = node.GetAttribute("without-battery");
    power_ = power_str=="true"?WithoutBattery:WithBattery;

    std::string timeout_str = node.GetAttribute("timeout-count");
    if(timeout_str.size()==0)
        m_timeout = 120000;
    else
        m_timeout = atoi(timeout_str.c_str());

    m_com_port = node.GetAttribute("com-port");
}

void ConnBromUSBSetting::SaveXML(XML::Node &node) const
{
    XML::Node child = node.AppendChildNode("connection");
    child.SetAttribute("type", "BromUSB");
    child.SetAttribute("high-speed",speed_==USBHighSpeed?"true":"false");
    child.SetAttribute("without-battery",power_==WithoutBattery?"true":"false");
    child.SetAttribute("timeout-count",QString::number(m_timeout).toStdString());
    child.SetAttribute("com-port",m_com_port);
}

}
