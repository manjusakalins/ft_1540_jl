#ifndef JLINKPARAMETERWIDGET_H
#define JLINKPARAMETERWIDGET_H

#include "TabWidgetBase.h"
#include "../../Public/AppTypes.h"
#include "../../ConsoleMode/GeneralSetting.h"
#include "../../Setting/DADownloadAllSetting.h"
#include "../../Setting/PlatformSetting.h"
#include <QSharedPointer>
#include <list>
#include <QtGui>
#include <QStringList>
#include "MainController.h"
#include "ScatterObserver.h"

#include "../../Rules/ReadbackRule.h"
#include "../../Public/AppTypes.h"
#include "../../Setting/ReadbackSetting.h"
#include "../../Setting/PlatformSetting.h"
#include "../../Arg/WriteMemoryArg.h"
#include "../../Setting/WriteMemorySetting.h"


namespace Ui
{
class JlinkParameterWidget;
}

class MainWindow;
class CheckHeader;
class PlatformSetting;
class QHexEdit;

class JlinkParameterWidget : public TabWidgetBase, public APCore::IPlatformOb, public IScatterObj
{
    Q_OBJECT
public:
    enum ImageColumn{
        ColumnEnable = 0,
        ColumnName,
        ColumnBeginAddr,
        ColumnEndAddr,
        columnRegion,
        ColumnLocation
    };

    explicit JlinkParameterWidget(QTabWidget *parent, MainWindow *window);
    ~JlinkParameterWidget();

    DECLARE_TABWIDGET_VFUNCS()

    virtual void onPlatformChanged();
    virtual void OnScatterChanged(bool showRegion);
    
    QHexEdit *hexEdit;
    QByteArray jlinkParam;

    //flash tool stuff
    void SetReadbackListItem(QSharedPointer<APCore::ReadbackSetting> &readback_setting);//for mainwindow to call init args
    QSharedPointer<APCore::WriteMemorySetting> CreateJlinkParamWriteMemSetting();//for mainwindow to call init args
private:
    HW_StorageType_E storage_;
    bool platform_changed_;
    MainWindow *main_window_;
    Ui::JlinkParameterWidget *ui_;
    U64 proinfo_addr;
    ReadbackItem GetJlinkParamRBItem();
    void jlinkParamReadBinData(void);
    void jlinkParamWriteBinData(void);
    int ArgFlashToolWriteMemory( int n, WriteFlashMemoryParameter *wm_arg, U64 address, char * data, unsigned int length );
    WriteFlashMemoryParameter *wm_arg;

protected:


signals:


public slots:

private slots:

    void on_pushButton_WriteParam_clicked();    
    void on_pushButton_ReadParam_clicked();
    void on_pushButton_stop_clicked();
    void dataChanged();//for hexedit.
    void setOverwriteMode(bool mode);//for hexedit.

};

#endif // JLINKPARAMETERWIDGET_H
