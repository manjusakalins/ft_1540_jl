#include "JlinkComboCustFormatSetting.h"

#include "../Logger/Log.h"
#include "../Err/Exception.h"
#include "../Cmd/ComboFormatCommand.h"
#include "../XMLParser/XMLNode.h"
#include "../Cmd/JlinkComboCustFormatCommand.h"


namespace APCore
{
JlinkComboCustFormatSetting::JlinkComboCustFormatSetting():
    format_setting_(new APCore::FormatSetting()),
    is_combo_dl_(false)
{
}

QSharedPointer<APCore::ICommand> JlinkComboCustFormatSetting::CreateCommand(APKey key)
{
    QSharedPointer<APCore::JlinkComboCustFormatCommand> cmd(new APCore::JlinkComboCustFormatCommand(key));

    cmd->set_is_combo_format(is_combo_dl_);
    cmd->set_format_setting(format_setting_);
	cmd->set_format_setting_list(format_setting_list_);
    cmd->set_is_auto_format(format_setting_->is_auto_format());
    cmd->set_auto_format_flag(format_setting_->get_format_flag());

    return cmd;
}

void JlinkComboCustFormatSetting::set_storage_type(HW_StorageType_E type)
{
    storage_type_ = type;
    this->format_setting_->set_storage_type(type);

}

void JlinkComboCustFormatSetting::LoadXML(const XML::Node &node)
{
    XML::Node child_node = node.GetFirstChildNode();

    while(!child_node.IsEmpty())
    {
        if(child_node.GetName() == "format")
        {
            this->format_setting_->set_storage_type(storage_type_);
            this->format_setting_->LoadXML(child_node);
        }

        child_node = child_node.GetNextSibling();
    }

}

void JlinkComboCustFormatSetting::SaveXML(XML::Node &node) const
{
    XML::Node root_node(node.AppendChildNode("combo-format"));

    this->format_setting_->SaveXML(root_node);
}

}
