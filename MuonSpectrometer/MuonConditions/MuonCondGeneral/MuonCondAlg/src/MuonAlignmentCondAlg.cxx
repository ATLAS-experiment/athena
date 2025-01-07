/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondAlg/MuonAlignmentCondAlg.h"

#include <fstream>
#include <map>
#include <string>
#include "AthenaKernel/IOVInfiniteRange.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeListSpecification.h"
#include "GaudiKernel/ConcurrencyFlags.h"
#include "CxxUtils/StringUtils.h"
#include "MuonReadoutGeometry/GlobalUtilities.h"
#include "PathResolver/PathResolver.h"
#include "SGTools/TransientAddress.h"

MuonAlignmentCondAlg::MuonAlignmentCondAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {
   
}

StatusCode MuonAlignmentCondAlg::initialize() {
    ATH_MSG_INFO("Initilalizing");
    ATH_MSG_INFO("In initialize ---- # of folders registered is " << m_alignKeys.size());
    // Read Handles Keys
    ATH_CHECK(m_alignKeys.initialize(m_readFromJSON.value().empty()));
   
    // Write Handles
    ATH_CHECK(m_writeALineKey.initialize());
    ATH_CHECK(m_writeBLineKey.initialize());   
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode MuonAlignmentCondAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());

    SG::WriteCondHandle<ALineContainer> writeALineHandle{m_writeALineKey, ctx};
    if (writeALineHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeALineHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    ///
    // =======================
    // Write BLine Cond Handle
    // =======================
    SG::WriteCondHandle<BLineContainer> writeBLineHandle{m_writeBLineKey, ctx};
    if (writeBLineHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeBLineHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    /// Declare the dependencies on the alignment constant data
    ATH_CHECK(attachDependencies(ctx, writeALineHandle, writeBLineHandle));
    /// Create the containers
    std::unique_ptr<ALineContainer> writeALineCdo{std::make_unique<ALineContainer>()};
    std::unique_ptr<BLineContainer> writeBLineCdo{std::make_unique<BLineContainer>()};

    for (const SG::ReadCondHandleKey<CondAttrListCollection>& key:  m_alignKeys){ 
        ATH_CHECK(loadCoolFolder(ctx, key, *writeALineCdo, *writeBLineCdo));
    }
    if (!m_readFromJSON.value().empty()) {
        std::ifstream inStream{PathResolverFindCalibFile(m_readFromJSON)};
        if (!inStream.good()) {
            ATH_MSG_FATAL("No such file or directory");
            return StatusCode::FAILURE;
        }
        nlohmann::json lines;
        inStream >> lines;
        ATH_CHECK(parseDataFromJSON(lines, *writeALineCdo, *writeBLineCdo));        
    }
    ATH_CHECK(writeALineHandle.record(std::move(writeALineCdo)));
    ATH_CHECK(writeBLineHandle.record(std::move(writeBLineCdo)));

    return StatusCode::SUCCESS;
}
StatusCode MuonAlignmentCondAlg::attachDependencies(const EventContext& ctx,
                                  SG::WriteCondHandle<ALineContainer>& alines,
                                  SG::WriteCondHandle<BLineContainer>& blines ) const {
    alines.addDependency(EventIDRange(IOVInfiniteRange::infiniteTime()));
    blines.addDependency(EventIDRange(IOVInfiniteRange::infiniteTime()));
    /// Loop over all input folder and attach their IOVs to the output conditions
    for (const SG::ReadCondHandleKey<CondAttrListCollection>& key : m_alignKeys) {
        SG::ReadCondHandle<CondAttrListCollection> readHandle{key, ctx};
        if (!readHandle.isValid()){
            ATH_MSG_FATAL("Failed to load alignment folder "<<key.fullKey());
            return StatusCode::FAILURE;
        }
        ATH_MSG_INFO("Attach new dependency from <"<<readHandle.key()<<"> to the A & B lines. IOV: "<<readHandle.getRange());
        alines.addDependency(readHandle);
        blines.addDependency(readHandle);
    }
    return StatusCode::SUCCESS;
}
StatusCode MuonAlignmentCondAlg::loadCoolFolder(const EventContext& ctx,
                                                const SG::ReadCondHandleKey<CondAttrListCollection>& key,
                                                ALineContainer& writeALineCdo, 
                                                BLineContainer& writeBLineCdo) const {
    
    SG::ReadCondHandle<CondAttrListCollection> readHandle{key, ctx};
    if (!readHandle.isValid()){
        ATH_MSG_FATAL("Failed to load alignment folder "<<key.fullKey());
        return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE("Load constants from folder "<<key.key());
    // unpack the strings in the collection and update the
    // ALlineContainer in TDS
    for (CondAttrListCollection::const_iterator itr = readHandle->begin(); itr != readHandle->end(); ++itr) {
        const coral::AttributeList& atr = itr->second;
        std::string data{};
        if (atr["data"].specification().type() == typeid(coral::Blob)) {
            ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
            if (!CoralUtilities::readBlobAsString(atr["data"].data<coral::Blob>(), data)) {
                ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
                return StatusCode::FAILURE;
            }
        } else {
            data = *(static_cast<const std::string*>((atr["data"]).addressOfData()));
        }
        nlohmann::json lines;

        // new format -----------------------------------
        if (m_newFormat2020) {
            nlohmann::json j = nlohmann::json::parse(data);
            lines = j["corrections"];
        } 
        // old format -----------------------------------
        else {
            ATH_CHECK(loadDataFromLegacy(data, lines, true));
        }
        ATH_CHECK(parseDataFromJSON(lines, writeALineCdo, writeBLineCdo));
    }    
    return StatusCode::SUCCESS;
}

StatusCode MuonAlignmentCondAlg::parseDataFromJSON(const nlohmann::json& lines,
                                                  ALineContainer& writeALineCdo, 
                                                  BLineContainer& writeBLineCdo) const{
    // loop over corrections ------------------------
    for (auto& corr : lines.items()) {
        nlohmann::json line = corr.value();

        /// Station Component identification
        const std::string stationType = line["typ"];
        const int stationPhi = line["jff"];
        const int stationEta = line["jzz"];
        const int multiLayer = line["job"];
        Identifier id{0};
        /// Micromega case
        if (stationType[0] == 'M') {
            // micromegas case
            if(!m_idHelperSvc->hasMM()) {
                ATH_MSG_WARNING("Micromega alignment parameter is part of the database "<<stationType<<","<<stationEta<<","<<stationPhi<<","<<multiLayer);
                continue;  // skip if geometry does not include MMs (e.g. RUN1 or RUN2 data or RUN3 muon calibration stream)
            }
            id = m_idHelperSvc->mmIdHelper().channelID(stationType, stationEta, stationPhi, multiLayer, 1, 1);
        } else if (stationType[0] == 'S') {
            // sTGC case
            if(!m_idHelperSvc->hasSTGC()) {
                ATH_MSG_WARNING("sTgc alignment parameter is part of the database "<<stationType<<","<<stationEta<<","<<stationPhi<<","<<multiLayer);
                continue;  // skip if geometry does not include MMs (e.g. RUN1 or RUN2 data or RUN3 muon calibration stream)
            }
            id = m_idHelperSvc->stgcIdHelper().elementID(stationType, stationEta, stationPhi);
            id = m_idHelperSvc->stgcIdHelper().multilayerID(id, multiLayer);
        } else if (stationType[0] == 'T') {
            /// Tgc case
            int stPhi = MuonGM::stationPhiTGC(stationType, stationPhi, stationEta);
            int stEta = stationEta > 0 ? 1 : -1;
            if (multiLayer != 0) {
                // this should become the default now
                stEta = stationEta > 0 ?  multiLayer: - multiLayer;
            }
            id = m_idHelperSvc->tgcIdHelper().elementID(stationType, stEta, stPhi);
        } else if (stationType[0] == 'C') {
            // csc case
            if(!m_idHelperSvc->hasCSC()) {
                ATH_MSG_WARNING("Csc alignment parameter is part of the database "<<stationType<<","<<stationEta<<","<<stationPhi<<","<<multiLayer);               
                continue; //skip if geometry doesn't include CSCs
            }
            id = m_idHelperSvc->cscIdHelper().elementID(stationType, stationEta, stationPhi);
        } else if (stationType.substr(0, 3) == "BML" && std::abs(stationEta) == 7) {
            // rpc case
            id = m_idHelperSvc->rpcIdHelper().elementID(stationType, stationEta, stationPhi, 1);
        } else {
            id = m_idHelperSvc->mdtIdHelper().elementID(stationType, stationEta, stationPhi);
        }
        ALinePar newALine{};
        newALine.setIdentifier(id);
        newALine.setAmdbId(stationType, stationEta, stationPhi, multiLayer);
        newALine.setParameters(line["svalue"], line["zvalue"], line["tvalue"], 
                               line["tsv"], line["tzv"], line["ttv"]);
        auto aLineInsert = writeALineCdo.insert(newALine);
        if (newALine && !aLineInsert.second) {
            ATH_MSG_WARNING("Failed to insert  A line "<<newALine<<" for "<<m_idHelperSvc->toString(id)
                            <<" because "<<(*aLineInsert.first)<<" has been added before");
        }
        ATH_MSG_VERBOSE("Inserted new a Line "<<newALine<<" "<<m_idHelperSvc->toString(id));
    
        if (line.find("bz") == line.end()) {
            continue;
        }
        BLinePar newBLine{};
        newBLine.setParameters(line["bz"], line["bp"], line["bn"], 
                               line["sp"], line["sn"], line["tw"], 
                               line["pg"], line["tr"], line["eg"], 
                               line["ep"], line["en"]);
        newBLine.setIdentifier(id);
        newBLine.setAmdbId(stationType, stationEta, stationPhi, multiLayer);
        ATH_MSG_VERBOSE(" HardwareChamberName " <<  static_cast<std::string>(line["hwElement"]));
        auto bLineInsert = writeBLineCdo.insert(newBLine);
        if (newBLine && !bLineInsert.second){
            ATH_MSG_WARNING("Failed to insert B line "<<newBLine<<" for "<<m_idHelperSvc->toString(id)
                            <<" because "<<(*bLineInsert.first)<<" has been added before.");
        }
    }    
    return StatusCode::SUCCESS;
}

StatusCode MuonAlignmentCondAlg::loadDataFromLegacy(const std::string& data, nlohmann::json& json,
                                                    bool loadBLines) const {

    // Parse corrections
    constexpr std::string_view delimiter{"\n"};

    json = nlohmann::json::array();
    auto lines = CxxUtils::tokenize(data, delimiter);
    for (const std::string& blobline : lines) {
        nlohmann::json line;
        constexpr std::string_view delimiter{":"};
        const auto tokens = CxxUtils::tokenize(blobline, delimiter);

        // Check if tokens is not empty
        if (tokens.empty()) {
            ATH_MSG_FATAL("Empty string retrieved from DB in folder ");
            return StatusCode::FAILURE;
        }
        const std::string_view &type = tokens[0];
        // Parse line
        if (type[0] == '#') {
            continue;
        }
        //#: Corr line is counter typ,  jff,  jzz, job,                         * Chamber information
        //#:                       svalue,  zvalue, tvalue,  tsv,  tzv,  ttv,   * A lines
        //#:                       bz, bp, bn, sp, sn, tw, pg, tr, eg, ep, en   * B lines
        //#:                       chamber                                      * Chamber name
        //.... example
        // Corr: EMS  4   1  0     2.260     3.461    28.639 -0.002402 -0.002013  0.000482    -0.006    -0.013 -0.006000  0.000000
        // 0.000000     0.026    -0.353  0.000000  0.070000  0.012000    -0.012    EMS1A08
        
        if (type.compare(0, 4, "Corr") == 0) {
            constexpr std::string_view delimiter{" "};
            auto tokens = CxxUtils::tokenize(blobline, delimiter);
            if (tokens.size() != 25) {
                ATH_MSG_FATAL("Invalid length in string retrieved. String length is " << tokens.size());
                return StatusCode::FAILURE;
            }
            // Start parsing
            int ival = 1;
            // Station Component identification
            line["typ"] = std::string(tokens[ival++]);
            line["jff"] = CxxUtils::atoi(tokens[ival++]);
            line["jzz"] = CxxUtils::atoi(tokens[ival++]);
            line["job"] = CxxUtils::atoi(tokens[ival++]);
            
            // A-line
            line["svalue"] = CxxUtils::atof(tokens[ival++]);
            line["zvalue"] = CxxUtils::atof(tokens[ival++]);
            line["tvalue"] = CxxUtils::atof(tokens[ival++]);
            
            line["tsv"] = CxxUtils::atof(tokens[ival++]);
            line["tzv"] = CxxUtils::atof(tokens[ival++]);
            line["ttv"] = CxxUtils::atof(tokens[ival++]);

            // B-line
            if (loadBLines) {
                line["bz"] = CxxUtils::atof(tokens[ival++]);
                line["bp"] = CxxUtils::atof(tokens[ival++]);
                line["bn"] = CxxUtils::atof(tokens[ival++]);
                line["sp"] = CxxUtils::atof(tokens[ival++]);
                line["sn"] = CxxUtils::atof(tokens[ival++]);
                line["tw"] = CxxUtils::atof(tokens[ival++]);
                line["pg"] = CxxUtils::atof(tokens[ival++]);
                line["tr"] = CxxUtils::atof(tokens[ival++]);
                line["eg"] = CxxUtils::atof(tokens[ival++]);
                line["ep"] = CxxUtils::atof(tokens[ival++]);
                line["en"] = CxxUtils::atof(tokens[ival++]);

                line["xAtlas"] = CxxUtils::atof(tokens[ival++]);
                line["yAtlas"] = CxxUtils::atof(tokens[ival++]);

                // ChamberName (hardware convention)
                line["hwElement"] = std::string(tokens[ival++]);
            }
            json.push_back(std::move(line));  
        }
    }
    return StatusCode::SUCCESS;
}
