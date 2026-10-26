/// emacs: this is -*- c++ -*- 
/**
 **     @file    TrigIDR4Mon.h
 **
 **     @author  mark sutton
 **     @date    Tue  29 Sep 2025 09:08:26 GMT
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef TRIGIDR4MONITORING_TRIGIDR4MON_H
#define TRIGIDR4MONITORING_TRIGIDR4MON_H

#include "GaudiKernel/ToolHandle.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "TrigInDetAnalysis/TrackFilter.h"
#include "TrigInDetAnalysisUtils/Associator_BestMatch.h"
#include "TrigInDetAnalysisUtils/TrackMatchDeltaR.h"
#include "TrigInDetAnalysisUtils/TrackMatchDeltaRCosmic.h"

#include "xAODTracking/VertexContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTau/TauJetContainer.h"

#include "TrackAnalysis.h"
#include "MsgMacros.h"

#include <string>



class TrigIDR4Mon : public AthMonitorAlgorithm {

public:

  TrigIDR4Mon( const std::string & name, ISvcLocator* pSvcLocator);

  virtual ~TrigIDR4Mon();

  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms(const EventContext &context) const override { return execute(context); }
  virtual StatusCode execute(const EventContext &context) const;
  virtual StatusCode finalize() override;

  virtual StatusCode bookHistograms();

  void addMonGroupFromBase( const std::string&  ) { }

protected:

  /// class structure items
  
  ToolHandleArray<IDTPM2::TrackAnalysis> m_monTools { this, "MonTools", {} }; 

  std::vector<IDTPM2::TrackAnalysis*> m_tools;

  /// analysis configuration informstion
  
  std::string m_chain; // trigger chain
  
  std::vector<std::string> m_chainNames;
  std::string              m_releaseMetaData;

  ToolHandle<Trig::TrigDecisionTool> m_tdt;

  /// additional string for the histogram directory

  /// turn this into a new style property
  std::string  m_sliceTag;

private:

  /// stuff to allow dynamic registration of ReadHandleKeys only for
  /// collections that are actually needed
  
  template<typename T, typename U> 
  StatusCode initializeKey( std::unique_ptr<SG::ReadHandleKey<T>>& rhkey, const std::string& key, const U& value ) {
    if ( rhkey==0 ) {
      MY_MSG_INFO << "registering: " << value << endmsg; 
      rhkey.reset( new SG::ReadHandleKey<T>{ this, key, value, "" } ); 
      return rhkey->initialize();
    }
    return StatusCode::SUCCESS;
  }
  
  StatusCode initializeKeys( std::string keyvalue ) {

    size_t pos = keyvalue.find(":");
    if ( pos!=std::string::npos ) keyvalue.resize( pos ); 
    
    ATH_CHECK( initializeKey( m_trackKeys, "TrackParticleKeys", "InDetTrackParticles" ) );
    ATH_CHECK( initializeKey(   m_vtxKeys,        "VertexKeys",     "PrimaryVertices" ) );
    
    if ( keyvalue=="TruthParticles" ) return initializeKey( m_truthKeys,    "TruthParticleKeys",  keyvalue );    
    if ( keyvalue=="Electrons"      ) return initializeKey( m_electronKeys, "ElectronKeys",       keyvalue ); 
    if ( keyvalue=="Muons"          ) return initializeKey( m_muonKeys,     "MuonKeys",           keyvalue ); 
    if ( keyvalue=="TauJets"        ) return initializeKey( m_tauJetKeys,   "TauJetKeys",         keyvalue ); 

    return StatusCode::SUCCESS;
  }

private:
  
  std::unique_ptr<SG::ReadHandleKey<xAOD::TrackParticleContainer>> m_trackKeys = 0;
  std::unique_ptr<SG::ReadHandleKey<xAOD::TruthParticleContainer>> m_truthKeys = 0;
  std::unique_ptr<SG::ReadHandleKey<xAOD::VertexContainer>>          m_vtxKeys = 0;
  
  std::unique_ptr<SG::ReadHandleKey<xAOD::ElectronContainer>>   m_electronKeys = 0;
  std::unique_ptr<SG::ReadHandleKey<xAOD::MuonContainer>>           m_muonKeys = 0;
  std::unique_ptr<SG::ReadHandleKey<xAOD::TauJetContainer>>       m_tauJetKeys = 0;

};



#endif //  TRIGIDR4MONITORING_TRIGIDR4MON_H
