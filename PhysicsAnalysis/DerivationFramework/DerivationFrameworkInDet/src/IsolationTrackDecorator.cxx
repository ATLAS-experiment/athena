/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// IsolationDecorator.cxx, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////

#include "DerivationFrameworkInDet/IsolationTrackDecorator.h"
#include "AthenaKernel/errorcheck.h"
#include <vector>
#include <string>

#include "CLHEP/Units/SystemOfUnits.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackingPrimitives.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "ExpressionEvaluation/MultipleProxyLoader.h"
#include "ExpressionEvaluation/SGxAODProxyLoader.h"
#include <StoreGate/WriteDecorHandle.h>

// Constructor
DerivationFramework::IsolationTrackDecorator::IsolationTrackDecorator(const std::string& t,
								      const std::string& n,
								      const IInterface* p):
  base_class(t, n, p){}

// Athena initialize and finalize
StatusCode DerivationFramework::IsolationTrackDecorator::initialize()
{
  ATH_MSG_DEBUG("initialize() ...");

  // load the matching tool
  if( ! m_caloIsolationTool.empty() ) {
    ATH_CHECK( m_caloIsolationTool.retrieve() );
    ATH_MSG_INFO( "Successfully retrived the CaloIsolationTool!" );
  }

  if( ! m_trackIsolationTool.empty() ) {
    ATH_CHECK( m_trackIsolationTool.retrieve() );
    ATH_MSG_INFO( "Successfully retrived the TrackIsolationTool!" );
  }

  ATH_CHECK(m_trackContainerKey.initialize());

  m_trkCorrList.trackbitset.set(static_cast<unsigned int>(xAOD::Iso::coreTrackPtr));
  m_topoconeCorrList.calobitset.set(static_cast<unsigned int>(xAOD::Iso::coreCone));
  m_topoconeCorrList.calobitset.set(static_cast<unsigned int>(xAOD::Iso::pileupCorrection));
  m_topoclusCorrList.calobitset.set(static_cast<unsigned int>(xAOD::Iso::pileupCorrection));

  /// create decorator list
  m_ptconeTypes.clear();
  m_ptvarconeTypes.clear();
  m_topoetconeTypes.clear();

  for(unsigned int i=0; i<m_iso.size(); i++){
    xAOD::Iso::IsolationType isoType = static_cast<xAOD::Iso::IsolationType>(m_iso[i]);
    xAOD::Iso::IsolationFlavour flavour = static_cast<xAOD::Iso::IsolationFlavour>(isoType/10.);

    if(flavour == xAOD::Iso::ptcone) {
      m_ptconeTypes.push_back(isoType);
      m_ptconeDecoratorsKey.emplace_back(m_trackContainerKey.key() + "." + m_prefix + m_iso_suffix[i]);
    }
    if(flavour == xAOD::Iso::ptvarcone) {
      m_ptvarconeTypes.push_back(isoType);
      m_ptvarconeDecoratorsKey.emplace_back(m_trackContainerKey.key() + "." + m_prefix + m_iso_suffix[i]);
    }
    if(flavour == xAOD::Iso::topoetcone) {
      m_topoetconeTypes.push_back(isoType);
      m_topoetconeDecoratorsKey.emplace_back(m_trackContainerKey.key() + "." + m_prefix + m_iso_suffix[i]);
      m_topoetconeNonCoreConeDecoratorsKey.emplace_back(m_trackContainerKey.key() + "." + m_prefix + m_iso_suffix[i] + "NonCoreCone");
    }
  }
  ATH_CHECK(m_ptconeDecoratorsKey.initialize());
  ATH_CHECK(m_ptvarconeDecoratorsKey.initialize());
  ATH_CHECK(m_topoetconeDecoratorsKey.initialize());
  ATH_CHECK(m_topoetconeNonCoreConeDecoratorsKey.initialize());

  // Set up the text-parsing machinery for thinning the tracks directly according to user cuts
  if (not m_selectionString.empty()) {
    ATH_CHECK(initializeParser(m_selectionString));
  }

  if(not m_selFlag.empty()){
    m_dec_trkFlagKey = m_trackContainerKey.key() + "." + m_selFlag;
  } else {
    m_dec_trkFlagKey = m_trackContainerKey.key() + ".no_key";
  }
  ATH_CHECK(m_dec_trkFlagKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::IsolationTrackDecorator::finalize()
{
  ATH_MSG_DEBUG("finalize() ...");
  ATH_CHECK(finalizeParser());

  return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::IsolationTrackDecorator::addBranches() const
{
  const EventContext& ctx = Gaudi::Hive::currentContext();
  // retrieve track container
  SG::ReadHandle<xAOD::TrackParticleContainer> toDecorate(m_trackContainerKey,ctx);
  ATH_CHECK( toDecorate.isValid() );

  // Execute the text parser and update the mask
  std::vector<int> entries(toDecorate->size(), 1);
  if (m_parser) {
    entries = m_parser->evaluateAsVector();
    if (entries.size() != toDecorate->size()) {
      ATH_MSG_ERROR("Sizes incompatible! Are you sure your selection string used ID TrackParticles?");
      return StatusCode::FAILURE;
    }
  }

  // build the decorators
  std::vector<SG::WriteDecorHandle<xAOD::TrackParticleContainer, float>> ptconeDecorators;
  for(unsigned int i=0; i<m_ptconeTypes.size(); i++){
    ptconeDecorators.emplace_back(m_ptconeDecoratorsKey[i], ctx);
  }
  std::vector<SG::WriteDecorHandle<xAOD::TrackParticleContainer, float>> ptvarconeDecorators;
  for(unsigned int i=0; i<m_ptvarconeTypes.size(); i++){
    ptvarconeDecorators.emplace_back(m_ptconeDecoratorsKey[i], ctx);
  }
  std::vector<SG::WriteDecorHandle<xAOD::TrackParticleContainer, float>> topoetconeDecorators;
  for(unsigned int i=0; i<m_topoetconeTypes.size(); i++){
    topoetconeDecorators.emplace_back(m_topoetconeDecoratorsKey[i], ctx);
  }
  std::vector<SG::WriteDecorHandle<xAOD::TrackParticleContainer, float>> topoetconeNonCoreConeDecorators;
  for(unsigned int i=0; i<m_topoetconeTypes.size(); i++){
    topoetconeNonCoreConeDecorators.emplace_back(m_topoetconeNonCoreConeDecoratorsKey[i], ctx);
  }
  
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, int> dec_trkFlag(m_dec_trkFlagKey, ctx);
  
  /// Loop over tracks
  int ipar=0;
  for(auto particle : *toDecorate) {
    bool IsPassed=true;

    /// Only decorate those passed selection
    if (!entries[ipar++]) IsPassed=false;

    /// check flag
    if(not m_selFlag.empty()){
      if(dec_trkFlag(*particle)!=m_selFlagValue) IsPassed=false;
    }

    if(m_iso.size()>0){
      if(IsPassed){
        /// track isolation
	xAOD::TrackIsolation resultTrack;
	xAOD::TrackIsolation resultTrackVar;
	xAOD::CaloIsolation resultCalo;
	xAOD::CaloIsolation resultCaloNonCoreCone; 
	if(m_ptconeTypes.size()) {

	  if (m_trackIsolationTool->trackIsolation(resultTrack, *particle, m_ptconeTypes, m_trkCorrList)){
	    for(unsigned int i=0; i<m_ptconeTypes.size(); i++){
	      ptconeDecorators[i](*particle) = resultTrack.ptcones[i];
	    }
	  }else{
	    ATH_MSG_WARNING("Failed to apply the track isolation for a particle");
	  }
	}

	if(m_ptvarconeTypes.size()) {

	  std::vector<xAOD::Iso::IsolationType> ptconeTypes = {};
	  for(auto isoType : m_ptvarconeTypes) {
	    int iso_tmp = static_cast<int>(isoType);
	    iso_tmp -= m_diff_ptvarcone;
	    ptconeTypes.push_back(static_cast<xAOD::Iso::IsolationType>(iso_tmp));
	  }

	  if (m_trackIsolationTool->trackIsolation(resultTrackVar, *particle, ptconeTypes, m_trkCorrList)){
	    for(unsigned int i=0; i<m_ptvarconeTypes.size(); i++){
	      ptvarconeDecorators[i](*particle) = resultTrackVar.ptvarcones_10GeVDivPt[i];
	    }
	  }else{
	    ATH_MSG_WARNING("Failed to apply the track isolation for a particle");
	  }
	}

	if(m_topoetconeTypes.size()) {

	  if (m_caloIsolationTool->caloTopoClusterIsolation(resultCalo, *particle, m_topoetconeTypes, m_topoconeCorrList)){
	    for(unsigned int i=0; i<m_topoetconeTypes.size(); i++){
	      topoetconeDecorators[i](*particle) = resultCalo.etcones[i];
	    }
	  }else{
	    ATH_MSG_WARNING("Failed to apply the track isolation for a particle");
	  }

	  if (m_caloIsolationTool->caloTopoClusterIsolation(resultCaloNonCoreCone, *particle, m_topoetconeTypes, m_topoclusCorrList)){
	    for(unsigned int i=0; i<m_topoetconeTypes.size(); i++){
	      topoetconeNonCoreConeDecorators[i](*particle) = resultCaloNonCoreCone.etcones[i];
	    }
	  }else {
	    ATH_MSG_WARNING("Failed to apply the topo calo isolation for a particle ( non CoreCone )");
	  }

	}

      }else{
	for(unsigned int i=0; i<m_ptconeTypes.size(); i++){
	  ptconeDecorators[i](*particle) = -999.e3;
	}
	for(unsigned int i=0; i<m_ptvarconeTypes.size(); i++){
	  ptvarconeDecorators[i](*particle) = -999.e3;
	}
	for(unsigned int i=0; i<m_topoetconeTypes.size(); i++){
	  topoetconeDecorators[i](*particle) = -999.e3;
	  topoetconeNonCoreConeDecorators[i](*particle) = -999.e3;
	}
      }
    }
  } 

  return StatusCode::SUCCESS;
}
