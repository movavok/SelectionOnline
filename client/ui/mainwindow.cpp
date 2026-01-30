#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->le_ip->setInputMask("000.000.000.000;_");

    ui->table_playersList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->table_playersList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    initButtons();

    m_playerPreview = ui->playerPreviewWidget;
    m_gameView = ui->gameViewHolder;

    updateLobbySelectionButtons();

    initNetClient();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initButtons() {
    //start screen
    connect(ui->b_hostServer, &QPushButton::clicked, this, &MainWindow::onHostServer);
    connect(ui->b_joinServer, &QPushButton::clicked, this, &MainWindow::onJoinServer);

    //settings
    connect(ui->b_back, &QPushButton::clicked, this, &MainWindow::goToPrevPage);

    //lobby
    connect(ui->b_playerReady, &QPushButton::clicked, this, &MainWindow::onPlayerReady);
    connect(ui->b_settings, &QPushButton::clicked, this, &MainWindow::goToSettings);
    connect(ui->b_backToStart, &QPushButton::clicked, this, &MainWindow::goToStartScreen);
    connect(ui->b_openGameView, &QPushButton::clicked, this, &MainWindow::startGame);

    initColorButtons();
    initWeaponButtons();
    initAbilityButtons();
    //actions
    connect(ui->act_settingsScreen, &QAction::triggered, this, &MainWindow::goToSettings);
}

void MainWindow::goToPage(Page page) {
    Page current = static_cast<Page>(ui->stackedWidget->currentIndex());

    if (page == PageSettings && current != PageSettings) m_prevPage = current;
    if (page == current) return;

    ui->stackedWidget->setCurrentIndex(page);

    if (page == PageGame) {
        m_gameView->useMovementScheme(getMovementScheme());
        m_gameView->setupSlotKeys(getSlotKeys());
        m_gameView->prepareSlotKeysVector();
        statusBar()->hide();
    }
    else statusBar()->show();

    if (page == PageLobby)
        updateLobbySelectionButtons();
}

void MainWindow::goToLobby() {
    resetLobbySelectionState();
    goToPage(PageLobby);
    ensureLocalPlayerRow();
    updateLocalPlayerRow();
    updateLobbySelectionButtons();
}

void MainWindow::onNetConnected() {
    statusBar()->showMessage("Підключено", 2000);
    statusBar()->setStyleSheet("color: #77dd77; font-size:12px; font-family:Fixedsys;");
    m_netClient->sendHello(m_localNickname);
}

void MainWindow::onNetDisconnected() {
    statusBar()->showMessage("Відключено", 2000);
    statusBar()->setStyleSheet("color: #dc3c3c; font-size:12px; font-family:Fixedsys;");
}

void MainWindow::onNetErrorText(const QString& text) {
    statusBar()->showMessage(text, 6000);
    statusBar()->setStyleSheet("color: #dc3c3c; font-size:12px; font-family:Fixedsys;");
}

void MainWindow::onNetTextReceived(const QString& text) {
    statusBar()->showMessage("Server: " + text.trimmed(), 3000);
}

void MainWindow::initNetClient() {
    m_netClient = new NetClient(this);
    connect(m_netClient, &NetClient::connected, this, &MainWindow::onNetConnected);
    connect(m_netClient, &NetClient::disconnected, this, &MainWindow::onNetDisconnected);
    connect(m_netClient, &NetClient::errorText, this, &MainWindow::onNetErrorText);
    connect(m_netClient, &NetClient::textReceived, this, &MainWindow::onNetTextReceived);
}

void MainWindow::onServerProcessError(QProcess::ProcessError error) {
    Q_UNUSED(error);

    statusBar()->showMessage("Не вдалося запустити сервер", 4000);
    statusBar()->setStyleSheet("color: #dc3c3c; font-size:12px; font-family:Fixedsys;");
}

void MainWindow::connectServerByFields() {
    const QString ip = ui->le_ip->text().trimmed();
    const unsigned short port = quint16(ui->sb_port->value());

    m_netClient->connectToServer(ip, port);
}

void MainWindow::onHostServer() {
    if (!applyNicknameFromStartScreen()) return;

    if (!m_serverProcess) {
        m_serverProcess = new QProcess(this);
        connect(m_serverProcess, &QProcess::errorOccurred, this, &MainWindow::onServerProcessError);
    }

    const QString serverExePath = QCoreApplication::applicationDirPath() + "/SelectionServer.exe";

    QStringList args;
    args << "--bind" << "0.0.0.0"
         << "--port" << QString::number(ui->sb_port->value());

    m_serverProcess->start(serverExePath, args);

    onJoinServer();
}

void MainWindow::onJoinServer() {
    if (!applyNicknameFromStartScreen()) return;

    connectServerByFields();

    goToLobby();
}

bool MainWindow::applyNicknameFromStartScreen() {
    const QString nick = ui->le_nickname->text().trimmed();
    if (nick.isEmpty()) {
        statusBar()->showMessage("Введи ім'я", 2500);
        statusBar()->setStyleSheet("color: #dc3c3c; font-size:12px; font-family:Fixedsys;");
        ui->le_nickname->setFocus();
        return false;
    }

    m_localNickname = nick;
    if (m_playerPreview)
        m_playerPreview->setNickname(m_localNickname);
    if (m_gameView)
        m_gameView->setLocalPlayerNickname(m_localNickname);
    return true;
}

void MainWindow::ensureLocalPlayerRow() {
    ui->table_playersList->setRowCount(1);
    ui->table_playersList->clearContents();

    for (int c = 0; c < ui->table_playersList->columnCount(); ++c) {
        if (!ui->table_playersList->item(0, c))
            ui->table_playersList->setItem(0, c, new QTableWidgetItem());
    }
}

QTableWidgetItem* MainWindow::createTableItem(int column) {
    QTableWidgetItem* item = ui->table_playersList->item(0, column);
    if (!item) {
        item = new QTableWidgetItem();
        ui->table_playersList->setItem(0, column, item);
    }
    return item;
}

QTableWidgetItem* MainWindow::setTableCellText(int column, const QString& text, const QFont& tableFont, const QColor& tableColor, bool center) {
    QTableWidgetItem* item = createTableItem(column);
    item->setText(text);
    item->setToolTip(text);
    item->setFont(tableFont);
    item->setForeground(QBrush(tableColor));
    if (center) item->setTextAlignment(Qt::AlignCenter);
    return item;
}

QString MainWindow::getSelectedWeaponText() const {
    if (!m_selectedWeaponButton) return QString();
    const int weapon = m_selectedWeaponButton->property("weapon").toInt();
    return (weapon == static_cast<int>(PlayerPreviewWidget::WeaponType::Katana)) ? "Катана" : QString::number(weapon);
}

QString MainWindow::getSelectedAbilityText() const {
    if (!m_selectedAbilityButton) return QString();
    const int ability = m_selectedAbilityButton->property("ability").toInt();
    return (ability == static_cast<int>(PlayerPreviewWidget::AbilityType::MirrorShield)) ? "Дзеркальний щит" : QString::number(ability);
}

QColor MainWindow::contrastingTextColor(const QColor& background) {
    const int brightness = (background.red() * 299 + background.green() * 587 + background.blue() * 114) / 1000;
    return (brightness > 140) ? Qt::black : Qt::white;
}

void MainWindow::updateLocalPlayerRow() {
    if (ui->table_playersList->rowCount() == 0) return;

    constexpr int ColName = 0;
    constexpr int ColWeapon = 1;
    constexpr int ColAbility = 2;
    constexpr int ColColor = 3;
    constexpr int ColReady = 4;

    QFont tableFont("Fixedsys");
    tableFont.setPixelSize(11);
    const QColor tableColor(224, 224, 224);

    setTableCellText(ColName, m_localNickname, tableFont, tableColor);
    setTableCellText(ColWeapon, getSelectedWeaponText(), tableFont, tableColor, true);
    setTableCellText(ColAbility, getSelectedAbilityText(), tableFont, tableColor, true);

    QColor color;
    if (m_selectedColorButton)
        color = m_selectedColorButton->property("color").value<QColor>();


    const QString colorText = color.isValid() ? color.name() : QString();
    QTableWidgetItem* colorItem = setTableCellText(ColColor, colorText, tableFont, tableColor, true);
    if (color.isValid()) {
        colorItem->setBackground(QBrush(color));
        colorItem->setForeground(QBrush(contrastingTextColor(color)));
    } else {
        colorItem->setBackground(QBrush());
        colorItem->setForeground(QBrush(tableColor));
    }

    const QString readyText = m_playerReady ? "так" : "ні";
    QTableWidgetItem* readyItem = setTableCellText(ColReady, readyText, tableFont, tableColor, true);
    const QColor readyBg = m_playerReady ? QColor(60, 220, 80) : QColor(220, 60, 60);
    readyItem->setBackground(readyBg);
    readyItem->setForeground(QBrush(contrastingTextColor(readyBg)));
}

void MainWindow::resetLobbySelectionState() {
    m_colorSelected = false;
    m_weaponSelected = false;
    m_abilitySelected = false;
    m_playerConfigured = false;
    m_playerReady = false;

    m_selectedColorButton = nullptr;
    m_selectedWeaponButton = nullptr;
    m_selectedAbilityButton = nullptr;

    ui->b_playerReady->setEnabled(false);
    ui->b_openGameView->setEnabled(false);
}

void MainWindow::updateLobbySelectionButtons() {
    for (QPushButton* button : m_colorButtons) {
        if (!button) continue;
        const bool reserved = button->property("reserved").toBool();
        const bool selected = (button == m_selectedColorButton);
        button->setEnabled(!reserved && !selected);
    }

    for (QPushButton* button : m_weaponButtons) {
        if (!button) continue;
        const bool selected = (button == m_selectedWeaponButton);
        button->setEnabled(!selected);
    }

    for (QPushButton* button : m_abilityButtons) {
        if (!button) continue;
        const bool selected = (button == m_selectedAbilityButton);
        button->setEnabled(!selected);
    }
}

void MainWindow::onPlayerReady() {
    m_playerReady = true;
    ui->b_openGameView->setEnabled(m_playerReady);
    updateLocalPlayerRow();
}

void MainWindow::goToSettings() {
    goToPage(PageSettings);
}

void MainWindow::goToStartScreen() {
    goToPage(PageStart);
}

void MainWindow::startGame() {
    goToPage(PageGame);
    m_gameView->startGameWithCountdown();
}

void MainWindow::goToPrevPage() {
    goToPage(m_prevPage);
}

void MainWindow::initColorButtons() {
    ui->b_red->setProperty("color", QColor(Qt::red));
    ui->b_blue->setProperty("color", QColor(Qt::blue));
    ui->b_yellow->setProperty("color", QColor(Qt::yellow));
    ui->b_green->setProperty("color", QColor(Qt::green));
    ui->b_cyan->setProperty("color", QColor(Qt::cyan));
    ui->b_white->setProperty("color", QColor(Qt::white));
    ui->b_black->setProperty("color", QColor(Qt::black));

    ui->b_purple->setProperty("color", QColor(128, 0, 128));
    ui->b_pink->setProperty("color", QColor(255, 105, 180));
    ui->b_orange->setProperty("color", QColor(255, 165, 0));

    m_colorButtons = {
        ui->b_red,
        ui->b_blue,
        ui->b_yellow,
        ui->b_green,
        ui->b_cyan,
        ui->b_white,
        ui->b_black,
        ui->b_purple,
        ui->b_pink,
        ui->b_orange
    };

    for (QPushButton* button : m_colorButtons) {
        connect(button, &QPushButton::clicked, this, &MainWindow::onColorClicked);
        button->setProperty("reserved", false);
        button->setEnabled(false);
    }
}

void MainWindow::onColorClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    if (m_selectedColorButton && m_selectedColorButton != button) {
        if (!m_selectedColorButton->property("reserved").toBool())
            m_selectedColorButton->setEnabled(true);
    }

    m_selectedColorButton = button;
    button->setEnabled(false);

    QColor color = button->property("color").value<QColor>();
    ui->playerPreviewWidget->setColor(color);
    if (m_gameView)
        m_gameView->setLocalPlayerUiColor(color);

    m_colorSelected = true;
    checkPlayerConfigured();
    updateLocalPlayerRow();
}

void MainWindow::initWeaponButtons() {
    ui->b_katana->setProperty("weapon", static_cast<int>(PlayerPreviewWidget::WeaponType::Katana));

    connect(ui->b_katana, &QPushButton::clicked, this, &MainWindow::onWeaponClicked);

    m_weaponButtons = { ui->b_katana };
    for (QPushButton* b : m_weaponButtons)
        if (b) b->setEnabled(false);
}

void MainWindow::onWeaponClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    if (m_selectedWeaponButton && m_selectedWeaponButton != button)
        m_selectedWeaponButton->setEnabled(true);

    m_selectedWeaponButton = button;
    button->setEnabled(false);

    int weapon = button->property("weapon").toInt();
    m_playerPreview->setWeapon(static_cast<PlayerPreviewWidget::WeaponType>(weapon));

    m_weaponSelected = true;
    checkPlayerConfigured();
    updateLocalPlayerRow();
}

void MainWindow::initAbilityButtons() {
    ui->b_mirrorShield->setProperty("ability", static_cast<int>(PlayerPreviewWidget::AbilityType::MirrorShield));

    connect(ui->b_mirrorShield, &QPushButton::clicked, this, &MainWindow::onAbilityClicked);

    m_abilityButtons = { ui->b_mirrorShield };
    for (QPushButton* button : m_abilityButtons)
        if (button) button->setEnabled(false);
}

void MainWindow::onAbilityClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    if (m_selectedAbilityButton && m_selectedAbilityButton != button)
        m_selectedAbilityButton->setEnabled(true);

    m_selectedAbilityButton = button;
    button->setEnabled(false);

    int ability = button->property("ability").toInt();
    m_playerPreview->setAbility(static_cast<PlayerPreviewWidget::AbilityType>(ability));

    m_abilitySelected = true;
    checkPlayerConfigured();
    updateLocalPlayerRow();
}

void MainWindow::checkPlayerConfigured() {
    m_playerConfigured = m_colorSelected && m_weaponSelected && m_abilitySelected;
    ui->b_playerReady->setEnabled(m_playerConfigured);
}

MovementScheme MainWindow::getMovementScheme() const { return static_cast<MovementScheme>(ui->cb_movement->currentIndex()); }

QVector<Qt::Key> MainWindow::getSlotKeys() {
    QVector<Qt::Key> keys;

    keys.append(ui->kse_firstSlot->keySequence()[0].key());
    keys.append(ui->kse_secondSlot->keySequence()[0].key());
    keys.append(ui->kse_thirdSlot->keySequence()[0].key());
    keys.append(ui->kse_fourthSlot->keySequence()[0].key());
    keys.append(ui->kse_fifthSlot->keySequence()[0].key());

    return keys;
}
