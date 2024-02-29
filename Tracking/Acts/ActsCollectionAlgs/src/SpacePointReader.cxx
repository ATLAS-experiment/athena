/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration 
*/

#include "SpacePointReader.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "AthLinks/ElementLink.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"

namespace ActsTrk {

  SpacePointReader::SpacePointReader(const std::string& name,
				     ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {}
  
  StatusCode SpacePointReader::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name() << " ... ");

    m_linkDecoration = m_spacePointKey.key() + "." + m_linkDecoration.key();
    m_clusterDecoration = m_spacePointKey.key() + "." + m_clusterDecoration.key();
    
    ATH_MSG_DEBUG("Properties:");
    ATH_MSG_DEBUG(m_spacePointKey);
    ATH_MSG_DEBUG(m_linkDecoration);
    ATH_MSG_DEBUG(m_clusterDecoration);
    
    ATH_CHECK(m_spacePointKey.initialize());
    ATH_CHECK(m_linkDecoration.initialize());
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

    ATH_MSG_DEBUG("Reading decoration to space point collection: element links to clusters");
    ATH_MSG_DEBUG("Decoration name: " << m_linkDecoration.key());
    using link_type = std::vector< ElementLink<xAOD::UncalibratedMeasurementContainer> >;
    SG::ReadDecorHandle< xAOD::SpacePointContainer,
			 link_type > elementLinksToClusters( m_linkDecoration, ctx );
    ATH_CHECK(elementLinksToClusters.isAvailable());
    
    ATH_MSG_DEBUG("Adding decoration to space point collection: bare pointers to clusters");
    ATH_MSG_DEBUG("Decoration name: " << m_clusterDecoration.key());
    using decoration_type = std::vector<const xAOD::UncalibratedMeasurement*>;
    SG::WriteDecorHandle< xAOD::SpacePointContainer,
			  decoration_type > barePointersToClusters( m_clusterDecoration, ctx );

    ATH_MSG_DEBUG("Retrieving Element Links to Clusters from the Space Points and attaching the bare pointers to the object");
    for (const xAOD::SpacePoint* sp : *spacePoints) {
      const link_type& els = elementLinksToClusters(*sp);

      std::vector< const xAOD::UncalibratedMeasurement* > meas;
      meas.reserve(els.size());
      for (const ElementLink< xAOD::UncalibratedMeasurementContainer >& el : els) {
	meas.push_back(*el);
      }

      barePointersToClusters(*sp) = meas;
    }

    ATH_MSG_DEBUG("Decorations have been attached to space point collection");
    return StatusCode::SUCCESS;
  }

}
