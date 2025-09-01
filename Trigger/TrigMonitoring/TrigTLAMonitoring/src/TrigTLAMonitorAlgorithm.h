/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTLAMONITORING_TRIGTLAMONITORALGORITHM_H
#define TRIGTLAMONITORING_TRIGTLAMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "TrigDecisionTool/TrigDecisionTool.h"

#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTrigger/TrigCompositeContainer.h"

class TrigTLAMonitorAlgorithm : public AthMonitorAlgorithm {
 public:
  TrigTLAMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~TrigTLAMonitorAlgorithm();
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

 private:
  Gaudi::Property<std::vector<std::string>> m_allChains{this,"AllChains",{}};

  SG::ReadHandleKey<xAOD::JetContainer   > m_jetContainerKey   {this,"JetContainerName"   ,"HLT_AntiKt4EMTopoJets_subjesIS_TLA"           , "Jet Container Name"};
  SG::ReadHandleKey<xAOD::JetContainer   > m_pfjetContainerKey {this,"PFJetContainerName" ,"HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_TLA", "Particle Flow Jet Container Name"};
  SG::ReadHandleKey<xAOD::PhotonContainer> m_photonContainerKey{this,"PhotonContainerName","HLT_egamma_Photons_TLA"                       , "Photon Container Name"};
  SG::ReadHandleKey<xAOD::MuonContainer  > m_muonContainerKey  {this,"MuonContainerName"  ,"HLT_MuonsCB_RoI_TLA"                          , "Muon Container Name"};
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleContainerKey{this,"TrackContainerName"      ,"HLT_IDTrack_FS_FTF"        , "Track Container Name"};
  SG::ReadHandleKey<xAOD::TrigCompositeContainer> m_tcEventInfoContainerKey  {this,"TCEventInfoContainerName","HLT_TCEventInfo_TLA"       , "TCEventInfo Container Name"};

  PublicToolHandle<Trig::TrigDecisionTool> m_trigDecisionTool{this,"TriggerDecisionTool","Trigger decision tool"};

  //! Retrieve a container of type `T`.
  /**
   * \param container: output container
   * \param key : name of container to be retrieved
   * \param ctx: Event Context
   */ 
  template <typename T>
  StatusCode readContainer(SG::ReadHandle<T> & container, const SG::ReadHandleKey<T>& key, const EventContext& ctx) const;

  //! Fill an event info variable to a 1D histogram
  /**
   * \param tcEventInfo: SG:ReadHandle to TCEventInfoContainer
   * \param varname: name of variable to be filled
   * \param prefix: string appended as prefix to histogram name
  */
  template <typename T>
  StatusCode fillEventInfoHistogram(SG::ReadHandle<DataVector<xAOD::TrigComposite_v1> >& tcEventInfo, const std::string& varname, const std::string& prefix) const;

  //! Fill kinematic histograms for a given particle of type `T`
  /**
   * The name of the histogram is `prefixVAR_trigName`, where the `VAR` is the name
   * of the variable (ie: `pt`).
   *
   * \tparam T Class of particle to fill (ie: `xAOD::Jet`)
   * \param particles Container with particles whose kinematics to fill
   * \param prefix Prefix for histogram name
   * \param trigName Suffix of the histogram name
   */
  template <class T>
  StatusCode fillParticleHistograms(SG::ReadHandle<DataVector<T>> particles, const std::string& prefix, const std::string& trigName) const;

  //! Fill Analysis Object feature histogram`
  /**
   * The name of the histogram is `prefixVAR_trigName`, where the `VAR` is the name
   * of the variable (ie: `pt`).
   *
   * \param container: Container to use
   * \param varname: name of variable to be filled
   * \param prefix: string appended as prefix to histogram name
   * \param trigName: Name of trigger that event is required to have fired
   * \param default_val: value filled in case variable cannot be retrieved. Defaults to -1.
   */
  template<class C, typename T>
  StatusCode fillObjectVariableHistogram(SG::ReadHandle<DataVector<C>>& container, const std::string& varname, const std::string& prefix , const std::string& trigName, T default_val=-1) const;

  //! Fill jet track variable to 1D histograms. Jet track variables are vectors where each element corresponds to the association of a different primary vertex.
  //  The first element (corresponding to the hard scatter vertex) is filled.
  /**
   * \param jets: SG:ReadHandle to jets
   * \param varname: name of variable to be filled
   * \param prefix: string appended as prefix to histogram name
   * \param trigName: Name of trigger that event is required to have fired
   * \param default_val: value filled in case variable cannot be retrieved. Defaults to -1.
  */
  template <typename T>
  StatusCode fillJetTrackVariableHistogram(SG::ReadHandle<xAOD::JetContainer>& jets,  const std::string& varname,  const std::string& prefix,  const std::string& trigName, T default_val=-1) const;
  
  //! A generic function to fill 2D histogram
  /**
   * \param varname1: Name of variable filled along x-axis
   * \param var1: variable filled along x-axis
   * \param varname2: Name of variable filled along x-axis
   * \param var2: variable filled along x-axis
   * \param prefix: string appended as prefix to histogram name
  */
  template <typename U, typename T>
  StatusCode fill2DHistogram(const std::string& varname1, U var1, const std::string& varname2, T var2, const std::string& prefix) const;

  //! Fill jet pT to 1D histograms at different calibration scales.
  /**
   * \param jets: SG:ReadHandle to jets
   * \param calibState: calibration state for which pT is retrieved and filled
   * \param prefix: string appended as prefix to histogram name
   * \param trigName: Name of trigger that event is required to have fired
  */
  StatusCode fillJetPtCalibStatesHistograms(SG::ReadHandle<xAOD::JetContainer>& jets, const std::string& calibState, const std::string& prefix,  const std::string& trigName) const;

  //! Fill kinematic histograms for a given particle of type `T`
  /**
   * The name of the histogram is `prefixVAR_trigName`, where the `VAR` is the name
   * of the variable (ie: `pt`).
   *
   * \tparam T Class of particle to fill (ie: `xAOD::Jet`)
   * \param particles Container with particles whose kinematics to fill
   * \param prefix Prefix for histogram name
   * \param trigName Suffix of the histogram name
   */
  StatusCode fillDeltaRHistograms(const xAOD::IParticle* p0, const xAOD::IParticle* p1, const std::string& prefix, const std::string& trigName) const;
};


/*************************************************
* Implementation of templated methods            *
**************************************************/

template <typename T>
StatusCode TrigTLAMonitorAlgorithm::readContainer(SG::ReadHandle<T> & container, const SG::ReadHandleKey<T>& key, const EventContext& ctx) const{

  container = SG::makeHandle( key, ctx );
  if (! container.isValid() ) {
    ATH_MSG_ERROR("evtStore() does not contain Collection with name "<< key);
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


template <typename T>
StatusCode TrigTLAMonitorAlgorithm::fillEventInfoHistogram(SG::ReadHandle<DataVector<xAOD::TrigComposite_v1> >& tcEventInfo, const std::string& varname, const std::string& prefix) const {

  Monitored::Scalar<T> variable (prefix+"_"+varname, -1);

  for(auto tcEI : *tcEventInfo) {
    auto status = tcEI->getDetail<T>(varname, variable);
    if (!status) ATH_MSG_WARNING("No "<<varname<<" for this event");
    else {
      fill("TrigTLAMonitor", variable);
      ATH_MSG_DEBUG("Retrieved EventInfo variable: "<<varname<<" = "<<variable);
    }
  }

  return StatusCode::SUCCESS;
}


template <class T>
StatusCode TrigTLAMonitorAlgorithm::fillParticleHistograms(SG::ReadHandle<DataVector<T>> particles, const std::string& prefix, const std::string& trigName) const {

  // histograms
  Monitored::Scalar<int   > n   ("n"+prefix+"_"+trigName,0  );
  Monitored::Scalar<double> pt  (prefix+"pt_"  +trigName,0.0);
  Monitored::Scalar<double> eta (prefix+"eta_" +trigName,0.0);
  Monitored::Scalar<double> phi (prefix+"phi_" +trigName,0.0);

  Monitored::Scalar<double> pt0 (prefix+"0pt_" +trigName,0.0);
  Monitored::Scalar<double> eta0(prefix+"0eta_"+trigName,0.0);
  Monitored::Scalar<double> phi0(prefix+"0phi_"+trigName,0.0);

  // fill
  n = particles->size();
  fill("TrigTLAMonitor", n);

  for(const T* particle : *particles) {
    pt  = particle->pt()/1e3;
    eta = particle->eta();
    phi = particle->phi();
    fill("TrigTLAMonitor",pt );
    fill("TrigTLAMonitor",eta);
    fill("TrigTLAMonitor",phi);
  }

  // fill leading particle
  if(particles->size()>0) {
    pt0 =particles->at(0)->pt ()/1e3;
    eta0=particles->at(0)->eta();
    phi0=particles->at(0)->phi();
    fill("TrigTLAMonitor",pt0 );
    fill("TrigTLAMonitor",eta0);
    fill("TrigTLAMonitor",phi0);
  }

  return StatusCode::SUCCESS;

}


template<class C,typename T>
StatusCode TrigTLAMonitorAlgorithm::fillObjectVariableHistogram(SG::ReadHandle<DataVector<C>>& container, const std::string& varname, const std::string& prefix , const std::string& trigName, T default_val) const {
  
  // change var name to avoid confilcts with Monitoring standards ("var_trigname")
  std::string hname = varname;
  std::replace( hname.begin(), hname.end(), '_', '-' );

  Monitored::Scalar<T> mon_var (prefix+hname+"_"+trigName,default_val);
  SG::AuxElement::ConstAccessor<T> accessor(varname);
  
  unsigned cnt(0);
  for(const auto element : *container) {
    mon_var = accessor(*element);
    fill("TrigTLAMonitor", mon_var);
    if (cnt < 5) ATH_MSG_DEBUG(prefix<<" "<<varname<<" = "<<mon_var);
    cnt++;
  }
  return StatusCode::SUCCESS;
}


template <typename T>
StatusCode TrigTLAMonitorAlgorithm::fillJetTrackVariableHistogram(SG::ReadHandle<xAOD::JetContainer>& jets,  const std::string& varname,  const std::string& prefix,  const std::string& trigName, T default_val) const {

  std::vector<T> variable_vec;
  Monitored::Scalar<T> variable  (prefix+varname+"_"+trigName,default_val);

  unsigned cnt(0);
  for(auto jet : *jets) {
    auto status = jet->getAttribute<std::vector<T>>(varname, variable_vec);
    if (!status){
      ATH_MSG_WARNING("Failed retrieving "<<varname<<" for "<<prefix);
    }
    variable = variable_vec.at(0);
    fill("TrigTLAMonitor",variable);
    if (cnt < 3) ATH_MSG_DEBUG(prefix<<" "<<varname<<" = "<<variable);
    cnt++;
  }

  return StatusCode::SUCCESS;
}


template <typename U, typename T>
StatusCode TrigTLAMonitorAlgorithm::fill2DHistogram(const std::string& varname1, U var1, const std::string& varname2, T var2, const std::string& prefix) const{

  Monitored::Scalar<U> mon_var1(prefix+"_"+varname1, var1);
  Monitored::Scalar<T> mon_var2(prefix+"_"+varname2, var2);
  ATH_MSG_DEBUG("Filling 2D histogram. "<<varname1<<" = "<<var1<<" ; "<<varname2<<" = "<<var2);
  fill("TrigTLAMonitor", mon_var1, mon_var2);

  return StatusCode::SUCCESS;
}


#endif
