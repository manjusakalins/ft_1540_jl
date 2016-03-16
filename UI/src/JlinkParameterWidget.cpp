#include "JlinkParameterWidget.h"
#include "ui_JlinkParameterWidget.h"
#include "MainWindow.h"
#include "CheckHeader.h"
#include "ICallback.h"
#include "ProcessingDialog.h"
#include "../../Utility/Utils.h"
#include "../../Utility/FileUtils.h"
#include "../../Utility/constString.h"
#include "../../Utility/IniItem.h"
#include "../../Host/Inc/RuntimeMemory.h"
#include "../../Utility/version.h"
#include "hexedit_src/qhexedit.h"

#include <QtDebug>
#include <QtGui>
#include <QTextCodec>

#define JP_FILE_NAME ("./jlinkParam.bin")
#define JP_FILE_LEN (512)
#define JP_PART_NAME ("proinfo")

JlinkParameterWidget::JlinkParameterWidget(QTabWidget *parent, MainWindow *window) :
    TabWidgetBase(2, tr("&Download"), parent),
    main_window_(window),
    ui_(new Ui::JlinkParameterWidget),
    wm_arg(new WriteFlashMemoryParameter),
    proinfo_addr(0)
{
    ui_->setupUi(this);	

	//hexedit init
    hexEdit = new QHexEdit;
	hexEdit->setMaximumSize(430,80);
	jlinkParam.fill('0', 20);
	hexEdit->setData(jlinkParam);
	connect(hexEdit, SIGNAL(overwriteModeChanged(bool)), this, SLOT(setOverwriteMode(bool)));
    connect(hexEdit, SIGNAL(dataChanged()), this, SLOT(dataChanged()));
	connect(hexEdit, SIGNAL(currentAddressChanged(qint64)), this, SLOT(setAddressChanged(qint64)));
	ui_->horizontalLayout->addWidget(hexEdit);
	info_label = new QLabel(tr("hwinfo[0]: Project"));
	info_label->setMinimumSize(350,80);
	info_label->setAlignment(Qt::AlignTop|Qt::AlignLeft);
	info_label->show();
	ui_->horizontalLayout->addWidget(info_label);	
	ui_->pushButton_WriteParam->setEnabled(false);//init the write can not func

    main_window_->main_controller()->GetPlatformSetting()->addObserver(this);
    main_window_->scatter_observer()->addObserver(this);
}

JlinkParameterWidget::~JlinkParameterWidget()
{
    //IniItem item("history.ini", "RecentOpenFile", "scatterHistory");
    //item.SaveStringListValue(scatterFile_historyList_);

    delete ui_;
}

//============base ui====================
void JlinkParameterWidget::LockOnUI()
{
//    ui_->groupBox->setEnabled(false);
    ui_->pushButton_ReadParam->setEnabled(false);
	ui_->pushButton_WriteParam->setEnabled(false);
    ui_->pushButton_stop->setEnabled(true);
}

void JlinkParameterWidget::DoFinished()
{
	jlinkParamReadBinData();
    ui_->pushButton_ReadParam->setEnabled(true);
	ui_->pushButton_WriteParam->setEnabled(true);
    ui_->pushButton_stop->setEnabled(false);

}

void JlinkParameterWidget::UpdateUI()
{
}
void JlinkParameterWidget::SetTabLabel(QTabWidget *tab_widget, int index)
{
    QString label = tr("jlink parameter");
    tab_widget->setTabText(index, label);
}
void JlinkParameterWidget::SetShortCut(int cmd, const QString &shortcut)
{
}


void JlinkParameterWidget::onPlatformChanged()
{
    storage_ = main_window_->main_controller()->GetPlatformSetting()->getFlashToolStorageConfig().GetStorageType();
}

void JlinkParameterWidget::OnScatterChanged(bool showRegion)
{	
	ui_->pushButton_WriteParam->setEnabled(false);
	jlinkParam.fill('0', 20);
	hexEdit->setData(jlinkParam);
}

//=================================================
int JlinkParameterWidget::ArgFlashToolWriteMemory(int n, WriteFlashMemoryParameter *wm_arg, U64 address, char * data, unsigned int length )
{
	memset(wm_arg,  0, sizeof(WriteFlashMemoryParameter));
	wm_arg->m_flash_type             = storage_;
	wm_arg->m_address                = address;
	wm_arg->m_addressing_mode        = AddressingMode_LogicalAddress;
	//g_WM_address;    // write start address
	wm_arg->m_container_length       = 0x20000;//write spare length;In Blocks,only valid for NAND.
	wm_arg->m_input_mode             = InputMode_FromBuffer;
	wm_arg->m_program_mode           = ProgramMode_PageOnly;
	wm_arg->m_input                  = data;//
	// if m_input is a file, the length is zero because file length will be used as data length;
	// if m_input is a buffer, the length will be used as data length.
	wm_arg->m_input_length           = length;

	wm_arg->m_cb_progress           = NULL;
	wm_arg->m_cb_progress_arg       = (void*)n;
	//wm_arg->m_part_id		= 0;
	return 0;
}

QSharedPointer<APCore::WriteMemorySetting> JlinkParameterWidget::CreateJlinkParamWriteMemSetting()
{
    QSharedPointer<APCore::WriteMemorySetting> setting(new APCore::WriteMemorySetting());

    setting->set_address(proinfo_addr);
    setting->set_input_mode(InputMode_FromFile);
    setting->set_input(std::string(JP_FILE_NAME));
    setting->set_input_length(JP_FILE_LEN);
	setting->set_part_id(EMMC_PART_USER);

    return setting;
}

void JlinkParameterWidget::on_pushButton_WriteParam_clicked()
{
	QBuffer tmpbuffer;
	tmpbuffer.setBuffer(&jlinkParam);
	hexEdit->write(tmpbuffer);
	LOGI("########## %s %d ########## 0x%x 0x%x\n", __func__, __LINE__, jlinkParam.at(0), hexEdit->dataAt(0,1).at(0));
	jlinkParamWriteBinData();

	if (main_window_->IsScatterFileLoad()) {
		main_window_->main_controller()->SetPlatformSetting();
		main_window_->main_controller()->SetConnSetting(main_window_->CreateConnSetting());
		main_window_->main_controller()->QueueAJob(main_window_->CreateJlinkParamWriteMemorySetting());
		if(!ToolInfo::IsCustomerVer())
		    main_window_->main_controller()->QueueAJob(main_window_->CreateWatchDogSetting());
		main_window_->main_controller()->StartExecuting(new SimpleCallback<MainWindow>(main_window_,&MainWindow::DoFinished));
		main_window_->LockOnUI();
		main_window_->GetOkDialog()->setWindowTitle(LoadQString(LANGUAGE_TAG,IDS_STRING_WRITE_MEMORY_OK));
	}
}




void JlinkParameterWidget::on_pushButton_stop_clicked()
{
	main_window_->main_controller()->StopByUser();
}

ReadbackItem JlinkParameterWidget::GetJlinkParamRBItem()
{
	NUTL_ReadFlag_E read_flag = NUTL_READ_PAGE_ONLY;
	NUTL_AddrTypeFlag_E addr_flag = NUTL_ADDR_LOGICAL;
	U32 region_id = EMMC_PART_USER;
	U64 addr;
    U64 len;

	std::list<ImageInfo> image_list;
	main_window_->main_controller()->GetImageInfoList(image_list, DOWNLOAD_ONLY);
	for(std::list<ImageInfo>::const_iterator it = image_list.begin(); it != image_list.end(); ++it)
	{
		LOGI("########## %s %d ########## %s\n", __func__, __LINE__, it->name.c_str());
		if( 0 == it->name.compare(std::string(JP_PART_NAME)) ){
			addr = it->begin_addr;
			proinfo_addr = addr;
			break;
		}
	}
	ReadbackItem item(0,1,addr,JP_FILE_LEN,string(JP_FILE_NAME),read_flag, (EMMC_Part_E)region_id, addr_flag);
	return item;
}


void JlinkParameterWidget::SetReadbackListItem(QSharedPointer<APCore::ReadbackSetting> &readback_setting)
{
	std::list<ReadbackItem> items;
	main_window_->main_controller()->ClearAllRBItem();
	ReadbackItem rbitem = GetJlinkParamRBItem();
	items.push_back(rbitem);
	readback_setting->set_readbackItems(items);
}

void JlinkParameterWidget::on_pushButton_ReadParam_clicked()
{
	if (main_window_->IsScatterFileLoad()) {
		main_window_->main_controller()->SetPlatformSetting();
		main_window_->main_controller()->SetConnSetting(main_window_->CreateConnSetting());
		main_window_->main_controller()->QueueAJob(main_window_->CreateJlinkParamReadbackSetting());
		main_window_->main_controller()->StartExecuting(
		new SimpleCallback<MainWindow>(main_window_,&MainWindow::DoFinished));
		main_window_->LockOnUI();
		main_window_->GetOkDialog()->setWindowTitle(LoadQString(main_window_->GetLanguageTag(), IDS_STRING_READ_BACK_OK));
	}
}
void JlinkParameterWidget::dataChanged()
{
	LOGI("########## %s %d ########## 0x%x 0x%x\n", __func__, __LINE__, jlinkParam.at(0), hexEdit->dataAt(0,1).at(0));
	
	//LOGI("########## %s %d ########## size: %d %d \n", __func__, __LINE__, info_label->minimumSizeHint().width(), info_label->minimumSizeHint().height());
	#if 0
	QBuffer tmpbuffer;
	tmpbuffer.setBuffer(&jlinkParam);
	hexEdit->write(tmpbuffer);
	LOGI("########## %s %d ########## %d %d\n", __func__, __LINE__, jlinkParam.at(0), hexEdit->dataAt(0,1).at(0));
    //setWindowModified(hexEdit->isModified());
	#endif
}

void JlinkParameterWidget::setOverwriteMode(bool mode)
{
	LOGI("########## %s %d ########## %d\n", __func__, __LINE__, mode);
	
}
void JlinkParameterWidget::setAddressChanged(qint64 address)
{
	char *info_name[20] = {
		"Project",
		"HW_ver",
		"LCD",
		"Camera",
		"TP",
		"customer_misc",
		"Battery",
		"TP_PS_Enable",
		"LOGO",
		"vibrator_voltage",
		"weixin_skype_rotation",
		"HW_INFO11",
		"HW_INFO12",
		"lcm_rotation",
		"modem_bands",
		"ddr_type",
		"cpu_frequency",
		"memory_size",
		"cpu_cores",
		"cpu platform"
	};

	//LOGI("########## %s %d ########## %d\n", __func__, __LINE__, address);
	char buffer[60];
	if (address < 20 &&  address >= 0) {
		snprintf(buffer, 60, "hwinfo[%d]: %s", address, info_name[address]);
		info_label->setText(buffer);
	}
}

void JlinkParameterWidget::jlinkParamReadBinData(void)
{
	QFile binf;
	binf.setFileName(JP_FILE_NAME);
	bool ok = binf.open(QIODevice::ReadOnly);
	if (!ok) {
		LOGI("########## %s %d ########## read bin file %d\n", __func__, __LINE__, ok);
		return;
	}
	binf.seek(0xfc);
	jlinkParam = binf.read(20);
	hexEdit->setData(jlinkParam);
	binf.close();
}

void JlinkParameterWidget::jlinkParamWriteBinData(void)
{
	QString dump(jlinkParam.toHex());
	LOGI("########## %s %d ########## to write:%s\n", __func__, __LINE__, dump.toStdString().c_str());
	QFile binf;
	binf.setFileName(JP_FILE_NAME);
	bool ok = binf.open(QIODevice::ReadWrite);
	if (!ok) {
		LOGI("########## %s %d ########## write bin file %d\n", __func__, __LINE__, ok);
		return;
	}
	binf.seek(0xfc);
	binf.write(jlinkParam);
	binf.close();
}

