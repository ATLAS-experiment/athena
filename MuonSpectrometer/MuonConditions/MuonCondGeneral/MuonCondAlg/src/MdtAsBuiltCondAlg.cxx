/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <fstream>
#include <MuonCondAlg/MdtAsBuiltCondAlg.h>
#include <StoreGate/ReadCondHandle.h>
#include <StoreGate/WriteCondHandle.h>
#include <AthenaKernel/IOVInfiniteRange.h>
#include <PathResolver/PathResolver.h>
#include <CxxUtils/StringUtils.h>
#include "GeoModelKernel/throwExcept.h"

MdtAsBuiltCondAlg::MdtAsBuiltCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
        AthCondAlgorithm{name, pSvcLocator} {}

StatusCode MdtAsBuiltCondAlg::initialize(){
    ATH_CHECK(m_readKey.initialize(m_readFromJSON.value().empty()));
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    if (!m_readFromJSON.value().empty()){
        ATH_MSG_INFO("Load Mdt as built parameters from JSON "<<m_readFromJSON);
    } else {
        ATH_MSG_INFO("Load Mdt as-built from COOL <"<<m_readKey.key()<<">");
    }    
    return StatusCode::SUCCESS;
}       

StatusCode MdtAsBuiltCondAlg::execute(const EventContext& ctx) const {
    SG::WriteCondHandle<MdtAsBuiltContainer> writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
         ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS; 
    }
    writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteTime()));
    std::unique_ptr<MdtAsBuiltContainer> writeCdo{std::make_unique<MdtAsBuiltContainer>()};
    
    /// Read the as-built parameters from JSON
    if (!m_readFromJSON.value().empty()) {
        std::ifstream inStream{PathResolverFindCalibFile(m_readFromJSON)};
        if (!inStream.good()) {
            ATH_MSG_FATAL("No such file or directory");
            return StatusCode::FAILURE;
        }
        nlohmann::json lines;
        inStream >> lines;
        ATH_CHECK(parseDataFromJSON(lines, *writeCdo));        
    }
    if (!m_readKey.empty()) {
        SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKey, ctx};
        if (!readHandle.isValid()){
            ATH_MSG_FATAL("Failed to retrieve "<<m_readKey.fullKey());
            return StatusCode::FAILURE;
        }
        writeHandle.addDependency(readHandle);

        for (CondAttrListCollection::const_iterator itr = readHandle->begin(); itr != readHandle->end(); ++itr) {
            const coral::AttributeList& atr = itr->second;
            const std::string data{*(static_cast<const std::string*>((atr["data"]).addressOfData()))};
            nlohmann::json lines;
            if (m_newFormat2020) {
                nlohmann::json j = nlohmann::json::parse(data);
                lines = j["corrections"];
            } else {
                ATH_CHECK(legacyFormatToJSON(data, lines));
            }
            ATH_CHECK(parseDataFromJSON(lines, *writeCdo)); 
        }
    }
    ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    ATH_MSG_INFO("Saved successfully Mdt as built "<<m_writeKey.fullKey()<<" with validity range "<<writeHandle.getRange());
    return StatusCode::SUCCESS;
}

StatusCode MdtAsBuiltCondAlg::parseDataFromJSON(const nlohmann::json& lines,
                                                 MdtAsBuiltContainer& asBuilt) const{

     // loop over corrections ------------------------
    for (auto& corr : lines.items()) {
        nlohmann::json line = corr.value();    
         /// Station Component identification
        const std::string stationType = line["typ"];
        const int stationPhi = line["jff"];
        const int stationEta = line["jzz"];
        bool is_valid{false};
        const Identifier id = m_idHelperSvc->mdtIdHelper().elementID(stationType, stationEta, stationPhi, is_valid);
        if (!is_valid) {
            ATH_MSG_FATAL("The AMDB identifier "<<stationType<<", "<<stationEta<<", "<<stationPhi<<" does not seem to be a MDT one");
            return StatusCode::FAILURE;
        }
        MdtAsBuiltPar xPar{};
        /// jobID not really needed here
        xPar.setAmdbId(stationType, stationEta, stationPhi, 0);
        xPar.setIdentifier(id);
        using multilayer_t = MdtAsBuiltPar::multilayer_t;
        using tubeSide_t = MdtAsBuiltPar::tubeSide_t;
        for (const multilayer_t ml : {multilayer_t::ML1, multilayer_t::ML2}){
            for (const tubeSide_t side : {tubeSide_t::POS, tubeSide_t::NEG}){
                std::stringstream prefix{};
                prefix<<"Ml"<<(static_cast<unsigned>(ml) + 1);
                prefix<<(side  == tubeSide_t::POS? "Pos" : "Neg")<<"TubeSide";
                auto getValue = [&prefix,&line, this](const std::string& val) -> float{
                    const std::string itrName = prefix.str()+val;
                    if (line.find(itrName) == line.end()) {
                        ATH_MSG_ERROR("JSON does not contain "<<itrName);
                        THROW_EXCEPTION("Bad JSON key");
                    }
                    return line[prefix.str()+val];
                };                
                xPar.setAlignmentParameters(ml, side, getValue("y0"), getValue("z0"),
                                            getValue("alpha"), getValue("ypitch"),
                                            getValue("zpitch"),getValue("stagg"));
            }
        }
        auto itr_pair = asBuilt.insert(xPar);
        if (!itr_pair.second){
            ATH_MSG_FATAL("Failed to insert "<<xPar<<" because the place in memory is already occupied by "
                         <<(*itr_pair.first));
            return StatusCode::FAILURE;
        }
        ATH_MSG_VERBOSE("Added "<<(*itr_pair.first)<<" to the container");
    }
    return StatusCode::SUCCESS;
}

StatusCode MdtAsBuiltCondAlg::legacyFormatToJSON(const std::string& data, 
                                                 nlohmann::json& jsonDump) const {

    // Parse corrections
    constexpr std::string_view delimiter{"\n"};
    auto lines = CxxUtils::tokenize(data, delimiter);
    unsigned int nLines{0};
    for (const std::string& blobline : lines) {
        ++nLines;
        constexpr std::string_view delimiter{":"};
        auto tokens = CxxUtils::tokenize(blobline, delimiter);
        // Check if tokens is not empty
        if (tokens.empty()) {
            ATH_MSG_FATAL("Empty string retrieved from DB in folder " << m_readKey.fullKey());
            return StatusCode::FAILURE;
        }
        const std::string_view &type = tokens[0];
        // Parse line
        if (type[0] == '#') {
            // skip it
            continue;
        }

        if (type.compare(0, 4, "Corr") == 0) {
            nlohmann::json newLine;
            ATH_CHECK(setFromAscii(std::string(blobline), newLine));
            jsonDump.push_back(newLine);
        }
    }
    ATH_MSG_VERBOSE("Decoded "<<nLines<<" new ascii lines");
    return StatusCode::SUCCESS;
}


StatusCode MdtAsBuiltCondAlg::setFromAscii(const std::string& asciiData,
                                       nlohmann::json& newChannel) const {
    std::istringstream in(asciiData);

    std::string tok;
    if (!((in >> tok) && (tok == "Corr:"))) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Failed to parse line "<<asciiData);
        return StatusCode::FAILURE;
    }
    std::string typ{};
    int jff{0}, jzz{0};
    if (!(in >> typ >> jff >> jzz)) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Failed to parse line "<<asciiData);
        return StatusCode::FAILURE;
    }
    newChannel["typ"] = typ;
    newChannel["jff"] = jff;
    newChannel["jzz"] = jzz;
    using multilayer_t = MdtAsBuiltPar::multilayer_t;
    using tubeSide_t = MdtAsBuiltPar::tubeSide_t;
    std::array<int, static_cast<unsigned>(multilayer_t::NMLTYPES)> stagg{};
    if (!(in >> stagg[static_cast<unsigned>(multilayer_t::ML1)] 
             >> stagg[static_cast<unsigned>(multilayer_t::ML2)])) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Failed to parse line "<<asciiData);
        return StatusCode::FAILURE;
    } 
    for (const multilayer_t ml : {multilayer_t::ML1, multilayer_t::ML2}){
        for (const tubeSide_t side : {tubeSide_t::POS, tubeSide_t::NEG}){
            std::stringstream prefix{};
            prefix<<"Ml"<<(static_cast<unsigned>(ml) + 1);
            prefix<<(side  == tubeSide_t::POS? "Pos" : "Neg")<<"TubeSide";
            auto dumpValue  = [&prefix, &newChannel](const std::string& field, const float val) {
                newChannel[prefix.str()+field] = val;
            } ;
            
            float y0{0.f}, z0{0.f}, alpha{0.f}, ypitch{0.f}, zpitch{0.f};
            if (!(in >> y0 >> z0 >> alpha >> ypitch >> zpitch)){
                ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Failed to parse line "<<asciiData);
                return StatusCode::FAILURE;
            }
            dumpValue("y0", y0);
            dumpValue("z0", z0);
            dumpValue("alpha", alpha);
            dumpValue("ypitch", ypitch);
            dumpValue("zpitch", zpitch);
            dumpValue("stagg", stagg[static_cast<unsigned int>(ml)]);           
        }
    }
    return StatusCode::SUCCESS;
}
