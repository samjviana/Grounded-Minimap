//
// Created by PC-SAMUEL on 20/12/2024.
//

#include "config.h"
#include "logger.h"
#include "globals.h"

#include <filesystem>
#include <regex>
#include <toml.hpp>

namespace grounded_minimap {

bool Config::debug = false;
int Config::zoom = 10;
ImVec2 Config::position = ImVec2(0, 0);
ImVec2 Config::size = ImVec2(0, 0);

void Config::LoadConfig(const std::string &configFilePath) {
    try {
        toml::table configToml;

        if (!std::filesystem::exists(configFilePath)) {
            Logger::Info("Config file not found. Creating a new one...");
            CreateConfig(configFilePath);

            std::ifstream configFile(configFilePath);
            if (!configFile.is_open()) {
                throw std::runtime_error("Failed to open " + configFilePath + " after creation.");
            }
            configToml = toml::parse(configFile);
        } else {
            std::ifstream configFile(configFilePath);
            if (!configFile.is_open()) {
                throw std::runtime_error("Failed to open " + configFilePath);
            }
            configToml = toml::parse(configFile);
        }

        debug = configToml["debug"].value_or(false);
        zoom = configToml["zoom"].value_or(10);
        position.x = configToml["position"]["x"].value_or(Globals::gGameWindowSize.x - Globals::gGameWindowSize.x * 0.175f - 10);
        position.y = configToml["position"]["y"].value_or(10.0f);
        size.x = configToml["size"]["width"].value_or(Globals::gGameWindowSize.x * 0.175f);
        size.y = configToml["size"]["height"].value_or(Globals::gGameWindowSize.x * 0.175f);

    } catch (const std::exception &e) {
        Logger::Error(std::string("Config::LoadConfig - Error: ") + e.what());
    }
}

void Config::SaveConfig(const std::string &configFilePath) {
    try {
        toml::table configToml;

        configToml.insert_or_assign("debug", debug);
        configToml.insert_or_assign("zoom", zoom);
        configToml.insert_or_assign("position", toml::table{
                {"x", position.x},
                {"y", position.y}
        });
        configToml.insert_or_assign("size", toml::table{
                {"width", size.x},
                {"height", size.y}
        });

        std::ofstream configFile(configFilePath);
        if (!configFile.is_open()) {
            throw std::runtime_error("Failed to save config file.");
        }

        configFile << configToml;
        configFile.close();

        AddComments(configFilePath);

        Logger::Info("Config file saved successfully.");
    } catch (const std::exception &e) {
        Logger::Error(std::string("Config::SaveConfig - Error: ") + e.what());
    }
}

void Config::CreateConfig(const std::string &configFilePath) {
    try {
        toml::table configToml;

        configToml.insert_or_assign("debug", false);
        configToml.insert_or_assign("zoom", 10);
        configToml.insert_or_assign("position", toml::table{
                {"x", Globals::gGameWindowSize.x - Globals::gGameWindowSize.x * 0.175f - 10},
                {"y", 10}
        });
        configToml.insert_or_assign("size", toml::table{
                {"width", Globals::gGameWindowSize.x * 0.175f},
                {"height", Globals::gGameWindowSize.x * 0.175f}
        });

        std::ofstream configFile(configFilePath);
        if (!configFile.is_open()) {
            throw std::runtime_error("Failed to create config file.");
        }

        configFile << configToml;
        configFile.close();

        Logger::Info("Config file created successfully.");
        AddComments(configFilePath);

    } catch (const std::exception &e) {
        Logger::Error(std::string("Config::CreateConfig - Error: ") + e.what());
    }
}

void Config::AddComments(const std::string &configFilePath) {
    try {
        std::ifstream configFile(configFilePath);
        if (!configFile.is_open()) {
            throw std::runtime_error("Failed to open " + configFilePath);
        }

        std::ostringstream buffer;
        buffer << configFile.rdbuf();
        std::string content = buffer.str();
        configFile.close();

        std::map<std::string, std::string> comments = {
            {"debug", "Enables or disables debug mode. If set to `true`, a console window will open with the game, showing the same information as the one found in the `grounded_minimap.log`"},
            {"zoom", "Sets the zoom level of the minimap. The higher the value, the more zoomed in the minimap will be."},
            {"x", "Horizontal position of the minimap on the screen. The default value is the top right corner of the screen."},
            {"y", "Vertical position of the minimap on the screen. The default value is the top right corner of the screen."},
            {"width", "Width of the minimap. The default value is 17.5% of the game window width."},
            {"height", "Height of the minimap. The default value is 17.5% of the game window width."}
        };

        std::regex configRegex(R"((\w+)\s*=\s*.+)");
        std::string updatedContent = content;

        for (const auto& [key, comment] : comments) {
            std::regex keyRegex(key + R"(\s*=\s*.+)");
            updatedContent = std::regex_replace(updatedContent, keyRegex, "$&  # " + comment);
        }

        std::ofstream outFile(configFilePath);
        if (!outFile.is_open()) {
            throw std::runtime_error("Failed to write to " + configFilePath);
        }

        outFile << updatedContent;
        outFile.close();

        Logger::Info("Comments added to config file successfully.");
    } catch (const std::exception &e) {
        Logger::Error(std::string("Config::AddComments - Error: ") + e.what());
    }
}

} // grounded_minimap