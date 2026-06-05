/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelCsvWaferIdTool.h"

#include "GaudiKernel/EventContext.h"
#include "PathResolver/PathResolver.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

ITkPixelCsvWaferIdAlg::ITkPixelCsvWaferIdAlg(const std::string& name,
                                               ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode ITkPixelCsvWaferIdAlg::initialize() {
    ATH_MSG_INFO("Initializing CsvWaferId algorithm");
    ATH_CHECK(detStore()->retrieve(m_pixIdHelper, "PixelID"));
    ATH_MSG_INFO("Retrieved PixelID helper");
    ATH_CHECK(loadCsv());
    ATH_MSG_INFO("Loaded CSV file");
    return StatusCode::SUCCESS;
}

StatusCode ITkPixelCsvWaferIdAlg::execute(const EventContext& ctx) const {
    if (m_done.load(std::memory_order_acquire)) {
        return StatusCode::SUCCESS;
    }

    bool expected = false;
    if (!m_done.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        return StatusCode::SUCCESS;
    }

    ATH_MSG_INFO("Executing CsvWaferId algorithm");

    std::ofstream output(m_outputFile.value());
    if (!output.good()) {
        ATH_MSG_FATAL("Could not open wafer ID output file: " << m_outputFile.value());
        return StatusCode::FAILURE;
    }

    for (const CsvRow& row : m_rows) {
        const Identifier id = waferId(row);
        const auto compactId = id.get_identifier32().get_compact();
        output << compactId << "\n";
    }

    ATH_MSG_INFO("Wrote " << m_rows.size() << " wafer IDs to " << m_outputFile.value());
    return StatusCode::SUCCESS;
}

StatusCode ITkPixelCsvWaferIdAlg::loadCsv() {
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

    ATH_MSG_INFO("Loading CSV");
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
        row.md = trim(fields[3]);
        row.fe = std::stoi(trim(fields[4]));

        m_rows.push_back(row);
    }

    ATH_MSG_INFO("Loaded " << m_rows.size() << " CSV rows from " << resolvedCsv);
    return StatusCode::SUCCESS;
}

Identifier ITkPixelCsvWaferIdAlg::waferId(const CsvRow& row) const {
    ATH_MSG_DEBUG("waferId lookup for SP chain " << row.spChain
                    << ", module " << row.md << ", FE " << row.fe);

    // Placeholder: the concrete mapping from SP chain/module/FE to a PixelID
    // should be implemented here using the PixelID helper.

    //SP chain is like G-IS-L05-R05-A-SP2
    std::vector<std::string> spChain_cur = parseSPChain(row.spChain);
    //
    int bec = barrel_ec(spChain_cur);
    int ld = layer_disk(spChain_cur);
    int phi = phi_module(spChain_cur, row.md, row.fe );
    int eta = eta_module(spChain_cur, row.md, row.fe );

    return m_pixIdHelper->wafer_id(bec, ld, phi, eta);
}

std::string ITkPixelCsvWaferIdAlg::trim(const std::string& input) {
    const auto begin = input.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        //std::cout <<  "TRIM: bad input " << std::endl;
        //ATH_MSG_INFO( "TOTO ");
        //std::cout << input << std::endl;
        return "";
    }

    const auto end = input.find_last_not_of(" \t\r\n");
    return input.substr(begin, end - begin + 1);
}

std::vector<std::string> ITkPixelCsvWaferIdAlg::splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;

    while (std::getline(ss, field, ',')) {
        fields.push_back(field);
    }

    return fields;
}

std::vector<std::string> ITkPixelCsvWaferIdAlg::parseSPChain(const std::string& spChain) {
    std::vector<std::string> elements;
    std::stringstream ss(spChain);
    std::string element;

    while (std::getline(ss, element, '-')) {
        elements.push_back(element);
    }

    return elements;
}


int ITkPixelCsvWaferIdAlg::barrel_ec(const std::vector<std::string>& spchain) const {
    int side = (spchain[4] == "A") ? 1 : -1; // A for side pos, C for side neg
    if(spchain[1] == "IS" && (spchain[2] == "L0" ||  spchain[2] == "L1")){ //inner flat barrel
        return side*1;
    }
    else if(spchain[1] == "OB" && (spchain[2] == "L2" ||  spchain[2] == "L3" || spchain[2] == "L4") && (spchain.at(3)[0] == 'B' )){ // Outer flat barrel has 3 layers
        return side*1;
    }
    else{
        return side*2; // endcap is +2 or -2 - this includes barrel rings, in offline they are treated as endcap
    }
}

int ITkPixelCsvWaferIdAlg::layer_disk(const std::vector<std::string>& spchain) const {
    int b_ec = barrel_ec(spchain);
    if( fabs(b_ec) == 1 ){ // flat barrel
        return spchain.at(2)[1] - '0';
    }
    else{ // endcap and barrel rings - all considered as 'endcap' disks
        if(spchain[2] == "L01" && (spchain.at(5) == "SP1" || spchain.at(5) == "SP3") ){ // barrel vertical small combined rings, disk 0
            return 0;
        }
        else if(spchain[2] == "L01" && (spchain.at(5) == "SP2" || spchain.at(5) == "SP4") ){ // barrel vertical large combined rings, disk 2
            return 2;
        }
        else if(spchain[1] == "OB" && (spchain[2] == "L2" || //OB inclined rings, disks 3, 5, 7
            spchain[2] == "L3" ||
            spchain[2] == "L4") ){
            return (2* ((spchain.at(2))[1] -'0') - 1);
        }
        else if(spchain[1] == "IS" && spchain[2] == "L05"){
            return 1; // end-cap rings, inner system, disk 1
        } 
        else if(spchain[1] == "IS" && spchain[2] == "L1"){
            return 2; // end-cap rings, inner system, layer 2
        }
        else if(spchain[1] == "EC"){ //outer end-cap
            return 2* (spchain.at(2)[1]-'0'); // disks 4, 6, 8
        }
        else{
            ATH_MSG_WARNING("Bad values in layer_disk function, return -9999");
            return -9999;
        }
    }
}

int ITkPixelCsvWaferIdAlg::phi_module(const std::vector<std::string>& spchain, const std::string& mod, int fe ) const {

    int b_ec = barrel_ec(spchain);
    int ld = layer_disk(spchain);
    
    if( fabs(b_ec) == 1 ){ // flat barrel
        std::string phi_str = (spchain.at(3)).substr(1,2);
        return std::stoi(phi_str) - 1;
    }
    else{ // endcap and barrel rings - all considered as disks
        if(spchain[2] == "L01" && (spchain.at(5) == "SP1" || spchain.at(5) == "SP3") ){ // barrel vertical small combined rings, disk 0
            std::string sp_str(1, (spchain.at(5)[2])); // SP=1 and SP=3 alternate in phi
            if(sp_str == "1"){
                return 6 * (stoi(mod) - 1 ) + 2 * (fe - 1); // probably wrong offset, TODO need to revisit
            }
            else if(sp_str == "3"){
                return 6 * (stoi(mod) - 1 ) + 2 * (fe - 1) + 1; // probably wrong offset, TODO need to revisit
            }
            else{
                ATH_MSG_WARNING("Bad input for phi_module,return -9999 ");
                return -9999;
            }
        }
        else if(spchain[2] == "L01" &&
            (spchain.at(5) == "SP2" || spchain.at(5) == "SP4") ){ // barrel vertical large combined rings (quad modules), disk 2
            std::string sp_str(1, spchain.at(5)[2]); // SP=2 and SP=4 alternate in phi
             if(sp_str == "2"){
                return 2 * (stoi(mod) - 1 ) ; // probably wrong offset, TODO need to revisit
            }
            else if(sp_str == "4"){
                return 2 * (stoi(mod) - 1 ) + 1; // probably wrong offset, TODO need to revisit
            }
            else{
                ATH_MSG_WARNING("Bad input for phi_module,return -9999 ");
                return -9999;
            }
        }
        //barrel inclined rings
        else if(ld == 3 || ld == 5 || ld ==7){ //OB inclined rings, disks 3, 5, 7
            std::string phi_str = mod.substr(3,2); 
            int phi = std::stoi(phi_str);
            return phi;
        }
        else if(ld == 1){  //end-cap intermediate rings, inner system, disk 1 - triplets: one module per FE /!\ 0-17
            return 6 * (stoi(mod) - 1 ) + 2 * (fe - 1); // probably wrong offset, TODO need to revisit
        }
        else if(ld == 2){  // end-cap rings, inner system, layer 2 // 0-19
            std::string sp_str(1, spchain.at(5)[2]); // SP=1 and SP=2 alternate in phi
            if(sp_str == "1"){
                return 2 * (stoi(mod) - 1 ) ; // probably wrong offset, TODO need to revisit
            }
            else if(sp_str == "2"){
                return 2 * (stoi(mod) - 1 ) + 1; // probably wrong offset, TODO need to revisit
            }
            //return stoi(mod) * fe; 
        }
        else if(ld == 4 || ld == 6 || ld ==8){ //Outer EC disks 4, 6, 8
            std::string phi_str = mod.substr(2,2); 
            int phi = std::stoi(phi_str);
            return phi;
        }
        else{
            ATH_MSG_WARNING("Bad input for phi_module,return -9999 ");
            return -9999;
        }
    }
    return -9999;
}


int ITkPixelCsvWaferIdAlg::eta_module(const std::vector<std::string>& spchain, const std::string& mod, int fe) const {
    int b_ec = barrel_ec(spchain);
    int ld = layer_disk(spchain);
    int side = (spchain[4] == "A") ? 1 : -1; // A for side pos, C for side neg

    if( fabs(b_ec) == 1 ){ // flat barrel
        if(ld ==0){ //triplets, one front-end is considered as one module
            return side * (3 * (std::stoi(mod) -1) + fe );
        }
        else if(ld == 1){
            return side * std::stoi(mod);
        }
        else if(ld < 5){
            std::string eta_str(1, mod[4]);
            int eta = std::stoi(eta_str);
            eta = (mod[2] == 'T') ? 2*eta + 1 : 2*eta; //starts with Bottom
            return side * eta;
        }
        else{
            ATH_MSG_WARNING("Bad layer for flat barrel: " << ld);
            return -9999;
        }
    }
    //barrel rings and end caps - all considered as disks
    else{
        if(spchain[2] == "L01"){ // barrel vertical small and large combined rings, disk 0
            std::string eta_str = (spchain.at(3)).substr(1,2);
            return std::stoi(eta_str);
        }
        else if(ld == 3 || ld == 5 || ld ==7){ //OB inclined rings, disks 3, 5, 7
            std::string eta_str = (spchain.at(3)).substr(1,2);
            return std::stoi(eta_str) - 1 ;
        }
        else if(ld == 1){  //end-cap rings, inner system, disk 1 (L05) - triplets: one module per FE
            std::string eta_str = (spchain.at(3)).substr(1,2);
            return std::stoi(eta_str) - 1;
        }
        else if(ld == 2){  // end-cap rings, inner system, disk 2 - eta from 15 to 22
            std::string eta_str = (spchain.at(3)).substr(1,2);
            return std::stoi(eta_str) + 14 ;
        }
        else if(ld == 4 || ld == 6 || ld ==8){ //Outer EC disks 4, 6, 8
            std::string eta_str = (spchain.at(3)).substr(1,2);
            return std::stoi(eta_str) - 1 ;
        }
        else{
            ATH_MSG_WARNING("Bad input for eta_module,return -9999 ");
            return -9999;
        }
    }
    return -9999;
}


