#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../game/gameview.h"
#include "playerpreviewwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    GameView* m_gameView;
    PlayerPreviewWidget* m_playerPreview;

    void initColorButtons();
    void initWeaponButtons();
    void initAbilityButtons();

private slots:
    void onColorClicked();
    void onWeaponClicked();
    void onAbilityClicked();

};

#endif // MAINWINDOW_H
