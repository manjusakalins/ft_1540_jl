#include "RomInfoFactory.h"
#include "../Logger/Log.h"
#include "../Resource/Handle/DLHandle.h"
#include "../Resource/CoreHandle.h"
#include "../Resource/ResourceManager.h"
#include "../BootRom/flashtool_api.h"
#include "../Conn/Connection.h"
#include "../Utility/Utils.h"

#include <QScopedPointer>


RomInfoFactory::RomInfoFactory(APKey key):
    key_(key)
{
}

#if _DEBUG
static void dump_rom_list(
        const std::list<ROM_INFO> &rom_info_list)
{
    std::list<ROM_INFO>::const_iterator it =
            rom_info_list.begin();

    while (it != rom_info_list.end())
    {
        LOGD("<%s>: 0x%08llx-0x%08llx-0x%d",
             it->name,
             it->begin_addr,
             it->end_addr,
             it->part_id);

        ++ it;
    }

}
#else
#define dump_rom_list(x)
#endif

int RomInfoFactory::GetRomList(const QSharedPointer<APCore::Connection> &conn,
                               std::list<ROM_INFO> &rom_info_list)
{
    unsigned int rom_count(0);
    DL_HANDLE_T dl_handle = GET_DL_HANDLE_T(key_);

    rom_info_list.clear();

    int ret = FlashTool_RomGetCount(conn->FTHandle(), dl_handle, &rom_count);

    if (S_DONE != ret || rom_count < 1) {
        LOGE("FlashTool_RomGetCount() failed, ret(%d).", ret);
    } else {
        QScopedPointer< ROM_INFO, QScopedPointerArrayDeleter<ROM_INFO> > p_roms(new ROM_INFO[rom_count]);
        ROM_INFO *p = p_roms.data();
        ret = FlashTool_RomGetInfoAll(conn->FTHandle(), dl_handle, p, rom_count);
        if (S_DONE == ret) {
            for (unsigned int i(0); i < rom_count; i++) {
                //To unify end_addr
                p[i].end_addr = p[i].begin_addr + p[i].partition_size;
                rom_info_list.push_back((p[i]));
            }
            dump_rom_list(rom_info_list);
        } else {
            LOGE("FlashTool_RomGetInfoAll() failed, ret(%d).", ret);
        }
    }

    return ret;
}
