#include "MemoryTestCommand.h"
#include "../Err/Exception.h"
#include "../Resource/ResourceManager.h"
#include "../Resource/CoreHandle.h"
#include "../Resource/Handle/DLHandle.h"

#include "../Logger/Log.h"
#include <QCoreApplication>
#include <QTime>

namespace APCore
{


MemoryTestCommand *MemoryTestCommand::instance = NULL;

int __stdcall MemoryTestCommand::cb_memorytest_progress(
    unsigned int progress,void *usr_arg)
{
    FlashTool_MemoryTest_Arg *mt_arg = (FlashTool_MemoryTest_Arg *)usr_arg;
    if(mt_arg != NULL && instance != NULL)
    {
        if(mt_arg->m_test_method == HW_MEM_DATA_BUS_TEST)
        {
            instance->UpdateUI(QString().sprintf("[D%d]",progress));
        }
        else if(mt_arg->m_test_method == HW_MEM_ADDR_BUS_TEST)
        {
            instance->UpdateUI(QString().sprintf("[A%d]",progress));
        }
        else if(mt_arg->m_test_method == HW_MEM_PATTERN_TEST)
        {
          //  instance->UpdateUI(QString().sprintf("(0x%08X)",progress));
        }
    }
    return 0;
}


inline int CHECK_METEST_RESULT(int ret)
{
    LOG("checking flashtoollib ret value(%d)",ret);
    if(ret != S_DONE)
    {
        THROW_BROM_EXCEPTION(ret,0);
    }
    return ret;
}

MemoryTestCommand::MemoryTestCommand(APKey key, const MemoryTestSetting *setting)
    : ICommand(key)
{
    if(setting != NULL)
    {
        memtest_setting = *setting;
    }
    MemoryTestCommand::instance = this;
}

MemoryTestCommand::~MemoryTestCommand()
{

}

void MemoryTestCommand::exec(const QSharedPointer<Connection> &conn)
{
    if(memtest_setting.ram_test()||
            memtest_setting.emmc_flash_test()||
            memtest_setting.nand_flash_test())
    {
    conn->ConnectDA();

    //1. enable DRAM if not
    if(conn->da_report().m_ext_ram_ret != S_DONE)
    {
        LOG("ext RAM not enabled, enabling it now...");
        EnableDRAM(conn->da_report(),
            conn->FTHandle(),
            GET_DL_HANDLE_T(key_));
    }

    //2. update memory detection report
    UpdateMemoryTestDetection(conn->da_report());

    FlashTool_MemoryTest_Arg* mt_arg = memtest_arg_.raw_arg();
    mt_arg->m_cb_memorytest_progress = MemoryTestCommand::cb_memorytest_progress;
    mt_arg->m_cb_memorytest_progress_arg = mt_arg;

    FlashTool_MemoryTest_Result mt_result;
    memset(&mt_result, 0, sizeof(mt_result));

    //3. RAM test
    if(memtest_setting.ram_test())
    {
        UpdateUI("============\t RAM Test \t============\n", Qt::cyan);

        if(HW_RAM_UNKNOWN == conn->da_report().m_ext_ram_type)
        {
            UpdateUI("SKIP! External RAM was not detected!\n", Qt::red);
        }
        else
        {
            RAMTest(mt_arg, &mt_result, conn->da_report(),conn->FTHandle());
        }
    }

    //4. NAND test
    if(memtest_setting.nand_flash_test())
    {
        UpdateUI("============\t NAND Test \t============\n", Qt::cyan);

        if(S_DONE != conn->da_report().m_nand_ret)
        {
            UpdateUI("SKIP! NAND Flash was not detected!\n", Qt::red);
        }
        else
        {
            NANDTest(mt_arg, &mt_result, conn->da_report(), conn->FTHandle());
        }
    }

    //5. EMMC test
    if(memtest_setting.emmc_flash_test())
    {
        UpdateUI("============\t EMMC Test \t============\n", Qt::cyan);

        if(S_DONE != conn->da_report().m_emmc_ret)
        {
            UpdateUI("SKIP! EMMC was not detected!\n", Qt::red);
        }
        else
        {
            EMMCTest(mt_arg, &mt_result, conn->da_report(),conn->FTHandle());
        }
    }
    }
}

void MemoryTestCommand::UpdateUI(const QString& text, QColor color)
{
    MEMORY_TEST_UI_CALLBACK callback = memtest_setting.cb_gui();
    if(callback != NULL)
    {
        callback(text, color);
    }
}

void MemoryTestCommand::UpdateMemoryTestDetection(const DA_REPORT_T &da_report)
{
    UpdateUI("============ Memory Detection Report ===========\n", Qt::cyan);

    QString msg;
    //internal SRAM report
    UpdateUI("Internal RAM:\n");
    switch(da_report.m_int_sram_ret)
    {
    case S_DONE:
        QString("\tSize = 0x%1 (%2KB)\n").arg(da_report.m_int_sram_size, 8, 16)
                            .arg(da_report.m_int_sram_size/1024);

        UpdateUI(msg);
        break;

    case S_DA_RAM_FLOARTING:
        UpdateUI("\tERROR: Internal SRAM data is floating!!\n", Qt::red);
        break;

    case S_DA_RAM_UNACCESSABLE:
        UpdateUI("\tERROR: Internal SRAM data is un-accessible!!\n", Qt::red);
        break;

    case S_DA_RAM_ERROR:
    default:
        UpdateUI("\tERROR: Internal SRAM was not detected!!\n", Qt::red);
        break;
    }

    //external RAM(SRAM/DRAM) report
    UpdateUI("External RAM:\n", Qt::red);
    switch(da_report.m_ext_ram_ret)
    {
    case S_DONE:
        if(HW_RAM_DRAM == da_report.m_ext_ram_type)
        {
            UpdateUI("\tType = DRAM\n");
        }
        else
        {
            UpdateUI("\tType = SRAM\n");
        }

        if(0x20000 <= da_report.m_ext_ram_size)
        {
            msg = QString("\tSize = 0x%1 (%2MB/%3Mb)\n").arg(da_report.m_ext_ram_size, 8, 16, QLatin1Char('0'))
                    .arg(da_report.m_ext_ram_size/1024/1024)
                    .arg(da_report.m_ext_ram_size/1024*8/1024);

            UpdateUI(msg);
        }
        else
        {
            msg = QString("\tSize = 0x%1 (%2K/%3Kb)\n").arg(da_report.m_ext_ram_size, 8, 16, QLatin1Char('0'))
                    .arg(da_report.m_ext_ram_size/1024)
                    .arg(da_report.m_ext_ram_size/1024*8);
            UpdateUI(msg);
        }
        break;

    case S_DA_RAM_FLOARTING:
        UpdateUI("\tERROR: External RAM data is floating!!\n", Qt::red);
        break;

    case S_DA_RAM_UNACCESSABLE:
        UpdateUI("\tERROR: External RAM data is un-accessible!!\n", Qt::red);
        break;

    case S_DA_RAM_ERROR:
    default:
        UpdateUI("\tERROR: External RAM was not detected!!\n", Qt::red);
        break;
    }

    //NAND flash report
    UpdateUI("NAND Flash:\n");
    if(S_DONE == da_report.m_nand_ret)
    {
        msg = QString("\tDevice Id = \"%1\" (%2)\n"
                      "\tSize = 0x%3 (%4MB/%5Mb)\n").arg(GetNandFlashNameByTypeId(da_report.m_nand_flash_id))
                .arg(da_report.m_nand_flash_id)
                .arg(da_report.m_nand_flash_size, 8, 16)
                .arg(da_report.m_nand_flash_size/1024/1024)
                .arg(da_report.m_nand_flash_size/1024*8/1024);

        UpdateUI(msg);
    }
    else
    {
        UpdateUI("\tERROR: NAND Flash was not detected!\n", Qt::red);
    }

    //EMMC report
    UpdateUI("EMMC: \n");
    if(S_DONE == da_report.m_emmc_ret){
        msg = QString("\t EMMC_PART_BOOT1 \tSize = 0x%1(%2MB)\n"
                      "\t EMMC_PART_BOOT2 \tSize = 0x%3(%4MB)\n"
                      "\t EMMC_PART_RPMB \tSize = 0x%5(%6MB)\n"
                      "\t EMMC_PART_GP1 \tSize = 0x%7(%8MB)\n"
                      "\t EMMC_PART_GP2 \tSize = 0x%9(%10MB)\n"
                      "\t EMMC_PART_GP3 \tSize = 0x%11(%12MB)\n"
                      "\t EMMC_PART_GP4 \tSize = 0x%13(%14MB)\n"
                      "\t EMMC_PART_USER \tSize = 0x%15(%16MB)\n").arg(da_report.m_emmc_boot1_size, 16, 16, QLatin1Char('0'))
                .arg(da_report.m_emmc_boot1_size/1024/1024)
                .arg(da_report.m_emmc_boot2_size, 16, 16, QLatin1Char('0'))
                .arg(da_report.m_emmc_boot2_size/1024/1024)
                .arg(da_report.m_emmc_rpmb_size,16,16,QLatin1Char('0'))
                .arg(da_report.m_emmc_rpmb_size/1024/1024)
                .arg(da_report.m_emmc_gp1_size, 16,16,QLatin1Char('0'))
                .arg(da_report.m_emmc_gp1_size/1024/1024)
                .arg(da_report.m_emmc_gp2_size, 16,16,QLatin1Char('0'))
                .arg(da_report.m_emmc_gp3_size/1024/1024)
                .arg(da_report.m_emmc_gp3_size, 16,16,QLatin1Char('0'))
                .arg(da_report.m_emmc_gp3_size/1024/1024)
                .arg(da_report.m_emmc_gp4_size, 16,16,QLatin1Char('0'))
                .arg(da_report.m_emmc_gp4_size/1024/1024)
                .arg(da_report.m_emmc_ua_size, 16,16, QLatin1Char('0'))
                .arg(da_report.m_emmc_ua_size/1024/1024);

        UpdateUI(msg);
    }
    else
    {
        UpdateUI("\tERROR: EMMC was not detected!\n", Qt::red);
    }
}


void MemoryTestCommand::EnableDRAM(DA_REPORT_T &da_report,
                FLASHTOOL_API_HANDLE_T ft_handle,
                DL_HANDLE_T dl_handle)
{
    int ret;
    DRAM_SETTING dram_setting;

#if 0
    unsigned int nand_id = ((unsigned int)da_report.m_nand_flash_dev_code_1)<<8 |
                           ((unsigned int)da_report.m_nand_flash_dev_code_2);
#endif

    ret = DL_GetDRAMSetting(dl_handle, &dram_setting, &da_report);
    if(S_DONE != ret)
    {
        THROW_BROM_EXCEPTION(ret, 0);
    }

    FlashTool_EnableDRAM_Result dram_result;
    ret = FlashTool_EnableDRAM(ft_handle, dram_setting, &dram_result);
    if(S_DONE != ret)
    {
        THROW_BROM_EXCEPTION(ret, 0);
    }

    //update connect result
    da_report.m_ext_ram_ret  = dram_result.m_ext_ram_ret;
    da_report.m_ext_ram_type = dram_result.m_ext_ram_type;
    da_report.m_ext_ram_chip_select = dram_result.m_ext_ram_chip_select;
    da_report.m_ext_ram_size = dram_result.m_ext_ram_size;
}

void MemoryTestCommand::RAMTest(FlashTool_MemoryTest_Arg *mt_arg,
             FlashTool_MemoryTest_Result *mt_result,
             const DA_REPORT_T &da_report,
             FLASHTOOL_API_HANDLE_T ft_handle)
{
    int ret = S_DONE;

    if(memtest_setting.ram_data_bus_test())
    {
        UpdateUI("Data Bus Test: \n", Qt::black);
        ret = RAMDataBusTest(mt_arg,mt_result,da_report, ft_handle);
        if(ret == S_DONE)
        {
            UpdateUI("OK!!\n", Qt::blue);
        }
        else
        {
            UpdateUI("Failed!!\n", Qt::red);
        }
    }

    if(memtest_setting.ram_addr_bus_test())
    {
        UpdateUI("Address Bus Test:\n", Qt::black);
        ret = RAMAddrBusTest(mt_arg, mt_result,da_report,ft_handle);
        if(ret == S_DONE)
        {
            UpdateUI("OK!!\n", Qt::blue);
        }
        else
        {
            UpdateUI("Failed!!\n", Qt::red);
        }
    }

    if(memtest_setting.ram_dedicated_pattern_test())
    {
        // define the pattern set for pattern test
        unsigned int test_pattern[9] = {0x44332211, 0xA5A5A5A5, 0xA5A5A500,
                               0xA500A500, 0xA5000000, 0x00000000,
                               0xFFFF0000, 0xFFFFFFFF, 0x5A5A5A5A};

        UpdateUI("RAM Pattern Test :\nWriting ...", Qt::black);
        ret = RAMPatternTest( mt_arg,mt_result,test_pattern,8,
                              da_report, ft_handle);
        if(ret == S_DONE)
        {
            UpdateUI("OK!!\n", Qt::blue);
        }
        else
        {
            UpdateUI("Failed!!\n", Qt::red);
        }
    }

    if(memtest_setting.ram_inc_dec_pattern_test())
    {
        UpdateUI("Increment/Decrement Test:\nWriting...", Qt::black);
        ret = RAMIncDecTest(mt_arg,mt_result, da_report, ft_handle);
    }
}


int MemoryTestCommand::RAMDataBusTest(FlashTool_MemoryTest_Arg *mt_arg,
                    FlashTool_MemoryTest_Result *mt_result,
                    const DA_REPORT_T &da_report,
                    FLASHTOOL_API_HANDLE_T ft_handle)
{
    // DRAM or SRAM
    if ( HW_RAM_DRAM == da_report.m_ext_ram_type ) {
        mt_arg->m_memory_device = HW_MEM_EXT_DRAM;
    } else {
        mt_arg->m_memory_device = HW_MEM_EXT_SRAM;
    }

    // Test method : data bus
    mt_arg->m_test_method = HW_MEM_DATA_BUS_TEST;

    return CHECK_METEST_RESULT(FlashTool_MemoryTest(ft_handle, mt_arg, mt_result));
}

int MemoryTestCommand::RAMAddrBusTest(FlashTool_MemoryTest_Arg *mt_arg,
                    FlashTool_MemoryTest_Result *mt_result,
                    const DA_REPORT_T &da_report,
                    FLASHTOOL_API_HANDLE_T ft_handle)
{
    // DRAM or SRAM
    if ( HW_RAM_DRAM == da_report.m_ext_ram_type ) {
        mt_arg->m_memory_device = HW_MEM_EXT_DRAM;
    } else {
        mt_arg->m_memory_device = HW_MEM_EXT_SRAM;
    }

    // Test method : address bus
    mt_arg->m_test_method = HW_MEM_ADDR_BUS_TEST;

    return CHECK_METEST_RESULT(FlashTool_MemoryTest(ft_handle, mt_arg, mt_result));
}

int MemoryTestCommand::RAMPatternTest(FlashTool_MemoryTest_Arg *mt_arg,
                    FlashTool_MemoryTest_Result *mt_result,
                    unsigned int pattern_set[],
                    int pattern_size,
                    const DA_REPORT_T &da_report,
                    FLASHTOOL_API_HANDLE_T ft_handle)
{
    int ret = S_DONE;
    QString msg;

    for ( int i = 0 ; i < pattern_size ; i++ )
    {
        UpdateUI(msg.sprintf("\t0x%08X, ", pattern_set[i]),  Qt::black);
        ret = RAMOnePatternTest( mt_arg, mt_result, pattern_set[i], da_report, ft_handle);
        if(ret != S_DONE)
        {
            LOG("RAMOnePatternTest failed(%d)", ret);
            break;
        }
    }

    return ret;
}

int MemoryTestCommand::RAMOnePatternTest(FlashTool_MemoryTest_Arg *mt_arg,
                       FlashTool_MemoryTest_Result *mt_result,
                       unsigned int pattern,
                       const DA_REPORT_T &da_report,
                       FLASHTOOL_API_HANDLE_T ft_handle)
{
    // DRAM or SRAM
    if ( HW_RAM_DRAM == da_report.m_ext_ram_type ) {
        mt_arg->m_memory_device = HW_MEM_EXT_DRAM;
    } else {
        mt_arg->m_memory_device = HW_MEM_EXT_SRAM;
    }
    // Test method : pattern test
    mt_arg->m_test_method  = HW_MEM_PATTERN_TEST;
    // Start addess
    mt_arg->m_start_addr   = 0;
    // Size
    mt_arg->m_size         = da_report.m_ext_ram_size;
    // Memory I/O
    mt_arg->m_memory_io    = HW_MEM_IO_32BIT;
    // Assign the Pattern
    mt_arg->m_test_pattern = pattern;

    return CHECK_METEST_RESULT(FlashTool_MemoryTest(ft_handle, mt_arg, mt_result));
}

int MemoryTestCommand::RAMIncDecTest(FlashTool_MemoryTest_Arg *mt_arg,
                   FlashTool_MemoryTest_Result *mt_result,
                   const DA_REPORT_T &da_report,
                   FLASHTOOL_API_HANDLE_T ft_handle)
{
    // Memory 32 bits I/O increasement/decreasement pattern test
    int ret = RAMIncDecTestIO( mt_arg, mt_result, HW_MEM_IO_32BIT, da_report, ft_handle);
    if(ret == S_DONE)
    {
        UpdateUI("OK!!\n", Qt::blue);
    }
    else
    {
        UpdateUI("Failed!!\n", Qt::red);
    }

    return ret;
}

int MemoryTestCommand::RAMIncDecTestIO(FlashTool_MemoryTest_Arg *mt_arg,
                     FlashTool_MemoryTest_Result *mt_result,
                     HW_MemoryIO_E io,
                     const DA_REPORT_T &da_report,
                     FLASHTOOL_API_HANDLE_T ft_handle)
{
    // DRAM or SRAM
    if ( HW_RAM_DRAM == da_report.m_ext_ram_type ) {
        mt_arg->m_memory_device = HW_MEM_EXT_DRAM;
    } else {
        mt_arg->m_memory_device = HW_MEM_EXT_SRAM;
    }

    // Test method : increment/ decrement test
    mt_arg->m_test_method = HW_MEM_INC_DEC_TEST;
    // Start addess
    mt_arg->m_start_addr  = 0;
    // Size
    mt_arg->m_size        = da_report.m_ext_ram_size;
    // IO
    mt_arg->m_memory_io   = io;

    return CHECK_METEST_RESULT(FlashTool_MemoryTest(ft_handle, mt_arg, mt_result));
}

void MemoryTestCommand::NANDTest(FlashTool_MemoryTest_Arg *mt_arg,
              FlashTool_MemoryTest_Result *mt_result,
              const DA_REPORT_T &da_report,
              FLASHTOOL_API_HANDLE_T ft_handle)
{
    int ret = S_DONE;

    // NAND Flash Test : Pattern Test
    if(memtest_setting.nand_flash_test())
    {
        UpdateUI("Pattern Test (0x5A):\r\n", Qt::black);
        ret = NANDPatternTest( mt_arg, mt_result, da_report, ft_handle);
        if(ret == S_DONE)
        {
            UpdateUI("OK!!\n", Qt::blue);
        }
        else
        {
            UpdateUI("Failed!!\n", Qt::red);
        }
    }
}

int MemoryTestCommand::NANDPatternTest(FlashTool_MemoryTest_Arg *mt_arg,
                     FlashTool_MemoryTest_Result *mt_result,
                     const DA_REPORT_T &da_report,
                     FLASHTOOL_API_HANDLE_T ft_handle)
{
    // memory device : nand
    mt_arg->m_memory_device = HW_MEM_NAND;
    // Test method
    mt_arg->m_test_method   = HW_MEM_PATTERN_TEST;
    // Start address
    mt_arg->m_start_addr    = 0;
    // Size
    mt_arg->m_size          = da_report.m_nand_flash_size;
    // Memory I/O
    mt_arg->m_memory_io     = HW_MEM_IO_8BIT;
    // Test pattern
    mt_arg->m_test_pattern  = 0x5A;

    return CHECK_METEST_RESULT(FlashTool_MemoryTest(ft_handle, mt_arg, mt_result));
}

void MemoryTestCommand::EMMCTest(FlashTool_MemoryTest_Arg *mt_arg,
              FlashTool_MemoryTest_Result *mt_result,
              const DA_REPORT_T &da_report,
              FLASHTOOL_API_HANDLE_T ft_handle)
{
    int ret = S_DONE;

    //Pattern Test
    if (memtest_setting.emmc_flash_test() && memtest_setting.emmc_dedicated_pattern_test())
    {
        UpdateUI("EMMC Pattern Test(0x5A5A):\n");
        ret = EMMCPatternTest( mt_arg, mt_result, da_report, ft_handle);
        if(ret == S_DONE)
        {
            UpdateUI("OK!!\n", Qt::blue);
        }
        else
        {
            UpdateUI("Failed!!\n", Qt::red);
        }
    }
}

int MemoryTestCommand::EMMCPatternTest(FlashTool_MemoryTest_Arg *mt_arg,
                     FlashTool_MemoryTest_Result *mt_result,
                     const DA_REPORT_T &da_report,
                     FLASHTOOL_API_HANDLE_T ft_handle)
{
    // memory device : NOR
    mt_arg->m_memory_device = HW_MEM_EMMC;
    // Test method
    mt_arg->m_test_method   = HW_MEM_PATTERN_TEST;
    // Start address
    mt_arg->m_start_addr    = 0;
    // Size
    mt_arg->m_size          = da_report.m_emmc_ua_size;
    // Memory I/O
    mt_arg->m_memory_io     = HW_MEM_IO_16BIT;
    // Test pattern
    mt_arg->m_test_pattern  = 0x5A5A;

    return CHECK_METEST_RESULT(FlashTool_MemoryTest(ft_handle, mt_arg, mt_result));
}
/*
void MemoryTestCommand::BatteryTest(const QSharedPointer<Connection> &conn)
{
    UpdateUI("============\t Battery Test \t============\n", Qt::cyan);
    if(memtest_setting.is_with_battery())
    {
        UpdateUI("======\t With Battery \t======\n", Qt::darkBlue);
        BatteryTestAndSleep(conn, FORCE_CHARGE_OFF);
    }

    if(memtest_setting.is_without_battery())
    {
        UpdateUI("======\t Without Battery \t======\n", Qt::darkBlue);
        BatteryTestAndSleep(conn, FORCE_CHARGE_ON);
    }

    if(memtest_setting.is_auto_detect())
    {
        UpdateUI("======\t Auto Detect Battery \t======\n", Qt::darkBlue);
        BatteryTestAndSleep(conn, FORCE_CHARGE_AUTO);
    }

    UpdateUI("============\t Battery Test Done!! \t============\n", Qt::cyan);
}

void MemoryTestCommand::BatteryTestAndSleep(const QSharedPointer<Connection> &conn, const int type)
{
    LOG("BatteryTest, type = %d!", type);
    if(conn->isDAConnected())
    {
        QSharedPointer<APCore::WatchDogSetting> ws(new APCore::WatchDogSetting());
        ws->CreateCommand(key_)->exec(conn);

        conn->Disconnect();

        QTime time;
        time.start();
        while(time.elapsed() < 500)
        {
           QCoreApplication::processEvents();
        }
        LOGD("BatteryTest disconnect,wait 500ms end!");

    }
    conn->setPowerType(type);
    conn->ConnectDA();
    GetVoltage(conn->FTHandle());
}

void MemoryTestCommand::GetVoltage(FLASHTOOL_API_HANDLE_T ft_handle)
{
    FlashTool_CheckBattery_Result bt_result;
    memset(&bt_result, 0, sizeof(bt_result));
    int ret = FlashTool_Check_Battery(ft_handle,NULL, &bt_result);
    LOG("ret = %d, GetVoltage = %d!", ret, bt_result.bat_voltage_value);
    if(ret == S_UNSUPPORTED_OPERATION)
    {
        UpdateUI("Battery is nonexistence OR Function is not supported!\n", Qt::darkYellow);
    }
    else if ( ret == S_DONE)
    {
        UpdateUI(QString().sprintf("Battery is %d\n", bt_result.bat_voltage_value), Qt::darkBlue);
    }
    else
    {
        CHECK_METEST_RESULT(ret);
    }
}*/

}
