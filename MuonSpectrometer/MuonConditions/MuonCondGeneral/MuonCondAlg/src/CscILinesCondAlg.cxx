/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <fstream>
#include <memory>
#include <MuonCondAlg/CscILinesCondAlg.h>
#include <StoreGate/ReadCondHandle.h>
#include <StoreGate/WriteCondHandle.h>
#include <AthenaKernel/IOVInfiniteRange.h>
#include <PathResolver/PathResolver.h>
#include <CxxUtils/StringUtils.h>


 CscILinesCondAlg::CscILinesCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
        AthCondAlgorithm{name,pSvcLocator}{}


StatusCode CscILinesCondAlg::initialize() {
    ATH_CHECK(m_readKey.initialize(m_readFromJSON.value().empty()));
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    if (!m_readFromJSON.value().empty()){
        ATH_MSG_INFO("Load Intenal Csc alignment from JSON "<<m_readFromJSON);
    } else {
        ATH_MSG_INFO("Load Internal Csc alignment from COOL <"<<m_readKey.key()<<">");
    }        
    return StatusCode::SUCCESS;

}
StatusCode CscILinesCondAlg::execute(const EventContext& ctx) const {
    SG::WriteCondHandle<ALineContainer> writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
         ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS; 
    }
    writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteTime()));
    std::unique_ptr<ALineContainer> writeCdo{std::make_unique<ALineContainer>()};
    if (!m_readKey.empty()) {
        SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKey, ctx};
        if (!readHandle.isValid()) {
            ATH_MSG_FATAL("Failed to load I lines from COOL "<<m_readKey.fullKey());
            return StatusCode::FAILURE;
        }
        writeHandle.addDependency(readHandle);
        // =======================
        // Retrieve the collection of strings read out from the DB
        // =======================
        // unpack the strings in the collection and update the
        // ALlineContainer in TDS       
        for (CondAttrListCollection::const_iterator itr = readHandle->begin(); 
            itr != readHandle->end(); ++itr) {
            
            const coral::AttributeList& atr = itr->second;
            std::string data;
            if (atr["data"].specification().type() == typeid(coral::Blob)) {
                ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
                if (!CoralUtilities::readBlobAsString(atr["data"].data<coral::Blob>(), data)) {
                    ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
                    return StatusCode::FAILURE;
                }
            } else {
                data = *(static_cast<const std::string*>((atr["data"]).addressOfData()));
            }
            ATH_MSG_DEBUG("Data load is " << data << " FINISHED HERE ");
            nlohmann::json lines = nlohmann::json::array();

            // new format -----------------------------------
            if (m_newFormat2020) {
                nlohmann::json j = nlohmann::json::parse(data);
                lines = j["corrections"];
            } else {
                // old format -----------------------------------
                ATH_CHECK(loadDataFromLegacy(data, lines));
            }
            ATH_CHECK(parseDataFromJSON(lines, *writeCdo));
        }
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
    }
    
    
    ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    ATH_MSG_INFO("Saved successfully internal CSC alignment "<<m_writeKey.fullKey()<<" with validity range "<<writeHandle.getRange());
    return StatusCode::SUCCESS;
}


StatusCode CscILinesCondAlg::loadDataFromLegacy(const std::string& data, nlohmann::json& json)  const {
    // Parse corrections
    constexpr std::string_view delimiter{"\n"};
    json = nlohmann::json::array();
    auto lines = CxxUtils::tokenize(data, delimiter);
    for (const std::string& blobline : lines) {
        nlohmann::json line;
        constexpr std::string_view delimiter {":"};
        auto tokens = CxxUtils::tokenize(blobline, delimiter);
        // Check if tokens is not empty
        if (tokens.empty()) {
            ATH_MSG_FATAL("Empty string retrieved from DB ");
            return StatusCode::FAILURE;
        }
        const std::string_view type = tokens[0];
        // Parse line
        if ('#' == type[0]) {
            // skip it
            continue;
        }
        if (type.compare(0, 4, "Corr") == 0) {
            //# Amdb like clob for ilines using geometry tag ISZT-R06-02
            //# ISZT_DATA_ID VERS TYP JFF JZZ JOB JLAY TRAS TRAZ TRAT ROTS ROTZ ROTT
            //
            //.... example
            // Corr:  CSL 1 -1 3 1 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000

            constexpr std::string_view delimiter{"  "};
            auto tokens = CxxUtils::tokenize(blobline, delimiter);
            if (tokens.size() != 12) {
                ATH_MSG_FATAL("Invalid length in string retrieved from DB in folder. String length is "
                                << tokens.size());
                return StatusCode::FAILURE;
            }

            ATH_MSG_VERBOSE("Parsing Line = ");
            for (std::string_view token : tokens) ATH_MSG_VERBOSE(token << " | ");
          

            // Start parsing
            int ival = 1;
            // Station Component identification
            line["typ"] = std::string(tokens[ival++]);
            line["jff"] = CxxUtils::atoi(tokens[ival++]);
            line["jzz"] = CxxUtils::atoi(tokens[ival++]);
            line["job"] = CxxUtils::atoi(tokens[ival++]);
            line["jlay"] = CxxUtils::atoi(tokens[ival++]);

            // I-line
            line["tras"] = CxxUtils::atof(tokens[ival++]);
            line["traz"] =  CxxUtils::atof(tokens[ival++]);
            line["trat"] = CxxUtils::atof(tokens[ival++]);
            line["rots"] = CxxUtils::atof(tokens[ival++]);
            line["rotz"] = CxxUtils::atof(tokens[ival++]);
            line["rott"] = CxxUtils::atof(tokens[ival++]);
        }
        if (line.empty()) continue;
        json.push_back(std::move(line));
    }
    return StatusCode::SUCCESS;
}
StatusCode CscILinesCondAlg::parseDataFromJSON(const nlohmann::json& lines,
                                               ALineContainer& writeCdo) const{
    
     for (auto& corr : lines.items()) {
        nlohmann::json line = corr.value();
        ALinePar newILine{};

        // Station Component identification
        const int jff = line["jff"];
        const int jzz = line["jzz"];
        const int job = line["job"];
        const int jlay = line["jlay"];
        const std::string stationType = line["typ"];            
        newILine.setAmdbId(stationType, jzz, jff, job);

        Identifier id{0};
        bool is_valid{false};
        if (stationType[0] == 'C') {
            // csc case
            constexpr int chamberLayer = 2;
            if (job != 3) ATH_MSG_WARNING("job = " << job << " is not 3 => chamberLayer should be 1 - not existing ! setting 2");
            id = m_idHelperSvc->cscIdHelper().channelID(stationType, jzz, jff, chamberLayer, jlay, 0, 1, is_valid);
            ATH_MSG_VERBOSE("identifier being assigned is " << m_idHelperSvc->toString(id));
        }   
        if (!is_valid) {
            ATH_MSG_ERROR("There is a non CSC chamber in the list of CSC internal alignment parameters.");
            return StatusCode::FAILURE;
        }
        newILine.setIdentifier(id);

        newILine.setParameters(line["tras"], line["traz"], line["trat"], 
                                line["rots"], line["rotz"], line["rott"]);
        // new Iline
        const auto insertItr = writeCdo.insert(newILine);
        if (!insertItr.second) {
            ATH_MSG_FATAL("Could not insert "<<newILine<<" as "<<(*insertItr.first)<<" has already been safed before" );
            return StatusCode::FAILURE;
            
        }
    }    
    return StatusCode::SUCCESS;
}
