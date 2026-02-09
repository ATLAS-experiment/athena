/*
 Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//             Interface for gFEXCondAlgo - Tool to read the COOL DB for gFEX
//                              -------------------
//     begin                : 23 10 2024
//     email                : jared.little@cern.ch
//***************************************************************************
#include "gFEXCondAlgo.h"
#include "nlohmann/json.hpp"
#include "CoralBase/Blob.h"
#include "AthenaKernel/IOVInfiniteRange.h"

namespace LVL1 {

gFEXCondAlgo::gFEXCondAlgo(const std::string& name, ISvcLocator* svc) : AthCondAlgorithm(name, svc){}

StatusCode gFEXCondAlgo::initialize() {

    ATH_MSG_INFO("Loading gFEX parameters from database");
    ATH_CHECK( m_GfexNoiseCutsKey.initialize(SG::AllowEmpty) );
    ATH_CHECK( m_GfexModuleConditionsKey.initialize(SG::AllowEmpty) );
    ATH_CHECK( m_gFEXDBParamsKey.initialize() );

    return StatusCode::SUCCESS;
}


StatusCode gFEXCondAlgo::execute(const EventContext& ctx) const {

    SG::WriteCondHandle<gFEXDBCondData> writeCHandle(m_gFEXDBParamsKey, ctx);
    if (writeCHandle.isValid()) {
        ATH_MSG_DEBUG("Existing gFEX condition is still valid");
        return StatusCode::SUCCESS;
    }

    bool validTimeStamp = (ctx.eventID().time_stamp() < m_dbBeginTimestamp) ? false : true;
    if (m_isMC) validTimeStamp = true;

    // Set DB to false if any of keys not provided
    bool anyKeyEmpty = m_GfexNoiseCutsKey.empty();
    bool useDBparams = (!anyKeyEmpty && validTimeStamp);

    // Construct the output Cond Object and fill it in
    std::unique_ptr<gFEXDBCondData> writeDBTool(std::make_unique<gFEXDBCondData>() );
    if (!m_GfexModuleConditionsKey.empty() && !m_isMC) {
        SG::ReadCondHandle <CondAttrListCollection> load_gFexModuleConditions{m_GfexModuleConditionsKey, ctx };
        if (load_gFexModuleConditions.isValid()) {
            // CHECK HERE
            writeCHandle.addDependency(load_gFexModuleConditions);
            for (auto itr = load_gFexModuleConditions->begin(); itr != load_gFexModuleConditions->end(); ++itr) {
                if ( itr->second["moduleName"].data<std::string>() !="l1calo-gfc0-gfex01" ) continue;
                const coral::Blob& blob = (itr->second["json"]).data<coral::Blob>();
                const std::string s((char*)blob.startingAddress(),blob.size());
                nlohmann::json attrList = nlohmann::json::parse(s);
                if (attrList.contains("FWVersion")) {
                    writeDBTool->set_FWVersion(attrList["FWVersion"]);
                }
                else {
                    writeDBTool->set_FWVersion(m_fwVersionDefault);
                }
            }
        }
        else {
            ATH_MSG_ERROR("Values from "<<m_GfexModuleConditionsKey<< " not loaded. Wrong key?");
            return StatusCode::FAILURE;
        }
    }
    else {
        writeCHandle.addDependency(IOVInfiniteRange::infiniteRunLB());
        writeDBTool->set_FWVersion(m_fwVersionDefault);
    }

    if(!m_GfexNoiseCutsKey.empty() && useDBparams) {

        SG::ReadCondHandle <CondAttrListCollection> load_gFexNoiseCut{m_GfexNoiseCutsKey, ctx };

        std::vector<std::string> myStringsNoise;
        myStringsNoise = { "Aslopes", "Bslopes", "Cslopes", "AnoiseCuts", "BnoiseCuts", "CnoiseCuts"};

        if (load_gFexNoiseCut.isValid()) {
            writeCHandle.addDependency(load_gFexNoiseCut);

            // setting the validity
            for (auto itr = load_gFexNoiseCut->begin(); itr != load_gFexNoiseCut->end(); ++itr) {

                const coral::Blob& blob = (itr->second["json"]).data<coral::Blob>();
                const std::string s((char*)blob.startingAddress(),blob.size());
                nlohmann::json attrList = nlohmann::json::parse(s);

                // Trying to update Noise cut values

                    bool allitemsPresent = true;
                    for(const auto & name:myStringsNoise ) {
                        allitemsPresent = allitemsPresent && attrList.contains(name);
                    }

                    if( allitemsPresent ) {
                        writeDBTool->set_Aslopes(attrList["Aslopes"]);
                        writeDBTool->set_Bslopes(attrList["Bslopes"]);
                        writeDBTool->set_Cslopes(attrList["Cslopes"]);
                        writeDBTool->set_AnoiseCuts(attrList["AnoiseCuts"]);
                        writeDBTool->set_BnoiseCuts(attrList["BnoiseCuts"]);
                        writeDBTool->set_CnoiseCuts(attrList["CnoiseCuts"]);                      
                    }
                    else {
                        throw (int16_t) itr->first;
                    }
            }
        }
        else {
            ATH_MSG_ERROR("Values from "<<m_GfexNoiseCutsKey<< " not loaded. Wrong key?");
            return StatusCode::FAILURE;
        }
    }
    else{
        writeCHandle.addDependency(IOVInfiniteRange::infiniteRunLB());
        writeDBTool->set_Aslopes(m_AslopesDefault);
        writeDBTool->set_Bslopes(m_BslopesDefault);
        writeDBTool->set_Cslopes(m_CslopesDefault);
        writeDBTool->set_AnoiseCuts(m_AnoiseCutsDefault);
        writeDBTool->set_BnoiseCuts(m_BnoiseCutsDefault);
        writeDBTool->set_CnoiseCuts(m_CnoiseCutsDefault);
    }

    if (useDBparams) ATH_MSG_DEBUG("Parameters obtained from COOL: " << m_GfexNoiseCutsKey);
    else ATH_MSG_DEBUG("Parameters are set to default");

    // Wrinting into SG!
    if (writeCHandle.record(std::move(writeDBTool)).isFailure()) {
        ATH_MSG_ERROR("Could not record " << writeCHandle.key() << " with EventRange " << writeCHandle.getRange() << " into Conditions Store");
        return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Recorded " << writeCHandle.key() << " with EventRange " << writeCHandle.getRange() << " into Conditions Store");

    return StatusCode::SUCCESS;
}
} // END LVL1 NAMESPACE
