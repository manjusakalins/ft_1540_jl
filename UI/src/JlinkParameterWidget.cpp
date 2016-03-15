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

JlinkParameterWidget::JlinkParameterWidget(QTabWidget *parent, MainWindow *window) :
    TabWidgetBase(2, tr("&Download"), parent),
    main_window_(window),
    ui_(new Ui::JlinkParameterWidget)
{
    ui_->setupUi(this);	
    hexEdit = new QHexEdit;
	ui_->verticalLayout->addWidget(hexEdit);
    //ui_->tableWidget->setHorizontalHeader(header_);
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
//    ui_->toolButton_start->setEnabled(false);
//    ui_->toolButton_stop->setEnabled(true);
}

void JlinkParameterWidget::DoFinished()
{
//    ui_->groupBox->setEnabled(true);
//    ui_->toolButton_start->setEnabled(true);
//    ui_->toolButton_stop->setEnabled(false);
}

void JlinkParameterWidget::UpdateUI()
{
}
void JlinkParameterWidget::SetTabLabel(QTabWidget *tab_widget, int index)
{
    QString label = tr("jlink parameter");

    tab_widget->setTabText(index, label);
/*
    for(int i = 0; i < ui_->comboBox_Scene->count(); i++)
    {
        ui_->comboBox_Scene->setItemText(i, LoadQString(LANGUAGE_TAG, IDS_STRING_SCENE_FMTALLDL + i));
    }
*/
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

void JlinkParameterWidget::on_pushButton_download_clicked()
{
}


void JlinkParameterWidget::on_pushButton_stop_clicked()
{

}

