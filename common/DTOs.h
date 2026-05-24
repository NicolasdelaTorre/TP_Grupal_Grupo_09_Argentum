//
// Created by nicolas on 19/5/26.
//

#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_DTOS_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_DTOS_H

#include <string>

// enum class Command { ... };
using Command = std::string;

// enum class ServerMessageType { ... };
using ServerMessageType = std::string;

struct LoginResult {
    std::string username;
    bool confirmed;  // true = presionó Enter, false = cerró la ventana
};

#endif  // TP_GRUPAL_GRUPO_09_ARGENTUM_DTOS_H
