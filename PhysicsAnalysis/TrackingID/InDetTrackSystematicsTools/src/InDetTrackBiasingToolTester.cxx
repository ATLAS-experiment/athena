 /*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// Framework include(s):
#include "xAODCore/ShallowCopy.h"

// EDM include(s):
#include "xAODTracking/TrackParticleContainer.h"

// Local include(s):
#include "InDetTrackBiasingToolTester.h"
#include <TH1.h>


namespace InDet {
   InDetTrackBiasingToolTester::InDetTrackBiasingToolTester( const std::string& name, ISvcLocator* svcLoc )
      : AthHistogramAlgorithm( name, svcLoc ),
        m_biasTool( "InDet::InDetTrackSystematicsTools/InDetTrackBiasingTool", this ){
          declareProperty( "InDetTrackBiasingTool", m_biasTool );
        }

   StatusCode InDetTrackBiasingToolTester::initialize() {

      ATH_MSG_INFO( "Initialising" );
      ATH_CHECK( m_trackKey.initialize() );
      ATH_CHECK( m_biasTool.retrieve() );

      // Nominal: correction is applied, tracks should differ from original
      ATH_CHECK( book( TH1F("d0_original",       "original d0",                           100, -5.0,  5.0) ) );
      ATH_CHECK( book( TH1F("z0_original",       "original z0",                           100, -200., 200.) ) );
      ATH_CHECK( book( TH1F("d0_nominal",        "d0 after nominal correction",            100, -5.0,  5.0) ) );
      ATH_CHECK( book( TH1F("z0_nominal",        "z0 after nominal correction",            100, -200., 200.) ) );
      ATH_CHECK( book( TH1F("d0_nominal_delta",  "d0 nominal - original (expect nonzero)", 100, -0.10, 0.10) ) );
      ATH_CHECK( book( TH1F("z0_nominal_delta",  "z0 nominal - original (expect nonzero)", 100, -0.50, 0.50) ) );

      // Systematic: correction is undone, tracks should be identical to original
      ATH_CHECK( book( TH1F("d0_systematic",        "d0 after systematic variation",            100, -5.0,  5.0) ) );
      ATH_CHECK( book( TH1F("z0_systematic",        "z0 after systematic variation",            100, -200., 200.) ) );
      ATH_CHECK( book( TH1F("d0_systematic_delta",  "d0 systematic - original (expect zero)",   100, -0.10, 0.10) ) );
      ATH_CHECK( book( TH1F("z0_systematic_delta",  "z0 systematic - original (expect zero)",   100, -0.50, 0.50) ) );

      return StatusCode::SUCCESS;
   }

   StatusCode InDetTrackBiasingToolTester::execute() {

      SG::ReadHandle<xAOD::TrackParticleContainer> IDParticles(m_trackKey);
      ATH_CHECK( IDParticles.isValid() );

      // --- Nominal: no systematics active, correction should be applied ---
      ATH_CHECK( m_biasTool->applySystematicVariation( {} ) );
      auto nominalCopy = xAOD::shallowCopyContainer( *IDParticles );
      for ( xAOD::TrackParticle* track : *nominalCopy.first ) {
         const double d0_orig = track->d0();
         const double z0_orig = track->z0();
         hist("d0_original")->Fill( d0_orig );
         hist("z0_original")->Fill( z0_orig );
         if ( m_biasTool->applyCorrection(*track) == CP::CorrectionCode::Error ) {
            ATH_MSG_ERROR( "Could not apply nominal correction." );
         }
         hist("d0_nominal")->Fill( track->d0() );
         hist("z0_nominal")->Fill( track->z0() );
         hist("d0_nominal_delta")->Fill( track->d0() - d0_orig );
         hist("z0_nominal_delta")->Fill( track->z0() - z0_orig );
      }
      delete nominalCopy.first;
      delete nominalCopy.second;

      // --- Systematic: all biasing systematics active, correction should be undone ---
      ATH_CHECK( m_biasTool->applySystematicVariation( m_biasTool->affectingSystematics() ) );
      auto systematicCopy = xAOD::shallowCopyContainer( *IDParticles );
      for ( xAOD::TrackParticle* track : *systematicCopy.first ) {
         const double d0_orig = track->d0();
         const double z0_orig = track->z0();
         if ( m_biasTool->applyCorrection(*track) == CP::CorrectionCode::Error ) {
            ATH_MSG_ERROR( "Could not apply systematic correction." );
         }
         hist("d0_systematic")->Fill( track->d0() );
         hist("z0_systematic")->Fill( track->z0() );
         hist("d0_systematic_delta")->Fill( track->d0() - d0_orig );
         hist("z0_systematic_delta")->Fill( track->z0() - z0_orig );
      }
      delete systematicCopy.first;
      delete systematicCopy.second;

      return StatusCode::SUCCESS;

   } // End of execute()

} // namespace InDet
