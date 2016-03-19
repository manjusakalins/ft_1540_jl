#include "JlinkComboCustFormatCommand.h"
#include "../Resource/ResourceManager.h"
#include "../Setting/JlinkComboCustFormatSetting.h"

namespace APCore
{
JlinkComboCustFormatCommand::JlinkComboCustFormatCommand(APKey key):
    ICommand(key),
    fmt_setting_(NULL)
{
}

JlinkComboCustFormatCommand::~JlinkComboCustFormatCommand()
{
	for(std::list<APCore::FormatSetting *>::const_iterator it = fmt_setting_list_.begin(); it != fmt_setting_list_.end(); ++it) {
		delete (*it);
	}
}

void JlinkComboCustFormatCommand::exec(const QSharedPointer<Connection> &conn)
{
    conn->ConnectDA();
	DoComboFormat(conn);
}

void JlinkComboCustFormatCommand::DoNormatFormat(const QSharedPointer<Connection> &conn)
{
    fmt_setting_->set_is_normal_format(!is_combo_format_);

    return fmt_setting_->CreateCommand(conn->ap_key())->exec(conn);
}

void JlinkComboCustFormatCommand::DoComboFormat(const QSharedPointer<Connection> &conn)
{
	for(std::list<APCore::FormatSetting *>::const_iterator it = fmt_setting_list_.begin(); it != fmt_setting_list_.end(); ++it) {
		LOGI("########## %s %d ########## %llx => %llx\n", __func__, __LINE__, (*it)->begin_addr, (*it)->length);
		
        fmt_setting_->set_auto_format(false);
        fmt_setting_->set_part_id((*it)->part_id);
        fmt_setting_->set_begin_addr((*it)->begin_addr);
        fmt_setting_->set_length((*it)->length);

        fmt_setting_->CreateCommand(conn->ap_key())->exec(conn);
    }
}

}
