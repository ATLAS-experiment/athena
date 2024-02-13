/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#include "FlavorTagDiscriminants/TrackLeptonDecoratorAlg.h"

#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"

#include "xAODTracking/TrackParticleContainer.h"

#include "xAODEgamma/ElectronxAODHelpers.h"
#include "ElectronPhotonSelectorTools/ElectronSelectorHelpers.h"


namespace FlavorTagDiscriminants {

  TrackLeptonDecoratorAlg::TrackLeptonDecoratorAlg(
    const std::string& name, ISvcLocator* loc )
    : AthReentrantAlgorithm(name, loc),
      m_electronID_tool("AsgElectronLikelihoodTool/electronID_tool") {}

  StatusCode TrackLeptonDecoratorAlg::initialize() {
    ATH_MSG_INFO( "Inizializing " << name() << "... " );

    // Initialize Container keys
    ATH_MSG_DEBUG( "Inizializing containers:"            );
    ATH_MSG_DEBUG( "    ** " << m_TrackContainerKey      );
    ATH_MSG_DEBUG( "    ** " << m_ElectronContainerKey   );
    ATH_MSG_DEBUG( "    ** " << m_MuonContainerKey       );

    ATH_CHECK( m_TrackContainerKey.initialize() );
    ATH_CHECK( m_ElectronContainerKey.initialize() );
    ATH_CHECK( m_MuonContainerKey.initialize() );

    // Initialise electron ID tool
    ATH_CHECK(m_electronID_tool.setProperty("WorkingPoint", m_electronID_wp));
    ATH_CHECK(m_electronID_tool.initialize());

    // Prepare decorators
    m_dec_lepton_id = m_TrackContainerKey.key() + "." + m_dec_lepton_id.key();
    m_dec_muon_quality = m_TrackContainerKey.key() + "." + m_dec_muon_quality.key();
    m_dec_muon_qOverPratio = m_TrackContainerKey.key() + "." + m_dec_muon_qOverPratio.key();
    m_dec_muon_momentumBalanceSignificance = m_TrackContainerKey.key() + "." + m_dec_muon_momentumBalanceSignificance.key();
    m_dec_muon_scatteringNeighbourSignificance = m_TrackContainerKey.key() + "." + m_dec_muon_scatteringNeighbourSignificance.key();

    // Initialize decorators
    ATH_MSG_DEBUG( "Inizializing decorators:"  );
    ATH_MSG_DEBUG( "    ** " << m_dec_lepton_id );
    ATH_MSG_DEBUG( "    ** " << m_dec_muon_quality );
    ATH_MSG_DEBUG( "    ** " << m_dec_muon_qOverPratio );
    ATH_MSG_DEBUG( "    ** " << m_dec_muon_momentumBalanceSignificance );
    ATH_MSG_DEBUG( "    ** " << m_dec_muon_scatteringNeighbourSignificance );

    ATH_CHECK( m_dec_lepton_id.initialize() );
    ATH_CHECK( m_dec_muon_quality.initialize() );
    ATH_CHECK( m_dec_muon_qOverPratio.initialize() );
    ATH_CHECK( m_dec_muon_momentumBalanceSignificance.initialize() );
    ATH_CHECK( m_dec_muon_scatteringNeighbourSignificance.initialize() );

    return StatusCode::SUCCESS;
  }

  StatusCode TrackLeptonDecoratorAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Executing " << name() << "... " );

    // read collections
    SG::ReadHandle<xAOD::TrackParticleContainer> tracks(
      m_TrackContainerKey,ctx);
    CHECK( tracks.isValid() );
    ATH_MSG_DEBUG( "Retrieved " << tracks->size() << " tracks..." );
    SG::ReadHandle<xAOD::ElectronContainer> electrons(
      m_ElectronContainerKey,ctx);
    CHECK( tracks.isValid() );
    ATH_MSG_DEBUG( "Retrieved " << electrons->size() << " electrons..." );
    SG::ReadHandle<xAOD::MuonContainer> muons(
      m_MuonContainerKey,ctx);
    CHECK( tracks.isValid() );
    ATH_MSG_DEBUG( "Retrieved " << muons->size() << " muons..." );

    using TPC = xAOD::TrackParticleContainer;
    SG::WriteDecorHandle<TPC, char> decor_lepton_id(m_dec_lepton_id, ctx);
    SG::WriteDecorHandle<TPC, char> decor_muon_quality(m_dec_muon_quality, ctx);
    SG::WriteDecorHandle<TPC, float> decor_muon_qOverPratio(m_dec_muon_qOverPratio, ctx);
    SG::WriteDecorHandle<TPC, float> decor_muon_momentumBalanceSignificance(m_dec_muon_momentumBalanceSignificance, ctx);
    SG::WriteDecorHandle<TPC, float> decor_muon_scatteringNeighbourSignificance(m_dec_muon_scatteringNeighbourSignificance, ctx);

    // give all tracks a default starting value
    for ( const xAOD::TrackParticle* track : *tracks ) {
      decor_lepton_id(*track) = 0;
      decor_muon_quality(*track) = -1;
      decor_muon_qOverPratio(*track) = -99.;
      decor_muon_momentumBalanceSignificance(*track) = -99.;
      decor_muon_scatteringNeighbourSignificance(*track) = -99.;
    }

    // loop over electrons
    for ( const auto electron : *electrons ) {

      // get associated InDet track (not the GSF track which is likely to have improved parameters,
      // more info: https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/EGammaD3PDtoxAOD#TrackParticle)
      const xAOD::TrackParticle* track = xAOD::EgammaHelpers::getOriginalTrackParticle(electron);
      if ( !track ) { continue; }

      // apply electron ID requirement
      bool LH_selection = (bool)m_electronID_tool.accept(electron);
      if (!LH_selection) { continue; }

      // decorate the track
      decor_lepton_id(*track) = -11 * electron->charge();
    }

    // loop over muons - do it last in case we have a track that was used in the 
    // reconstruction of an electron and a muon (which can happen in rare cases)
    for ( const auto muon : *muons ) {

      // minimal quality requirement: check we have a combined muon
      if (muon->muonType() != xAOD::Muon::Combined) { continue; }

      // get associated InDet track
      auto track_link = muon->inDetTrackParticleLink();
      if ( !track_link.isValid() ) { continue; }
      auto track = *track_link;

      // get associated muon spectrometer track
      auto ms_track_link = muon->extrapolatedMuonSpectrometerTrackParticleLink();
      if ( !ms_track_link.isValid() ) { continue; }
      auto ms_track = *ms_track_link;

      float momBalSig = 0.;
      float scatNeighSig = 0.;
      muon->parameter(momBalSig, xAOD::Muon::momentumBalanceSignificance);
      muon->parameter(scatNeighSig, xAOD::Muon::scatteringNeighbourSignificance);

      // decorate the track
      decor_lepton_id(*track) = -13 * muon->charge();
      decor_muon_quality(*track) = muon->quality();
      decor_muon_qOverPratio(*track) = track->qOverP() / ms_track->qOverP();
      decor_muon_momentumBalanceSignificance(*track) = momBalSig;
      decor_muon_scatteringNeighbourSignificance(*track) = scatNeighSig;
    }

    return StatusCode::SUCCESS;
  }

}


