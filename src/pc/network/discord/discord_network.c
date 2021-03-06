#include "discord_network.h"
#include "lobby.h"
#include "pc/debuglog.h"

int64_t gNetworkUserIds[MAX_PLAYERS] = { 0 };

u8 discord_user_id_to_local_index(int64_t userId) {
    for (int i = 1; i < MAX_PLAYERS; i++) {
        if (gNetworkPlayers[i].connected && gNetworkUserIds[i] == userId) {
            return i;
        }
    }
    return UNKNOWN_LOCAL_INDEX;
}

int ns_discord_network_send(u8 localIndex, u8* data, u16 dataLength) {
    if (!gDiscordInitialized) { return 1; }
    if (gCurLobbyId == 0) { return 2; }
    DISCORD_REQUIRE(app.lobbies->send_network_message(app.lobbies, gCurLobbyId, gNetworkUserIds[localIndex], 0, data, dataLength));
    return 0;
}

void discord_network_on_message(UNUSED void* eventData, int64_t lobbyId, int64_t userId, uint8_t channelId, uint8_t* data, uint32_t dataLength) {
    gNetworkUserIds[0] = userId;

    u8 localIndex = UNKNOWN_LOCAL_INDEX;
    for (int i = 1; i < MAX_PLAYERS; i++) {
        if (gNetworkUserIds[i] == userId) {
            localIndex = i;
            break;
        }
    }

    network_receive(localIndex, (u8*)data, (u16)dataLength);
}

void discord_network_flush(void) {
    app.lobbies->flush_network(app.lobbies);
}

void ns_discord_save_id(u8 localId) {
    assert(localId > 0);
    assert(localId < MAX_PLAYERS);
    gNetworkUserIds[localId] = gNetworkUserIds[0];
    LOG_INFO("saved user id %d == %lld", localId, gNetworkUserIds[localId]);
}

void ns_discord_clear_id(u8 localId) {
    if (localId == 0) { return; }
    assert(localId < MAX_PLAYERS);
    gNetworkUserIds[localId] = 0;
    LOG_INFO("cleared user id %d == %lld", localId, gNetworkUserIds[localId]);
}

void discord_network_init(int64_t lobbyId) {
    DISCORD_REQUIRE(app.lobbies->connect_network(app.lobbies, lobbyId));
    DISCORD_REQUIRE(app.lobbies->open_network_channel(app.lobbies, lobbyId, 0, false));
    LOG_INFO("network initialized");
}

void discord_network_shutdown(void) {
    app.lobbies->flush_network(app.lobbies);
    if (gCurLobbyId == 0) { return; }
    app.lobbies->disconnect_network(app.lobbies, gCurLobbyId);
    LOG_INFO("shutdown network, lobby = %lld", gCurLobbyId);
}