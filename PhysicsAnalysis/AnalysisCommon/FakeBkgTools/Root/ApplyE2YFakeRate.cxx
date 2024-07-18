/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FakeBkgTools/ApplyE2YFakeRate.h"
#include "FakeBkgTools/FakeBkgInternals.h"
#include <cmath>

using namespace CP;
using namespace FakeBkgTools;
//=============================================================================
// Constructor
//=============================================================================
ApplyE2YFakeRate::ApplyE2YFakeRate(const std::string& name) :
  BaseLinearFakeBkgTool(name)
{
    declareProperty("e2y_option", m_e2y_option, "0-apply e2y rate SF, 1-apply e2y rate");

}

//=============================================================================
// Destructor
//=============================================================================
ApplyE2YFakeRate::~ApplyE2YFakeRate()
{
}

FakeBkgTools::Client ApplyE2YFakeRate::clientForDB()
{
    return FakeBkgTools::Client::E2Y_FAKE;
}

StatusCode ApplyE2YFakeRate::initialize()
{
  return BaseLinearFakeBkgTool::initialize();
}

StatusCode ApplyE2YFakeRate::addEventCustom()
{
  m_cachedWeights.clear();

  return incrementTotalYield();
}

StatusCode ApplyE2YFakeRate::getEventWeightCustom(
    FakeBkgTools::Weight& weight, const FakeBkgTools::FinalState& fs) {

  const size_t n = m_particles.size();
  const size_t nc = (1 << n);

  // the E2Y method has no extra selection information in tool
  // fill all particles bitset to 1
  FSBitset tights(nc-1);
  std::array<double,maxParticles()>dev;//first-order partial derivative
  std::fill_n(dev.begin(), n, 0);

  // m_e2y_option == 1: Apply the electron to photon fake rate
  // m_e2y_option == 0: Apply the electron to photon fake rate scale factor
  // In general, we could only condider one electron fake a photon.
  // the rate of electron wrongly reconstructed rate and idenified as photon:f = N_y^{reco}/N_e^{true}.
  // the fake rate is defined:F = N_y^{reco}/N^_e{reco} = f/(1-f)

  for(size_t i=0;i<nc;i++){ 

      // all possible conbination (e.g. for 2 particle final state):
      // FF, FR, RF, RR
      // if we want to calculate the weight with process = "1F"
      // the accepted process : FR, RF
      // weight = F1 + F2
      // dev[0] = 1, dev[1] = 1
      // unc = unc1 + unc2

      FSBitset reals(i);
      if(!fs.accept_process(n, reals, tights)) continue;
      double wei = 1.;
      for(size_t j=0;j<n;j++){
          if(m_e2y_option == 1 && m_particles[j].type == xAOD::Type::Electron){
              double x = reals[j]? 1 : m_particles[j].fake_factor.nominal;
              wei *= x;
              double theta = reals[j]? 0 : 1;
              for(size_t k=0;k<n;k++){
                  if(k != j) theta *= reals[k]? 1 : m_particles[k].fake_factor.nominal;
              }
              dev[j] += theta;
          }
          else if(m_e2y_option == 0 && m_particles[j].type == xAOD::Type::Photon){
              if(reals.count() == 0){  //only consider case all photons are from the electron fake
                  double x = m_particles[j].fake_factor.nominal;
                  wei *= x;
                  double theta = 1.;
                  for(size_t k=0;k<n;k++){
                      if(k != j) theta *= m_particles[k].fake_factor.nominal;
                  }
                  dev[j] += theta;
              }
              else{
                  ATH_MSG_ERROR("the process is not supported.Please try to use process = '0R' instead");
                  return StatusCode::FAILURE;
              }
                  
          }
          else{
              ATH_MSG_ERROR("the option and particle type are not supported by this method");
              return StatusCode::FAILURE;
          }
      }
     weight.nominal += wei;
        
  }
  for(size_t i=0;i<n;i++){
      for(auto const &kv :m_particles[i].fake_factor.uncertainties){
          auto & uncertainties = weight.uncertainties[kv.first];
          uncertainties.up += dev[i] * kv.second.up;
          uncertainties.down += dev[i] * kv.second.down;
      }
  }


  return StatusCode::SUCCESS;
}


      
