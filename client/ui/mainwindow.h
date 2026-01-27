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
    enum Page {
        PageStart,
        PageLobby,
        PageGame,
        PageSettings
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    GameView* m_gameView;
    PlayerPreviewWidget* m_playerPreview;

    Page m_prevPage = PageStart;
    void goToPage(Page);

    bool m_colorSelected = false;
    bool m_weaponSelected = false;
    bool m_abilitySelected = false;
    bool m_playerConfigured = false;
    bool m_playerReady = false;

    void initButtons();
    //settings

    //lobby
    void initColorButtons();
    void initWeaponButtons();
    void initAbilityButtons();

    void checkPlayerConfigured();

private slots:
    void onHostServer();
    void onJoinServer();

    void onPlayerReady();
    void goToSettings();
    void startGame();

    void goToPrevPage();

    void onColorClicked();
    void onWeaponClicked();
    void onAbilityClicked();
};

#endif // MAINWINDOW_H
