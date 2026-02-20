/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_TAULOOSEELEOVERLAPTOOL_H
#define ASSOCIATIONUTILS_TAULOOSEELEOVERLAPTOOL_H

// Framework includes
#include "AsgTools/AsgTool.h"

// EDM includes
#include "xAODTau/TauJetContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "ColumnarTau/TauJetDef.h"
#include "ColumnarEgamma/EgammaDef.h"

// Local includes
#include "AssociationUtils/IOverlapTool.h"
#include "AssociationUtils/BaseOverlapTool.h"
#include "AssociationUtils/DeltaRMatcher.h"

namespace ORUtils
{

  /// @class TauLooseEleOverlapTool
  /// @brief A tool implementing the recommended tau-electron overlap removal.
  ///
  /// This implementation has a custom loose-electron selection as recommended
  /// in the harmonization document. Note: this functionality should be already
  /// available in the TauAnalysisTools package, but I'm adding it here for
  /// compatibility with the old OverlapRemovalTool.
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class TauLooseEleOverlapTool : public virtual IOverlapTool,
                                 public BaseOverlapTool
  {

      /// Create proper constructor for Athena
      ASG_TOOL_CLASS(TauLooseEleOverlapTool, IOverlapTool)

    public:

      /// Standalone constructor
      TauLooseEleOverlapTool(const std::string& name);

      /// @brief Identify overlapping taus and loose electrons.
      /// TODO: add description of the method.
      virtual StatusCode
      findOverlaps(columnar::Particle1Range cont1,
                   columnar::Particle2Range cont2,
                   columnar::EventContextId eventContext) const override;

      /// @brief Identify overlapping taus and loose electrons.
      /// See the documentation in the above method.
      virtual StatusCode
      internalFindOverlaps(columnar::Particle1Range taus,
                           columnar::Particle2Range electrons) const;

    protected:

      /// Initialize the tool
      virtual StatusCode initializeDerived() override;

    protected:

      StatusCode checkElectronID(columnar::Particle2Id electron, bool& pass) const;

    private:

      //
      // Configurable properties
      //

      /// Maximum dR for objects flagged as overlap
      float m_maxDR;
      /// Calculate delta-R using rapidity
      bool m_useRapidity;

      /// Loose electron selection criteria string (e.g. Loose)
      std::string m_eleID;

      /// Alternate fallback loose ID string; mainly convenient for testing.
      /// You should only set this if you really know what you're doing.
      std::string m_altEleID;

      //
      // Utilities
      //

      /// Columnar accessors
      struct Accessors final : columnar::ColumnarTool<>
      {
        columnar::Particle2Accessor<char> m_eleIDAcc;
        columnar::Particle2Accessor<char> m_altEleIDAcc;
        using ColumnarTool::ColumnarTool;
      };
      std::unique_ptr<Accessors> m_accessors {std::make_unique<Accessors> (this)};

      /// Delta-R matcher
      std::unique_ptr<DeltaRMatcher> m_dRMatcher;

  }; // class TauLooseEleOverlapTool

} // namespace ORUtils

#endif
