#include "DownloadWidget.h"
#include "ui_DownloadWidget.h"
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
#define JP_PART_NAME1 ("PRO_INFO")


JlinkParameterWidget::JlinkParameterWidget(QTabWidget *parent, MainWindow *window) :
    TabWidgetBase(2, tr("&Download"), parent),
    main_window_(window),
    ui_(new Ui::JlinkParameterWidget),
    wm_arg(new WriteFlashMemoryParameter),
	header_(new CheckHeader(Qt::Horizontal, this)),
    proinfo_addr(0)
{
    ui_->setupUi(this);	

	//hexedit init
	hexedit_hori_layout = new QHBoxLayout();
    hexEdit = new QHexEdit;
	hexEdit->setMaximumSize(405,80);
	hexEdit->setMinimumSize(405,80);
	jlinkParam.fill('0', 20);
	hexEdit->setData(jlinkParam);
	connect(hexEdit, SIGNAL(overwriteModeChanged(bool)), this, SLOT(setOverwriteMode(bool)));
    connect(hexEdit, SIGNAL(dataChanged()), this, SLOT(dataChanged()));
	connect(hexEdit, SIGNAL(currentAddressChanged(qint64)), this, SLOT(setAddressChanged(qint64)));
	hexedit_hori_layout->addWidget(hexEdit);
	info_label = new QLabel(tr("hwinfo[0]: Project"));
	info_label->setMinimumSize(350,80);
	info_label->setMaximumSize(350,80);
	
	info_label->setAlignment(Qt::AlignTop|Qt::AlignLeft);
	info_label->show();
	hexedit_hori_layout->addWidget(info_label);
	ui_->pushButton_WriteParam->setEnabled(false);//init the write can not func
	ui_->horizontalLayout->addLayout(hexedit_hori_layout);
	horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	ui_->horizontalLayout->addItem(horizontalSpacer_2);

	//start for format widget:
	#if 0
	jlinkFormatTableWidget = new QTableWidget();
	if (jlinkFormatTableWidget->columnCount() < 6)
		jlinkFormatTableWidget->setColumnCount(6);
	QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
	jlinkFormatTableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
	QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
	jlinkFormatTableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
	QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
	jlinkFormatTableWidget->setHorizontalHeaderItem(2, __qtablewidgetitem2);
	QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
	jlinkFormatTableWidget->setHorizontalHeaderItem(3, __qtablewidgetitem3);
	QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
	jlinkFormatTableWidget->setHorizontalHeaderItem(4, __qtablewidgetitem4);
	QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
	jlinkFormatTableWidget->setHorizontalHeaderItem(5, __qtablewidgetitem5);

	jlinkFormatTableWidget->setAcceptDrops(true);
	jlinkFormatTableWidget->setAutoFillBackground(false);
	jlinkFormatTableWidget->setFrameShape(QFrame::StyledPanel);
	jlinkFormatTableWidget->setFrameShadow(QFrame::Sunken);
	jlinkFormatTableWidget->setLineWidth(2);
	jlinkFormatTableWidget->setMidLineWidth(1);
	jlinkFormatTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	jlinkFormatTableWidget->setProperty("showDropIndicator", QVariant(true));
	jlinkFormatTableWidget->setAlternatingRowColors(true);
	jlinkFormatTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
	jlinkFormatTableWidget->setWordWrap(false);
	jlinkFormatTableWidget->setCornerButtonEnabled(false);
	jlinkFormatTableWidget->setRowCount(0);
	jlinkFormatTableWidget->setColumnCount(6);
	jlinkFormatTableWidget->horizontalHeader()->setVisible(false);
	jlinkFormatTableWidget->horizontalHeader()->setCascadingSectionResizes(true);
	jlinkFormatTableWidget->horizontalHeader()->setHighlightSections(true);
	jlinkFormatTableWidget->horizontalHeader()->setMinimumSectionSize(15);
	jlinkFormatTableWidget->horizontalHeader()->setProperty("showSortIndicator", QVariant(false));
	jlinkFormatTableWidget->horizontalHeader()->setStretchLastSection(true);
	jlinkFormatTableWidget->verticalHeader()->setVisible(false);
	jlinkFormatTableWidget->verticalHeader()->setDefaultSectionSize(23);
	jlinkFormatTableWidget->verticalHeader()->setHighlightSections(false);
	jlinkFormatTableWidget->verticalHeader()->setMinimumSectionSize(34);
	jlinkFormatTableWidget->verticalHeader()->setProperty("showSortIndicator", QVariant(false));


	//jlinkFormatTableWidget->setColumnHidden(columnRegion, true);
	QTableWidgetItem *___qtablewidgetitem = jlinkFormatTableWidget->horizontalHeaderItem(1);
	___qtablewidgetitem->setText("Name");
	QTableWidgetItem *___qtablewidgetitem1 = jlinkFormatTableWidget->horizontalHeaderItem(2);
	___qtablewidgetitem1->setText("Begin Address");
	QTableWidgetItem *___qtablewidgetitem2 = jlinkFormatTableWidget->horizontalHeaderItem(3);
	___qtablewidgetitem2->setText("End Address");
	QTableWidgetItem *___qtablewidgetitem3 = jlinkFormatTableWidget->horizontalHeaderItem(4);
	___qtablewidgetitem3->setText("Region");
	QTableWidgetItem *___qtablewidgetitem4 = jlinkFormatTableWidget->horizontalHeaderItem(5);
	___qtablewidgetitem4->setText("Location");

	//jlinkFormatTableWidget->setMinimumSize(600,300);
	//jlinkFormatTableWidget->setMinimumSize(600,1200);
	//jlinkFormatTableWidget->setMaximumSize (1600,1300);
	jlinkFormatTableWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	//ui_->verticalLayout->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	ui_->verticalLayout->addWidget(jlinkFormatTableWidget);
#endif
	jlinkFormatTableWidget = ui_->jlinkFormatTableWidget;
	jlinkFormatTableWidget->setHorizontalHeader(header_);
	jlinkFormatTableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	//jlinkFormatTableWidget->setColumnHidden(columnRegion, true);


    main_window_->main_controller()->GetPlatformSetting()->addObserver(this);
    main_window_->scatter_observer()->addObserver(this);
	connect(main_window_->get_DLWidget(), SIGNAL(signal_load_finished()),this, SLOT(slot_OnLoadByScatterEnd_JlinkFormat()));
	//for headr all check or not checked:
	connect(header_,SIGNAL(sectionClicked(int)), this, SLOT(slot_OnHeaderView_click_jlink_format(int)));
}

JlinkParameterWidget::~JlinkParameterWidget()
{
    //IniItem item("history.ini", "RecentOpenFile", "scatterHistory");
    //item.SaveStringListValue(scatterFile_historyList_);

    delete ui_;
	delete jlinkFormatTableWidget;
	delete info_label;
	delete hexEdit;
	delete hexedit_hori_layout;
	delete wm_arg;
	delete header_;
}

//============base ui====================
void JlinkParameterWidget::LockOnUI()
{
//    ui_->groupBox->setEnabled(false);
    ui_->pushButton_ReadParam->setEnabled(false);
	ui_->pushButton_WriteParam->setEnabled(false);
    ui_->pushButton_stop->setEnabled(true);
	ui_->toolButton_Format->setEnabled(false);
}

void JlinkParameterWidget::DoFinished()
{
	jlinkParamReadBinData();
    ui_->pushButton_ReadParam->setEnabled(true);
	ui_->pushButton_WriteParam->setEnabled(true);
	ui_->toolButton_Format->setEnabled(true);
    ui_->pushButton_stop->setEnabled(false);

}

void JlinkParameterWidget::UpdateUI()
{
}
void JlinkParameterWidget::SetTabLabel(QTabWidget *tab_widget, int index)
{
    QString label = tr("Parameter");
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

	std::list<ImageInfo> image_list;
	main_window_->main_controller()->GetImageInfoList(image_list, DOWNLOAD_ONLY);
	for(std::list<ImageInfo>::const_iterator it = image_list.begin(); it != image_list.end(); ++it)
	{
		LOGI("########## %s %d ########## %s\n", __func__, __LINE__, it->name.c_str());
		if( 0 == it->name.compare(std::string(JP_PART_NAME))||
			0 == it->name.compare(std::string(JP_PART_NAME1))){
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

void JlinkParameterWidget::SetRomAddress(int row, int column, U64 address)
{
    QTableWidgetItem *tableItem = jlinkFormatTableWidget->item(row, column);
    if (tableItem == NULL) {
        tableItem = new QTableWidgetItem();
        jlinkFormatTableWidget->setItem(row, column,tableItem);
    }
    tableItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    tableItem->setText(QString("0x%1").arg(address,16,16,QChar('0')));
}
void JlinkParameterWidget::slot_OnLoadByScatterEnd_JlinkFormat()
{
	LOGI("########## %s %d ##########\n", __func__, __LINE__);
	std::list<ImageInfo> image_list;
    QTableWidgetItem * tableItem;
	int row_count = 0;
    int row = 0;
	main_window_->main_controller()->GetImageInfoList(image_list, DOWNLOAD_ONLY);
    for(std::list<ImageInfo>::const_iterator it = image_list.begin(); it != image_list.end(); ++it) {
		//U64 len = it->end_addr - it->begin_addr;
		//LOGI("########## %s %d ########## %s: %llx: %llx %llx, %llx\n", __func__, __LINE__, it->name.c_str(), it->region_addr, it->begin_addr, it->end_addr, len);
		row_count++;
    }
	jlinkFormatTableWidget->setRowCount(row_count);

    for(std::list<ImageInfo>::const_iterator it = image_list.begin(); it != image_list.end(); ++it) {
		SetRomAddress(row, ColumnBeginAddr, it->begin_addr);
		SetRomAddress(row, ColumnEndAddr, it->end_addr);

        tableItem = jlinkFormatTableWidget->item(row, columnRegion);
        if(tableItem == NULL){
            tableItem = new QTableWidgetItem();
            jlinkFormatTableWidget->setItem(row, columnRegion, tableItem);
        }
        tableItem->setText(QString::fromLocal8Bit(it->region.c_str()));
#if 0
        tableItem = jlinkFormatTableWidget->item(row, ColumnLocation);
        if (tableItem == NULL) {
            tableItem = new QTableWidgetItem();
            jlinkFormatTableWidget->setItem(row, ColumnLocation,tableItem);
        }
        tableItem->setText(QDir::toNativeSeparators(QString::fromLocal8Bit(it->location.c_str())));
#endif
        tableItem = jlinkFormatTableWidget->item(row, ColumnName);
        if (tableItem == NULL) {
            tableItem = new QTableWidgetItem();
            jlinkFormatTableWidget->setItem(row, ColumnName, tableItem);
        }
        tableItem->setText(it->name.c_str());

        tableItem = jlinkFormatTableWidget->item(row, ColumnEnable);
        if (tableItem == NULL) {
            tableItem = new QTableWidgetItem();
            jlinkFormatTableWidget->setItem(row, ColumnEnable, tableItem);
        }
		//tableItem->setCheckState(Qt::Checked);
		tableItem->setCheckState(Qt::Unchecked);

        row++;
	}
}

void JlinkParameterWidget::slot_OnHeaderView_click_jlink_format(int index)
{
	//LOGI("########## %s %d ########## %d\n", __func__, __LINE__, index);
	if (index == 0) {
		bool checked = header_->GetChecked();
	    QTableWidgetItem * tableItem;
		std::list<ImageInfo> image_list;
		main_window_->main_controller()->GetImageInfoList(image_list, DOWNLOAD_ONLY);
		int row=0;
	    for(std::list<ImageInfo>::const_iterator it = image_list.begin(); it != image_list.end(); ++it) {
	        tableItem = jlinkFormatTableWidget->item(row, ColumnEnable);
			tableItem->setCheckState(checked?Qt::Checked:Qt::Unchecked);
			row++;
		}
	}
}

void JlinkParameterWidget::on_toolButton_Format_clicked()
{
	LOGI("########## %s %d ##########\n", __func__, __LINE__);
    if(1)
    {
        main_window_->main_controller()->SetPlatformSetting();
        main_window_->main_controller()->SetConnSetting(main_window_->CreateConnSetting());
        main_window_->main_controller()->QueueAJob(main_window_->CreateJlinkComboCustFormatSetting());
        main_window_->main_controller()->StartExecuting(
                    new SimpleCallback<MainWindow>(main_window_,&MainWindow::DoFinished));
        main_window_->LockOnUI();
        main_window_->GetOkDialog()->setWindowTitle(LoadQString(LANGUAGE_TAG, IDS_STRING_FORMAT_OK));
    }
}

void JlinkParameterWidget::SetFormatSettingList(QSharedPointer<APCore::JlinkComboCustFormatSetting> &jlink_combo_format_setting)
{
	std::list<APCore::FormatSetting *> format_list;
	QTableWidgetItem * tableItem;
	std::list<ImageInfo> image_list;
	main_window_->main_controller()->GetImageInfoList(image_list, DOWNLOAD_ONLY);
	int row=0;
	//FIXME: can not format first and last one.
	U64 pre_rom_start_addr=0;
	QTableWidgetItem *pre_tableItem = NULL;

	for(std::list<ImageInfo>::const_iterator it = image_list.begin(); it != image_list.end(); ++it) {
	    tableItem = jlinkFormatTableWidget->item(row, ColumnEnable);
		if (pre_tableItem && pre_tableItem->checkState() == Qt::Checked) {
			APCore::FormatSetting *fmt_setting_ = new APCore::FormatSetting();
			fmt_setting_->set_part_id(EMMC_PART_USER);
	        fmt_setting_->set_begin_addr(pre_rom_start_addr);
	        fmt_setting_->set_length(it->begin_addr-pre_rom_start_addr);
			format_list.push_back(fmt_setting_);
		}
		pre_rom_start_addr = it->begin_addr;
		pre_tableItem = tableItem;
		row++;
	}

	jlink_combo_format_setting->set_format_setting_list(format_list);

}

