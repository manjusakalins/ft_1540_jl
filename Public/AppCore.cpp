/*
 * AppCore.cpp
 *
 *  Created on: Aug 26, 2011
 *      Author: MTK81019
 */

#include "AppCore.h"

#include "../Resource/ResourceManager.h"
#include "../Resource/CoreHandle.h"
#include "../Resource/Handle/DLHandle.h"
#include "../Resource/Handle/RBHandle.h"
#include "../Resource/Handle/DAHandle.h"
#include "../Resource/Handle/ScretHandle.h"
#include "../Resource/Handle/AuthHandle.h"

#include "../Logger/Log.h"
#include "../Err/Exception.h"
#include "../Err/FlashToolErrorCodeDef.h"
#include "../Utility/Utils.h"
#include "../Utility/FileUtils.h"

#include <QtGui>
#include <QFile>
#include <QDir>

AppCore::AppCore()
{

}

AppCore::~AppCore()
{
    APCore::ResourceManager::instance()->FreeAll();
}

APKey AppCore::NewKey()
{
    APKey key = APCore::ResourceManager::instance()->NewAPKey();
    LOG("generated AP key(%d)", key);
    return key;
}

//load DA file
void AppCore::LoadDA(const APKey key, const std::string &da_file)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    LOG("loading DA (%d, %s)", key, da_file.c_str());
    core_handle->LoadDA(da_file);
}

void AppCore::LoadCertFile(const APKey key, const std::string &secu_file)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    core_handle->sc_handle()->LoadSecurityFile(secu_file);
}

void AppCore::LoadAuthFile(const APKey key, const std::string &auth_file)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    core_handle->auth_handle()->LoadAuthFile(auth_file);
}

//get DA infomation
void AppCore::GetDAInfo(const APKey key, DA_INFO *da_info)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    core_handle->da_handle()->GetDAInfo(da_info);
}

bool AppCore::IsDALoaded(const APKey key)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    return core_handle->da_handle()->IsReady();
}

bool AppCore::IsSecuLoaded(const APKey key)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    return core_handle->sc_handle()->IsReady();
}

bool AppCore::IsAuthLoaded(const APKey key)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);

    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    return core_handle->auth_handle()->IsReady();
}

int AppCore::CheckSecUnlock(const APKey key, const int index)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);

    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    return core_handle->CheckSecUnlock(index);
}

bool AppCore::GetScatterVersion(const APKey key, char* version)
{
    APCore::CoreHandle *core_handle = APCore::ResourceManager::instance()->GetHandle(key);

    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    return core_handle->GetScatterVersion(version);
}

bool AppCore::GetScatterHeadInfo(const APKey key, SCATTER_Head_Info *p_info)
{
    APCore::CoreHandle *core_handle = APCore::ResourceManager::instance()->GetHandle(key);

    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    return core_handle->GetScatterInfo(p_info);
}

bool AppCore::IsComboSCIDL(const APKey key){
    APCore::CoreHandle *core_handle = APCore::ResourceManager::instance()->GetHandle(key);

    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    return core_handle->IsComboSCIDL();
}

bool AppCore::LoadScatter(
        DL_SCATTER_TYPE type,
                         const APKey key,
        const std::string & scatter_file)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle != NULL)
    {
        LOG("loading scatter (%d, %s)", key, scatter_file.c_str());
        int ret = core_handle->LoadScatter(type, scatter_file);
        if(S_STOP == ret)
        {
            THROW_USER_STOP;
        }
        else if(S_DONE != ret)
        {
            THROW_APP_EXCEPTION(FT_FLASHTOOL_INVALIDE_SCATTER, "");
        }

        return true;
    }
    return false;
}

bool AppCore::VerifyImageChksum(const APKey key, const std::string &scatter_file)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);

    QDir load_dir = QFileInfo(scatter_file.c_str()).dir();
    std::string path = load_dir.path().toLocal8Bit().constData();
    ChksumRule chksum_rule(path);

    if(!chksum_rule.is_do_chksum())
    {
        LOGD("Checksum is not on, skip...");
        return true;
    }

    RomMemChecksumResult chksum_res;
    RomMemChecksumArg chksum_arg;
    memset(&chksum_res, 0, sizeof(chksum_res));

    std::ostringstream oss_msg;

    std::list<ROM_INFO> rom_info_list = core_handle->dl_handle()->GetAllRomInfo();
    for(std::list<ROM_INFO>::const_iterator rom_it = rom_info_list.begin();
        rom_it != rom_info_list.end(); ++rom_it)
    {
        if(!rom_it->item_is_visable)
        {
            LOGD("skip chksum for rom(%s).", rom_it->name);
            continue;
        }

        chksum_setting_->set_chksum_value(chksum_rule.QueryChksumByImageName(rom_it->name));
        chksum_setting_->set_chksum_index(rom_it->index);

       chksum_arg = chksum_setting_->GetBromChksumArg().GetChkusmArg();

        LOGD("image_name(%s), chksum_value(%d), index(%d)", rom_it->name,
             chksum_arg.chksum,chksum_arg.index);

        int ret = DL_VerifyROMMemBuf(core_handle->dl_handle()->handle_t(),
                                 &chksum_arg, &chksum_res);

        if(S_STOP == ret)
        {
            THROW_USER_STOP;
            return false;
        }
        else if(S_DONE != ret)
        {
             oss_msg<<"Image "<<rom_it->name<<" checksum failed!"<<std::endl
                    <<std::hex
                    <<"Checksum reference value(0x"
                    <<chksum_setting_->GetBromChksumArg().GetChkusmArg().chksum
                    <<")in the config ini file." <<std::endl
                    <<std::dec
                    <<"Please re-fill correct checksum value and re-load scatter file again."
                    <<std::endl;

            THROW_APP_EXCEPTION(-1, oss_msg.str().c_str());
            return false;
        }
    }

    return true;
}

bool AppCore::LoadROM(const APKey key, const std::string & rom_file, int rom_index)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle != NULL)
    {
        LOG("loading image (%d, %s, %d)", key, rom_file.c_str(), rom_index);
        return core_handle->LoadRomFile(rom_file, rom_index);
    }
    return false;
}

void AppCore::GetPlatformInfo(const APKey key, DL_PlatformInfo & info)
{
     APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
     if(core_handle == NULL)
     {
         THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
     }
     core_handle->dl_handle()->GetPlatformInfo(info);
}

bool AppCore::AutoLoadRomImages(const APKey key, const std::string &scatter_file)
{
    int ret = S_UNDEFINED_ERROR;
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }
    ret = core_handle->AutoLoadRomImages(scatter_file);
    if(ret == S_STOP)
    {
        THROW_USER_STOP;
    }

    if(ret == S_DL_LOAD_REGION_IS_OVERLAP)
    {
        THROW_APP_EXCEPTION(S_DL_LOAD_REGION_IS_OVERLAP, "S_DL_LOAD_REGION_IS_OVERLAP \n please check partition size");
    }

    if(ret == S_DL_LOAD_REGION_NOT_FOUND)
    {
        THROW_APP_EXCEPTION(S_DL_LOAD_REGION_NOT_FOUND, "S_DL_LOAD_REGION_NOT_FOUND \n please check partition exist or not");
    }

    return ret==S_DONE;
}

_BOOL AppCore::IsEnableRom(const std::string &rom_name, Download_Scene scene)
{
        QSettings settings(ABS_PATH_C("download_scene.ini"), QSettings::IniFormat);

        settings.beginGroup("Download");

    if(scene == WIPE_DATA)
    {
        QStringList wipeList = settings.value("WIPE_DATA").toStringList();

        if(wipeList.contains(QString(QString::fromLocal8Bit(rom_name.c_str()))))
            return _TRUE;
        else
            return _FALSE;
    }

    settings.endGroup();

    return _TRUE;
}

void AppCore::GetROMList(const APKey key,
                         std::list<ImageInfo>& image_list,
                         Download_Scene scene)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    image_list.clear();

    std::list<ROM_INFO> rom_info_list = core_handle->dl_handle()->GetAllRomInfo();

    for(std::list<ROM_INFO>::const_iterator rom_it = rom_info_list.begin();
        rom_it != rom_info_list.end(); ++rom_it)
    {
        ImageInfo info;
        info.index       = rom_it->index;
        info.enabled     = rom_it->enable && IsEnableRom(rom_it->name, scene);
        info.visible     = rom_it->item_is_visable == _TRUE;
        info.name        = rom_it->name;
        info.region_addr = rom_it->region_addr;
        info.begin_addr  = rom_it->begin_addr;
        info.end_addr    = rom_it->end_addr;
        info.location    = rom_it->filepath;
        info.region      = EnumToRomString(rom_it->part_id);
        info.partition_size = rom_it->partition_size;
        image_list.push_back(info);
    }
}

void AppCore::EnableROM(const APKey key, int rom_index, bool enable)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    LOG("enable ROM(%d): %d", rom_index, enable);
    core_handle->dl_handle()->EnableROM(rom_index, enable);
}

void AppCore::EnableROM(const APKey key, const std::string &name, bool enable)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    core_handle->dl_handle()->EnableROM(name, enable);
}

void AppCore::ClearAllRBItem(const APKey key)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    core_handle->rb_handle()->ClearAll();
}

void AppCore::AppendRBItem(const APKey key,const ReadbackItem& item)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    LOG("append RB item (%d)", item.index());
    core_handle->rb_handle()->AppendItem(item);
}

void AppCore::DeleteRBItem(const APKey key,unsigned int index)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    LOG("delete RB item (%d)", index);
    core_handle->rb_handle()->DeleteItem(index);
}

void AppCore::EnableRBItem(const APKey key,unsigned int index, bool does_enable)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    LOG("enable RB item(%d, %d)", index, does_enable);
    core_handle->rb_handle()->EnableItem(index, does_enable);
}

void AppCore::UpdateRBItem(const APKey key,const ReadbackItem& item)
{
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    LOG("update RB item (%d)", item.index());
    core_handle->rb_handle()->UpdateItem(item);
}

void AppCore::EnableLog(LoggingSwitch logging)
{
    if(logging == EnableLogging)
    {
        //Logger::DebugOn();
        FileUtils::DebugLogsOn();
        //Brom_DebugOn();
    }
    else
    {
        FileUtils::DebugLogsOff();
        //Brom_DebugOff();
    }
}

void AppCore::EnableDAChksum(const APKey key,int level)
{
   APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
   if(core_handle == NULL)
   {
       THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
   }

   LOGD("DA Chksum Level: %d.", level);
   core_handle->dl_handle()->EnableDAChksum(level);
}

void AppCore::SetChksumSetting(
    const QSharedPointer<APCore::ChksumSetting> &chksumSetting)
{
      this->chksum_setting_ =
          (QSharedPointer<APCore::ChksumSetting>)chksumSetting->Clone();
}

void AppCore::SetStopLoadFlag(const APKey key,int* stop_flag)
{
    if(!chksum_setting_.isNull())
    {
       this->chksum_setting_->set_stop_flag(stop_flag);
    }
    APCore::CoreHandle *core_handle = __POOL_OF_HANDLE(key);
    if(core_handle == NULL)
    {
        THROW_APP_EXCEPTION(FT_INVALIDE_SESSION_KEY, "");
    }

    core_handle->dl_handle()->SetStopLoadFlag(stop_flag);
}
