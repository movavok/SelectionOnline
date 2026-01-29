#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QString>
#include <QPushButton>
#include <QTableWidgetItem>

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

    QString m_localNickname;

    QList<QPushButton*> m_colorButtons;
    QList<QPushButton*> m_weaponButtons;
    QList<QPushButton*> m_abilityButtons;

    QPushButton* m_selectedColorButton = nullptr;
    QPushButton* m_selectedWeaponButton = nullptr;
    QPushButton* m_selectedAbilityButton = nullptr;

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

    void resetLobbySelectionState();
    void updateLobbySelectionButtons();

    bool applyNicknameFromStartScreen();
    void ensureLocalPlayerRow();
    void updateLocalPlayerRow();

    QTableWidgetItem* createTableItem(int);
    QTableWidgetItem* setTableCellText(int, const QString&, const QFont&, const QColor&, bool = false);

    QString getSelectedWeaponText() const;
    QString getSelectedAbilityText() const;
    static QColor contrastingTextColor(const QColor&);

    void checkPlayerConfigured();

    MovementScheme getMovementScheme() const;
    QVector<Qt::Key> getSlotKeys();

private slots:
    void onHostServer();
    void onJoinServer();

    void onPlayerReady();
    void goToSettings();
    void goToStartScreen();
    void startGame();

    void goToPrevPage();

    void onColorClicked();
    void onWeaponClicked();
    void onAbilityClicked();
};

#endif // MAINWINDOW_H
