#include <list>
#include "../Public/AppCore.h"
#include "FirmwareUpgradeCommand.h"
#include "../Utility/Utils.h"
#include "../Utility/FileUtils.h"
#include "../Resource/ResourceManager.h"
#include "../Resource/CoreHandle.h"
#include "../Resource/Handle/RBHandle.h"
#include "../Resource/Handle/DLHandle.h"
#include "../Logger/Log.h"

#include "../Flow/storhelper.h"
#include "../Flow/romfactory.h"
#include "../Flow/concreterom.h"
#include "../Flow/dlproxy.h"
#include "../Public/RomInfoFactory.h"
#include "../Host/Inc/RuntimeMemory.h"

#define _PRE_CHECK_ALL_

#define theClass    APCore::FirmwareUpgradeCommand

theClass::FirmwareUpgradeCommand(APKey key):
    ICommand(key),
    rb_setting_(NULL),
    wm_setting_(NULL),
    fmt_setting_(NULL),
    dl_setting_(NULL)
{
}

theClass::~FirmwareUpgradeCommand()
{
}


ERROR_T theClass::DoFirmwareUpgrade(
        IRomBase *rom,
        CDLProxy *proxy)
{
    ERROR_T error = ERROR_OK;

#ifndef _PRE_CHECK_ALL_
    if ((error = rom->check(proxy)) != ERROR_OK)
        return error;
#endif

    if (rb_setting_ != NULL)
    {
        if ((error = rom->read(proxy)) != ERROR_OK)
            return error;
    }

    if (fmt_setting_ != NULL)
    {
        if ((error = rom->format(proxy)) != ERROR_OK)
            return error;
    }

    if (dl_setting_ != NULL)
    {
        if ((error = rom->download(proxy)) != ERROR_OK)
            return error;
    }

    if (wm_setting_ != NULL)
    {
        if ((error = rom->restore(proxy)) != ERROR_OK)
            return error;
    }
    return ERROR_OK;
}

ERROR_T theClass::ProduceRomList(
    const QSharedPointer<APCore::Connection> &conn,
    RomBaseList &roms_2dl)
{
    CRomFactory rom_factory(roms_2dl, fmt_setting_->does_format_whole());

    ERROR_T error = ERROR_OK;

    std::list<ROM_INFO> rom_list;

    RomInfoFactory info_factory(key_);
    info_factory.GetRomList(conn, rom_list);

    std::list<ROM_INFO>::iterator it = rom_list.begin();

    while (it != rom_list.end())
    {
        error = rom_factory.produce(*it, dl_scene_);

        if (error != ERROR_OK)
        {
            break;
        }
        ++ it;
    }

    return error;
}

ERROR_T theClass::DoFirmwareUpgrade(const QSharedPointer<APCore::Connection> &conn,
        CDLProxy *proxy)
{
    std::string errInfo;

    RomBaseList rom_list_2dl;

    ERROR_T error = ProduceRomList(conn,  rom_list_2dl);

    RomBaseList::iterator it;

#ifdef _PRE_CHECK_ALL_
    if (error == ERROR_OK)
    {
        it = rom_list_2dl.begin();

        while (it != rom_list_2dl.end())
        {
            error = (*it)->check(proxy);

            if (error != ERROR_OK)
            {
                errInfo = ErrMessage(it->data(), error);
            }
            ++ it;
        }
    }
#endif

    if (error == ERROR_OK && errInfo.empty())
    {
        it = rom_list_2dl.begin();

        while (it != rom_list_2dl.end())
        {
            error = DoFirmwareUpgrade(it->data(), proxy);

            if (error != ERROR_OK)
            {
                errInfo = ErrMessage(it->data(), error);
                break;
            }

            ++ it;
        }
    }

    if (!errInfo.empty())
    {
        //errInfo.erase(errInfo.length()-1);
        fw_throw_error(error, errInfo.c_str());
    }

    return error;
}

void theClass::exec(
        const QSharedPointer<Connection> &conn)
{
    conn->ConnectDA();

    APCore::RBHandle *rb_handle = GET_RB_HANDLE(key_);

    CStorHelper storage(conn.data());

    CDLProxy proxy(conn, rb_handle, storage);

    ERROR_T error = DoFirmwareUpgrade(conn, &proxy);

    if (error == ERROR_OK)
    {
        error = proxy.validate_storage(
                    storage.AlignSize(),
                    storage.LogicalSize());
    }

    if (error == ERROR_OK)
    {
        error = proxy.commit(
                    rb_setting_.data(),
                    fmt_setting_.data(),
                    dl_setting_.data(),
                    wm_setting_.data());
    }

    if (error != ERROR_OK)
    {
        fw_throw_error(error);
    }
}

std::string theClass::ErrMessage(
        const IRomBase *rom, ERROR_T err)
{
    std::string msg = rom->name();
    msg += ": ";
    msg += fw_error_text(err);
    msg += "\n";
    return msg;
}
