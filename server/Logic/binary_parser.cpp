#include "binary_parser.h"

#include <cstdint>
#include <fstream>
#include <string>

BinaryParser::BinaryParser(): lastOffset(0) {
    std::ifstream playersFile("server/Logic/players_data.bin", std::ios::binary | std::ios::ate);

    if (playersFile.is_open()) {
        lastOffset = static_cast<uint32_t>(playersFile.tellg());
    }
}

bool BinaryParser::checkPlayerExists(const std::string& name) {
    std::ifstream playersFile("server/Logic/players.bin", std::ios::binary);
    if (!playersFile.is_open()) {
        return false;
    }

    while (true) {
        uint16_t nameLength = 0;

        if (!playersFile.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength))) {
            break;
        }

        if (nameLength == 0) {
            break;
        }

        std::string storedName(nameLength, '\0');
        if (!playersFile.read(&storedName[0], nameLength)) {
            break;
        }

        uint32_t dataOffset = 0;
        if (!playersFile.read(reinterpret_cast<char*>(&dataOffset), sizeof(dataOffset))) {
            break;
        }

        if (storedName == name) {
            return true;
        }
    }

    return false;
}

void BinaryParser::savePlayerData(const std::string& name, PlayerData data) {
    uint16_t nameLength = name.size();

    std::ofstream playersFile("server/Logic/players.bin", std::ios::binary | std::ios::app);
    playersFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
    playersFile.write(name.c_str(), nameLength);
    playersFile.write(reinterpret_cast<const char*>(&lastOffset), sizeof(lastOffset));
    playersFile.close();

    std::ofstream dataFile("server/Logic/players_data.bin", std::ios::binary | std::ios::app);
    dataFile.write(reinterpret_cast<const char*>(&data), sizeof(data));
    lastOffset += sizeof(data);
    dataFile.close();
}

PlayerData BinaryParser::loadPlayerData(const std::string& name) {
    std::ifstream playersFile("server/Logic/players.bin", std::ios::binary);
    if (!playersFile.is_open()) {
        throw std::runtime_error("BinaryParser Error: could not open players file");
    }

    while (true) {
        uint16_t nameLength = 0;

        if (!playersFile.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength))) {
            break;
        }

        if (nameLength == 0) {
            break;
        }

        std::string storedName(nameLength, '\0');
        if (!playersFile.read(&storedName[0], nameLength)) {
            break;
        }

        uint32_t dataOffset = 0;
        if (!playersFile.read(reinterpret_cast<char*>(&dataOffset), sizeof(dataOffset))) {
            break;
        }

        if (storedName == name) {
            std::ifstream dataFile("server/Logic/players_data.bin", std::ios::binary);
            if (!dataFile.is_open()) {
                throw std::runtime_error("BinaryParser Error: could not open player data file");
            }
            dataFile.seekg(dataOffset);
            PlayerData data;
            dataFile.read(reinterpret_cast<char*>(&data), sizeof(data));
            return data;
        }
    }

    throw std::runtime_error("BinaryParser Error: player not found");
}

void BinaryParser::updatePlayerData(const std::string& name, PlayerData data) {
    std::ifstream playersFile("server/Logic/players.bin", std::ios::binary);
    if (!playersFile.is_open()) {
        throw std::runtime_error("BinaryParser Error: could not open players file");
    }

    while (true) {
        uint16_t nameLength = 0;

        if (!playersFile.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength))) {
            break;
        }

        if (nameLength == 0) {
            break;
        }

        std::string storedName(nameLength, '\0');
        if (!playersFile.read(&storedName[0], nameLength)) {
            break;
        }

        uint32_t dataOffset = 0;
        if (!playersFile.read(reinterpret_cast<char*>(&dataOffset), sizeof(dataOffset))) {
            break;
        }

        if (storedName == name) {
            std::fstream dataFile("server/Logic/players_data.bin",
                                  std::ios::binary | std::ios::in | std::ios::out);
            if (!dataFile.is_open()) {
                throw std::runtime_error("BinaryParser Error: could not open player data file");
            }
            dataFile.seekp(dataOffset);
            dataFile.write(reinterpret_cast<const char*>(&data), sizeof(data));
            return;
        }
    }
}
