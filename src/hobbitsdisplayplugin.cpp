#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QAction>
#include <QScrollBar>

#include "hobbitsdisplayplugin.h"
#include "common/TempConfig.h"
#include "common/Configuration.h"
#include "core/MainWindow.h"
#include "settingsmanager.h"

void HobbitsDisplayPlugin::setupPlugin()
{
}

void HobbitsDisplayPlugin::setupInterface(MainWindow *main)
{
    QAction *action = new QAction("Hobbits Display", main);
    action->setCheckable(true);
    HobbitsDisplayPluginWidget *widget = new HobbitsDisplayPluginWidget(main, action);
    main->addPluginDockWidget(widget, action);
}

HobbitsDisplayPluginWidget::HobbitsDisplayPluginWidget(MainWindow *main, QAction *action) :
    CutterDockWidget(main, action),
    m_bitContainerManager(QSharedPointer<BitContainerManager>(new BitContainerManager())),
    m_pluginManager(QSharedPointer<HobbitsPluginManager>(new HobbitsPluginManager())),
    m_currControlWidget(nullptr),
    m_currAddr(RVA_INVALID)
{
    this->setObjectName("CutterSamplePluginWidget");
    this->setWindowTitle("Hobbits Display");
    QWidget *content = new QWidget();
    this->setWidget(content);

    // These are just for the display handle and won't show up
    m_vScroll = new QScrollBar(content);
    m_hScroll = new QScrollBar(content);
    m_vScroll->setHidden(true);
    m_hScroll->setHidden(true);

    m_displayHandle = QSharedPointer<DisplayHandle>(
            new DisplayHandle(
                    m_bitContainerManager,
                    m_vScroll,
                    m_hScroll));

    m_layout = new QVBoxLayout(content);
    content->setLayout(m_layout);

    m_tabs = new QTabWidget(content);
    m_layout->addWidget(m_tabs);
    m_tabs->setFont(Config()->getFont());
    m_tabs->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_tabs->setElideMode(Qt::ElideLeft);
    m_tabs->setDocumentMode(true);

    m_pluginManager->getDisplay("DUMB");

    loadPlugins();
    initializeDisplays();

    connect(Core(), &CutterCore::refreshAll, this, [this]() { refresh(); });
    connect(Core(), &CutterCore::instructionChanged, this, [this]() { refresh(); });
    connect(Core(), &CutterCore::stackChanged, this, [this]() { refresh(); });
    connect(Core(), &CutterCore::registersChanged, this, [this]() { refresh(); });
    connect(Core(), &CutterCore::seekChanged, this, &HobbitsDisplayPluginWidget::on_seekChanged);
}

void HobbitsDisplayPluginWidget::on_seekChanged(RVA addr)
{
    m_currAddr = addr;
    refresh();
}

void HobbitsDisplayPluginWidget::refresh()
{
    m_bitContainerManager->deleteCurrentContainer();

    if (m_currAddr == RVA_INVALID) {
        return;
    }

    QByteArray data = Core()->ioRead(m_currAddr, 100000);
    auto bits = QSharedPointer<BitContainer>(new BitContainer());
    bits->setBits(data);
    QModelIndex addedIndex = m_bitContainerManager->getTreeModel()->addContainer(bits);
    m_bitContainerManager->getCurrSelectionModel()->setCurrentIndex(
            addedIndex,
            QItemSelectionModel::ClearAndSelect);
}

void HobbitsDisplayPluginWidget::initializeDisplays()
{
    // Instantiate displays for this display set
    QList<QSharedPointer<DisplayInterface>> displays;
    QSet<QString> queued;
    for (QString pluginString : SettingsManager::getInstance().getPluginLoaderSetting(
            SettingsData::DISPLAY_DISPLAY_ORDER_KEY).toStringList()) {
        QSharedPointer<DisplayInterface> plugin = m_pluginManager->getDisplay(pluginString.trimmed());
        if (!plugin.isNull()) {
            displays.append(QSharedPointer<DisplayInterface>(plugin->createDefaultDisplay()));
            queued.insert(pluginString.trimmed());
        }
    }
    for (QSharedPointer<DisplayInterface> plugin : m_pluginManager->getAllDisplays()) {
        if (!queued.contains(plugin->getName())) {
            displays.append(QSharedPointer<DisplayInterface>(plugin->createDefaultDisplay()));
        }
    }

    // Add the widgets to the tabs
    m_tabs->setUpdatesEnabled(false);
    for (QSharedPointer<DisplayInterface> displayPlugin : displays) {
        QWidget *display = displayPlugin->getDisplayWidget(m_displayHandle);
        int idx = m_tabs->addTab(display, displayPlugin->getName());
        m_displayMap.insert(idx, displayPlugin);
    }
    m_tabs->setUpdatesEnabled(true);

    // Set up the display controls when necessary
    if (m_displayMap.size() > 0) {
        m_tabs->setCurrentIndex(0);
        checkCurrentDisplay();
    }
    connect(m_tabs, SIGNAL(currentChanged(int)), this, SLOT(checkCurrentDisplay()));
}


void HobbitsDisplayPluginWidget::checkCurrentDisplay()
{
    if (m_currControlWidget) {
        m_currControlWidget->setVisible(false);
        m_layout->removeWidget(m_currControlWidget);
    }
    m_currControlWidget = nullptr;

    QSet<DisplayInterface*> focusDisplays;
    QSharedPointer<DisplayInterface> currDisplay = m_displayMap.value(m_tabs->currentIndex());
    if (!currDisplay.isNull()) {
        focusDisplays.insert(currDisplay.data());
        m_currControlWidget = currDisplay->getControlsWidget(m_displayHandle);
    }
    m_displayHandle->setFocusDisplays(focusDisplays);

    if (m_currControlWidget) {
        m_layout->addWidget(m_currControlWidget);
        m_currControlWidget->setVisible(true);
    }
}

void HobbitsDisplayPluginWidget::loadPlugins()
{
    QStringList warnings;
    QStringList pluginPaths;
    pluginPaths.append(QDir::homePath()+"/.local/share/hobbits/plugins");
    for (QString pluginPath : pluginPaths) {
        warnings.append(m_pluginManager->loadPlugins(pluginPath));
    }

    if (!warnings.isEmpty()) {
        QMessageBox msg;
        msg.setWindowTitle("Hobbits Plugin Load Warnings");
        msg.setText(warnings.join("\n"));
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
    }
}
