#include "gameserver.h"

GameServer::GameServer(QObject *parent)
    : QObject(parent)
{
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &GameServer::on_new_connection);
}

bool GameServer::start_listening(quint16 port)
{
    if (!server->listen(QHostAddress::Any, port)) {
        qDebug() << "Server failed to start:" << server->errorString();
        return false;
    }
    qDebug() << "Server is listening on port" << port;
    return true;
}

QString GameServer::generate_room_code()
{
    int code = QRandomGenerator::global()->bounded(100000, 999999);
    return QString::number(code);
}

void GameServer::on_new_connection()
{
    QTcpSocket *newClient = server->nextPendingConnection();
    qDebug() << "New client connected:" << newClient->peerAddress().toString();

    connect(newClient, &QTcpSocket::readyRead, this, &GameServer::on_client_data_ready);
    connect(newClient, &QTcpSocket::disconnected, this, &GameServer::on_client_disconnected);
}

void GameServer::on_client_data_ready()
{
    QTcpSocket *senderSocket = qobject_cast<QTcpSocket*>(sender());
    if (!senderSocket) return;

    while (senderSocket->canReadLine()) {
        QByteArray line = senderSocket->readLine().trimmed();
        if (line.isEmpty()) continue;

        QJsonDocument doc = QJsonDocument::fromJson(line);
        if (!doc.isObject()) {
            qDebug() << "Received invalid JSON:" << line;
            continue;
        }

        handle_message(senderSocket, doc.object());
    }
}

void GameServer::handle_message(QTcpSocket *senderSocket, const QJsonObject &message)
{
    QString type = message["type"].toString();

    if (type == "create_room") {
        handle_create_room(senderSocket, message);
    }
    else if (type == "join_room") {
        handle_join_room(senderSocket, message);
    }
    else if (type == "start_combat") {
        handle_start_combat(senderSocket, message);
    }
    else if (type == "play_card") {
        handle_play_card(senderSocket, message);
    }
    else if (type == "end_turn") {
        handle_end_turn(senderSocket, message);
    }
    else if (type == "map_data") {
        handle_map_data(senderSocket, message);
    }
    else if (type == "room_selected") {
        handle_room_selected(senderSocket, message);
    }
    else {
        QString roomCode = client_room.value(senderSocket);
        if (roomCode.isEmpty()) {
            qDebug() << "Client not in a room, ignoring message.";
            return;
        }

        QJsonDocument forwardDoc(message);
        for (QTcpSocket *client : rooms[roomCode]) {
            if (client != senderSocket) {
                client->write(forwardDoc.toJson(QJsonDocument::Compact) + "\n");
            }
        }
    }
}

void GameServer::on_client_disconnected()
{
    QTcpSocket *disconnectedSocket = qobject_cast<QTcpSocket*>(sender());
    if (!disconnectedSocket) return;

    QString roomCode = client_room.value(disconnectedSocket);
    if (!roomCode.isEmpty()) {
        rooms[roomCode].removeAll(disconnectedSocket);

        if (roomGames.contains(roomCode)) {
            RoomGame &game = roomGames[roomCode];
            game.socketToPlayer.remove(disconnectedSocket);
            game.playerAlive.remove(disconnectedSocket);
            game.endedTurn.remove(disconnectedSocket);

            if (!rooms[roomCode].isEmpty()) {
                // FIX: only actually reassign leadership if the socket that
                // disconnected WAS the leader. Previously this always
                // crowned rooms[roomCode][0] as leader on any disconnect,
                // and never touched game.leaderSocket at all, so the
                // server's own idea of "who is leader" (used by
                // handle_start_combat / handle_map_data / handle_room_selected)
                // could permanently disagree with what clients were told.
                if (game.leaderSocket == disconnectedSocket || game.leaderSocket == nullptr) {
                    QTcpSocket *newLeader = rooms[roomCode][0];
                    game.leaderSocket = newLeader;

                    QJsonObject leaderMsg;
                    leaderMsg["type"] = "leader_changed";
                    leaderMsg["you_are_leader"] = true;
                    send_to_client(newLeader, leaderMsg);

                    for (int i = 1; i < rooms[roomCode].size(); ++i) {
                        QJsonObject notLeaderMsg;
                        notLeaderMsg["type"] = "leader_changed";
                        notLeaderMsg["you_are_leader"] = false;
                        send_to_client(rooms[roomCode][i], notLeaderMsg);
                    }
                }

                // FIX: a disconnect can complete the "everyone ended turn"
                // condition if the remaining player(s) already ended theirs.
                // Without this, the turn could stay stuck forever since
                // nobody left would call handle_end_turn again.
                check_turn_advance(roomCode);
            }

            if (rooms[roomCode].isEmpty()) {
                delete game.battleManager;
                roomGames.remove(roomCode);
                rooms.remove(roomCode);
            }
        }
        client_room.remove(disconnectedSocket);
    }

    socketUsernames.remove(disconnectedSocket);

    qDebug() << "Client disconnected.";
    disconnectedSocket->deleteLater();
}

// ---Broadcast helpers---

void GameServer::send_to_client(QTcpSocket *client, const QJsonObject &message)
{
    QJsonDocument doc(message);
    client->write(doc.toJson(QJsonDocument::Compact) + "\n");
}

void GameServer::broadcast_to_room(const QString &roomCode, const QJsonObject &message)
{
    QJsonDocument doc(message);
    for (QTcpSocket *client : rooms[roomCode]) {
        client->write(doc.toJson(QJsonDocument::Compact) + "\n");
    }
}

QJsonObject GameServer::buildCombatOver(bool victory)
{
    QJsonObject state;
    state["type"] = "combat_over";
    state["victory"] = victory;
    return state;
}

//--- State building ---

QJsonObject GameServer::buildStateUpdate(RoomGame &game, const QString &roomCode)
{
    QJsonObject state;
    state["type"] = "state_update";
    state["is_player_turn"] = game.isPlayerTurn;

    QJsonArray playersArray;
    for (QTcpSocket *socket : rooms[roomCode]) {
        Player* p = game.socketToPlayer.value(socket, nullptr);
        if (!p) continue;

        QJsonObject pObj;
        pObj["hp"] = p->getHp();
        pObj["max_hp"] = p->getMaxHp();
        pObj["energy"] = p->getEnergy();
        pObj["max_energy"] = p->getMaxEnergy();
        pObj["block"] = p->getBlock();
        pObj["gold"] = p->getGold();
        pObj["is_alive"] = game.playerAlive.value(socket, false);
        pObj["username"] = QString::fromStdString(p->getName());

        QJsonArray handArray;
        for (Card* c : p->getHand()) {
            if (c) handArray.append(QString::fromStdString(c->getName()));
        }
        pObj["hand"] = handArray;
        pObj["hand_size"] = (int)p->getHand().size();

        QJsonArray drawArray;
        for (Card* c : p->getDrawPile()) {
            if (c) drawArray.append(QString::fromStdString(c->getName()));
        }
        pObj["draw_pile"] = drawArray;
        pObj["draw_pile_size"] = (int)p->getDrawPile().size();

        QJsonArray discardArray;
        for (Card* c : p->getDiscardPile()) {
            if (c) discardArray.append(QString::fromStdString(c->getName()));
        }
        pObj["discard_pile"] = discardArray;
        pObj["discard_pile_size"] = (int)p->getDiscardPile().size();

        QJsonArray exhaustArray;
        for (Card* c : p->getExhaustPile()) {
            if (c) exhaustArray.append(QString::fromStdString(c->getName()));
        }
        pObj["exhaust_pile"] = exhaustArray;
        pObj["exhaust_pile_size"] = (int)p->getExhaustPile().size();

        QJsonArray effectsArray;
        for (auto* effect : p->getEffects()) {
            if (!effect) continue;
            QJsonObject effObj;
            effObj["name"] = QString::fromStdString(effect->getName());
            effObj["amount"] = effect->getAmount();
            effectsArray.append(effObj);
        }
        pObj["effects"] = effectsArray;

        playersArray.append(pObj);
    }
    state["players"] = playersArray;

    QJsonArray enemiesArray;
    for (Enemy* e : game.battleManager->getEnemies()) {
        QJsonObject eObj;
        eObj["name"] = QString::fromStdString(e->getName());
        eObj["hp"] = e->getHp();
        eObj["max_hp"] = e->getMaxHp();
        eObj["block"] = e->getBlock();
        eObj["intent"] = QString::fromStdString(e->getIntentString());

        QJsonArray effArray;
        for (auto* effect : e->getEffects()) {
            if (!effect) continue;
            QJsonObject effObj;
            effObj["name"] = QString::fromStdString(effect->getName());
            effObj["amount"] = effect->getAmount();
            effArray.append(effObj);
        }
        eObj["effects"] = effArray;

        enemiesArray.append(eObj);
    }
    state["enemies"] = enemiesArray;

    return state;
}

void GameServer::broadcast_state_update(const QString &roomCode)
{
    if (!roomGames.contains(roomCode)) return;
    RoomGame &game = roomGames[roomCode];
    if (!game.battleManager) return;

    QList<QTcpSocket*> roomClients = rooms[roomCode];
    QJsonObject baseState = buildStateUpdate(game, roomCode);
    QJsonArray basePlayers = baseState["players"].toArray();

    for (int i = 0; i < roomClients.size(); ++i) {
        QTcpSocket *socket = roomClients[i];
        QJsonObject state = baseState;
        QJsonArray playersArray = basePlayers;

        if (i < playersArray.size()) {
            QJsonObject playerObj = playersArray[i].toObject();
            // FIX: was `(i == 0)`, silently assuming the leader is always
            // whoever is first in rooms[roomCode]. That contradicts every
            // other leader check in this file (handle_start_combat,
            // handle_map_data, handle_room_selected, on_client_disconnected),
            // which all compare against game.leaderSocket because list
            // position doesn't change when the leader dies mid-combat.
            // Clients were shown the wrong "is_leader" flag once leadership
            // had actually transferred.
            playerObj["is_leader"] = (socket == game.leaderSocket);
            playerObj["is_you"] = true;
            playersArray[i] = playerObj;
        }

        for (int j = 0; j < playersArray.size(); ++j) {
            if (j != i) {
                QJsonObject playerObj = playersArray[j].toObject();
                playerObj["is_you"] = false;
                playersArray[j] = playerObj;
            }
        }

        state["players"] = playersArray;
        send_to_client(socket, state);
    }
}

// ---Combat: initialize player deck---

void GameServer::initialize_player_deck(Player* player)
{
    for (int i = 0; i < 5; ++i) {
        Card* strike = createCardByName("Strike");
        if (strike) player->addCardToDrawPile(strike);
    }
    for (int i = 0; i < 4; ++i) {
        Card* defend = createCardByName("Defend");
        if (defend) player->addCardToDrawPile(defend);
    }
}

// ---Combat: spawn enemy---

void GameServer::spawn_enemy_for_room(RoomGame &game, const QString &enemyName)
{
    Enemy* enemy = nullptr;

    if (enemyName == "JawWorm") enemy = new JawWorm();
    else if (enemyName == "Cultist") enemy = new Cultist();
    else if (enemyName == "Louse") enemy = new Louse();
    else if (enemyName == "SmallSlime") enemy = new SmallSlime();
    else if (enemyName == "MediumSlime") enemy = new MediumSlime();
    else if (enemyName == "LargeSlime") enemy = new LargeSlime();
    else if (enemyName == "BlueSlaver") enemy = new BlueSlaver();
    else if (enemyName == "RedSlaver") enemy = new RedSlaver();
    else if (enemyName == "Looter") enemy = new Thief("Looter");
    else if (enemyName == "Mugger") enemy = new Thief("Mugger");
    else if (enemyName == "SphericGuardian") enemy = new SphericGuardian();
    else if (enemyName == "GremlinKnob") enemy = new GremlinKnob();
    else if (enemyName == "ThreeSentries") enemy = new ThreeSentries("ThreeSentries", 38, 42);
    else if (enemyName == "BookOfStabbing") enemy = new BookOfStabbing();
    else if (enemyName == "Taskmaster") enemy = new Taskmaster();
    else if (enemyName == "KingSlime") enemy = new KingSlime(game.battleManager);
    else if (enemyName == "Hexaghost") enemy = new Hexaghost();
    else if (enemyName == "TheChamp") enemy = new TheChamp();
    else enemy = new JawWorm();

    if (game.isMultiplayer && enemy) {
        int newMaxHp = enemy->getMaxHp() * 2;
        int newHp = enemy->getHp() * 2;
        enemy->setMaxHp(newMaxHp);
        enemy->setHp(newHp);
    }

    if (enemy) {
        game.battleManager->spawnEnemy(enemy);
    }
}

// ---Combat: check game over (with persistent state save) ---

void GameServer::check_combat_over(const QString &roomCode)
{
    if (!roomGames.contains(roomCode)) return;
    RoomGame &game = roomGames[roomCode];
    if (!game.combatActive || !game.battleManager) return;

    if (game.battleManager->getEnemies().empty()) {
        // VICTORY: save persistent state before ending
        for (QTcpSocket *socket : rooms[roomCode]) {
            Player* p = game.socketToPlayer.value(socket, nullptr);
            if (!p) continue;

            PlayerRunState &rs = game.playerRunStates[socket];
            rs.hp = p->getHp();
            rs.maxHp = p->getMaxHp();
            rs.gold = p->getGold();
            rs.wasAlive = (p->getHp() > 0);

            rs.deckCardNames.clear();
            for (Card* c : p->getFullDeck()) {
                if (c) rs.deckCardNames.append(QString::fromStdString(c->getName()));
            }
        }

        game.combatActive = false;
        QJsonObject msg = buildCombatOver(true);
        broadcast_to_room(roomCode, msg);
        qDebug() << "Combat over - VICTORY in room:" << roomCode;
        return;
    }

    bool anyAlive = false;
    for (QTcpSocket *socket : rooms[roomCode]) {
        if (game.playerAlive.value(socket, false)) {
            anyAlive = true;
            break;
        }
    }

    if (!anyAlive) {
        game.combatActive = false;
        QJsonObject msg = buildCombatOver(false);
        broadcast_to_room(roomCode, msg);
        qDebug() << "Combat over - DEFEAT in room:" << roomCode;
    }
}

// ---Combat: leader transfer---

void GameServer::transfer_leader_if_needed(const QString &roomCode, QTcpSocket *deadSocket)
{
    if (!rooms.contains(roomCode) || rooms[roomCode].isEmpty()) return;

    QTcpSocket *currentLeader = nullptr;
    for (QTcpSocket *socket : rooms[roomCode]) {
        if (socket != deadSocket) {
            currentLeader = socket;
            break;
        }
    }

    if (!currentLeader) return;

    for (QTcpSocket *socket : rooms[roomCode]) {
        QJsonObject msg;
        msg["type"] = "leader_changed";
        msg["you_are_leader"] = (socket == currentLeader);
        send_to_client(socket, msg);
    }

    qDebug() << "Leader transferred in room:" << roomCode
             << "new leader:" << currentLeader->peerAddress().toString();
}

// ---Combat: enemy turn---

void GameServer::process_enemy_turn(const QString &roomCode)
{
    if (!roomGames.contains(roomCode)) return;
    RoomGame &game = roomGames[roomCode];
    if (!game.combatActive || !game.battleManager) return;

    game.isPlayerTurn = false;
    game.endedTurn.clear();

    game.battleManager->enemyTurn();
    game.battleManager->cleanupDeadEnemies();

    for (QTcpSocket *socket : rooms[roomCode]) {
        Player* p = game.socketToPlayer.value(socket, nullptr);
        if (p && p->getHp() <= 0 && game.playerAlive.value(socket, false)) {
            game.playerAlive[socket] = false;

            // FIX: compare against the authoritative leaderSocket instead of
            // list position 0. Previously this correctly detected the
            // leader dying, but only ever told clients via a message —
            // it never updated game.leaderSocket itself, so every
            // subsequent server-side leader check kept failing for the
            // player the clients now believed was leader.
            if (game.leaderSocket == socket) {
                QTcpSocket *newLeader = nullptr;
                for (QTcpSocket *candidate : rooms[roomCode]) {
                    if (candidate != socket) { newLeader = candidate; break; }
                }
                game.leaderSocket = newLeader;
                transfer_leader_if_needed(roomCode, socket);
            }
        }
    }

    broadcast_state_update(roomCode);
    check_combat_over(roomCode);

    if (game.combatActive) {
        game.isPlayerTurn = true;
        game.currentPlayerIndex = 0;

        for (QTcpSocket *socket : rooms[roomCode]) {
            Player* p = game.socketToPlayer.value(socket, nullptr);
            if (p && game.playerAlive.value(socket, false)) {
                p->resetEnergy();
                p->applyTurnStartEffects();
                p->drawCards(5);
            }
        }

        broadcast_state_update(roomCode);
    }
}

// ---Map / Room sync---

void GameServer::handle_map_data(QTcpSocket *senderSocket, const QJsonObject &message)
{
    QString roomCode = client_room.value(senderSocket);
    if (roomCode.isEmpty()) return;

    // FIX: check against the authoritative leaderSocket, not list position —
    // list position never updates when the leader dies mid-combat, so this
    // used to keep rejecting map actions from the player who actually
    // became leader.
    RoomGame &game = roomGames[roomCode];
    if (game.leaderSocket != senderSocket) return;

    game.mapData = message["data"].toObject();

    broadcast_to_room(roomCode, message);
}

void GameServer::handle_room_selected(QTcpSocket *senderSocket, const QJsonObject &message)
{
    QString roomCode = client_room.value(senderSocket);
    if (roomCode.isEmpty() || !roomGames.contains(roomCode)) return;

    // FIX: check against the authoritative leaderSocket, not list position.
    if (roomGames[roomCode].leaderSocket != senderSocket) return;

    // Forward to all clients (non-leaders auto-enter the room)
    broadcast_to_room(roomCode, message);
}

// ---Room management---

void GameServer::handle_create_room(QTcpSocket *senderSocket, const QJsonObject &message)
{
    QString roomCode = generate_room_code();
    rooms[roomCode].append(senderSocket);
    client_room[senderSocket] = roomCode;

    // FIX: the room creator is the authoritative leader from the start,
    // regardless of list position later.
    roomGames[roomCode].leaderSocket = senderSocket;

    QString username = message["username"].toString("Player");
    socketUsernames[senderSocket] = username;

    QJsonObject response;
    response["type"] = "room_created";
    response["room_code"] = roomCode;
    send_to_client(senderSocket, response);

    qDebug() << "Room created:" << roomCode << "by" << username;
}

void GameServer::handle_join_room(QTcpSocket *senderSocket, const QJsonObject &message)
{
    QString roomCode = message["room_code"].toString();

    if (!rooms.contains(roomCode) || rooms[roomCode].size() >= 2) {
        QJsonObject response;
        response["type"] = "error";
        response["message"] = "Room not found or full";
        send_to_client(senderSocket, response);
        return;
    }

    rooms[roomCode].append(senderSocket);
    client_room[senderSocket] = roomCode;

    QString username = message["username"].toString("Player");
    socketUsernames[senderSocket] = username;

    QJsonObject response;
    response["type"] = "room_joined";
    response["room_code"] = roomCode;
    broadcast_to_room(roomCode, response);

    qDebug() << "Client joined room:" << roomCode << "- Room size:" << rooms[roomCode].size();
}

// ---Combat: start (with persistent state loading) ---

void GameServer::handle_start_combat(QTcpSocket *senderSocket, const QJsonObject &message)
{
    QString roomCode = client_room.value(senderSocket);
    if (roomCode.isEmpty()) return;

    RoomGame &game = roomGames[roomCode];

    // FIX: check against the authoritative leaderSocket, not list position —
    // list position never changes when the leader dies mid-combat, so this
    // used to silently ignore start_combat from the player who actually
    // became leader, leaving the room permanently stuck on the map screen.
    if (rooms[roomCode].isEmpty() || game.leaderSocket != senderSocket) {
        qDebug() << "Non-leader tried to start combat, ignoring.";
        return;
    }

    // NOTE: 'game' already bound above (was previously re-declared here).
    if (game.battleManager) {
        delete game.battleManager;
        game.socketToPlayer.clear();
        game.playerAlive.clear();
        game.endedTurn.clear();
        // NOTE: intentionally keep playerRunStates and mapData
    }

    game.battleManager = new BattleManager();
    game.combatActive = true;
    game.isPlayerTurn = true;
    game.currentPlayerIndex = 0;
    game.endedTurn.clear();
    game.isMultiplayer = (rooms[roomCode].size() >= 2);

    for (QTcpSocket *clientSocket : rooms[roomCode]) {
        QString username = socketUsernames.value(clientSocket, "Player");
        if (username.isEmpty()) username = "Player";

        // Load persistent run state (or init defaults on first combat)
        PlayerRunState runState;
        if (game.playerRunStates.contains(clientSocket)) {
            runState = game.playerRunStates[clientSocket];
        } else {
            for (int i = 0; i < 5; ++i) runState.deckCardNames.append("Strike");
            for (int i = 0; i < 4; ++i) runState.deckCardNames.append("Defend");
            game.playerRunStates[clientSocket] = runState;
        }

        Player* p = new Player(username.toStdString(),
                               runState.hp, runState.maxHp,
                               3, runState.gold,
                               game.battleManager);
        game.battleManager->addPlayer(p);
        game.socketToPlayer[clientSocket] = p;
        game.playerAlive[clientSocket] = (runState.hp > 0);

        // Build deck from saved run state
        for (const QString &cardName : runState.deckCardNames) {
            Card* c = createCardByName(cardName.toStdString());
            if (c) p->addCardToDrawPile(c);
        }
    }

    QString enemyName = message["enemy_name"].toString("JawWorm");
    game.currentEnemyName = enemyName;
    spawn_enemy_for_room(game, enemyName);

    game.battleManager->startCombat();

    QJsonObject startMsg;
    startMsg["type"] = "combat_started";
    startMsg["is_multiplayer"] = game.isMultiplayer;
    broadcast_to_room(roomCode, startMsg);

    broadcast_state_update(roomCode);

    qDebug() << "Combat started in room:" << roomCode
             << "Players:" << rooms[roomCode].size()
             << "Multiplayer:" << game.isMultiplayer;
}

// ---Combat: play card---

void GameServer::handle_play_card(QTcpSocket *senderSocket, const QJsonObject &message)
{
    QString roomCode = client_room.value(senderSocket);
    if (roomCode.isEmpty() || !roomGames.contains(roomCode)) return;

    RoomGame &game = roomGames[roomCode];
    if (!game.combatActive || !game.battleManager) return;

    if (!game.isPlayerTurn) {
        qDebug() << "Not player's turn, ignoring play_card.";
        return;
    }

    Player* actingPlayer = game.socketToPlayer.value(senderSocket, nullptr);
    if (!actingPlayer) return;

    if (!game.playerAlive.value(senderSocket, false)) {
        qDebug() << "Dead player tried to play card, ignoring.";
        return;
    }

    QString cardName = message["card_name"].toString();

    Card* cardToPlay = nullptr;
    for (Card* c : actingPlayer->getHand()) {
        if (c && QString::fromStdString(c->getName()) == cardName) {
            cardToPlay = c;
            break;
        }
    }

    if (!cardToPlay) {
        qDebug() << "Card not found in hand:" << cardName;
        return;
    }

    // FIX: nothing was checking isPlayable() here, so an unplayable card
    // (e.g. CurseOfBellCard, or any Curse whose isPlayable() returns false)
    // could be forced into play by a hand-crafted play_card message.
    if (!cardToPlay->isPlayable()) {
        qDebug() << "Card is not playable, ignoring:" << cardName;
        return;
    }

    const auto& enemies = game.battleManager->getEnemies();
    if (enemies.empty()) return;

    int targetIndex = message.contains("target_enemy_index")
                          ? message["target_enemy_index"].toInt(0)
                          : 0;
    if (targetIndex < 0 || targetIndex >= (int)enemies.size())
        targetIndex = 0;

    Enemy* target = enemies[targetIndex];

    game.battleManager->playCardAction(actingPlayer, cardToPlay, target);
    game.battleManager->cleanupDeadEnemies();

    broadcast_state_update(roomCode);
    check_combat_over(roomCode);
}

// ---Combat: end turn---

void GameServer::handle_end_turn(QTcpSocket *senderSocket, const QJsonObject &message)
{
    QString roomCode = client_room.value(senderSocket);
    if (roomCode.isEmpty() || !roomGames.contains(roomCode)) return;

    RoomGame &game = roomGames[roomCode];
    if (!game.combatActive || !game.battleManager) return;

    if (!game.isPlayerTurn) {
        qDebug() << "Not player's turn, ignoring end_turn.";
        return;
    }

    if (!game.playerAlive.value(senderSocket, false)) {
        qDebug() << "Dead player tried to end turn, ignoring.";
        return;
    }

    if (game.endedTurn.contains(senderSocket)) {
        qDebug() << "Player already ended turn, ignoring.";
        return;
    }

    game.endedTurn.insert(senderSocket);

    qDebug() << "Player ended turn. Ended:" << game.endedTurn.size()
             << "Total alive:" << game.battleManager->getPlayers().size();

    check_turn_advance(roomCode);
}

// ---Combat: shared turn-advance check---
// FIX: extracted from handle_end_turn so on_client_disconnected can also
// trigger it. Previously only handle_end_turn ever checked whether
// "everyone has ended turn" — so if a player disconnected mid-combat while
// a teammate was already waiting on them, nobody was left to call
// handle_end_turn again and the turn (and therefore card-playing) stayed
// stuck forever.
void GameServer::check_turn_advance(const QString &roomCode)
{
    if (!roomGames.contains(roomCode)) return;
    RoomGame &game = roomGames[roomCode];
    if (!game.combatActive || !game.battleManager) return;
    if (!game.isPlayerTurn) return;

    int aliveCount = 0;
    for (QTcpSocket *socket : rooms[roomCode]) {
        if (game.playerAlive.value(socket, false)) {
            aliveCount++;
        }
    }

    if (aliveCount > 0 && (int)game.endedTurn.size() >= aliveCount) {
        qDebug() << "All players ended turn. Processing enemy turn...";
        process_enemy_turn(roomCode);
    } else {
        broadcast_state_update(roomCode);
    }
}