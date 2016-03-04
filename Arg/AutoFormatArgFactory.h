#ifndef AUTOFORMATARGFACTORY_H
#define AUTOFORMATARGFACTORY_H


#include <string>
#include <stdexcept>

#include <QSharedPointer>
#include "AutoFormatArg.h"
#include "../ResourceManager/NonCopyable.h"

namespace APCore
{
class AutoFormatArgFactory : private NonCopyable
{
public:
    AutoFormatArgFactory():
        //auto_fmt_flag_(FORMAT_ALL_EXP_BL_NVRAM_BIN),
        da_report_(NULL),
        oper_flag_(NUTL_ADDR_LOGICAL),
        dl_handle_(NULL),
        flashtool_handle_(NULL)
    {}

    ~AutoFormatArgFactory() {}

    //QSharedPointer<AutoFormatArg> CreateAutoFormatArg(const StorageLayoutType_E type);
private:
    void Dump(void);
private:
    //Nand auto format input parameters
    //NandAutoFormat_E auto_fmt_flag_;
    const DA_REPORT_T* da_report_;
    NUTL_AddrTypeFlag_E oper_flag_;
    DL_HANDLE_T dl_handle_;
    FLASHTOOL_API_HANDLE_T flashtool_handle_;
};
}
#endif // AUTOFORMATARGFACTORY_H
