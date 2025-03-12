/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TriggerProcessorTool.h"

namespace NSWL1 {

  TriggerProcessorTool::TriggerProcessorTool(const std::string& type, const std::string& name, const IInterface* parent) :
    base_class(type,name,parent) {}

  StatusCode TriggerProcessorTool::initialize() {

    ATH_MSG_DEBUG("In initialize()");
    return StatusCode::SUCCESS;
  }

  StatusCode TriggerProcessorTool::mergeRDO(const Muon::NSW_PadTriggerDataContainer* padTriggerContainer,
                                            const Muon::NSW_TrigRawDataContainer* stripTriggerContainer,
                                            const Muon::NSW_TrigRawDataContainer* MMTriggerContainer,
                                            Muon::NSW_TrigRawDataContainer* trigRdoContainer) const {
    ATH_MSG_DEBUG("------------- TriggerProcessorTool::mergeRDO ---------------------");
    ATH_MSG_DEBUG("Pad Trigger Container size: " << padTriggerContainer->size());
    constexpr bool HAS_PHI_RESOLUTION{true};
    constexpr uint8_t SPARE_IS_PAD{1};
    for ( const auto padTriggerData : *padTriggerContainer ) {
      ATH_MSG_DEBUG("Pad Trigger data: " << *padTriggerData);
      const char sectorSide = (padTriggerData->sideA()) ? 'A' : 'C';
      auto trigRawData = new Muon::NSW_TrigRawData(padTriggerData->getSecid(), sectorSide, padTriggerData->getBcid());
      for (size_t it = 0; it < padTriggerData->getNumberOfTriggers(); ++it) {
        auto trigRawDataSegment = new Muon::NSW_TrigRawDataSegment();
        trigRawDataSegment->setRIndex(padTriggerData->getTriggerBandIds().at(it));
        trigRawDataSegment->setPhiIndex(padTriggerData->getTriggerPhiIds().at(it));
        trigRawDataSegment->setPhiRes(HAS_PHI_RESOLUTION);
        trigRawDataSegment->setSpare(SPARE_IS_PAD);
        trigRawData->push_back(trigRawDataSegment);
      }
      ATH_MSG_DEBUG("L1NSW-Pad Trigger Output: "
                    << "sectorSide=" << trigRawData->sectorSide() << " "
                    << "sectorId=" << trigRawData->sectorId() << " "
                    << "bcId=" << trigRawData->bcId());
      for(const auto seg : *trigRawData){
        ATH_MSG_DEBUG("\tPadSegment: "
                      << "deltaTheta=" << static_cast<int16_t>(seg->deltaTheta()) << " "
                      << "phiIndex=" << static_cast<int16_t>(seg->phiIndex()) << " "
                      << "rIndex=" << static_cast<int16_t>(seg->rIndex()) << " "
                      << "spare=" << static_cast<int16_t>(seg->spare()) << " "
                      << "lowRes=" << seg->lowRes() << " "
                      << "phiRes=" << seg->phiRes() << " "
                      << "monitor=" << seg->monitor());
      }
      trigRdoContainer->push_back(trigRawData);
    }
    ATH_MSG_DEBUG("After PadTrigger filling -> NSW Trigger RDO size: " << trigRdoContainer->size());

    for (const auto rawData : *stripTriggerContainer) {
      Muon::NSW_TrigRawData* trigRawData = new Muon::NSW_TrigRawData(*rawData, true);
      ATH_MSG_DEBUG("L1NSW-Strip Trigger Output: "
                    << "sectorSide=" << trigRawData->sectorSide() << " "
                    << "sectorId=" << trigRawData->sectorId() << " "
                    << "bcId=" << trigRawData->bcId());
      for(const auto seg : *trigRawData){
        ATH_MSG_DEBUG("\tStripSegment: "
                      << "deltaTheta=" << static_cast<int16_t>(seg->deltaTheta()) << " "
                      << "phiIndex=" << static_cast<int16_t>(seg->phiIndex()) << " "
                      << "rIndex=" << static_cast<int16_t>(seg->rIndex()) << " "
                      << "spare=" << static_cast<int16_t>(seg->spare()) << " "
                      << "lowRes=" << seg->lowRes() << " "
                      << "phiRes=" << seg->phiRes() << " "
                      << "monitor=" << seg->monitor());
      }
      trigRdoContainer->push_back(trigRawData);
    }
    ATH_MSG_DEBUG("After sTGC strip trigger filling -> NSW Trigger RDO size: " << trigRdoContainer->size());

    for (const auto rawData : *MMTriggerContainer) {
      Muon::NSW_TrigRawData* trigRawData = new Muon::NSW_TrigRawData(*rawData, false);
      ATH_MSG_DEBUG("L1NSW-MM Trigger Output: "
                    << "sectorSide=" << trigRawData->sectorSide() << " "
                    << "sectorId=" << trigRawData->sectorId() << " "
                    << "bcId=" << trigRawData->bcId());
      for(const auto seg : *trigRawData){
        ATH_MSG_DEBUG("\tMMSegment: "
                      << "deltaTheta=" << static_cast<int16_t>(seg->deltaTheta()) << " "
                      << "phiIndex=" << static_cast<int16_t>(seg->phiIndex()) << " "
                      << "rIndex=" << static_cast<int16_t>(seg->rIndex()) << " "
                      << "spare=" << static_cast<int16_t>(seg->spare()) << " "
                      << "lowRes=" << seg->lowRes() << " "
                      << "phiRes=" << seg->phiRes() << " "
                      << "monitor=" << seg->monitor());
      }
      trigRdoContainer->push_back(trigRawData);
    }
    ATH_MSG_DEBUG("After MMTrigger filling -> NSW Trigger RDO size: " << trigRdoContainer->size());

    return StatusCode::SUCCESS;
  }
}
