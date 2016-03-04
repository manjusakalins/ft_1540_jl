#include "MemoryTestWidget.h"

#include "ui_MemoryTestWidget.h"
#include "MainWindow.h"
#include "MainController.h"
#include "ICallback.h"
#include "../../Utility/constString.h"
#include "../../Host/Inc/RuntimeMemory.h"

MemoryTestWidget::MemoryTestWidget(QTabWidget *parent, MainWindow *window) :
    TabWidgetBase(4, tr("Memory&Test"), parent),
    main_window_(window),
    ui_(new Ui::MemoryTestWidget)
{
    ui_->setupUi(this);

    main_window_->main_controller()->GetPlatformSetting()->addObserver(this);
    main_window_->scatter_observer()->addObserver(this);

    connect(main_window_, SIGNAL(signal_MemoryTestCallback(const QString &,QColor)),
            this, SLOT(slot_MemoryTestCallback(const QString &,QColor)));

    QRegExp regExpHex("0x[\\da-fA-F]{16}");

    ui_->lineEdit_address->setValidator(new QRegExpValidator(regExpHex, ui_->lineEdit_address));
    ui_->lineEdit_length->setValidator(new QRegExpValidator(regExpHex, ui_->lineEdit_length));


    QStringList regionList;

    regionList << "EMMC_BOOT_1" << "EMMC_BOOT_2" << "EMMC_RPMB" << "EMMC_USER";

    ui_->comboBox_region->addItems(regionList);
    ui_->comboBox_region->setVisible(false);

    ui_->label_region->setVisible(false);
    ui_->label_address->setVisible(false);
    ui_->lineEdit_address->setVisible(false);
    ui_->lineEdit_length->setVisible(false);
    ui_->label_length->setVisible(false);

    //hide flash test in normal_mode
    HideFlashTestItems();
}

MemoryTestWidget::~MemoryTestWidget()
{
    delete ui_;
}

void MemoryTestWidget::LockOnUI()
{
    ui_->groupBox->setEnabled(false);
    ui_->toolButton_start->setEnabled(false);
    ui_->toolButton_stop->setEnabled(true);
}

void MemoryTestWidget::DoFinished()
{
    ui_->groupBox->setEnabled(true);
    ui_->toolButton_start->setEnabled(true);
    ui_->toolButton_stop->setEnabled(false);
}

void MemoryTestWidget::UpdateUI()
{
    ui_->retranslateUi(this);
    ui_->SELECT_ALL->setShortcut(Qt::CTRL + Qt::Key_A);

}

void MemoryTestWidget::SetTabLabel(QTabWidget *tab_widget, int index)
{
    QString label = LoadQString(LANGUAGE_TAG, IDS_STRING_MEMORY_TEST);

    tab_widget->setTabText(index, label);
}

void MemoryTestWidget::SetShortCut(int cmd, const QString &shortcut)
{
    switch(cmd)
    {
    case MT_START:
        ui_->toolButton_start->setShortcut(shortcut);
        ui_->toolButton_start->setToolTip(shortcut);
        break;

    case MT_STOP:
        ui_->toolButton_stop->setShortcut(shortcut);
        ui_->toolButton_stop->setToolTip(shortcut);
        break;
    }
}

void MemoryTestWidget::OnScatterChanged(bool showRegion)
{
    // hide, maybe modify future
    showRegion = false;
    ui_->comboBox_region->setVisible(showRegion);
    ui_->label_region->setVisible(showRegion);
}

void MemoryTestWidget::on_toolButton_start_clicked()
{
    if(main_window_->ValidateBeforeMemoryTest())
    {
        main_window_->main_controller()->SetPlatformSetting();
        main_window_->main_controller()->SetConnSetting(main_window_->CreateConnSetting());
        main_window_->main_controller()->QueueAJob(main_window_->CreateMemtestSetting());
        main_window_->main_controller()->StartExecuting(
                    new SimpleCallback<MainWindow>(main_window_,&MainWindow::DoFinished));
        main_window_->LockOnUI();
        main_window_->GetOkDialog()->setWindowTitle(LoadQString(main_window_->GetLanguageTag(), IDS_STRING_MEMORY_TEST_OK));
    }
}

void MemoryTestWidget::on_SELECT_ALL_clicked()
{
#define RAM_TEST_CHECKED \
    (ui_->RAM_TEST->isChecked()    \
         && ui_->RAM_DataBusTest->isChecked()    \
         && ui_->RAM_AddrBusTest->isChecked()    \
         && ui_->RAM_DedicatedPatternTest->isChecked()    \
         && ui_->RAM_IncDecPatternTest->isChecked())

#define NAND_FLASH_TEST_CHECKED \
    (ui_->NAND_FLASH_TEST->isHidden()    \
        || (ui_->NAND_FLASH_TEST->isChecked()    \
            && ui_->NAND_DedicatedPatternTest->isChecked()))

#define EMMC_FLASH_TEST_CHECKED \
    (ui_->EMMC_FLASH_TEST->isHidden()    \
        || (ui_->EMMC_FLASH_TEST->isChecked()    \
            && ui_->EMMC_DedicatedPatternTest->isChecked()))

    if ( RAM_TEST_CHECKED
         && NAND_FLASH_TEST_CHECKED
         && EMMC_FLASH_TEST_CHECKED )
    {
        selectAllTestItems(false);
    }
    else
    {
        selectAllTestItems(true);
    }
}

bool MemoryTestWidget::isSelectNothing()
{
    bool isAdvanceMode = main_window_->main_controller()->advance_on();
    if(isAdvanceMode)
    {
        bool isEMMCOperation = main_window_->main_controller()->GetPlatformSetting()->getFlashToolStorageConfig().IsEMMCOperation();
        if(isEMMCOperation)
            return (ui_->EMMC_FLASH_TEST->isChecked() == false
                 && ui_->RAM_TEST->isChecked() == false);

        bool isNANDOperation = main_window_->main_controller()->GetPlatformSetting()->getFlashToolStorageConfig().IsNandOperation();
        if(isNANDOperation)
            return (ui_->NAND_FLASH_TEST->isChecked() == false
                 && ui_->RAM_TEST->isChecked() == false);
    }
    else
    {
        return ui_->RAM_TEST->isChecked() == false;
    }

    return true;
}

void MemoryTestWidget::ShowEMMCTest(bool isShow)
{
    ui_->EMMC_FLASH_TEST->setVisible(isShow);
    ui_->EMMC_DedicatedPatternTest->setVisible(isShow);
}

void MemoryTestWidget::ShowNANDTest(bool isShow)
{
    ui_->NAND_FLASH_TEST->setVisible(isShow);
    ui_->NAND_DedicatedPatternTest->setVisible(isShow);
}

void MemoryTestWidget::selectAllTestItems(bool select)
{
    ui_->RAM_TEST->setChecked(select);
    ui_->RAM_DataBusTest->setChecked(select);
    ui_->RAM_AddrBusTest->setChecked(select);
    ui_->RAM_DedicatedPatternTest->setChecked(select);
    ui_->RAM_IncDecPatternTest->setChecked(select);

    ui_->NAND_FLASH_TEST->setChecked(select);
    ui_->NAND_DedicatedPatternTest->setChecked(select);

    ui_->EMMC_FLASH_TEST->setChecked(select);
    ui_->EMMC_DedicatedPatternTest->setChecked(select);
}

QSharedPointer<APCore::MemoryTestSetting> MemoryTestWidget::CreateMemtestSetting()
{
    QSharedPointer<APCore::MemoryTestSetting> setting(new APCore::MemoryTestSetting());

    setting->set_ram_test(ui_->RAM_TEST->isChecked());
    setting->set_ram_data_bus_test(ui_->RAM_DataBusTest->isChecked());
    setting->set_ram_addr_bus_test(ui_->RAM_AddrBusTest->isChecked());
    setting->set_ram_dedicated_pattern_test(ui_->RAM_DedicatedPatternTest->isChecked());
    setting->set_ram_inc_dec_pattern_test(ui_->RAM_IncDecPatternTest->isChecked());

    if(!ui_->NAND_FLASH_TEST->isHidden())
    {
        setting->set_nand_flash_test(ui_->NAND_FLASH_TEST->isChecked());
        setting->set_nand_dedicated_pattern_test(ui_->NAND_DedicatedPatternTest->isChecked());
    }

    if(!ui_->EMMC_FLASH_TEST->isHidden())
    {
        setting->set_emmc_flash_test(ui_->EMMC_FLASH_TEST->isChecked());
        setting->set_emmc_dedicated_pattern_test(ui_->EMMC_DedicatedPatternTest->isChecked());
    }

    return setting;
}

void MemoryTestWidget::slot_MemoryTestCallback(const QString &msg, QColor color)
{
    ui_->MemoryTestReport->setUpdatesEnabled(FALSE);
    ui_->MemoryTestReport->setTextColor(color);
    ui_->MemoryTestReport->append(msg);
    /* mtk10304[nailiang.song] data_bus_test + addr_bus_test, hit append(msg) 93 times, then tool crash,
    comment qApp->processEvents(), tool NOT crash */
    //qApp->processEvents();
    ui_->MemoryTestReport->setUpdatesEnabled(TRUE);
}

void MemoryTestWidget::on_toolButton_stop_clicked()
{
    main_window_->main_controller()->StopByUser();
}

void MemoryTestWidget::HideFlashTestItems()
{
    ShowEMMCTest(false);
    ShowNANDTest(false);
}

void MemoryTestWidget::onPlatformChanged()
{
    HW_StorageType_E storage =
              main_window_->main_controller()->GetPlatformSetting()->getFlashToolStorageConfig().GetStorageType();

    if(main_window_->main_controller()->advance_on())
    {
        ShowEMMCTest(storage == HW_STORAGE_EMMC);
        ShowNANDTest(storage == HW_STORAGE_NAND);
    }
}

