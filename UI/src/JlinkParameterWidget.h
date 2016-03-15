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
private:
    MainWindow *main_window_;
    Ui::JlinkParameterWidget *ui_;

protected:


signals:


public slots:

private slots:

    void on_pushButton_download_clicked();
    void on_pushButton_stop_clicked();
    void dataChanged();//for hexedit.
    void setOverwriteMode(bool mode);//for hexedit.

};

#endif // JLINKPARAMETERWIDGET_H
