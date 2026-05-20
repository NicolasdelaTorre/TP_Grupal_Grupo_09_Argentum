//
// Created by nicolas on 19/5/26.
//

#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_DTOS_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_DTOS_H


enum Command{};

enum ServerMessageType{};

struct LoginResult {
    std::string username;
    bool confirmed;  // true = presionó Enter, false = cerró la ventana
};

#endif //TP_GRUPAL_GRUPO_09_ARGENTUM_DTOS_H
