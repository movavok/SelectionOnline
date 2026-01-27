#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->table_playersList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    initButtons();

    m_playerPreview = ui->playerPreviewWidget;

    m_gameView = ui->gameViewHolder;
    m_gameView->useMovementScheme(MovementScheme::WASD);
    m_gameView->setupSlotKeys();

    updateLobbySelectionButtons();
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

    if (page == PageGame) statusBar()->hide();
    else statusBar()->show();

    if (page == PageLobby)
        updateLobbySelectionButtons();
}

void MainWindow::onHostServer() {
    resetLobbySelectionState();
    goToPage(PageLobby);
    updateLobbySelectionButtons();
}

void MainWindow::onJoinServer() {
    resetLobbySelectionState();
    goToPage(PageLobby);
    updateLobbySelectionButtons();
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
}

void MainWindow::goToSettings() {
    goToPage(PageSettings);
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

    m_colorSelected = true;
    checkPlayerConfigured();
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
}

void MainWindow::checkPlayerConfigured() {
    m_playerConfigured = m_colorSelected && m_weaponSelected && m_abilitySelected;
    ui->b_playerReady->setEnabled(m_playerConfigured);
}
