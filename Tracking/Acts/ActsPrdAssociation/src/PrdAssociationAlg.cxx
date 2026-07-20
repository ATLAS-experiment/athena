/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "src/PrdAssociationAlg.h"
#include "Acts/EventData/TrackStateType.hpp"
#include "xAODMeasurementBase/MeasurementDefs.h"

#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include <utility>
#include "ActsInterop/TableUtils.h"

namespace ActsTrk {

  StatusCode PrdAssociationAlg::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name() << " ...");

    ATH_CHECK(m_inputTrackCollections.initialize());
    ATH_CHECK(m_inputPrdMap.initialize(not m_inputPrdMap.empty()));
    ATH_CHECK(m_outputPrdMap.initialize());
    
    return StatusCode::SUCCESS;
  }

  StatusCode PrdAssociationAlg::finalize()
  {
    ATH_MSG_INFO("Prd Map statistics" << std::endl << makeTable(m_stat,
                                std::array<std::string, kNStat>{
                                  "N. Tracks",
                                  "N. Pixel Measurements",
                                  "N. Strip Measurements",
                                  "N. Hgtd Measurements"
                                }).columnWidth(10));
    return StatusCode::SUCCESS;
  }
  
  StatusCode PrdAssociationAlg::execute(const EventContext& ctx) const
  {
    ATH_MSG_DEBUG("Executing " << name() << " ...");
    
    const ActsTrk::TrackContainer *tracks{nullptr};
    ATH_CHECK(SG::get(tracks, m_inputTrackCollections, ctx));
    ATH_MSG_DEBUG("   \\__ Number of retrieved tracks: " << tracks->size());
    m_stat[kNTracks] += tracks->size();
    
    const ActsTrk::PrepRawDataAssociation *inputPrdMap = nullptr;
    ATH_CHECK(SG::get(inputPrdMap, m_inputPrdMap, ctx));

    if (inputPrdMap) {
      ATH_MSG_DEBUG("   \\__ Number of already used measurement from previous passes: " << inputPrdMap->size());
    }
    
    ATH_MSG_DEBUG("Creating new Prd Map for ACTS measurements with key: " << m_outputPrdMap.key());
    SG::WriteHandle outputPrdMapHandle{ m_outputPrdMap, ctx};
    if (inputPrdMap == nullptr) {
      ATH_CHECK( outputPrdMapHandle.record( std::make_unique< ActsTrk::PrepRawDataAssociation >() ) );
    } else {
      ATH_CHECK( outputPrdMapHandle.record( std::make_unique< ActsTrk::PrepRawDataAssociation >( *inputPrdMap ) ) );
    }
    ActsTrk::PrepRawDataAssociation *prdMap = outputPrdMapHandle.ptr();

    ATH_MSG_DEBUG("Filling map");
    enum class RecordStatus : int {UNKNOWN=0, SUCCESS, NOSOURCELINK, NULLSOURCELINK, ALREADYSTORED};   
    std::size_t nMeasurements = 0ul;
    RecordStatus status = RecordStatus::UNKNOWN;
    
    for (std::size_t i(0ul); i<tracks->size(); ++i) {
      tracks->trackStateContainer().visitBackwards(tracks->getTrack(i).tipIndex(),
                           [&prdMap, &nMeasurements, &status, this]
                           (const auto state) -> bool
                           {
                             // only consider measurements
                             if (not state.typeFlags().isMeasurement()) {
                                return true;
                            } 
                            ++nMeasurements;
                             // Check it has link to uncalibrated cluster
                             if ( not state.hasUncalibratedSourceLink() ) {
                               status = RecordStatus::NOSOURCELINK;
                               return false;
                             }
                             // Get the uncalibrated measurement
                            if (detail::xAODUncalibMeasCalibrator::getType(state.getUncalibratedSourceLink()) !=
                                detail::SourceLinkType::xAODUnCalibMeas) {
                                return false;
                            }
                            
                            const auto* uncalibMeas =  detail::xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink());
                            
                            if (uncalibMeas == nullptr) {
                               status = RecordStatus::NULLSOURCELINK;
                               return false;
                            }
  
                             switch(uncalibMeas->type()) {
                             case xAOD::UncalibMeasType::PixelClusterType:
                               ++m_stat[kNPixelMeasurements];
                               break;
                             case xAOD::UncalibMeasType::StripClusterType:
                               ++m_stat[kNStripMeasurements];
                               break;
                             case xAOD::UncalibMeasType::HGTDClusterType:
                               ++m_stat[kNHgtdMeasurements];
                               break;
                             default:
                               break;
                             };

                             // Store the identifier
                             const auto& [itr, inserted] = prdMap->markAsUsed(uncalibMeas->identifier());
                             if (not inserted) {
                               status = RecordStatus::ALREADYSTORED;
                               return true;
                             }
                             status = RecordStatus::SUCCESS;
                             return true;
                           });
      if (status == RecordStatus::SUCCESS) continue;

      switch(status) {
      case RecordStatus::NOSOURCELINK:
    { ATH_MSG_ERROR("There was a problem when storing the Prd collections");
      ATH_MSG_ERROR("Source link was not available."); }
    return StatusCode::FAILURE;
      case RecordStatus::NULLSOURCELINK:
    { ATH_MSG_ERROR("There was a problem when storing the Prd collections");
      ATH_MSG_ERROR("Source link was nullptr."); }
    return StatusCode::FAILURE;
      case RecordStatus::ALREADYSTORED:
    // do not stop execution, right now it is still possible at this stage two
    // tracks use the same measurement
    { ATH_MSG_DEBUG("There was a problem when storing the Prd collections");
      ATH_MSG_DEBUG("Measurement was already stored."); }
    break;
      default:
    { ATH_MSG_WARNING("There was a problem when storing the Prd collections");
      ATH_MSG_WARNING("No iteration on track states has been performed."); }
    break;
      }
    } // loop on tracks

    ATH_MSG_DEBUG("Number of used measurements: " << prdMap->size() << " from " << tracks->size() << " tracks");
    ATH_MSG_DEBUG("   \\__ Found a total of " << nMeasurements << " measurements");
    
    return StatusCode::SUCCESS;
  }
  
}


