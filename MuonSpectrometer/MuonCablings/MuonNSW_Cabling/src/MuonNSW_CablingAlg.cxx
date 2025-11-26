/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonNSW_CablingAlg.h"

#include <stdlib.h>

#include <fstream>
#include <map>
#include <string>

#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeListSpecification.h"
#include "MuonIdHelpers/MdtIdHelper.h"
#include "PathResolver/PathResolver.h"
#include "SGTools/TransientAddress.h"
#include "nlohmann/json.hpp"
#include "AthenaKernel/IOVInfiniteRange.h"

MuonNSW_CablingAlg::MuonNSW_CablingAlg(const std::string& name,
                                     ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MuonNSW_CablingAlg::initialize() {
    ATH_MSG_DEBUG("initialize " << name());
    ATH_CHECK(m_readCablingKeys.initialize(m_JSONFile.value().empty()));  
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode MuonNSW_CablingAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Load the Micro mega cabling map");
    // Write Cond Handle
    SG::WriteCondHandle<Nsw_CablingMap> writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle "<< writeHandle.fullKey() << " is already valid."
                      << ". In theory this should not be called, but may happen"
                      << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteRunLB()));
    
    std::unique_ptr<Nsw_CablingMap> writeCdo{std::make_unique<Nsw_CablingMap>(m_idHelperSvc.get())};

    /// JSON file
    if (!m_JSONFile.value().empty()) {
        std::ifstream inf{m_JSONFile};
        if (!inf.good()) {
            ATH_MSG_FATAL("Cannot locate external JSON file " << m_JSONFile);
            return StatusCode::FAILURE;
        }
        std::string payload{};
        while (std::getline(inf, payload)) {
            ATH_CHECK(loadCablingSchema(payload, *writeCdo));
        }
    }

    /// Conditions DB
    else if (!m_readCablingKeys.empty()) {
        for (const SG::ReadCondHandleKey<CondAttrListCollection>& key : m_readCablingKeys){
            SG::ReadCondHandle<CondAttrListCollection> readHandle{key, ctx};
            if (!readHandle.isValid()) {
                ATH_MSG_FATAL("Failed to retrieve the cabling data from the database "
                            << key.fullKey());
                return StatusCode::FAILURE;
            }

            writeHandle.addDependency(readHandle);
            ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle.fullKey() 
                        << " readCdo->size()= " << readHandle->size());
            ATH_MSG_DEBUG("Range of input is " << readHandle.getRange() 
                        << ", range of output is " << writeHandle.getRange());
        
            // iterate through data
            CondAttrListCollection::const_iterator itr;
            for(itr = readHandle->begin(); itr != readHandle->end(); ++itr) {
            	const coral::AttributeList& atr = itr->second;
            	std::string payload = *(static_cast<const std::string *>((atr["data"]).addressOfData()));
                ATH_CHECK(loadCablingSchema(payload, *writeCdo));
            }
        }        
    }

    ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    ATH_MSG_INFO("recorded new " << writeHandle.key() << " with range "
                                 << writeHandle.getRange()
                                 << " into Conditions Store");
    return StatusCode::SUCCESS;
}

StatusCode MuonNSW_CablingAlg::loadCablingSchema(const std::string& theJSON, 
                                                 Nsw_CablingMap& cabling_map) const {
    
    if (theJSON.empty())
        return StatusCode::SUCCESS;
    nlohmann::json payload = nlohmann::json::parse(theJSON);
    const MmIdHelper& mm_helper{m_idHelperSvc->mmIdHelper()};
    const sTgcIdHelper& sTgc_helper{m_idHelperSvc->stgcIdHelper()};
    for (const auto& db_channel : payload.items()) {
        nlohmann::json cabling_payload = db_channel.value();

        std::string stName = cabling_payload["station"];
        const int eta = cabling_payload["eta"];
        const int phi = cabling_payload["phi"];
        const int multilayer = cabling_payload["multilayer"];
        const int gap = cabling_payload["gasgap"];
        bool isValid{false};
        Identifier gap_id{};
        if (stName[0] == 'M') {
            gap_id = mm_helper.channelID(stName, eta, phi, multilayer, gap, 1, isValid);
        } else {
            const int chType{cabling_payload["channeltype"]};
            gap_id = sTgc_helper.channelID(stName, eta, phi, multilayer,gap, chType, 1, isValid);
        }
        if (!isValid) {
            ATH_MSG_FATAL("Failed to deduce a valid identifier from st:"
                          << stName << " eta: " << eta << " phi: " << phi
                          << " multilayer: " << multilayer
                          << " gasgap: " << gap);
            return StatusCode::FAILURE;
        }

        NswZebraData zebra_connector{};
        zebra_connector.firstChannel = cabling_payload["FirstZebra"];
        zebra_connector.lastChannel = cabling_payload["LastZebra"];
        zebra_connector.shiftChannel = cabling_payload["ZebraShift"];
        if (!cabling_map.addConnector(gap_id, zebra_connector, msgStream()))
            return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}
