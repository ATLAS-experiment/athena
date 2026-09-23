/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PseudoTrackSelector.h
///////////////////////////////////////////////////////////////////
#ifndef DERIVATIONFRAMEWORK_PseudoTrackSelector_H
#define DERIVATIONFRAMEWORK_PseudoTrackSelector_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "InDetTrackSystematicsTools/IInDetTrackTruthOriginTool.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackStateValidationContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "ExpressionEvaluation/ExpressionParserUser.h"

#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"

#include "GaudiKernel/ToolHandle.h"

namespace DerivationFramework {

  /** @brief Class-algorithm for pseudo track selection */
  class PseudoTrackSelector : public AthReentrantAlgorithm
    {
    public:
      ///////////////////////////////////////////////////////////////////
      /** @brief Standard Algotithm methods:                           */
      ///////////////////////////////////////////////////////////////////

      using AthReentrantAlgorithm::AthReentrantAlgorithm;

      virtual StatusCode initialize() override final;
      virtual StatusCode execute(const EventContext& ctx) const override final;

    protected:
      ///////////////////////////////////////////////////////////////////
      /** @brief Protected data:                                       */
      ///////////////////////////////////////////////////////////////////
      SG::ReadHandleKey<xAOD::TrackParticleContainer>  m_in_recoTrackParticleLocation{this, "RecoTrackParticleLocation", ""};       /** Reco track collection.   */
      SG::ReadHandleKey<xAOD::TrackParticleContainer>  m_in_pseudoTrackParticleLocation{this, "PseudoTrackParticleLocation", ""};     /** Pseudo track collection. */
      SG::WriteHandleKey<xAOD::TrackParticleContainer> m_out_recoReplacedWithPseudo{this, "OutputRecoReplacedWithPseudo", ""};         /** Output track collection. */
      SG::WriteHandleKey<xAOD::TrackParticleContainer> m_out_recoReplacedWithPseudoFromB{this, "OutputRecoReplacedWithPseudoFromB", ""};    /** Output track collection. */
      SG::WriteHandleKey<xAOD::TrackParticleContainer> m_out_recoReplacedWithPseudoNotFromB{this, "OutputRecoReplacedWithPseudoNotFromB", ""}; /** Output track collection. */
      SG::WriteHandleKey<xAOD::TrackParticleContainer> m_out_recoPlusPseudo{this, "OutputRecoPlusPseudo", ""};                 /** Output track collection. */
      SG::WriteHandleKey<xAOD::TrackParticleContainer> m_out_recoPlusPseudoFromB{this, "OutputRecoPlusPseudoFromB", ""};            /** Output track collection. */
      SG::WriteHandleKey<xAOD::TrackParticleContainer> m_out_recoPlusPseudoNotFromB{this, "OutputRecoPlusPseudoNotFromB", ""};         /** Output track collection. */
      SG::WriteHandleKey<xAOD::TrackParticleContainer> m_out_recoNoFakes{this, "OutputRecoNoFakes", ""};                    /** Output track collection. */
      SG::WriteHandleKey<xAOD::TrackParticleContainer> m_out_recoNoFakesFromB{this, "OutputRecoNoFakesFromB", ""};               /** Output track collection. */
      SG::WriteHandleKey<xAOD::TrackParticleContainer> m_out_recoNoFakesNotFromB{this, "OutputRecoNoFakesNotFromB", ""};            /** Output track collection. */

      ///////////////////////////////////////////////////////////////////
      /** @brief Protected methods:                                    */
      ///////////////////////////////////////////////////////////////////

      /** @brief Routines that selects the relevant (pseudo) tracks. */
      void fillRecoReplacedWithPseudo(const xAOD::TrackParticleContainer* recoTrackParticleCol,
                                      const xAOD::TrackParticleContainer* pseudoTrackParticleCol,
                                      xAOD::TrackParticleContainer* outputCol,
                                      bool onlyFromB = false,
                                      bool onlyNotFromB = false) const;
      void fillRecoPlusPseudo(const xAOD::TrackParticleContainer* recoTrackParticleCol,
                              const xAOD::TrackParticleContainer* pseudoTrackParticleCol,
                              xAOD::TrackParticleContainer* outputCol,
                              bool onlyFromB = false,
                              bool onlyNotFromB = false) const;
      void fillRecoNoFakes(const xAOD::TrackParticleContainer* recoTrackParticleCol,
                           xAOD::TrackParticleContainer* outputCol,
                           bool onlyFromB = false,
                           bool onlyNotFromB = false) const;
      // Get truth particle associated to a given track particle
      static const xAOD::TruthParticle* getTruth( const xAOD::TrackParticle* track ) ;

    private:
        // convience types
        typedef ElementLink<xAOD::TruthParticleContainer> TruthLink;
        // Track origin tool
        ToolHandle< InDet::IInDetTrackTruthOriginTool > m_trackOriginTool{this, "trackTruthOriginTool", "InDet::InDetTrackTruthOriginTool", "truth track origin tool"};
    };
}
#endif // DERIVATIONFRAMEWORK_PseudoTrackSelector_H
