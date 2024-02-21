/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration 
*/

#include "SpacePointReader.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "AthLinks/ElementLink.h"
#include "StoreGate/WriteDecorHandle.h"

namespace ActsTrk {

  SpacePointReader::SpacePointReader(const std::string& name,
				     ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {}
  
  StatusCode SpacePointReader::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name() << " ... ");

    m_clusterDecoration = m_spacePointKey.key() + "." + m_clusterDecoration.key();

    ATH_MSG_DEBUG("Properties:");
    ATH_MSG_DEBUG(m_spacePointKey);
    ATH_MSG_DEBUG(m_clusterDecoration);
    
    ATH_CHECK(m_spacePointKey.initialize());
    ATH_CHECK(m_clusterDecoration.initialize());

    return StatusCode::SUCCESS;
  }
  
  StatusCode SpacePointReader::execute(const EventContext& ctx) const
  {
    ATH_MSG_DEBUG("Executing " << name() << " ...");

    ATH_MSG_DEBUG("Retrieving input space point collection with key: " << m_spacePointKey.key());
    SG::ReadHandle< xAOD::SpacePointContainer > spHandle = SG::makeHandle( m_spacePointKey, ctx );
    ATH_CHECK(spHandle.isValid());
    const xAOD::SpacePointContainer* spacePoints = spHandle.cptr();
    ATH_MSG_DEBUG("Retrieved " << spacePoints->size() << " elements from space point container");

    ATH_MSG_DEBUG("Adding decoration to space point collection: bare pointers to clusters");
    ATH_MSG_DEBUG("Decoration name: " << m_clusterDecoration.key());
    using decoration_type = std::vector<const xAOD::UncalibratedMeasurement*>;
    SG::WriteDecorHandle< xAOD::SpacePointContainer,
			  decoration_type > barePointersToClusters( m_clusterDecoration, ctx );
    
    ATH_MSG_DEBUG("Retrieving Element Links to Clusters from the Space Points and attaching the bare pointers to the object");
    static const SG::AuxElement::Accessor< std::vector<ElementLink<xAOD::UncalibratedMeasurementContainer>> > accesor("measurementLink");
    for (const xAOD::SpacePoint* sp : *spacePoints) {
      if (not accesor.isAvailable(*sp)) {
	ATH_MSG_ERROR("Space point does not possess element link to cluster. Decoration `measurementLink` is not available and this should not happen!");
	return StatusCode::FAILURE;
      }
      
      const std::vector< ElementLink< xAOD::UncalibratedMeasurementContainer > >& els = accesor(*sp);
      std::vector< const xAOD::UncalibratedMeasurement* > meas;
      meas.reserve(els.size());
      for (const ElementLink< xAOD::UncalibratedMeasurementContainer >& el : els) {
	meas.push_back(*el);
      }

      barePointersToClusters(*sp) = meas;
    }
    
    return StatusCode::SUCCESS;
  }

}
