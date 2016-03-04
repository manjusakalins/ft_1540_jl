#include "WriteMemoryWidget.h"
#include "ui_WriteMemoryWidget.h"
#include "MainWindow.h"
#include "MainController.h"
#include "ICallback.h"
#include "../../Utility/FileUtils.h"
#include "../../Utility/constString.h"
#include "../../Host/Inc/RuntimeMemory.h"

#include <QFile>

WriteMemoryWidget::WriteMemoryWidget(QTabWidget *parent, MainWindow *window) :
    TabWidgetBase(5, tr("Write &Memory"), parent),
    main_window_(window),
    ui_(new Ui::WriteMemoryWidget)
{
    ui_->setupUi(this);

    QRegExp regExpHex("0x[\\da-fA-F]{16}");
    ui_->lineEdit_address->setValidator(new QRegExpValidator(regExpHex, ui_->lineEdit_address));

    main_window_->scatter_observer()->addObserver(this);

    QStringList regionList;

    regionList << "EMMC_BOOT_1" << "EMMC_BOOT_2" << "EMMC_RPMB" << "EMMC_USER";

    ui_->comboBox_region->addItems(regionList);
    ui_->comboBox_region->setVisible(false);

    ui_->label_region->setVisible(false);
    ui_->comboBox_region->setCurrentIndex(3);
}

WriteMemoryWidget::~WriteMemoryWidget()
{
    delete ui_;
}

void WriteMemoryWidget::LockOnUI()
{
    ui_->lineEdit_address->setEnabled(false);
    ui_->lineEdit_FilePath->setEnabled(false);
    ui_->comboBox_region->setEnabled(false);
    ui_->toolButton_openFile->setEnabled(false);
    ui_->toolButton_writeMemory->setEnabled(false);
    ui_->toolButton_stop->setEnabled(true);
}

void WriteMemoryWidget::DoFinished()
{
    ui_->lineEdit_address->setEnabled(true);
    ui_->lineEdit_FilePath->setEnabled(true);
    ui_->comboBox_region->setEnabled(true);
    ui_->toolButton_openFile->setEnabled(true);
    ui_->toolButton_writeMemory->setEnabled(true);
    ui_->toolButton_stop->setEnabled(false);
}

void WriteMemoryWidget::UpdateUI()
{
    ui_->retranslateUi(this);
}

void WriteMemoryWidget::SetTabLabel(QTabWidget *tab_widget, int index)
{
    QString label = LoadQString(LANGUAGE_TAG, IDS_STRING_WRITE_MEMORY);

    tab_widget->setTabText(index, label);
}

void WriteMemoryWidget::SetShortCut(int cmd, const QString &shortcut)
{
    switch(cmd)
   {
    case WM_START:
        ui_->toolButton_writeMemory->setShortcut(shortcut);
        ui_->toolButton_writeMemory->setToolTip(shortcut);
        break;

    case WM_STOP:
        ui_->toolButton_stop->setShortcut(shortcut);
        ui_->toolButton_stop->setToolTip(shortcut);
        break;
    }
}

void WriteMemoryWidget::OnScatterChanged(bool showRegion)
{
    ui_->label_region->setVisible(showRegion);
    ui_->comboBox_region->setVisible(showRegion);
}

void WriteMemoryWidget::on_toolButton_writeMemory_clicked()
{
    if(main_window_->ValidateBeforeMemoryTest())
    {
        main_window_->main_controller()->SetPlatformSetting();
        main_window_->main_controller()->SetConnSetting(main_window_->CreateConnSetting());
        main_window_->main_controller()->QueueAJob(main_window_->CreateWriteMemorySetting());
        main_window_->main_controller()->StartExecuting(
                    new SimpleCallback<MainWindow>(main_window_,&MainWindow::DoFinished));
        main_window_->LockOnUI();
        main_window_->GetOkDialog()->setWindowTitle(LoadQString(LANGUAGE_TAG,IDS_STRING_WRITE_MEMORY_OK));
    }
}

void WriteMemoryWidget::on_toolButton_stop_clicked()
{
    main_window_->main_controller()->StopByUser();
}

void WriteMemoryWidget::on_toolButton_openFile_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(
                this,
                tr("Open Raw Data File"),
                FileUtils::GetAppDirectory().c_str(),
                tr("All Files (*.*)"));

    if(!file_name.isEmpty())
    {
        QFileInfo fileInfo(file_name);
        file_length_ = fileInfo.size();

        ui_->lineEdit_FilePath->setText(file_name);
    }
    else
    {
        file_length_ = 0;
        ui_->lineEdit_FilePath->setText("");
    }
}

U64 WriteMemoryWidget::getBeginAddress() const
{
    QString addr_str = ui_->lineEdit_address->text();
    bool convert_ok = true;
    U64 addr = addr_str.toULongLong(NULL, 16);

    return convert_ok ? addr : 0;
}

QSharedPointer<APCore::WriteMemorySetting> WriteMemoryWidget::CreateWriteMemSetting()
{
    QSharedPointer<APCore::WriteMemorySetting> setting(new APCore::WriteMemorySetting());

    setting->set_address(getBeginAddress());
    setting->set_input_mode(InputMode_FromFile);
    setting->set_input(ui_->lineEdit_FilePath->text().toStdString());
    setting->set_input_length(file_length_);

    if(ui_->comboBox_region->isVisible())
    {
        EMMC_Part_E part_id = EMMC_PART_UNKNOWN;
        switch(ui_->comboBox_region->currentIndex())
        {
        case 0:
            part_id = EMMC_PART_BOOT1;
            break;

        case 1:
            part_id = EMMC_PART_BOOT2;
            break;

        case 2:
            part_id = EMMC_PART_RPMB;
            break;

        case 3:
            part_id = EMMC_PART_USER;
            break;
        }

        setting->set_part_id(part_id);
    }

    return setting;
}
