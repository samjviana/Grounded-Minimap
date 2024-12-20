//
// Created by PC-SAMUEL on 20/12/2024.
//

#include "config.h"
#include "logger.h"

#include <filesystem>
#include <regex>
#include <toml.hpp>

namespace grounded_minimap {

bool Config::debug = false;

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
    } catch (const std::exception &e) {
        Logger::Error(std::string("Config::LoadConfig - Error: ") + e.what());
    }
}

void Config::SaveConfig(const std::string &configFilePath) {

}

void Config::CreateConfig(const std::string &configFilePath) {
    try {
        toml::table configToml;

        configToml.insert_or_assign("debug", false);

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