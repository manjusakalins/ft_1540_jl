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
#define JP_PART_NAME ("proinfo")

JlinkParameterWidget::JlinkParameterWidget(QTabWidget *parent, MainWindow *window) :
    TabWidgetBase(2, tr("&Download"), parent),
    main_window_(window),
    ui_(new Ui::JlinkParameterWidget)
{
    ui_->setupUi(this);	

	//hexedit init
    hexEdit = new QHexEdit;
	jlinkParam.fill('0', 20);
	hexEdit->setData(jlinkParam);
	connect(hexEdit, SIGNAL(overwriteModeChanged(bool)), this, SLOT(setOverwriteMode(bool)));
    connect(hexEdit, SIGNAL(dataChanged()), this, SLOT(dataChanged()));
	ui_->verticalLayout->addWidget(hexEdit);


    //ui_->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

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
    //storage_ = main_window_->main_controller()->GetPlatformSetting()->getFlashToolStorageConfig().GetStorageType();
}

void JlinkParameterWidget::OnScatterChanged(bool showRegion)
{
      //ui_->tableWidget->setColumnHidden(columnRegion, !showRegion);
}

//=================================================

void JlinkParameterWidget::on_pushButton_WriteParam_clicked()
{
	QBuffer tmpbuffer;
	tmpbuffer.setBuffer(&jlinkParam);
	hexEdit->write(tmpbuffer);
	LOGI("########## %s %d ########## %d %d\n", __func__, __LINE__, jlinkParam.at(0), hexEdit->dataAt(0,1).at(0));
}


void JlinkParameterWidget::on_pushButton_stop_clicked()
{
	main_window_->main_controller()->StopByUser();
}

ReadbackItem JlinkParameterWidget::GetJlinkParamRBItem()
{
#if 0
	int ret=0;
	ret = RB_SetReadbackFlag(g_RB_HANDLE, 0, (NUTL_ReadFlag_E)NUTL_READ_PAGE_ONLY);
	if (S_DONE != ret)
	{
		goto _error;
	}
	ret = RB_SetAddrTypeFlag(g_RB_HANDLE, 0, NUTL_ADDR_LOGICAL);
	if (S_DONE != ret)
	{
		goto _error;
	}
	ret = RB_SetEnableAttr(g_RB_HANDLE, 0, _TRUE);
	if (S_DONE != ret)
	{
		goto _error;
	}
	ret = ArgFlashToolAdvancedReadback(g_RB_HANDLE, rb_arg, rb_result, n);
	if (S_DONE != ret)
	{
		goto _error;
	}

	if(g.Platform.getStorageType()==HW_STORAGE_NAND)
	{
		//ret = RB_SetNANDInfo(g_RB_HANDLE,1024,512);

	}

	g_ComX[n].EnterCS();
	if (g_ComX[n].ft_api_handle)
	{
		ret = FlashTool_Readback(g_ComX[n].ft_api_handle, &rb_arg, &rb_result);
		if (S_DONE != ret)
		{
			g_ComX[n].LeaveCS();
			g.g_log.SaveLog(ERROR_LOG, NULL, "[COM%d]FlashTool_Readback() failed code %d %s %d\r\n", g_ComX[n].ComNO_Current, ret, __FILE__, __LINE__);
			goto _error;
		}
	}
	else
	{
		ret = BOOT_STOP;
		g.g_log.SaveLog(ERROR_LOG, NULL, "[COM%d]FlashTool_Readback() ft_api_handle is NULL\r\n", g_ComX[n].ComNO_Current);
		g_ComX[n].LeaveCS();
		goto _error;
	}
	g_ComX[n].LeaveCS();
#endif
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
			len = 512;
			break;
		}
	}
	ReadbackItem item(0,1,addr,len,string(JP_FILE_NAME),read_flag, (EMMC_Part_E)region_id, addr_flag);
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


   main_window_->main_controller()->SetPlatformSetting();

   main_window_->main_controller()->SetConnSetting(main_window_->CreateConnSetting());
   main_window_->main_controller()->QueueAJob(main_window_->CreateJlinkParamReadbackSetting());
   main_window_->main_controller()->StartExecuting(
	  new SimpleCallback<MainWindow>(main_window_,&MainWindow::DoFinished));
   main_window_->LockOnUI();
   main_window_->GetOkDialog()->setWindowTitle(LoadQString(main_window_->GetLanguageTag(), IDS_STRING_READ_BACK_OK));

}
void JlinkParameterWidget::dataChanged()
{
	LOGI("########## %s %d ########## 0x%x 0x%x\n", __func__, __LINE__, jlinkParam.at(0), hexEdit->dataAt(0,1).at(0));
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
}
