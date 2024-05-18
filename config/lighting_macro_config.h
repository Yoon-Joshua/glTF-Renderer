#pragma once
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <vector>
using json = nlohmann::json;
struct ShaderMacroConfig {
  std::vector<std::string> macros;

  std::unordered_map<std::string, int> macros_to_index;

  void load_config(
      std::string file_path = "../config/lighting_macro_config.json") {
    std::ifstream f(file_path);
    json data = json::parse(f);
    f.close();

    macros = data["macro"];

    for (unsigned i = 0; i < macros.size(); ++i) {
      macros[i] = "D" + macros[i];
      macros_to_index[macros[i]] = i;
    }
  }

  std::string shitf_processes_to_index(std::vector<std::string>& processes) {
    uint32_t bits = 0;
    for (std::string& p : processes) {
      if (macros_to_index.find(p) != macros_to_index.end()) {
        auto index = macros_to_index[p];
        bits = bits | (1 << index);
      }
    }
    return std::to_string(bits);
  }
};