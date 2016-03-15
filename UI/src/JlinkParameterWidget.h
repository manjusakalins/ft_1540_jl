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
    void SetReadbackListItem(QSharedPointer<APCore::ReadbackSetting> &readback_setting);
private:
    MainWindow *main_window_;
    Ui::JlinkParameterWidget *ui_;    
    ReadbackItem GetJlinkParamRBItem();
    void jlinkParamReadBinData(void);
    //void jlinkParamWriteBinData(void);

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
