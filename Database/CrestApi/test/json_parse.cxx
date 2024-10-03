#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include <iomanip> 

int main() {
    std::string jsonString = R"({
        "data": {
            "0": [
                "[{\"stationEta\": -2, \"stationPhi\": 1, \"stationName\": 56, \"multiLayer\": 1, \"gasGap\": 1, \"pcbPos\": 6, \"indiv\": 0.0, \"extra\": 0.0, \"total\": 0.0, \"position\": \"right\", \"litmeasb\": 6}, {\"stationEta\": -2, \"stationPhi\": 1, \"stationName\": 56, \"multiLayer\": 1, \"gasGap\": 1, \"pcbPos\": 6, \"indiv\": 0.0, \"extra\": 0.0, \"total\": 0.0, \"position\": \"left\", \"litmeasb\": 6}, {\"stationEta\": -2, \"stationPhi\": 1, \"stationName\": 56, \"multiLayer\": 1, \"gasGap\": 1, \"pcbPos\": 7, \"indiv\": 0.0, \"extra\": 0.0, \"total\": 0.0, \"position\": \"right\", \"litmeasb\": 7}, {\"stationEta\": -2, \"stationPhi\": 1, \"stationName\": 56, \"multiLayer\": 1, \"gasGap\": 1, \"pcbPos\": 7, \"indiv\": 0.0, \"extra\": 0.0, \"total\": 0.0, \"position\": \"left\", \"litmeasb\": 7}]"
            ]
        }
    })";

    try {
        json parsedJson = json::parse(jsonString);
        
        // Access the inner string and parse it as JSON
        std::string innerString = parsedJson["data"]["0"][0];
        json innerJson = json::parse(innerString);

        // Print the parsed inner JSON
        std::cout << "Parsed Inner JSON:\n" << std::setw(4) << innerJson << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }

    return 0;
}
