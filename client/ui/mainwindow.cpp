#include "mainwindow.h"
#include "ui_mainwindow.h"

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
}

void MainWindow::onHostServer() {
    goToPage(PageLobby);
}

void MainWindow::onJoinServer() {
    goToPage(PageLobby);
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

    QList<QPushButton*> buttons = {
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

    for (QPushButton* button : buttons)
        connect(button, &QPushButton::clicked, this, &MainWindow::onColorClicked);
}

void MainWindow::onColorClicked() {
    QColor color = sender()->property("color").value<QColor>();
    ui->playerPreviewWidget->setColor(color);

    m_colorSelected = true;
    checkPlayerConfigured();
}

void MainWindow::initWeaponButtons() {
    ui->b_katana->setProperty("weapon", static_cast<int>(PlayerPreviewWidget::WeaponType::Katana));

    connect(ui->b_katana, &QPushButton::clicked, this, &MainWindow::onWeaponClicked);
}

void MainWindow::onWeaponClicked() {
    int weapon = sender()->property("weapon").toInt();
    m_playerPreview->setWeapon(static_cast<PlayerPreviewWidget::WeaponType>(weapon));

    m_weaponSelected = true;
    checkPlayerConfigured();
}

void MainWindow::initAbilityButtons() {
    ui->b_mirrorShield->setProperty("ability", static_cast<int>(PlayerPreviewWidget::AbilityType::MirrorShield));

    connect(ui->b_mirrorShield, &QPushButton::clicked, this, &MainWindow::onAbilityClicked);
}

void MainWindow::onAbilityClicked() {
    int ability = sender()->property("ability").toInt();
    m_playerPreview->setAbility(static_cast<PlayerPreviewWidget::AbilityType>(ability));

    m_abilitySelected = true;
    checkPlayerConfigured();
}

void MainWindow::checkPlayerConfigured() {
    m_playerConfigured = m_colorSelected && m_weaponSelected && m_abilitySelected;
    ui->b_playerReady->setEnabled(m_playerConfigured);
}
