#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QList>
#include <QString>
#include <QPushButton>
#include <QTableWidgetItem>

#include <QHostAddress>

#include "../net/netclient.h"
#include "../game/gameview.h"
#include "playerpreviewwidget.h"

#include "../net/playerstateupdates.h"

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

    QTableWidgetItem* createTableItem(int, int);
    QTableWidgetItem* setTableCellText(int, int, const QString&, const QFont&, const QColor&, bool = false);

    QString getSelectedWeaponText() const;
    QString getSelectedAbilityText() const;
    static QColor contrastingTextColor(const QColor&);

    void checkPlayerConfigured();
    void preparePlayerConfigUpdate();

    MovementScheme getMovementScheme() const;
    QVector<Qt::Key> getSlotKeys();
    bool m_isHost = false;
    QProcess* m_serverProcess = nullptr;
    NetClient* m_netClient = nullptr;

    unsigned short m_pendingHostPort = 0;
    
    quint32 m_hostPlayerId = 0;
    bool m_canStartGame = false;
    void initNetClient();
    void connectToServer(const QString&, unsigned short);
    
    void disconnectServerGraceful();
    void stopServerProcessGraceful();

    quint32 m_localPlayerId = 0;
    int m_localSlotIndex = -1;

    bool m_closing = false;
    void shutdownServerProcess();

    //start
    void goToLobby();
    bool applyNicknameFromStartScreen();
    void ensureLocalPlayerRow();
    void updateLocalPlayerRow();
    void renderLobbyRow(int row, const LobbySlot&);

    //settings

    //lobby
    void initColorButtons();
    void initWeaponButtons();
    void initAbilityButtons();

    void resetLobbySelectionState();
    void applyReservedColorsFromLobby(const QVector<LobbySlot>&);
    void updateLobbySelectionButtons();

protected slots:
    void closeEvent(QCloseEvent*) override;

private slots:
    void onServerProcessError(QProcess::ProcessError);
    void onServerProcessStarted();
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

    //net client
    void onNetConnected();
    void onNetDisconnected();
    void onNetErrorText(const QString&);
    void onWelcomeReceived(quint32, quint8);
    void onLobbyStateReceived(const QVector<LobbySlot>&);
    void onLobbyControlReceived(bool canStart, quint32 hostPlayerId);
    void onStartGameReceived();

    void onLocalPlayerStateProduced(const LocalPlayerStateUpdate&);
    void onRemotePlayerStateReceived(const RemotePlayerStateUpdate&);

    void onLocalTileChanged(qint16 tileX, qint16 tileY, quint8 tileType);
    void onRemoteTileUpdateReceived(qint16 tileX, qint16 tileY, quint8 tileType);

    void onLocalPlayerHitProduced(quint32 targetPlayerId, quint16 damage);
    void onPlayerHitReceived(quint32 attackerPlayerId, quint32 targetPlayerId, quint16 damage);

    void onLocalPlayerAttackProduced(quint32 tick, float dirX, float dirY);
    void onPlayerAttackReceived(quint32 attackerPlayerId, quint32 tick, float dirX, float dirY);

    void onLocalPickupCollected(qint16 tileX, qint16 tileY);
    void onRemotePickupCollectedReceived(qint16 tileX, qint16 tileY);
};

#endif // MAINWINDOW_H
