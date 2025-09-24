/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "NswCondTestAlg.h"

// STL
#include <stdio.h>

#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>
#include <fstream>



// Gaudi and Athena
#include "AthenaKernel/IOVInfiniteRange.h"
#include "CoralBase/Blob.h"
#include "CoralUtilities/blobaccess.h"
#include "GaudiKernel/StatusCode.h"
#include "Identifier/Identifier.h"

// constructor
NswCondTestAlg::NswCondTestAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {}

// destructor
NswCondTestAlg::~NswCondTestAlg() = default;

// initialize
StatusCode NswCondTestAlg::initialize() {
    ATH_MSG_INFO("Calling initialize");
    ATH_CHECK(m_readKey_tdopdo.initialize());
    ATH_CHECK(m_readKey_vmm.initialize(m_isMC));
    ATH_CHECK(m_readKey_t0.initialize(!m_isMC));
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}

// execute
StatusCode NswCondTestAlg::execute(const EventContext& ctx) const {
    ATH_MSG_INFO("Calling execute");

    // setup parameters
    std::chrono::duration<double> retrieving_MM_TDO_A{};
    std::chrono::duration<double> retrieving_MM_TDO_C{};
    std::chrono::duration<double> retrieving_MM_PDO_A{};
    std::chrono::duration<double> retrieving_MM_PDO_C{};
    std::chrono::duration<double> retrieving_MM_VMM_A{};
    std::chrono::duration<double> retrieving_MM_VMM_C{};
    std::chrono::duration<double> retrieving_STGC_TDO_A{};
    std::chrono::duration<double> retrieving_STGC_TDO_C{};
    std::chrono::duration<double> retrieving_STGC_PDO_A{};
    std::chrono::duration<double> retrieving_STGC_PDO_C{};
    std::chrono::duration<double> retrieving_STGC_VMM_A{};
    std::chrono::duration<double> retrieving_STGC_VMM_C{};
    std::chrono::duration<double> retrieving_T0{};

    // retrieve all folders
    ATH_CHECK(retrieveTdoPdo(ctx, TimeChargeType::PDO, "MM", "A", retrieving_MM_PDO_A));
    ATH_CHECK(retrieveTdoPdo(ctx, TimeChargeType::PDO, "MM", "C", retrieving_MM_PDO_C));
    ATH_CHECK(retrieveTdoPdo(ctx, TimeChargeType::TDO, "MM", "A", retrieving_MM_TDO_A));
    ATH_CHECK(retrieveTdoPdo(ctx, TimeChargeType::TDO, "MM", "C", retrieving_MM_TDO_C));
    if(m_isMC) ATH_CHECK(retrieveVmm(ctx, "MM", "A", retrieving_MM_VMM_A));
    if(m_isMC) ATH_CHECK(retrieveVmm(ctx, "MM", "C", retrieving_MM_VMM_C));
    ATH_CHECK(retrieveTdoPdo(ctx, TimeChargeType::TDO, "STGC", "A", retrieving_STGC_TDO_A));
    ATH_CHECK(retrieveTdoPdo(ctx, TimeChargeType::TDO, "STGC", "C", retrieving_STGC_TDO_C));
    ATH_CHECK(retrieveTdoPdo(ctx, TimeChargeType::PDO, "STGC", "A", retrieving_STGC_PDO_A));
    ATH_CHECK(retrieveTdoPdo(ctx, TimeChargeType::PDO, "STGC", "C", retrieving_STGC_PDO_C));
    if(m_isMC) ATH_CHECK(retrieveVmm(ctx, "STGC", "A", retrieving_STGC_VMM_A));
    if(m_isMC) ATH_CHECK(retrieveVmm(ctx, "STGC", "C", retrieving_STGC_VMM_C));
    if(!m_isMC) ATH_CHECK(retrieveT0(ctx,retrieving_T0));

    // postprocess
    ATH_MSG_INFO("Retrieving time for (MM  , TDO, Side A) = "
                 << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_MM_TDO_A).count() * 1.0) << "s ");
    ATH_MSG_INFO("Retrieving time for (MM  , TDO, Side C) = "
                 << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_MM_TDO_C).count() * 1.0) << "s ");
    ATH_MSG_INFO("Retrieving time for (MM  , PDO, Side A) = "
                 << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_MM_PDO_A).count() * 1.0) << "s ");
    ATH_MSG_INFO("Retrieving time for (MM  , PDO, Side C) = "
                 << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_MM_PDO_C).count() * 1.0) << "s ");
    if(m_isMC){
        ATH_MSG_INFO("Retrieving time for (MM  , VMM, Side A) = "
                 << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_MM_VMM_A).count() * 1.0) << "s ");
        ATH_MSG_INFO("Retrieving time for (MM  , VMM, Side C) = "
                 << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_MM_VMM_C).count() * 1.0) << "s ");
    }

    ATH_MSG_INFO("Retrieving time for (STGC, TDO, Side A) = "
                 << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_STGC_TDO_A).count() * 1.0) << "s ");
    ATH_MSG_INFO("Retrieving time for (STGC, TDO, Side C) = "
                 << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_STGC_TDO_C).count() * 1.0) << "s ");
    ATH_MSG_INFO("Retrieving time for (STGC, PDO, Side A) = "
                 << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_STGC_PDO_A).count() * 1.0) << "s ");
    ATH_MSG_INFO("Retrieving time for (STGC, PDO, Side C) = "
                 << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_STGC_PDO_C).count() * 1.0) << "s ");
    if(m_isMC) {
        ATH_MSG_INFO("Retrieving time for (STGC, VMM, Side A) = "
               << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_STGC_VMM_A).count() * 1.0) << "s ");
        ATH_MSG_INFO("Retrieving time for (STGC, VMM, Side C) = "
               << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_STGC_VMM_C).count() * 1.0) << "s ");
    }

    if(!m_isMC){
        ATH_MSG_INFO("retrieving time for T0 for NSW = "
               << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_T0).count() * 1.0) << "s ");
    }

    ATH_MSG_INFO("MADE IT TO THE END!!");
    return StatusCode::SUCCESS;
}

// retrieveTdoPdo
StatusCode NswCondTestAlg::retrieveTdoPdo(const EventContext& ctx, TimeChargeType data, const std::string& tech,
                                          const std::string& side, std::chrono::duration<double>& timer) const {
    ATH_MSG_INFO("Starting with " << " data for " << tech << " and " << side << " at " << timestamp());
    auto start1 = std::chrono::high_resolution_clock::now();

    // Start with an infinte range and narrow it down as needed
    EventIDRange rangeW = IOVInfiniteRange::infiniteMixed();

    // Retrieve Data Object
    SG::ReadCondHandle<NswCalibDbTimeChargeData> readHandle{m_readKey_tdopdo, ctx};
    const NswCalibDbTimeChargeData* readCdo{*readHandle};
    if (!readCdo) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }

    EventIDRange range;
    if (!readHandle.range(range)) {
        ATH_MSG_ERROR("Failed to retrieve validity range for " << readHandle.key());
        return StatusCode::FAILURE;
    }

    // Intersect validity range of this obj with the validity of already-loaded objs
    rangeW = EventIDRange::intersect(range, rangeW);

    // retrieve all channels
    std::vector<Identifier> channelIds = readCdo->getChannelIds(data, tech, side);
    ATH_MSG_INFO("Found data for " << channelIds.size() << " channels!");

    // retrieve data for the first channel
    std::stringstream sstr{};
    if (!channelIds.empty()) {
        const Identifier& channel = channelIds[0];
        
        const NswCalibDbTimeChargeData::CalibConstants& calib_data = *readCdo->getCalibForChannel(data, channel);
        ATH_MSG_INFO("Checking channel 0 (Id = " << channel.get_compact() << ") "<<calib_data);
        if (!m_logName.empty()){
            for (const Identifier& chan_id : channelIds) {
                const NswCalibDbTimeChargeData::CalibConstants& calib_data = *readCdo->getCalibForChannel(data, chan_id);
                sstr<<m_idHelperSvc->toString(chan_id)<<" "<<calib_data<<std::endl;
            }                   
        }
    } else if (!m_logName.empty()) {
       const NswCalibDbTimeChargeData::CalibConstants& calib_data = *readCdo->getZeroCalibChannel(data, tech == "MM" ? TimeTech::MM : TimeTech::STGC); 
        sstr<<"Dummy calib channel "<<calib_data<<std::endl; 
    }

    if (!m_logName.empty()){
        std::ofstream ostr{m_logName+"_"+ (data == TimeChargeType::TDO ? "TDO" : "PDO") + "_"+tech+side+".txt"};
        ostr<<sstr.str()<<std::endl;
    }
    
    ATH_MSG_ALWAYS(sstr.str());

    auto end1 = std::chrono::high_resolution_clock::now();
    timer += end1 - start1;
    ATH_MSG_INFO("Ending at " << timestamp());
    return StatusCode::SUCCESS;
}

// retrieveVmm
StatusCode NswCondTestAlg::retrieveVmm(const EventContext& ctx, const std::string& tech, const std::string& side,
                                       std::chrono::duration<double>& timer) const {
    ATH_MSG_INFO("Starting with VMM data for " << tech << " and " << side << " at " << timestamp());
    auto start1 = std::chrono::high_resolution_clock::now();

    // Start with an infinte range and narrow it down as needed
    EventIDRange rangeW = IOVInfiniteRange::infiniteMixed();

    // Retrieve Data Object
    SG::ReadCondHandle<NswCalibDbThresholdData> readHandle{m_readKey_vmm, ctx};
    const NswCalibDbThresholdData* readCdo{*readHandle};
    if (!readCdo) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }

    EventIDRange range;
    if (!readHandle.range(range)) {
        ATH_MSG_ERROR("Failed to retrieve validity range for " << readHandle.key());
        return StatusCode::FAILURE;
    }

    // Intersect validity range of this obj with the validity of already-loaded objs
    rangeW = EventIDRange::intersect(range, rangeW);

    // retrieve all channels
    std::vector<Identifier> channelIds = readCdo->getChannelIds(tech, side);
    ATH_MSG_INFO("Found data for " << channelIds.size() << " channels!");

    // retrieve data for the first channel
    std::stringstream sstr {};
    if (!channelIds.empty()) {
        const Identifier& channel = channelIds[0];
        float threshold{0.};
        readCdo->getThreshold(channel, threshold);
        ATH_MSG_INFO("Checking channel 0 (Id = " << m_idHelperSvc->toString(channel)<< ")  threshold "<< threshold);
        if (!m_logName.empty()){
           for (const Identifier& id : channelIds){
            sstr<<m_idHelperSvc->toString(id)<<" threshold "<<threshold<<std::endl;
           }  
        }
    }
    if (!m_logName.empty()){
        std::ofstream ostr{m_logName+"_THRESH_"+tech+side+".txt"};
        ostr<<sstr.str()<<std::endl;
    }

    auto end1 = std::chrono::high_resolution_clock::now();
    timer += end1 - start1;
    ATH_MSG_INFO("Ending at " << timestamp());
    return StatusCode::SUCCESS;
}

// retrieve T0
StatusCode NswCondTestAlg::retrieveT0(const EventContext& ctx, 
                                       std::chrono::duration<double>& timer) const {
    ATH_MSG_INFO("Starting to retrieve T0 values at " << timestamp());
    auto start1 = std::chrono::high_resolution_clock::now();

    // Start with an infinte range and narrow it down as needed
    EventIDRange rangeW = IOVInfiniteRange::infiniteMixed();

    // Retrieve Data Object
    SG::ReadCondHandle<NswT0Data> readHandle{m_readKey_t0,ctx};
    const NswT0Data* readCdo{*readHandle};
    if (!readCdo) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }

    EventIDRange range;
    if (!readHandle.range(range)) {
        ATH_MSG_ERROR("Failed to retrieve validity range for " << readHandle.key());
        return StatusCode::FAILURE;
    }

    // Intersect validity range of this obj with the validity of already-loaded objs
    rangeW = EventIDRange::intersect(range, rangeW);

    std::stringstream sstr {};
    const MmIdHelper& id_helper = m_idHelperSvc->mmIdHelper();
    auto it = id_helper.detectorElement_begin();
    for(; it!= id_helper.detectorElement_end(); it++){
        for(int i_layer=id_helper.gasGapMin(*it); i_layer <= id_helper.gasGapMax(*it); i_layer++){
            Identifier layerId  = id_helper.channelID(*it,id_helper.multilayer(*it), i_layer,1);
            for(int i_channel=id_helper.channelMin(layerId); i_channel <  id_helper.channelMax(layerId); i_channel++){
                Identifier  channelId = id_helper.channelID(layerId,id_helper.multilayer(*it), i_layer, i_channel);
                float t0{0};
                if(!readCdo->getT0(channelId, t0)){
                    ATH_MSG_ERROR("Failed to retrieve t0 calibration for channel"<< m_idHelperSvc->toString(channelId));
                    return StatusCode::FAILURE;
                }
                if (!m_logName.empty()){
                    sstr<<m_idHelperSvc->toString(channelId)<<" T0 "<< t0 <<std::endl;
                }  
            }
        }
    }
    
    if (!m_logName.empty()){
        std::ofstream ostr{m_logName+"_T0.txt"};
        ostr<<sstr.str()<<std::endl;
    }

    auto end1 = std::chrono::high_resolution_clock::now();
    timer += end1 - start1;
    ATH_MSG_INFO("Ending at " << timestamp());
    return StatusCode::SUCCESS;
}

std::string NswCondTestAlg::timestamp() const {
    auto now = std::chrono::system_clock::now();
    std::time_t time_now = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm;
    localtime_r(&time_now, &local_tm);
    auto duration_since_epoch = now.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration_since_epoch) % 1000;
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%H:%M:%S") << '.' << std::setw(3) << std::setfill('0') << milliseconds.count();
    return oss.str();
}
