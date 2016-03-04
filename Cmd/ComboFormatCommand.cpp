#include "ComboFormatCommand.h"
#include "../Resource/ResourceManager.h"

namespace APCore
{
ComboFormatCommand::ComboFormatCommand(APKey key):
    ICommand(key),
    fmt_setting_(NULL)
{
}

ComboFormatCommand::~ComboFormatCommand()
{

}

void ComboFormatCommand::exec(const QSharedPointer<Connection> &conn)
{
    conn->ConnectDA();

    if(is_combo_format_ && does_auto_fmt_)
        DoComboFormat(conn);
    else
        DoNormatFormat(conn);
}

void ComboFormatCommand::DoNormatFormat(const QSharedPointer<Connection> &conn)
{
    fmt_setting_->set_is_normal_format(!is_combo_format_);

    return fmt_setting_->CreateCommand(conn->ap_key())->exec(conn);
}

void ComboFormatCommand::DoComboFormat(const QSharedPointer<Connection> &conn)
{
    QSharedPointer<APCore::EMMCFormatArg> auto_fmt_ptr(new EMMCFormatArg(conn, true, EMMC_PART_UNKNOWN, true));

    std::list<com_fmt_range> fmt_range = auto_fmt_ptr.data()->get_format_range();

    std::list<com_fmt_range>::const_iterator it = fmt_range.begin();

    while(it != fmt_range.end())
    {
        if(this->auto_fmt_flag_ == FormatSetting::Format_Except_BL &&
                it->part_id == EMMC_PART_BOOT1)
        {
            ++it;
            continue;
        }

        fmt_setting_->set_auto_format(false);
        fmt_setting_->set_part_id((EMMC_Part_E)it->part_id);
        fmt_setting_->set_begin_addr(it->fmt_begin);
        fmt_setting_->set_length(it->fmt_length);

        fmt_setting_->CreateCommand(conn->ap_key())->exec(conn);

        ++it;
    }
}

}
