/*
*   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "./GepMETPufitAlg.h"

#include <xAODTrigger/EnergySumRoIAuxInfo.h>
#include "TrigEFMissingET/PufitUtils.h"
#include "TrigEFMissingET/PufitGrid.h"
#include "TrigEFMissingET/METComponent.h"
#include "TrigEFMissingET/PeriodicGridBase.h"

namespace PUfitVar{
  constexpr float maxEta = 4.8;
  constexpr float caloResSqrtTerm = 15.81;
  constexpr float caloResFloor = 50;
  constexpr float nSigma = 5.0;
  constexpr float constraintWeight = 1.;
  constexpr float trimFactor = 0.9;
  constexpr std::size_t nEtaBins = 14;
  constexpr std::size_t nPhiBins = 8;
}

GepMETPufitAlg::GepMETPufitAlg( const std::string& name, ISvcLocator* pSvcLocator ) :
  AthReentrantAlgorithm( name, pSvcLocator ){
}


GepMETPufitAlg::~GepMETPufitAlg() {}


StatusCode GepMETPufitAlg::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");
  CHECK(m_caloClustersKey.initialize());
  CHECK(m_outputMETPufitKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode GepMETPufitAlg::finalize() {
  ATH_MSG_INFO ("Finalizing " << name() << "...");
  return StatusCode::SUCCESS;
}

StatusCode GepMETPufitAlg::execute(const EventContext& context) const {
  ATH_MSG_DEBUG ("Executing " << name() << "...");
  setFilterPassed(false, context); //optional: start with algorithm not passed

  // read in clusters
  auto h_caloClusters = SG::makeHandle(m_caloClustersKey, context);
  CHECK(h_caloClusters.isValid());
  ATH_MSG_DEBUG("Read in " << h_caloClusters->size() << " clusters");
  const auto& caloClusters = *h_caloClusters;
  
  ATH_CHECK(PufitMET(caloClusters,
		     PUfitVar::nSigma,
		     context));
  
  setFilterPassed(true, context); //if got here, assume that means algorithm passed
  return StatusCode::SUCCESS;
}

StatusCode GepMETPufitAlg::PufitMET(const xAOD::CaloClusterContainer& caloClusters,
				    float inputSigma,
				    const EventContext& context) const {

  using namespace HLT::MET;
  GridParameters params{
    PUfitVar::maxEta, PUfitVar::nEtaBins, PUfitVar::nPhiBins};
  PufitGrid grid(params);


  // Start by filling the grid with the towers
  for ( const auto* cluster : caloClusters ) {
    grid += SignedKinematics::fromEtEtaPhi(cluster->et(), cluster->eta(), cluster->phi() );
  }
  // Then calculate mean and variance
  double mean;
  double variance;
  PufitUtils::trimmedMeanAndVariance(grid,
				     PUfitVar::trimFactor,
				     mean,
				     variance);
  // Calculate the threshold
  // double threshold = mav.first + inputSigma*std::sqrt(mav.second);
  double threshold = mean + inputSigma*std::sqrt(variance);

  // Apply the masks, store the masked towers and calculate the pileup
  // quantities
  PufitUtils::CovarianceSum pileupSum;
  std::vector<SignedKinematics> masked;
  for (PufitGrid::Tower& tower : grid) {
    if (tower.sumEt() > threshold) {
      tower.mask(true);
      masked.push_back(tower);
    }
    else {
      double sigma =
        PUfitVar::caloResFloor*PUfitVar::caloResFloor +
        tower.kinematics().absPt()*PUfitVar::caloResSqrtTerm*PUfitVar::caloResSqrtTerm;
      pileupSum.add(tower, sigma);
    }
  }

  // Now derive the corrections
  std::vector<SignedKinematics> corrections = PufitUtils::pufit(
                                pileupSum.sum,
                                pileupSum.covariance,
                                mean,
                                variance,
                                masked,
                                PUfitVar::constraintWeight);

  // Sum over the masked towers
  METComponent sum = grid.sum(PufitGrid::SumStrategy::Masked);
  // Now add the corrections - the function above returned them with the right
  // sign for this to work
  for (const SignedKinematics& kin : corrections){
    sum += kin;
  }

  
  // write out the MET object
  auto h_outputMET = SG::makeHandle(m_outputMETPufitKey, context);
  
  auto METObj = std::make_unique<xAOD::EnergySumRoI>();
  METObj->setStore(new xAOD::EnergySumRoIAuxInfo());
  
  METObj->setEnergyX(sum.mpx);
  METObj->setEnergyY(sum.mpy);
  METObj->setEnergyT(sum.met());

  h_outputMET = std::move(METObj);

  ATH_MSG_DEBUG("No of MET objects: 1");

  return StatusCode::SUCCESS;

}
