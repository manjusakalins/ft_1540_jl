#include "DLHandle.h"

#include "../../Logger/Log.h"
#include "../../Err/Exception.h"
#include <dirent.h>
#include <stdio.h>
#include <algorithm>

namespace APCore
{

DLHandle::DLHandle()
{
    int ret = DL_Create(&handle_t_);
    LOG("DL_Create(%d)", ret);
}

DLHandle::~DLHandle()
{
    if(handle_t_ != NULL)
    {
        DL_Rom_UnloadAll(handle_t_);
        DL_Destroy(&handle_t_);
        handle_t_ = NULL;
    }
    LOG("dtor of DLHandle");
}

//====================
bool DLHandle::createDocList(std::string &real_dir, std::list<std::string> &doc_list)
{
    int return_code;
    DIR *dir;
    struct dirent entry;
    struct dirent *res;
	//std::list<std::string> doc_list;
    if ((dir = opendir(real_dir.c_str())) != NULL) {
        for (return_code = readdir_r(dir, &entry, &res);res != NULL && return_code == 0;return_code = readdir_r(dir, &entry, &res)) {
            if (entry.d_type != DT_DIR) {
                doc_list.push_back(std::string(entry.d_name));
				//LOGI("skip chksum for rom(%s).", (entry.d_name));
            }
        }
        closedir(dir);
    }
	return 1;
}
bool DLHandle::custom_rom_load(const std::string &scat_file)
{
	int ret = 0;
	LOGI("custom load start, ret = %d. %s: %d", ret, scat_file.c_str(), scat_file.rfind('/'));
	char dir[256];
	memset(dir, 0, 256);
	memcpy(dir, scat_file.c_str(), scat_file.rfind('/')+1);
	std::string paradir = dir;
	std::list<std::string> doc_list;
	createDocList(paradir, doc_list);

	//init a map
	std::map<std::string, char> file_name_map;
	file_name_map.insert(std::pair<std::string, char>("preloader", 'P'));
	file_name_map.insert(std::pair<std::string, char>("lk", 'U'));
	file_name_map.insert(std::pair<std::string, char>("boot", 'B'));
	file_name_map.insert(std::pair<std::string, char>("recovery", 'R'));
	file_name_map.insert(std::pair<std::string, char>("logo", 'L'));
	file_name_map.insert(std::pair<std::string, char>("custom", 'C'));
	file_name_map.insert(std::pair<std::string, char>("secro", 'X'));
	file_name_map.insert(std::pair<std::string, char>("system", 'Y'));
	file_name_map.insert(std::pair<std::string, char>("cache", 'H'));
	file_name_map.insert(std::pair<std::string, char>("userdata", 'S'));
	file_name_map.insert(std::pair<std::string, char>("tee1", 'T'));
	file_name_map.insert(std::pair<std::string, char>("tee2", 'T'));

	//4.4
	file_name_map.insert(std::pair<std::string, char>("mbr", 'M'));
	file_name_map.insert(std::pair<std::string, char>("ebr1", 'E'));
	file_name_map.insert(std::pair<std::string, char>("sec_ro", 'X'));
	file_name_map.insert(std::pair<std::string, char>("ebr2", 'G'));
	file_name_map.insert(std::pair<std::string, char>("lk", 'U'));
	file_name_map.insert(std::pair<std::string, char>("fat", 'F'));
	file_name_map.insert(std::pair<std::string, char>("usrdata", 'S'));
	file_name_map.insert(std::pair<std::string, char>("android", 'Y'));
	file_name_map.insert(std::pair<std::string, char>("bootimg", 'B'));
	file_name_map.insert(std::pair<std::string, char>("uboot", 'U'));

    std::list<ROM_INFO> rom_info_list = this->GetAllRomInfo();
    for(std::list<ROM_INFO>::const_iterator rom_it = rom_info_list.begin(); rom_it != rom_info_list.end(); ++rom_it) {
		//LOGI("TO LOAD: %s\n", rom_it->name);
		//LOGI("TO LOAD:%s %d %d %d\n", rom_it->name, rom_it->item_is_visable, rom_it->enable, rom_it->filesize);
		if (rom_it->item_is_visable == FALSE)
			continue;
		if (rom_it->filesize != 0)
			continue;
		LOGI("### start TO LOAD: %s\n", rom_it->name);

		//gen lower rom name
		std::string uper_pt_name(rom_it->name);
		std::transform(uper_pt_name.begin(), uper_pt_name.end(), uper_pt_name.begin(), ::tolower);
		//LOGI("### %d start TO LOAD: %c\n", file_name_map.size(), file_name_map[uper_pt_name]);

		for(std::list<std::string>::const_iterator doc_it = doc_list.begin(); doc_it != doc_list.end(); ++doc_it) {
			//LOGI("skip chksum for rom(%s) find :%d", doc_it->c_str(), doc_it->find(".img"));
			if ((int)doc_it->find(".img") <= 0){
				//LOGI("skip file(%s).", doc_it->c_str());
				continue;
			}

			std::string tmpdir=paradir;
			tmpdir.append(*doc_it);//full file name.
			int toload = 0;

			std::string uper_file_name;
			std::transform(doc_it->begin(), doc_it->end(), uper_file_name.begin(), ::toupper);
			//LOGI("file name upper: %s\n", uper_file_name.c_str());

			//new file map
			if (uper_file_name.c_str()[0] == 'J' && uper_file_name.c_str()[1] == file_name_map[uper_pt_name]){
				toload = 1;
				//LOGI("##########%s==>%s\n", rom_it->name, uper_file_name.c_str());
			}
			//old file map
			if (uper_file_name.c_str()[0] != 'J' && uper_file_name.c_str()[0] == file_name_map[uper_pt_name]){
				toload = 1;
				//LOGI("##########%s==>%s\n", rom_it->name, uper_file_name.c_str());
			}

			if (toload) {
				ret = DL_Rom_Load(handle_t_, rom_it->index, tmpdir.c_str());
				LOGI("%s==>%s, %d\n", rom_it->name, tmpdir.c_str(), ret);
			}

        }
		//LOGI("skip chksum for rom(%s).", rom_it->name);
	}
	return 1;
}


//====================
int DLHandle::AutoLoadRomImages(const std::string &scat_file)
{
    int ret = DL_AutoLoadRomImages(handle_t_, scat_file.c_str());
	custom_rom_load(scat_file);

    if(ret != S_DONE)
    {
        LOG("DL_AutoLoad failed, ret = %d.", ret);
    }
    return ret;
}

int DLHandle::LoadScatter(const std::string &scat_file)
{
    int ret = DL_LoadScatter(handle_t_,scat_file.c_str(),NULL);
    if(ret != S_DONE && ret != S_STOP)
    {
        LOG("DL_LoadScatter failed, ret = %d.", ret);
    }
    return ret;
}

int DLHandle::LoadSCIScatter(const std::string &sci_file)
{
    int ret = DL_LoadSCIScatter(handle_t_, sci_file.c_str());

    if(S_DONE != ret)
    {
        LOG("DL_LoadSCIScatter failed, ret = %d.", ret);
    }

    return ret;
}

bool DLHandle::IsComboSCIDL()
{
    bool isCombo_sci = false;

    int ret = DL_IsComboSCI(handle_t_, isCombo_sci);

    if(ret != S_DONE)
    {
        LOG("Get SCI Image info failed. (%d)", ret);
        return false;
    }


    return isCombo_sci;
}

bool DLHandle::GetPlatformInfo(DL_PlatformInfo &info)
{
    int ret = DL_GetPlatformInfo(handle_t_, &info);
    if(ret != S_DONE)
    {
        LOG("Get Platform Infomation failed. (%d)", ret);
        return false;
    }
    else
    {
        //add for new ARCH lib
        if(info.m_bbchip_type == UNKNOWN_BBCHIP_TYPE)
        {
            SCATTER_Head_Info shi;
            DL_GetScatterInfo(handle_t_, &shi);
            strcpy(info.m_szBBChipType, shi.platform);
        }
        //end
        LOG("Platform Info: storage(%s), chip(%s)", info.m_szStorageType, info.m_szBBChipType);
        return true;
    }
}

bool DLHandle::GetScatterVersion(char* version)
{
    int ret = DL_GetScatterVersion(handle_t_, version);

    if(ret != S_DONE)
    {
        LOG("Get Scatter version failed. (%d)", ret);
        return false;
    }
    else
    {
        LOG("Scatter version(%s)", version);
        return true;
    }
}

bool DLHandle::GetScatterInfo(SCATTER_Head_Info *p_info)
{
    int ret = DL_GetScatterInfo(handle_t_, p_info);

    if(ret != S_DONE)
    {
        LOG("Get Scatter info failed. (%d)", ret);
        return false;
    }
    else
    {
        LOG("Scatter version(%s)", p_info->version);
        return true;
    }
}

bool DLHandle::LoadRomFile(const std::string &rom_file, const int rom_index)
{
    LOG("Loading Rom File: %s",rom_file.c_str());

    int ret;
    ret = DL_Rom_Unload(handle_t_, rom_index);
    if(ret != S_DONE)
    {
        LOG("Unload before LoadRomFile failed(%d)",ret);
        return false;
    }

    ret = DL_Rom_Load(handle_t_,rom_index,rom_file.c_str());
    if(ret != S_DONE)
    {
        LOG("LoadRomFile failed(%d)",ret);
        return false;
    }
    return true;
}

int DLHandle::CheckSecUnlock(const int rom_index)
{
    return DL_CheckSecUnlockSecro(handle_t_, rom_index);
}

std::list<ROM_INFO> DLHandle::GetAllRomInfo()
{
    unsigned short rom_count;
    ROM_INFO rom_info[MAX_LOAD_SECTIONS];

    int ret = DL_GetCount(handle_t_, &rom_count);
    if(ret != S_DONE)
    {
        THROW_BROM_EXCEPTION(ret,0);
    }

    ret = DL_Rom_GetInfoAll(handle_t_, rom_info, MAX_LOAD_SECTIONS);
    if(ret != S_DONE)
    {
        THROW_BROM_EXCEPTION(ret,0);
    }

    std::list<ROM_INFO> rom_info_list;
    for(unsigned int i=0; i<rom_count; i++)
    {
        rom_info_list.push_back(rom_info[i]);
    }

    return rom_info_list;
}

void DLHandle::EnableROM(const int rom_index, bool enable)
{
    int ret = DL_Rom_SetEnableAttr(handle_t_, rom_index, enable?_TRUE:_FALSE);
    if(ret != S_DONE)
    {
        THROW_BROM_EXCEPTION(ret,0);
    }
}

void DLHandle::EnableROM(const std::string &rom_name, bool enable)
{
    int ret = DL_Rom_SetEnableAttrByRomName(handle_t_, rom_name.c_str(), enable?_TRUE:_FALSE);

    if(S_DONE != ret)
    {
        THROW_BROM_EXCEPTION(ret, 0);
    }
}

 void DLHandle::EnableDAChksum(int level)
 {
     LOGD("set DL_SetChecksumLevel(%d).", level);
     int ret = DL_SetChecksumLevel(handle_t_, level);
     if (S_DONE != ret)
     {
        LOGE("DL_SetChecksumEnable() failed, error code(%d).", ret);
        THROW_BROM_EXCEPTION(ret,0);
     }
 }

 void DLHandle::SetStopLoadFlag(int* stop_flag)
 {
     LOGD("set DL_SetStopFlag(%d).",*stop_flag);
     int ret = DL_SetStopLoadFlag(handle_t_, stop_flag);
     if (S_DONE != ret)
     {
        LOGE("DL_SetStopFlag() failed, error code(%d).", ret);
        THROW_BROM_EXCEPTION(ret,0);
     }
 }
}
