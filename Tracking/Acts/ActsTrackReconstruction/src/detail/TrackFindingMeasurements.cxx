/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/detail/TrackFindingMeasurements.h"

namespace ActsTrk::detail {

  TrackFindingMeasurements::TrackFindingMeasurements(std::size_t nMeasurementContainerMax)
    : m_measurementOffsets(nMeasurementContainerMax, 0ul)
  {}
  
  // NB. all addDetectorElements() must have been done before calling first addMeasurements().
  void TrackFindingMeasurements::addMeasurements(std::size_t typeIndex,
						 const xAOD::UncalibratedMeasurementContainer &clusterContainer,
						 const DetectorElementToActsGeometryIdMap &detectorElementToGeoid)
  {
    if (typeIndex >= m_measurementOffsets.size()) {
      throw std::runtime_error("Adding collection with too big a typeIndex");
    }    
    m_measurementOffsets[typeIndex] = m_measurementsTotal;
    
    if (m_measurementRanges.empty()) {
      // try to reserve needed space,
      // this however will reserve more than necessary not just the space needed for the surfaces of
      // all the measurements that are going to be added (e.g. pixel+strips).
      m_measurementRanges.reserve(detectorElementToGeoid.size());
    }        
    m_measurementRanges.setContainer(typeIndex, &clusterContainer);
    
    xAOD::UncalibMeasType lastMeasurementType = xAOD::UncalibMeasType::Other;
    xAOD::DetectorIDHashType lastIdHash = std::numeric_limits<xAOD::DetectorIDHashType>::max();
    MeasurementRange *currentRange = nullptr;
    
    std::size_t n_elements = clusterContainer.size();;
    std::size_t sl_idx = 0;
    for( ; sl_idx < n_elements; ++sl_idx) {
      const auto *measurement  = clusterContainer[sl_idx];
      if (measurement->identifierHash() != lastIdHash or
	  measurement->type() != lastMeasurementType)
        {
          if (currentRange) {
	    currentRange->updateEnd(typeIndex, sl_idx);
          }
          lastIdHash = measurement->identifierHash();
          lastMeasurementType = measurement->type();
	  
          Acts::GeometryIdentifier measurementSurfaceId = ActsTrk::getSurfaceGeometryIdOfMeasurement(detectorElementToGeoid,
												     *measurement);
          if (measurementSurfaceId.value() == 0u) {
	    // @TODO improve error message.
	    throw std::domain_error("No Acts surface associated to measurement");
          }
	  
          // start with en empty range which is updated later.
          auto ret = m_measurementRanges.insert( std::make_pair( measurementSurfaceId.value(),
                                                                 MeasurementRange( typeIndex, sl_idx, sl_idx) ));
          if (!ret.second) {
	    std::stringstream msg;
	    msg << "Measurement not clustered by identifierHash / geometryId. New measurement "
		<< sl_idx << " with geo Id " << measurementSurfaceId
		<< " type = " << static_cast<unsigned int>(measurement->type())
		<< " idHash=" << measurement->identifierHash()
		<< " but already recorded for this geo ID the range : [" << ret.first->second.containerIndex() << "]"
		<< ret.first->second.elementBeginIndex()
		<< " .. " << ret.first->second.elementEndIndex()
		<< (ret.first->second.isConsistentRange() ? "" : " !Container index inconsistent or not in increasing order!");
	    throw std::runtime_error(msg.str());
          }
          currentRange = &ret.first->second;
        }
    }
    
    if (currentRange) {
      currentRange->updateEnd(typeIndex, sl_idx);
    }
    
    m_measurementsTotal += clusterContainer.size();
  }
  
  std::vector<std::pair<const xAOD::UncalibratedMeasurementContainer *, std::size_t>>
  TrackFindingMeasurements::measurementContainerOffsets() const
  {
    std::vector<std::pair<const xAOD::UncalibratedMeasurementContainer *, std::size_t>> offsets;
    if (m_measurementRanges.numContainers() == 0) return offsets;
    offsets.reserve(m_measurementRanges.numContainers() - 1); // first one usually 0
    
    for (std::size_t typeIndex = 0; typeIndex < m_measurementRanges.numContainers(); ++typeIndex)
      {
        const xAOD::UncalibratedMeasurementContainer *theContainer
	  = std::visit( [] (const auto &a) -> const xAOD::UncalibratedMeasurementContainer *
	  { return a.containerPtr(); },
	    m_measurementRanges.container(typeIndex));
	
        if (measurementOffset(typeIndex) > 0 and theContainer != nullptr)
	  {
	    offsets.emplace_back(theContainer, measurementOffset(typeIndex));
	  }
      }
    
    return offsets;
  }
  
} // namespace ActsTrk::detail

