/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "NswDcsTestAlg.h"

// STL
#include <stdio.h>

#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>
#include <fstream>

// Gaudi and Athena
#include "AthenaKernel/IOVInfiniteRange.h"
#include "GaudiKernel/StatusCode.h"
#include "Identifier/Identifier.h"

// constructor
NswDcsTestAlg::NswDcsTestAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {}

// destructor
NswDcsTestAlg::~NswDcsTestAlg() = default;

// initialize
StatusCode NswDcsTestAlg::initialize() {
    ATH_MSG_INFO("Calling initialize");
    ATH_CHECK(m_readKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}

// execute
StatusCode NswDcsTestAlg::execute(const EventContext& ctx) const {
    ATH_MSG_INFO("Calling execute");

    // setup parameters
    std::chrono::duration<double> retrieving_MMG_HV_A{};
    std::chrono::duration<double> retrieving_MMG_HV_C{};
    std::chrono::duration<double> retrieving_MMD_HV_A{};
    std::chrono::duration<double> retrieving_MMD_HV_C{};
    std::chrono::duration<double> retrieving_STG_HV_A{};
    std::chrono::duration<double> retrieving_STG_HV_C{};

    // retrieve all folders
    if (!retrieveData(ctx, DcsDataType::HV, DcsTechType::MMG, "A", retrieving_MMG_HV_A).isSuccess()) return StatusCode::FAILURE;
    if (!retrieveData(ctx, DcsDataType::HV, DcsTechType::MMG, "C", retrieving_MMG_HV_A).isSuccess()) return StatusCode::FAILURE;
    if (!retrieveData(ctx, DcsDataType::HV, DcsTechType::MMD, "A", retrieving_MMD_HV_A).isSuccess()) return StatusCode::FAILURE;
    if (!retrieveData(ctx, DcsDataType::HV, DcsTechType::MMD, "C", retrieving_MMD_HV_A).isSuccess()) return StatusCode::FAILURE;
    if (!retrieveData(ctx, DcsDataType::HV, DcsTechType::STG, "A", retrieving_STG_HV_A).isSuccess()) return StatusCode::FAILURE;
    if (!retrieveData(ctx, DcsDataType::HV, DcsTechType::STG, "C", retrieving_STG_HV_A).isSuccess()) return StatusCode::FAILURE;

    // postprocess
    ATH_MSG_INFO("Retrieving time for (MMG, HV, Side A) = "
                 << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_MMG_HV_A).count() * 0.001) << "s ");
    ATH_MSG_INFO("Retrieving time for (MMG, HV, Side C) = "
                 << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_MMG_HV_C).count() * 0.001) << "s ");
    ATH_MSG_INFO("Retrieving time for (MMD, HV, Side A) = "
                 << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_MMD_HV_A).count() * 0.001) << "s ");
    ATH_MSG_INFO("Retrieving time for (MMD, HV, Side C) = "
                 << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_MMD_HV_C).count() * 0.001) << "s ");
    ATH_MSG_INFO("Retrieving time for (STG, HV, Side A) = "
                 << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_STG_HV_A).count() * 0.001) << "s ");
    ATH_MSG_INFO("Retrieving time for (STG, HV, Side C) = "
                 << (std::chrono::duration_cast<std::chrono::microseconds>(retrieving_STG_HV_C).count() * 0.001) << "s ");

    ATH_MSG_INFO("MADE IT TO THE END!!");
    return StatusCode::SUCCESS;
}

// retrieveTdoPdo
StatusCode NswDcsTestAlg::retrieveData(const EventContext& ctx, const DcsDataType data, const DcsTechType tech,
                                       const std::string& side, std::chrono::duration<double>& timer) const {

    std::string sdata = data == DcsDataType::HV  ? "HV" : "LV"; // for now we will only look at HV, LV not implemented yet
    std::string stech = tech == DcsTechType::MMG ? "MMG" : (tech == DcsTechType::MMD ? "MMD" : "STG");
    ATH_MSG_INFO("Starting with " << sdata << " data for " << stech << " and " << side << " at " << timestamp());
    auto start1 = std::chrono::high_resolution_clock::now();

    // Start with an infinte range and narrow it down as needed
    EventIDRange rangeW = IOVInfiniteRange::infiniteMixed();

    // Retrieve Data Object
    SG::ReadCondHandle<NswDcsDbData> readHandle{m_readKey, ctx};
    const NswDcsDbData* readCdo{*readHandle};
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
    std::vector<Identifier> channelIds = readCdo->getChannelIdsHv(tech, side);
    ATH_MSG_INFO("Found data for " << channelIds.size() << " channels!");

    // retrieve data for the first channel
    std::stringstream sstr{};
    if (!channelIds.empty()) {
        const Identifier& channel = channelIds[0];
        const NswDcsDbData::DcsConstants& dcs_data = *readCdo->getDataForChannelHv(tech, channel, false);
        ATH_MSG_INFO("Checking channel 0 (Id = " << channel.get_compact() << ") "<<dcs_data);
        if (!m_logName.empty()){
            for (const Identifier& chan_id : channelIds) {
                const NswDcsDbData::DcsConstants& thedata = *readCdo->getDataForChannelHv(tech, chan_id, false);
                bool permanentlyDisabled{false};
                sstr<<m_idHelperSvc->toString(chan_id)<<" "<<thedata<<" "<<readCdo->isGood(ctx, chan_id)<< " " << readCdo->isGoodTDaq(ctx,chan_id, permanentlyDisabled) <<std::endl;
            }                   
        }
    }

    if (!m_logName.empty()){
        std::ofstream ostr{m_logName+"_"+sdata+"_"+stech+"_"+side+".txt"};
        ostr<<sstr.str()<<std::endl;
    }
    
    ATH_MSG_ALWAYS(sstr.str());

    auto end1 = std::chrono::high_resolution_clock::now();
    timer += end1 - start1;
    ATH_MSG_INFO("Ending at " << timestamp());
    return StatusCode::SUCCESS;
}

std::string NswDcsTestAlg::timestamp() const {
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
