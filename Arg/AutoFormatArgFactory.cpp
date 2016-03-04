#include "AutoFormatArgFactory.h"
#include "NANDAutoFormatArg.h"
#include "EMMCAutoFormatArg.h"
#include "../Logger/Log.h"
#include "../BootRom/flashtool.h"


/*QSharedPointer<AutoFormatArg> AutoFormatArgFactory::CreateAutoFormatArg(const StorageLayoutType_E type)
{
    Dump();
    if ( FIXED_NAND == type) {
        return QSharedPointer<AutoFormatArg>(new FixedNandAutoFormatArg(this->auto_fmt_flag_, this->da_report_, this->dl_handle_));
    } else if ( DYNAMIC_NAND == type){
        return QSharedPointer<AutoFormatArg>(new DynamicNandAutoFormatArg(
                this->auto_fmt_flag_,
                this->da_report_,
                this->oper_flag_,
                this->dl_handle_,
                this->flashtool_handle_));
    } else if ( EMMC == type) {
        return QSharedPointer<AutoFormatArg>(new EMMCAutoFormatArg(
            this->auto_fmt_flag_,
            this->da_report_,
            this->dl_handle_));
    } else {
        assert(0 && "Un-support type!");
        return QSharedPointer<AutoFormatArg>(new DynamicNandAutoFormatArg(
                this->auto_fmt_flag_,
                this->da_report_,
                this->oper_flag_,
                this->dl_handle_,
                this->flashtool_handle_));
    }
}*/

void APCore::AutoFormatArgFactory::Dump(void)
{
        /*LOG("%s(): auto_fmt_flag(%d), da_report_->m_nand_flash_size(0x%x).",
                        __FUNCTION__, this->auto_fmt_flag_, this->da_report_->m_nand_flash_size);
    LOG("%s(): oper_flag_(%s), dl_handle_(0x%x), flashtool_handle_(0x%x)",
                        __FUNCTION__,
            AddrTypeFlagToString(this->oper_flag_),
                        this->dl_handle_,this->flashtool_handle_);*/
}

