#include "runtime/resource/asset_manager.h"

#include <limits>

AssetManager::AssetManager() {
  uint64_t max = std::numeric_limits<uint64_t>::max();
  for (uint64_t i = 0; i < max; ++i) {
    uid_queue.push(i);
  }
  uid_queue.push(max);
}

void AssetManager::load_asset(std::string){
    
}