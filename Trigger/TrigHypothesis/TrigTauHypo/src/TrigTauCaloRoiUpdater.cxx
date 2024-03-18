/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigTauCaloRoiUpdater.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/StatusCode.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "CxxUtils/phihelper.h"

#include "TLorentzVector.h"

TrigTauCaloRoiUpdater::TrigTauCaloRoiUpdater(const std::string & name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode TrigTauCaloRoiUpdater::initialize() {

  ATH_MSG_DEBUG( "Initializing " << name() );
  ATH_MSG_DEBUG( "z0HalfWidth  " << m_z0HalfWidth );
  if(m_z0HalfWidth <= 0.) {
    ATH_MSG_DEBUG( "z0HalfWidth <= 0:  will use the original RoIInput z0HalfWidth" );
  }
  ATH_MSG_DEBUG( "etaHalfWidth " << m_etaHalfWidth );
  ATH_MSG_DEBUG( "phiHalfWidth " << m_phiHalfWidth );
  ATH_MSG_DEBUG( "dRForCenter  " << m_dRForCenter );

  ATH_MSG_DEBUG( "Initialising HandleKeys" );
  CHECK( m_roIInputKey.initialize()        );
  CHECK( m_clustersKey.initialize() );
  CHECK( m_roIOutputKey.initialize()  );  

  return StatusCode::SUCCESS;
}



StatusCode TrigTauCaloRoiUpdater::execute(const EventContext& ctx) const {

  ATH_MSG_DEBUG( "Running "<< name() <<" ... " );

  // Prepare Outputs
  std::unique_ptr< TrigRoiDescriptorCollection > roICollection( new TrigRoiDescriptorCollection() );

  // Retrieve Input CaloClusterContainer
  SG::ReadHandle< xAOD::CaloClusterContainer > CCContainerHandle = SG::makeHandle( m_clustersKey,ctx );
  CHECK( CCContainerHandle.isValid() );
  const xAOD::CaloClusterContainer *RoICaloClusterContainer = CCContainerHandle.get();

  if(RoICaloClusterContainer != nullptr) {
    ATH_MSG_DEBUG( "Size of vector CaloCluster container is " << RoICaloClusterContainer->size());
    if(RoICaloClusterContainer->empty()) {
      ATH_MSG_DEBUG( "CaloCluster container is empty");
    }
  }else {
    ATH_MSG_ERROR( "no CaloCluster container found " );
    return StatusCode::FAILURE;
  }

  //get RoI descriptor
  SG::ReadHandle< TrigRoiDescriptorCollection > roisHandle = SG::makeHandle( m_roIInputKey, ctx );
  ATH_MSG_DEBUG("Size of roisHandle: "<<roisHandle->size());
  const TrigRoiDescriptor *roiDescriptor = roisHandle->at(0);

  // fill local variables for RoI reference position
  float eta  = roiDescriptor->eta();
  float phi  = roiDescriptor->phi();
  float dEta = m_etaHalfWidth; 
  float dPhi = m_phiHalfWidth;

  float zed  = roiDescriptor->zed();
  float zedPlus = roiDescriptor->zedPlus();
  float zedMinus = roiDescriptor->zedMinus();
  if(m_z0HalfWidth > 0.) {
    zedPlus = zed + m_z0HalfWidth;
    zedMinus = zed - m_z0HalfWidth;
  }

  ATH_MSG_DEBUG( "; RoI ID = " << roiDescriptor->roiId()
                 << ": Eta = " << eta
                 << ", Phi = " << phi );

  // Make a minimal effort to speed things up ;)
  TLorentzVector myCluster;
  TLorentzVector TauBarycenter(0., 0., 0., 0.);

  xAOD::CaloClusterContainer::const_iterator clusterIt;
  for (clusterIt=RoICaloClusterContainer->begin(); clusterIt != RoICaloClusterContainer->end(); ++clusterIt) {
    if((*clusterIt)->e() < 0)
      continue;

    myCluster.SetPtEtaPhiE((*clusterIt)->pt(), (*clusterIt)->eta(), (*clusterIt)->phi(), (*clusterIt)->e());
    TauBarycenter += myCluster;
  }

  // Determine the LC tau pT at detector axis
  TLorentzVector TauDetectorAxis(0.,0.,0.,0.);
  for (clusterIt=RoICaloClusterContainer->begin(); clusterIt != RoICaloClusterContainer->end(); ++clusterIt) {

    if((*clusterIt)->e() < 0)
      continue;

    myCluster.SetPtEtaPhiE((*clusterIt)->pt(), (*clusterIt)->eta(), (*clusterIt)->phi(), (*clusterIt)->e());
    if(TauBarycenter.DeltaR(myCluster) > m_dRForCenter)
      continue;

    TauDetectorAxis += myCluster;
  } // end loop on clusters

  //Only update the roi if TauDetectorAxis.Pt() is larger than zero, in other words, if the calo clusters sum makes sense
  if(TauDetectorAxis.Eta()!=roiDescriptor->eta() && TauDetectorAxis.Pt()>0.) eta = TauDetectorAxis.Eta();
  if(TauDetectorAxis.Phi()!=roiDescriptor->phi() && TauDetectorAxis.Pt()>0.) phi = TauDetectorAxis.Phi();

  // Prepare the new RoI
  TrigRoiDescriptor *outRoi = new TrigRoiDescriptor(roiDescriptor->roiWord(), roiDescriptor->l1Id(), roiDescriptor->roiId(),
                                                    eta, eta-dEta, eta+dEta,
                                                    phi, CxxUtils::wrapToPi(phi-dPhi), CxxUtils::wrapToPi(phi+dPhi),
                                                    zed, zedMinus, zedPlus);

  ATH_MSG_DEBUG("Input RoI " << *roiDescriptor);
  ATH_MSG_DEBUG("Output RoI " << *outRoi);

  roICollection->push_back(outRoi);

  // Save Outputs
  ATH_MSG_DEBUG( "Saving RoIs to be used as input to Fast Tracking -- TO BE CHANGED -- ::: " << m_roIOutputKey.key() );
  SG::WriteHandle< TrigRoiDescriptorCollection > outputRoiHandle = SG::makeHandle( m_roIOutputKey,ctx );
  CHECK( outputRoiHandle.record( std::move( roICollection ) ) );

  return StatusCode::SUCCESS;
}
