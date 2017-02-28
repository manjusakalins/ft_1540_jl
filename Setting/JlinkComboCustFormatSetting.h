#ifndef JlinkComboCustFormatSetting_H
#define JlinkComboCustFormatSetting_H

#include "ISetting.h"
#include "FormatSetting.h"
#include <QSharedPointer>

namespace APCore
{

class JlinkComboCustFormatSetting : public ISetting
{
public:
    JlinkComboCustFormatSetting();
    QSharedPointer<APCore::ICommand> CreateCommand(APKey key);

    virtual void LoadXML(const XML::Node &node);
    virtual void SaveXML(XML::Node &node) const;

    void set_format_setting(QSharedPointer<APCore::FormatSetting> setting)
    {
        this->format_setting_ = setting;
    }

    void set_is_combo_dl(bool is_combo_dl)
    {
        this->is_combo_dl_ = is_combo_dl;
    }

    QSharedPointer<APCore::FormatSetting> format_setting()
    {
        return format_setting_;
    }
    void set_format_setting_list(std::list<APCore::FormatSetting *>& _items)
    {
        format_setting_list_ = _items;
    }
    std::list<APCore::FormatSetting *> format_setting_list()
    {
        return format_setting_list_;
    }

    virtual void set_storage_type(HW_StorageType_E type);
private:
    QSharedPointer<APCore::FormatSetting> format_setting_;
    std::list<APCore::FormatSetting *> format_setting_list_;
    bool is_combo_dl_;
};

}

#endif // JlinkComboCustFormatSetting_H