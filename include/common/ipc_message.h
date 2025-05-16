#pragma once

#include "global_enums.h"
#include <cstdint>

// Structure pour les messages échangés entre les tâches
struct IPCMessage {
    MessageType type;       // Type de message
    uint32_t senderTaskId;  // ID de la tâche expéditrice (optionnel)
    void* payload;          // Pointeur vers les données du message (doit être casté)
    size_t payloadSize;     // Taille des données du message

    IPCMessage(MessageType t = MessageType::SYSTEM_SHUTDOWN_REQUEST, void* p = nullptr, size_t s = 0, uint32_t senderId = 0)
        : type(t), senderTaskId(senderId), payload(p), payloadSize(s) {}
};

// TODO: Envisager un pool d'objets pour les payloads ou des types de payload spécifiques pour éviter les allocations dynamiques fréquentes.

