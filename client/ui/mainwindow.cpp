#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_gameView = ui->gameViewHolder;
    m_gameView->useMovementScheme(MovementScheme::WASD);
    m_gameView->setupSlotKeys();
}

MainWindow::~MainWindow()
{
    delete ui;
}
