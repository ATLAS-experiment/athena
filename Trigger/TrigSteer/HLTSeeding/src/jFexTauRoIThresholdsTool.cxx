/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "jFexTauRoIThresholdsTool.h"




uint64_t jFexTauRoIThresholdsTool::getPattern(const EventContext& /*ctx*/,
                                              const xAOD::jFexTauRoI& roi,
                                              const RoIThresholdsTool::ThrVec& menuThresholds,
                                              const TrigConf::L1ThrExtraInfoBase& menuExtraInfo) const {
    
    // Get RoI properties (once, rather than for every threshold in the menu)
    unsigned int et     = roi.et();
    unsigned int iso    = roi.iso();
    int          ieta   = roi.globalEta(); //Note: possible this may need to replaced by: std::abs(TSU::toTopoEta( roi.eta() ))/4;
    uint64_t     thresholdMask = 0;

    // calculate the isolation bit from the thresholds
    const TrigConf::L1ThrExtraInfo_jTAU& extra = static_cast<const TrigConf::L1ThrExtraInfo_jTAU&>(menuExtraInfo);
    int loose = extra.isolation(TrigConf::Selection::WP::LOOSE,0).isolation_fw();
    int medium = extra.isolation(TrigConf::Selection::WP::MEDIUM,0).isolation_fw();
    int tight = extra.isolation(TrigConf::Selection::WP::TIGHT,0).isolation_fw();
    unsigned int isobit = 0;
    if ( iso*1024 < et*loose ) isobit = 1;
    if ( iso*1024 < et*medium ) isobit = 2;
    if ( iso*1024 < et*tight ) isobit = 3;

    // Iterate through thresholds and see which ones are passed
    for (const std::shared_ptr<TrigConf::L1Threshold>& thrBase : menuThresholds) {
        auto thr = static_cast<TrigConf::L1Threshold_jTAU*>(thrBase.get());

        
        // Checking et and isolation thresholds
        if (et > thr->thrValueMeV(ieta) && isobit >= static_cast<unsigned int>(thr->isolation()) ) {
            thresholdMask |= (1<<thr->mapping());
        }
        
        ATH_MSG_DEBUG("jFEX Taus HLT seeding for ("<< thr->name() <<"): et=" << et << " > "<<thr->thrValueMeV(ieta) << " and iso="<<iso << " >= "<<static_cast<unsigned int>(thr->isolation()));
    }
    return thresholdMask;    

}
