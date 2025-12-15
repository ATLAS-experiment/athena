/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_TAULOOSEMUOVERLAPTOOL_H
#define ASSOCIATIONUTILS_TAULOOSEMUOVERLAPTOOL_H

// Framework includes
#include "AsgTools/AsgTool.h"

// EDM includes
#include "xAODTau/TauJetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "ColumnarMuon/MuonDef.h"
#include "ColumnarTau/TauJetDef.h"

// Local includes
#include "AssociationUtils/IOverlapTool.h"
#include "AssociationUtils/BaseOverlapTool.h"
#include "AssociationUtils/DeltaRMatcher.h"

namespace ORUtils
{

  /// @class TauLooseMuOverlapTool
  /// @brief A tool implementing the recommended tau-muon overlap removal.
  ///
  /// This implementation has a custom loose-muon selection as recommended
  /// in the harmonization document. Note: this functionality should be already
  /// available in the TauAnalysisTools package, but I'm adding it here for
  /// compatibility with the old OverlapRemovalTool.
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class TauLooseMuOverlapTool : public virtual IOverlapTool,
                                public BaseOverlapTool
  {

      /// Create proper constructor for Athena
      ASG_TOOL_CLASS(TauLooseMuOverlapTool, IOverlapTool)

    public:

      /// Standalone constructor
      TauLooseMuOverlapTool(const std::string& name);

      /// @brief Identify overlapping taus and loose muons.
      /// TODO: add description of the method.
      virtual StatusCode
      findOverlaps(columnar::Particle1Range cont1,
                   columnar::Particle2Range cont2,
                   columnar::EventContextId eventContext) const override;

      /// @brief Identify overlapping taus and loose muons.
      /// See the documentation in the above method.
      virtual StatusCode
      internalFindOverlaps(columnar::Particle1Range taus,
                           columnar::Particle2Range muons) const;

    protected:

      /// Initialize the tool
      virtual StatusCode initializeDerived() override;

    private:

      //
      // Configurable properties
      //

      /// Maximum dR for objects flagged as overlap
      float m_maxDR;
      /// Calculate delta-R using rapidity
      bool m_useRapidity;

      /// Minimum muon PT to reject a tau
      float m_minMuPt;
      /// Tau PT threshold to compare to combined muons only
      float m_minTauPtMuComb;

      /// Columnar accessors
      struct Accessors final : columnar::ColumnarTool<>
      {
        columnar::Particle1Accessor<float> m_tauPtAcc {*this, "pt"};
        columnar::Particle2Accessor<float> m_muPtAcc {*this, "pt"};
        columnar::Particle2Accessor<columnar::RetypeColumn<xAOD::Muon::MuonType,std::uint16_t>> m_muonTypeAcc {*this, "muonType"};
        using ColumnarTool::ColumnarTool;
      };
      std::unique_ptr<Accessors> m_accessors {std::make_unique<Accessors> (this)};

      //
      // Utilities
      //

      /// Delta-R matcher
      std::unique_ptr<DeltaRMatcher> m_dRMatcher;

  }; // class TauLooseMuOverlapTool

} // namespace ORUtils

#endif
