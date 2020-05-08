#ifndef HOBBITSDISPLAYSPLUGIN_H
#define HOBBITSDISPLAYSPLUGIN_H

#include <QObject>
#include <QtPlugin>
#include "CutterPlugin.h"
#include "bitcontainermanager.h"
#include "hobbitspluginmanager.h"
#include <QTabWidget>
#include <QVBoxLayout>

class HobbitsDisplayPlugin : public QObject, CutterPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.radare.cutter.plugins.CutterPlugin")
    Q_INTERFACES(CutterPlugin)

public:
    void setupPlugin() override;
    void setupInterface(MainWindow *main) override;

    QString getName() const override         { return "Hobbits Display Plugin"; }
    QString getAuthor() const override       { return "Adam Nash"; }
    QString getDescription() const override  { return "Plugin that loads \"Hobbits\" display plugins"; }
    QString getVersion() const override      { return "0.1"; }
};

class HobbitsDisplayPluginWidget : public CutterDockWidget
{
    Q_OBJECT

public:
    explicit HobbitsDisplayPluginWidget(MainWindow *main, QAction *action);

private:
    QSharedPointer<BitContainerManager> m_bitContainerManager;
    QSharedPointer<HobbitsPluginManager> m_pluginManager;
    QMap<int, QSharedPointer<DisplayInterface>> m_displayMap;
    QVBoxLayout *m_layout;
    QTabWidget* m_tabs;
    QSharedPointer<DisplayHandle> m_displayHandle;
    QScrollBar* m_vScroll;
    QScrollBar* m_hScroll;
    QWidget* m_currControlWidget;
    RVA m_currAddr;

private slots:
    void on_seekChanged(RVA addr);
    void refresh();

    void loadPlugins();
    void initializeDisplays();
    void checkCurrentDisplay();
};


#endif // HOBBITSDISPLAYSPLUGIN_H
