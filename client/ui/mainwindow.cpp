#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->table_playersList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    initColorButtons();
    initWeaponButtons();
    initAbilityButtons();

    m_playerPreview = ui->playerPreviewWidget;

    m_gameView = ui->gameViewHolder;
    m_gameView->useMovementScheme(MovementScheme::WASD);
    m_gameView->setupSlotKeys();
}

MainWindow::~MainWindow()
{
    delete ui;
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
}

void MainWindow::initWeaponButtons() {
    ui->b_katana->setProperty("weapon", static_cast<int>(PlayerPreviewWidget::WeaponType::Katana));

    connect(ui->b_katana, &QPushButton::clicked, this, &MainWindow::onWeaponClicked);
}

void MainWindow::onWeaponClicked() {
    int weapon = sender()->property("weapon").toInt();
    m_playerPreview->setWeapon(static_cast<PlayerPreviewWidget::WeaponType>(weapon));
}

void MainWindow::initAbilityButtons() {
    ui->b_mirrorShield->setProperty("ability", static_cast<int>(PlayerPreviewWidget::WeaponType::Katana));

    connect(ui->b_mirrorShield, &QPushButton::clicked, this, &MainWindow::onAbilityClicked);
}

void MainWindow::onAbilityClicked() {
    int ability = sender()->property("ability").toInt();
    m_playerPreview->setAbility(static_cast<PlayerPreviewWidget::AbilityType>(ability));
}
