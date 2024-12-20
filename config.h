//
// Created by PC-SAMUEL on 20/12/2024.
//
// Manages loading, saving, and creating configuration files.
//

#ifndef GROUNDED_MINIMAP_CONFIG_H
#define GROUNDED_MINIMAP_CONFIG_H

#include <string>

namespace grounded_minimap {

/**
 * Handles the configuration files and parameters, including loading, saving, and creating default settings.
 */
class Config {
public:
    static bool debug;  // Indicates whether debug mode is enabled.

    /**
     * Loads configuration settings from a file.
     *
     * @param configFilePath The full path to the configuration file.
     * @note If the file does not exist, default settings are created.
     */
    static void LoadConfig(const std::string& configFilePath);

    /**
     * Saves the current configuration settings to a file.
     *
     * @param configFilePath The full path to the configuration file.
     * @note Overwrites the file if it already exists.
     */
    static void SaveConfig(const std::string& configFilePath);
private:
    /**
     * Creates a default configuration file.
     *
     * @param configFilePath The full path to the configuration file to be created.
     * @note The file will include default values for all settings.
     */
    static void CreateConfig(const std::string& configFilePath);

    /**
     * Adds comments to the configuration file for clarity.
     *
     * @param configFilePath The full path to the configuration file.
     * @note This function is called during configuration creation to annotate settings.
     */
    static void AddComments(const std::string& configFilePath);
};

} // grounded_minimap

#endif //GROUNDED_MINIMAP_CONFIG_H
