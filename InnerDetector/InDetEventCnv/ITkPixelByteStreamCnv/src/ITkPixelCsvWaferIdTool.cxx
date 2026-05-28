/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelCsvWaferIdTool.h"

#include "PathResolver/PathResolver.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

ITkPixelCsvWaferIdTool::ITkPixelCsvWaferIdTool(const std::string& type,
                                               const std::string& name,
                                               const IInterface* parent)
  : AthAlgTool(type, name, parent)
{
}

StatusCode ITkPixelCsvWaferIdTool::initialize() {
    ATH_CHECK(detStore()->retrieve(m_pixIdHelper, "PixelID"));
    ATH_CHECK(loadCsv());
    return StatusCode::SUCCESS;
}

StatusCode ITkPixelCsvWaferIdTool::loadCsv() {
    const std::string resolvedCsv = PathResolver::find_file(m_csvFile.value(), "DATAPATH");
    if (resolvedCsv.empty()) {
        ATH_MSG_FATAL("Could not resolve CSV file: " << m_csvFile.value());
        return StatusCode::FAILURE;
    }

    std::ifstream input(resolvedCsv);
    if (!input.good()) {
        ATH_MSG_FATAL("Could not open CSV file: " << resolvedCsv);
        return StatusCode::FAILURE;
    }

    m_rows.clear();

    std::string line;
    bool firstLine = true;
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }

        if (firstLine) {
            firstLine = false;
            continue;
        }

        const std::vector<std::string> fields = splitCsvLine(line);
        if (fields.size() < 5) {
            ATH_MSG_WARNING("Skipping malformed CSV line: " << line);
            continue;
        }

        CsvRow row;
        row.spChain = trim(fields[0]);
        row.module = std::stoi(trim(fields[3]));
        row.fe = std::stoi(trim(fields[4]));

        m_rows.push_back(row);
    }

    ATH_MSG_INFO("Loaded " << m_rows.size() << " CSV rows from " << resolvedCsv);
    return StatusCode::SUCCESS;
}

Identifier ITkPixelCsvWaferIdTool::waferId(const CsvRow& row) const {
    ATH_MSG_WARNING("waferId lookup for SP chain " << row.spChain
                    << ", module " << row.module << ", FE " << row.fe
                    << " is not implemented yet.");

    // Placeholder: the concrete mapping from SP chain/module/FE to a PixelID
    // should be implemented here using the PixelID helper.

    //SP chain is like G-IS-L05-R05-A-SP2
    std::vector<std::string> spChain_cur = parseSPChain(row.spChain);
    //
    barrelLayer

        return m_pixIdHelper->wafer_id(barrelLayer, layerDisk, phiModule, etaModule);
    }

    return Identifier();
}

std::string ITkPixelCsvWaferIdTool::trim(const std::string& input) {
    const auto begin = input.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return "";
    }

    const auto end = input.find_last_not_of(" \t\r\n");
    return input.substr(begin, end - begin + 1);
}

std::vector<std::string> ITkPixelCsvWaferIdTool::splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;

    while (std::getline(ss, field, ',')) {
        fields.push_back(field);
    }

    return fields;
}

std::vector<std::string> ITkPixelCsvWaferIdTool::parseSPChain(const std::string& spChain) {
    std::vector<std::string> elements;
    std::stringstream ss(spChain);
    std::string element;

    while (std::getline(ss, element, '-')) {
        elements.push_back(element);
    }

    return elements;
}


int ITkPixelCsvWaferIdTool::barrel_ec(std::vector<std::string> spchain){
    int side = (spchain[4] == "A") ? 1 : -1; // A for side pos, C for side neg
    if(spchain[1] == "IS" && (spchain[2] == "L0" ||  spchain[2] == "L1"){ //inner flat barrel
        return side*1;
    }
    // Outer flat barrel has 3 layers
    else if(spchain[1] == "OB" && (spchain[2] == "L2" ||  spchain[2] == "L3" || spchain[2] == "L4") && (spchain.at(3)[0] == "B" ){
        return side*1;
    }
    else{
        return side*2; // endcap is +2 or -2 - this includes barrel rings, in offline they are treated as endcap
    }
}

int ITkPixelCsvWaferIdTool::layer_disk(std::vector<std::string> spchain){
    int b_ec = barrel_ec(spchain);
    if( fabs(b_ec) == 1 ){ // flat barrel
        return spchain.at(2)[1];
    }
    else{ // endcap and barrel rings - all considered as disks
        if(spchain[2] == "L01"){ // barrel rings, first layer (disk 0)
            return 0;
        }
        else if(spchain[1] == "OB" && (spchain[2] == "L2" || //OB inclined rings, disks 3, 5, 7
            spchain[2] == "L3" ||
            spchain[2] == "L4") ){
            return (2* (int)(spchain.at(2)[1]) - 1);
        }
        else if(spchain[1] == "IS" && spchain[2] == "L05"){
            return 1; // end-cap rings, inner system, layer 1
        } 
        else if(spchain[1] == "IS" && spchain[2] == "L1"){
            return 2; // end-cap rings, inner system, layer 2
        }
        else if(spchain[1] == "EC"){ //outer end-cap
            return (2* (int)(spchain.at(2)[1])); // disks 4, 6, 8
        }


    }
}

int ITkPixelCsvWaferIdTool::eta_module(std::vector<std::string> spchain){
    

}