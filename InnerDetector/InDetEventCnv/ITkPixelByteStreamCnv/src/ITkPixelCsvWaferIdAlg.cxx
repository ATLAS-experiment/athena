/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelCsvWaferIdAlg.h"
#include "InDetIdentifier/PixelID.h"


#include "GaudiKernel/EventContext.h"
#include "PathResolver/PathResolver.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <limits>
#include <bit>


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

    ATH_CHECK ( ctx.valid() );

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
    output << "DetectorResourceID,True DetectorResourceID,FELIX Card Name,Uplink Pin,DMA buffer,SourceID\n"; 

    std::vector<int> dma_buffer_vec = DmaBuffer();

    for (size_t i = 0; i < m_rows.size(); ++i) {
        const CsvRow& row = m_rows[i];
        const auto w_and_fe_id = waferId(row);
        const Identifier& id = std::get<0>(w_and_fe_id);
        const int fe = std::get<1>(w_and_fe_id);
        std::bitset<32> onlineId = std::get<2>(w_and_fe_id);
        const std::string waferId_str = id.get_identifier32().getString();
        const auto bec = m_pixIdHelper->barrel_ec(id);
        const auto ld = m_pixIdHelper->layer_disk(id);
        const auto phi = m_pixIdHelper->phi_module(id);
        const auto eta = m_pixIdHelper->eta_module(id);

        std::bitset<32> sID = sourceID( parseSPChain(row.spChain), row.flx_card_device, dma_buffer_vec[i]);

        std::string x = waferId_str.substr(0, waferId_str.length() - 2 );
        std::stringstream ss;
        ss << std::hex << x;
        unsigned n;
        ss >> n;
        std::bitset<32> b(n);
        b <<= 2; //shift left by two bits, to add FE bits
        // Check bit position
        bitcheck(b, 2, 25, "DetResID: Wafer ID");

        std::bitset<32> febits = std::bitset<32>(fe);
        // Check bit position
        bitcheck(febits, 0, 1, "DetResID: Front-End ID");

        b |= febits;

        if (msgLvl(MSG::DEBUG)) {
            output << waferId_str << "\t"
                   << x << "\t"
                   << b.to_string() << "\t"
                   << febits.to_string() << "\t"
                   << bec << "\t"
                   << ld << "\t"
                   << phi << "\t"
                   << eta << "\t"
                   << std::hex<< onlineId.to_ulong() << "\t";
        }
        //convert back to hex
        std::stringstream res;
        res << std::hex << std::uppercase << b.to_ulong();
        // keep this, might be useful later
        //output << res.str() << "\n";
        output << std::hex << (b| onlineId).to_ulong() << ","
                << (b| onlineId).to_ulong() << ","
                << row.flx_card_device << ","
                << row.fiber << ","
                << dma_buffer_vec[i] << ","
                << sID.to_ulong() << "\n";

    }
    output.close();
    ATH_MSG_INFO("Wrote " << m_rows.size() << " wafer IDs to " << m_outputFile.value());

    // Perform sanity checks 
    ATH_MSG_INFO("Performing sanity checks of " << m_outputFile.value());
    ATH_CHECK( sanityCheck(m_outputFile.value()) );

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
    const std::string resolvedFelixCsv = PathResolver::find_file(m_FelixCardFile.value(), "DATAPATH");
    if (resolvedFelixCsv.empty()) {
        ATH_MSG_FATAL("Could not resolve FELIX CSV file: " << m_FelixCardFile.value());
        return StatusCode::FAILURE;
    }
    std::ifstream inputFelix(resolvedFelixCsv);
    if (!inputFelix.good()) {
        ATH_MSG_FATAL("Could not open FELIX CSV file: " << resolvedFelixCsv);
        return StatusCode::FAILURE;
    }

    //Load and store module CSV sheet
    m_rows.clear();

    ATH_MSG_INFO("Loading FE chips CSV");
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
            ATH_MSG_WARNING("Skipping malformed Chips CSV line: " << line);
            continue;
        }

        CsvRow row;
        row.spChain = trim(fields[0]);
        row.md = trim(fields[1]);
        row.fe = std::stoi(trim(fields[2]));
        row.flx_card_device = trim(fields[3]);
        row.fiber = static_cast<unsigned int>(std::stoul(trim(fields[4])));

        m_rows.push_back(std::move(row));
    }

    ATH_MSG_INFO("Loaded " << m_rows.size() << " Front-End Chips CSV rows from " << resolvedCsv);


    //Load and store FELIX CSV sheet


    m_felix_rows.clear();

    ATH_MSG_INFO("Loading FELIX CSV");
    firstLine = true;
    while (std::getline(inputFelix, line)) {
        if (line.empty()) {
            continue;
        }
        if (firstLine) {
            firstLine = false;
            continue;
        }
        const std::vector<std::string> fields = splitCsvLine(line);
        if (fields.size() < 3) {
            ATH_MSG_WARNING("Skipping malformed FELIX CSV line: " << line);
            continue;
        }

        FelixCsvRow row;
        row.host = trim(fields[0]);
        row.card1 = (unsigned int) std::stoi(trim(fields[1]));
        row.card2 = (unsigned int) std::stoi(trim(fields[2]));
        m_felix_rows.push_back(std::move(row));
    }

    ATH_MSG_INFO("Loaded " << m_felix_rows.size() << " FELIX CSV rows from " << resolvedFelixCsv);
    return StatusCode::SUCCESS;
}

std::tuple< Identifier, int, std::bitset<32> > ITkPixelCsvWaferIdAlg::waferId(const CsvRow& row) const {
    ATH_MSG_DEBUG("waferId lookup for SP chain " << row.spChain
                    << ", module " << row.md << ", FE " << row.fe);

    //SP chain is like G-IS-L05-R05-A-SP2
    std::vector<std::string> spChain_cur = parseSPChain(row.spChain);
    
    int bec = barrel_ec(spChain_cur);
    int ld = layer_disk(spChain_cur);
    int phi = phi_module(spChain_cur, row.md, row.fe );
    int eta = eta_module(spChain_cur, row.md, row.fe );
    int fe_n = feID(spChain_cur, row.fe );
    std::bitset<32> online_id = onlineId(spChain_cur, row.md, row.fe);
    
    return std::make_tuple(m_pixIdHelper->wafer_id(bec, ld, phi, eta), fe_n, online_id);
}

std::string ITkPixelCsvWaferIdAlg::trim(const std::string& input) {
    const auto begin = input.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
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
     
    if (spchain[1] == "IS" &&
        (spchain[2] == "L0" || spchain[2] == "L1") &&
        spchain.at(3)[0] != 'R') { // inner flat barrel
        return 0;
    }
    else if (spchain[1] == "OB" &&
             (spchain[2] == "L2" || spchain[2] == "L3" ||
              spchain[2] == "L4") &&
             (spchain.at(3)[0] == 'B')) { // Outer flat barrel has 3 layers
        return 0;
    }
    else{
        //Need to calculat the side sign because eta is always positive.
        std::string sideAC = spchain[4];
        if(sideAC != "A" && sideAC != "C"){
            sideAC = spchain[5];
        }
        int side = (sideAC == "A")? 1 : -1; // A for side pos, C for side neg
        return side*2; // endcap is +2 or -2 - this includes barrel rings, in offline they are treated as endcap
    }
}

int ITkPixelCsvWaferIdAlg::layer_disk(const std::vector<std::string>& spchain) const {
    int b_ec = barrel_ec(spchain);
    if( b_ec == 0 ){ // flat barrel
        return spchain.at(2)[1] - '0';
    }
    else{ // endcap and barrel rings - all considered as 'endcap' disks
        if (spchain[2] == "L01" &&
            (spchain.at(5) == "SP1" || spchain.at(5) == "SP3")) { // barrel vertical small combined rings, disk 0
            ATH_MSG_DEBUG("added layer 0 barrel vertical combined ring: SP chain "
                << spchain[1]<< spchain[2]<< spchain[3]<< spchain[4]<< spchain[5]);
            return 0;
        }
        else if (spchain[2] == "L01" &&
                 (spchain.at(5) == "SP2" || spchain.at(5) == "SP4")) { // barrel vertical large combined rings, disk 2
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
    
    if( b_ec == 0 ){ // flat barrel
        std::string phi_str = (spchain.at(3)).substr(1,2);
        if(ld == 0 || ld ==1 ){ //inner system flat barrel
            return std::stoi(phi_str) - 1;
        }
        else{ //outer flat barrel
            int phi = std::stoi(phi_str) - 1 ;
            phi = (mod[2] == 'T') ? 2*phi + 1 : 2*phi;
            return phi;
        }
    }
    else{ // endcap and barrel rings - all considered as disks
        if (ld == 0) { // barrel vertical small combined rings, disk 0
            std::string sp_str(1, (spchain.at(5)[2])); // SP=1 and SP=3 alternate in phi
            if(sp_str == "1"){
                ATH_MSG_DEBUG( "return phi = " << 6 * (stoi(mod) - 1 ) + 2 * (fe - 1));
                return 6 * (stoi(mod) - 1 ) + 2 * (fe - 1); // probably wrong offset, TOCHECK
            }
            else if(sp_str == "3"){
                ATH_MSG_DEBUG( "return phi = " << 6 * (stoi(mod) - 1 ) + 2 * (fe - 1) + 1);
                return 6 * (stoi(mod) - 1 ) + 2 * (fe - 1) + 1; // probably wrong offset, TOCHECK
            }
            else{
                ATH_MSG_WARNING("Bad input for phi_module,return -9999 ");
                return -9999;
            }
        }
        else if (spchain[2] == "L01" &&
                 (spchain.at(5) == "SP2" || spchain.at(5) == "SP4")) {
            // barrel vertical large combined rings (quad modules), disk 2
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
            std::string phi_str = mod.substr(2,2); 
            int phi = std::stoi(phi_str)-1;
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
        }
        else if(ld == 4 || ld == 6 || ld ==8){ //Outer EC disks 4, 6, 8
            std::string phi_str = mod.substr(2,2); 
            int phi = std::stoi(phi_str) - 1;
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

    if( b_ec == 0 ){ // flat barrel
        if(ld ==0){ //triplets, one front-end is considered as one module
            return side * (3 * (std::stoi(mod) -1) + fe );
        }
        else if(ld == 1){
            return side * std::stoi(mod);
        }
        else if(ld < 5){
            std::string eta_str(1, mod[4]);
            int eta = std::stoi(eta_str);
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
            return std::stoi(eta_str)-1;
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


int ITkPixelCsvWaferIdAlg::feID(const std::vector<std::string>& spchain, int fe) const {
    int b_ec = barrel_ec(spchain);
    int ld = layer_disk(spchain);
    if(ld ==0 || (ld == 1 && fabs(b_ec) == 2 ) ){ //triplets
        return 0;
    }
    else{ // quads
        return fe-1;
    }
    
}

std::bitset<32> ITkPixelCsvWaferIdAlg::onlineId(const std::vector<std::string>& spchain, const std::string& mod, int fe) const {
    // onlineID = chipID (4b) chipID ON/OFF (1b) RD53C (1b)
    std::bitset<32> febits(0);
    int ld = layer_disk(spchain);
    int fe_id = feID(spchain, fe);
    int b_ec = barrel_ec(spchain);
    int etamod = eta_module(spchain, mod, fe);
    //these bits encode the merging scheme of modules
    if( b_ec == 0 ){ // barrel
        if(ld == 1 || ld ==2 ){ //L1, L2 flat, 4 to 2 merging
            if(fe_id < 2){
                febits = std::bitset<32>(0x30000000);
            }
            else{
                febits = std::bitset<32>(0xC0000000);
            }
        }
        if(ld == 3 || ld == 4 ){ //L3, L4 flat, 4 to 1 merging
            febits = std::bitset<32>(0xF0000000);
        }
    }
    else{
        if(ld == 3){ // Barrel ring L2
            if(fe_id < 2){
                febits = std::bitset<32>(0x30000000);
            }
            else{
                febits = std::bitset<32>(0xC0000000);
            }
        }
        if(ld == 5 || ld ==7){ // Barrel rings L3, L4
            febits = std::bitset<32>(0xF0000000);
        }
        if(ld == 4 && etamod < 5){ // first half of EC L2
            if(fe_id < 2){
                febits = std::bitset<32>(0x30000000);
            }
            else{
                febits = std::bitset<32>(0xC0000000);
            }
        }
        if(ld == 6){ // EC L3
            if(fe_id < 2){
                febits = std::bitset<32>(0x30000000);
            }
            else{
                febits = std::bitset<32>(0xC0000000);
            }
        }
        if(ld == 8 && etamod < 7){ // EC L4, 1-7
            febits = std::bitset<32>(0xF0000000);
        }
        if(ld == 8 && etamod >= 7){ // EC L4, 8-9
            if(fe_id < 2){
                febits = std::bitset<32>(0x30000000);
            }
            else{
                febits = std::bitset<32>(0xC0000000);
            }
        }
    }
    //add the chip ID ON/OFF (1b) and RD53c (1b) bits
    febits |= std::bitset<32>("00001100000000000000000000000000");

    bitcheck(febits, 26, 31, "DetResID: Online ID");

    return febits;
}


std::vector<int> ITkPixelCsvWaferIdAlg::DmaBuffer() const {
    std::vector<int> dmaNumbers;
    dmaNumbers.reserve(m_rows.size());

    size_t i = 0;
    while (i < m_rows.size()) {
        const std::string& card = m_rows[i].flx_card_device;
        size_t j = i;
        unsigned int minFiber = std::numeric_limits<unsigned int>::max();
        for (; j < m_rows.size() ; ++j) {
            if(m_rows[j].flx_card_device != card){
                continue;
            }
            if (m_rows[j].fiber > 0 && m_rows[j].fiber < minFiber){
                minFiber = m_rows[j].fiber;
            }
        }
        
        if (minFiber == std::numeric_limits<unsigned int>::max()) {
            // no valid fiber found in this block, assign 9
            for (size_t k = i; k < j; ++k){
                dmaNumbers.push_back(9);
            }
            i = j;
            continue;
        }

        // assign DMA groups based on minFiber, groups of 3 fibers -> DMA 0..3
        for (size_t k = i; k < j; ++k) {
            int dma = 0;
            if (m_rows[k].fiber >= minFiber) {
                const int offset = static_cast<int>(m_rows[k].fiber) - static_cast<int>(minFiber);
                // 3 fibers per DMA buffer except for L0 flat barrel (4 lanes FE --> 2 fibers)
                int b_ec = barrel_ec(parseSPChain(m_rows[k].spChain));
                int ld = layer_disk(parseSPChain(m_rows[k].spChain));
                if(b_ec == 0 && ld ==0){
                    dma = offset / 2;
                }
                else{
                    dma = offset / 3;
                }
            }
            if (dma < 0){
                ATH_MSG_WARNING("Assigned DMA buffer goes negative: " << dma);
            }
            if (dma > 3){
                ATH_MSG_WARNING("Assigned DMA buffer goes higher than 3: for card "
                    << card << " ; nDMAs =" << dma << ", line " << k );

            }
            dmaNumbers.push_back(std::move(dma));
        }
        i = j;
    }
    return dmaNumbers;
}


std::bitset<32> ITkPixelCsvWaferIdAlg::sourceID(const std::vector<std::string>& spchain, const std::string& flx, const unsigned int dma) const {
    // ((subdetector ID) << 16) |((FELIX card ID) << 8) | ((FELIX device ID) << 7) | ((DMA buffer number) << 5)

    int b_ec = barrel_ec(spchain);
    int ld = layer_disk(spchain);

    std::bitset<32> b(0);
    std::bitset<32> subdet = subDetID(b_ec, ld); 
    subdet <<= 16 ; //shift left by 16 bits
    
    bitcheck(subdet, 16, 23, "SourceID : SubDetectorID " );

    std::vector<std::string> flx_card_device = splitFLX_card_device(flx);
    auto hostIndex = flxHost((unsigned int) std::stoi(flx_card_device[0]));
    std::bitset<32> flx_host = std::bitset<32>(hostIndex.first); 
    flx_host <<= 8; //shift left by 8 bits
    bitcheck(flx_host, 8, 15, "SourceID : FELIX host " );

    std::bitset<32> flxCard = std::bitset<32>(hostIndex.second);
    flxCard <<= 7; //shift left by 7 bits
    bitcheck(flxCard, 7, 7, "SourceID : FELIX card " );

    std::bitset<32> flxDev = std::bitset<32>(std::stoi(flx_card_device[1]) - 1 );
    flxDev <<= 6; //shift left by 6 bits
    bitcheck(flxDev, 6, 6, "SourceID : card device " ); 

    std::bitset<32> dma_b = std::bitset<32>(dma);
    dma_b <<= 4; //shift left by 4 bits
    bitcheck(dma_b, 4, 5, "SourceID : DMA buffer index " ); 

    b = subdet | flx_host | flxCard | flxDev | dma_b;
    return b;
}



std::vector<std::string> ITkPixelCsvWaferIdAlg::splitFLX_card_device(const std::string& s) const {
    std::vector<std::string> fields;
    std::stringstream ss(s);
    std::string field;

    while (std::getline(ss, field, '.')) {
        fields.push_back(field);
    }
    if (fields.size() !=2){
        ATH_MSG_ERROR("Wrong FLX_card_device input: " << s);
    }
    if (fields[1] !="1" && fields[1] !="2" ){
        ATH_MSG_ERROR("Wrong FLX device input: " << fields[1]);
    }
    return fields;
}


std::bitset<32> ITkPixelCsvWaferIdAlg::subDetID(int barrel_endcap, int layer_disk) const {
    if(barrel_endcap == 0){
        if(layer_disk < 2){
            return std::bitset<32>(0x16);
        }
        else{
            return std::bitset<32>(0x17);
        }
    }
    else if(barrel_endcap == 2){
        return std::bitset<32>(0x18);
    }
    else if(barrel_endcap == -2){
        return std::bitset<32>(0x19);
    }
    else{
        ATH_MSG_ERROR("Wrong barrel_endcap value: " << barrel_endcap);
        ATH_MSG_ERROR("return subDetectorID =0 " );
        return std::bitset<32>(0);
    }

}


const StatusCode ITkPixelCsvWaferIdAlg::sanityCheck(std::string s) const {

    std::ifstream fcheck(s);
    if (!fcheck.good()) {
        ATH_MSG_FATAL("Could not open output file: " << s);
        return StatusCode::FAILURE;
    }

    // Read all rows, fill a vector with the full info

    std::string line;
    bool firstLine = true;
    std::vector < OutputCsvRow > rowVec;
    while (std::getline(fcheck, line)) {
        if (line.empty()) {
            continue;
        }
        if (firstLine) {
            firstLine = false;
            continue;
        }
        const std::vector<std::string> fields = splitCsvLine(line);
        if (fields.size() < 6) {
            ATH_MSG_WARNING("Skipping malformed Chips CSV line: " << line);
            continue;
        }
        OutputCsvRow row;

        const auto parseHex = [](const std::string& value) -> unsigned int {
            std::istringstream iss(value);
            iss >> std::hex;
            unsigned int parsed = 0;
            iss >> parsed;
            return parsed;
        };

        row.detResId = std::bitset<32>(parseHex(fields[0]));
        row.tdetResId = std::bitset<32>(parseHex(fields[1]));
        row.card_dev = fields[2];
        row.fiber = parseHex(fields[3]);
        row.dma = std::stoi(fields[4]);
        row.sourceId = std::bitset<32>(parseHex(fields[5]));
        rowVec.push_back(std::move(row));
        line = "";
    }

    // Now, perform the checks
    size_t i = 0;
    std::vector<std::string > cards;
    while (i < rowVec.size()) {
        const std::string& card = rowVec[i].card_dev;
        if(std::find(cards.begin(), cards.end(), card) != cards.end()) {
            continue; // already checked that FELIX device
        }  
        cards.push_back(card);
        

        std::map < int, float> fiber_links_map ;
        std::map < int, int> fiber_dma_map ;
        int dma_nfiber[4] = {};

        size_t j = i;
        for (; j < rowVec.size() ; ++j) {
            if(rowVec[j].card_dev != card){
                continue;
            }

            fiber_dma_map[rowVec[j].fiber] =  rowVec[j].dma ;
            // 1. Check that there are <= 12 fibers for one device
            if(rowVec[j].fiber > 12){
                ATH_MSG_WARNING("Sanity check of file " << s << "  :   Bad number of fibers, line: " << j );
                ATH_MSG_WARNING("Nfiber = " << rowVec[j].fiber);
            }

            if( (rowVec[j].detResId & std::bitset<32>(0xf0000000)) == std::bitset<32>(0xf0000000) ){
                fiber_links_map[rowVec[j].fiber] += 0.25;
            }
            else if( (rowVec[j].detResId & std::bitset<32>(0xf0000000)) == std::bitset<32>(0xc0000000) ||
                (rowVec[j].detResId & std::bitset<32>(0xf0000000)) == std::bitset<32>(0x30000000)) {
                fiber_links_map[rowVec[j].fiber] += 0.5;
            }
            else if( (rowVec[j].detResId & std::bitset<32>(0xf0000000)) == std::bitset<32>(0x00000000)) {
                fiber_links_map[rowVec[j].fiber] += 1;
            }
            else{
                ATH_MSG_WARNING("Sanity check of file " << s << "  :   Bad Chip ID value in DetResID, line: " << j );
            }
        }

        // 2. Check that there are <= 6 lpGBT links per fiber
        for ( const auto & p : fiber_links_map ) {
            if(ceilf(p.second) !=  p.second){
                ATH_MSG_WARNING("Sanity check of file " << s << " : Card.device = "
                    << card << " ; Fiber " << p.first <<  "N links = " << p.second );
            }
            if( p.second > 6){
                ATH_MSG_WARNING("More than 6 physical lpGBT links, file " << s << " : Card.device = "
                    << card << " ; Fiber " << p.first <<  " ; N links = " << p.second );
            }
        }
        // 3. Check that there are <= 3 fibers per DMA buffer

        for ( const auto & p : fiber_dma_map ) {

            if( p.second > 3){
                ATH_MSG_WARNING("DMA buffer index higher than 3, file " << s << " : Card.device = "
                    << card << " ; DMA " << p.second <<  " ; fiber = " << p.first );
                continue;
            }
            dma_nfiber[p.second] ++;
        }
        for (std::size_t g = 0; g < std::size(dma_nfiber); ++g) {
            if(dma_nfiber[g] > 3){
                ATH_MSG_WARNING("More than 3 fibers, file " << s << " : Card.device = "
                    << card << " ; DMA " << g <<  " ; N fibers = " << dma_nfiber[g] );
            }
        }
        i = j;
    }

    fcheck.close();
    return StatusCode::SUCCESS;

}




void ITkPixelCsvWaferIdAlg::bitcheck(std::bitset<32> b, uint32_t lsb_lim, uint32_t msb_lim , const std::string& s) const {

    uint32_t x = static_cast<uint32_t> (b.to_ulong());
    if(x!=0){
        uint32_t lsb_used = std::countr_zero(x);
        uint32_t msb_used  = std::bit_width(x) - 1;

        if( (msb_used >  msb_lim) || (lsb_used < lsb_lim) ){
            ATH_MSG_WARNING("Bits used for this ID (" << s <<  ") are not correctly set");
            ATH_MSG_WARNING("    MSB used = " << msb_used << "   MSB allowed = " << msb_lim);
            ATH_MSG_WARNING("    LSB used = " << lsb_used << "   LSB allowed = " << lsb_lim);
            ATH_MSG_WARNING("Full bitset = " << b.to_string());
        }
    }
}


std::pair <unsigned int, unsigned int> ITkPixelCsvWaferIdAlg::flxHost(unsigned int card) const {
    auto it1 = std::find_if(m_felix_rows.begin(), m_felix_rows.end(),
    [card](const FelixCsvRow& p) {
        return (p.card1 == card);
    });
    auto it2 = std::find_if(m_felix_rows.begin(), m_felix_rows.end(),
    [card](const FelixCsvRow& p) {
        return (p.card2 == card);
    });
    auto it = (it1 != m_felix_rows.end()) ? it1 : ( (it2 != m_felix_rows.end()) ? it2 : m_felix_rows.end());
    unsigned int Ncard = (it1 != m_felix_rows.end()) ? 0 : ( (it2 != m_felix_rows.end()) ? 1 : 0 );
    if (it != m_felix_rows.end()) {
        std::size_t index = it - m_felix_rows.begin();   
        std::string s= m_felix_rows[index].host;
        //Ex: pc-tdq-ro-pix-is-a-15, need to return 15
        std::size_t pos = s.rfind('-');
        return (pos == std::string::npos) ? std::pair <unsigned int, unsigned int>(0,0) : std::pair <unsigned int, unsigned int>(std::stoi(s.substr(pos + 1)), Ncard);
    }
    else{
        ATH_MSG_WARNING("Couldn't find a host for card number " << card << ", returning (0, 0)");
        return std::pair <unsigned int, unsigned int>(0, 0);
    }
}
