#pragma once

#include <cstdint>
#include <queue>
#include <string>
#include <unordered_map>

enum class AssetType { MESH, MATERIAL };

class AssetManager {
 public:
  AssetManager();
  void load_asset(std::string);

 private:
  std::queue<uint64_t> uid_queue;
  std::unordered_map<uint64_t, int> assets;
};