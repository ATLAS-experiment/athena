/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/CaloBasedRoICreatorTool.h"

namespace ActsTrk {

CaloBasedRoICreatorTool::CaloBasedRoICreatorTool(const std::string& type,
						 const std::string& name,
						 const IInterface* parent)
  : base_class(type, name, parent) 
{}

StatusCode CaloBasedRoICreatorTool::initialize()
{
  ATH_MSG_DEBUG("Inizializing " << name() << " ..." );

  ATH_CHECK(m_caloClusterROIKey.initialize());
  ATH_CHECK(m_beamSpotKey.initialize());
  
  return StatusCode::SUCCESS;
}

StatusCode CaloBasedRoICreatorTool::defineRegionsOfInterest(const EventContext& ctx,
							    TrigRoiDescriptorCollection& collectionRoI) const
{
  // Define RoI as not a FS RoI
  collectionRoI.push_back( new TrigRoiDescriptor(false) );
  // Define RoI as composite RoI
  collectionRoI.back()->setComposite(true);

  // Retrieve Beam Spot data
  SG::ReadCondHandle< InDet::BeamSpotData > beamSpotHandle{ m_beamSpotKey, ctx };
  ATH_CHECK( beamSpotHandle.isValid() );
  const InDet::BeamSpotData* beamSpotData = beamSpotHandle.cptr();
  
  // Retrieve the Calo info
  ATH_MSG_DEBUG("Retrieving Calo info from key " << m_caloClusterROIKey.key());
  SG::ReadHandle< ROIPhiRZContainer > caloClustersHandle = SG::makeHandle( m_caloClusterROIKey, ctx );
  ATH_CHECK( caloClustersHandle.isValid() );
  const ROIPhiRZContainer* caloClusters = caloClustersHandle.cptr();
  ATH_MSG_DEBUG("   \\__ Retrived " << caloClusters->size() << " elements");

  // Add component RoIs
  collectionRoI.back()->reserve(caloClusters->size());
  double beamZ = beamSpotData->beamVtx().position().z();
  for (const ROIPhiRZ& calo_roi : *caloClusters) {
    double phi = calo_roi.phi();
    // skip duplicates < -pi and >pi
    if (std::abs(phi) >= M_PI && phi != -M_PI)
      continue; 
    
    double eta = calo_roi.eta();
    double z = beamZ;
    double roiPhiMin = phi - m_deltaPhi;
    double roiPhiMax = phi + m_deltaPhi;
    double roiEtaMin = eta - m_deltaEta;
    double roiEtaMax = eta + m_deltaEta;
    double roiZMin = beamZ - m_deltaZ;
    double roiZMax = beamZ + m_deltaZ;
    
    collectionRoI.back()->push_back( new TrigRoiDescriptor(eta, roiEtaMin, roiEtaMax,
							   phi, roiPhiMin ,roiPhiMax,
							   z, roiZMin, roiZMax ) );
  }

  ATH_MSG_DEBUG("Created composite RoI from Calo with " << collectionRoI.back()->size() << " RoIs"); 
  return StatusCode::SUCCESS;
}

}
