#pragma once
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <format>
#include <variant>
#include <type_traits>
#include "raylib.h"

struct ShaderAsset {
    Shader shader;
    std::unordered_map<std::string, int> locations;
};

class AssetManager {
private:    
    std::unordered_map<std::string, std::variant<Texture2D, Model, ShaderAsset>> assets;

public:
    ~AssetManager() {
        UnloadAll();
    }

    template<typename T>
    bool LoadAsset(const std::filesystem::path& filePath, const std::string& id) {
        static_assert(std::is_same_v<T, Model> || std::is_same_v<T, Texture2D> || std::is_same_v<T, ShaderAsset>, 
                      "[AssetManager] : Wrong type, please submit Model, Texture2D or Shader.");

        if (assets.find(id) != assets.end()) {
            std::cout << std::format("[AssetManager] Error: ID '{}' already exists in the assets map.\n", id);
            return false;
        }

        T asset;

        if constexpr (std::is_same_v<T, Model>) {
            asset = ::LoadModel(filePath.string().c_str());
            if (asset.meshCount <= 0) {
                std::cout << std::format("[AssetManager] Error: Failed to load Model from path '{}'\n", filePath);
                return false;
            }
        } else if constexpr (std::is_same_v<T, Texture2D>) {
            asset = ::LoadTexture(filePath.string().c_str());
            if (asset.id == 0) {
                std::cout << std::format("[AssetManager] Error: Failed to load Texture2D from path '{}'\n", filePath);
                return false;
            }
        } else if constexpr (std::is_same_v<T, ShaderAsset>) {
            Shader customShader = ::LoadShader(nullptr, filePath.string().c_str());
            if (customShader.id == 0) {
                std::cout << std::format("[AssetManager] Error: Failed to load Shader from path '{}'\n", filePath);
                return false;
            }
            int roughnessLocation = GetShaderLocation(customShader, "u_roughness");
            int metallicLocation = GetShaderLocation(customShader, "u_metallic");
            ShaderAsset shaderAsset;
            shaderAsset.locations["u_roughness"] = roughnessLocation;
            shaderAsset.locations["u_metallic"] = metallicLocation;
            shaderAsset.shader = customShader;
            asset = shaderAsset;
        }

        assets[id] = asset;
        return true;
    }

    template<typename T>
    T& GetAsset(const std::string& id) {
        static_assert(std::is_same_v<T, Model> || std::is_same_v<T, Texture2D> || std::is_same_v<T, ShaderAsset>, 
                      "[AssetManager] : Wrong type requested.");

        auto it = assets.find(id);
        if (it == assets.end()) {
            std::cout << std::format("[AssetManager] Warning: Asset ID '{}' not found!\n", id);
            if constexpr (std::is_same_v<T, Texture2D>) {
                static Texture2D emptyTexture = { 0 };
                return emptyTexture;
            } else if constexpr (std::is_same_v<T, Model>) {
                static Model emptyModel = { 0 };
                return emptyModel;
            } else if constexpr(std::is_same_v<T, Shader>) {
                static Shader emptyShader = { 0 };
                return emptyShader;
            }
        }
        return std::get<T>(it->second);
    }  
    void UnloadAll() {
        for (auto& [id, asset] : assets) {
            std::visit([](auto&& arg) {
                using Type = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<Type, Model>) {
                    ::UnloadModel(arg);
                } else if constexpr (std::is_same_v<Type, Texture2D>) {
                    ::UnloadTexture(arg);
                } else if constexpr(std::is_same_v<Type, ShaderAsset>) {
                    ::UnloadShader(arg.shader);
                }
            }, asset);
        }
        assets.clear();
    }
};