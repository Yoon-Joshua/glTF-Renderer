#pragma once
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <vector>
using json = nlohmann::json;
struct ShaderMacroConfig {
  std::vector<std::string> attribute_macros;
  std::vector<std::string> binding_macros;

  std::unordered_map<std::string, int> attribute_macros_to_index;
  std::unordered_map<std::string, int> binding_macros_to_index;

  void load_config(
      std::string file_path = "../config/shader_macro_config.json") {
    std::ifstream f(file_path);
    json data = json::parse(f);
    f.close();

    attribute_macros = data["attribute_macro"];
    binding_macros = data["binding_macro"];

    for (unsigned i = 0; i < attribute_macros.size(); ++i) {
      attribute_macros[i] = "D" + attribute_macros[i];
      attribute_macros_to_index[attribute_macros[i]] = i;
    }
    for (unsigned i = 0; i < binding_macros.size(); ++i) {
      binding_macros[i] = "D" + binding_macros[i];
      binding_macros_to_index[binding_macros[i]] = i;
    }
  }

  std::string shitf_processes_to_index(std::vector<std::string>& processes) {
    uint32_t attribute_bit = 0;
    uint32_t binding_bit = 0;
    for (std::string& p : processes) {
      if (attribute_macros_to_index.find(p) !=
          attribute_macros_to_index.end()) {
        auto index = attribute_macros_to_index[p];
        attribute_bit = attribute_bit | (1 << index);
      } else if (binding_macros_to_index.find(p) !=
                 binding_macros_to_index.end()) {
        auto index = binding_macros_to_index[p];
        binding_bit = binding_bit | (1 << index);
      }
    }
    return std::to_string(attribute_bit) + "." + std::to_string(binding_bit);
  }

  // 第2代
  std::string shitf_processes_to_index(std::string abs_path,
                                       std::vector<std::string>& processes) {
    size_t pos = abs_path.find_last_of('.');
    std::string basename = abs_path.substr(0, pos);
    std::ifstream f(basename + ".macro.json");
    json data = json::parse(f);
    f.close();

    std::vector<std::vector<std::string>> macros;
    std::vector<std::unordered_map<std::string, int>> macros_to_index;

    std::vector<std::string> keys = data.at("all");
    // for (auto i = data.begin(); i != data.end(); i++) {
    //   macros.push_back(i.value()["macros"]);
    //   macros_to_index.push_back(std::unordered_map<std::string, int>());
    // }
    for (auto k : keys) {
      macros.push_back(data[k.c_str()]["macros"]);
      macros_to_index.push_back(std::unordered_map<std::string, int>());
    }

    for (int i = 0; i < macros.size(); ++i) {
      for (int j = 0; j < macros[i].size(); ++j) {
        macros[i][j] = "D" + macros[i][j];
        macros_to_index[i][macros[i][j]] = j;
      }
    }

    std::string ret = "";
    for (int i = 0; i < macros.size(); ++i) {
      uint32_t bits = 0;
      for (std::string& p : processes) {
        if (macros_to_index[i].find(p) != macros_to_index[i].end()) {
          auto index = macros_to_index[i][p];
          bits = bits | (1 << index);
        }
      }
      if (ret == "") {
        ret = ret + std::to_string(bits);
      } else {
        ret = ret + "." + std::to_string(bits);
      }
    }
    return ret;
  }
};