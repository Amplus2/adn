#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

std::string json2adn(json j) {
    if(j.is_object()) {
        std::string s = "{";
        for(auto& [k, v] : j.items()) {
            s += json2adn(k) + " " + json2adn(v) + " ";
        }
        s.pop_back();
        return s + "}";
    }
    if(j.is_array()) {
        std::string s = "[";
        for(auto& e : j) {
            s += json2adn(e) + " ";
        }
        s.pop_back();
        return s + "]";
    }
    if(j.is_string()) {
        std::string s = j;
        size_t i = 0;
        while((i = s.find("\"", i)) != std::string::npos) {
            s.replace(i, 1, "\\\"");
            i += 2;
        }
        return "\"" + s + "\"";
    }
    if(j.is_null()) return "null";
    if(j.is_boolean()) return j ? "true" : "false";
    if(j.is_number_float()) return std::to_string((double)j);
    if(j.is_number_integer()) return std::to_string((int64_t)j);
    if(j.is_number_unsigned()) return std::to_string((uint64_t)j);
    return j;
}

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << json2adn(json::parse(std::cin)) << std::endl;
    } else {
        for(int i = 1; i < argc; i++) {
            std::cout << json2adn(strcmp(argv[i], "-") ? json::parse(std::cin)
                                                       : json::parse(std::ifstream(argv[i])))
                      << std::endl;
        }
    }
}
