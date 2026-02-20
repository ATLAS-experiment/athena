/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_TAUJETOVERLAPTOOL_H
#define ASSOCIATIONUTILS_TAUJETOVERLAPTOOL_H

// Framework includes
#include "AsgTools/AsgTool.h"

// EDM includes
#include "xAODTau/TauJetContainer.h"
#include "xAODJet/JetContainer.h"
#include "ColumnarCore/ColumnAccessor.h"
#include "ColumnarTau/TauJetDef.h"
#include "ColumnarJet/JetDef.h"

// Local includes
#include "AssociationUtils/IOverlapTool.h"
#include "AssociationUtils/BaseOverlapTool.h"
#include "AssociationUtils/IObjectAssociator.h"

namespace ORUtils
{

  /// @class TauJetOverlapTool
  /// @brief A tool implementing the recommended tau-jet overlap removal.
  ///
  /// This tool takes taus and jets and removes their overlaps based on
  /// delta-R and btagging results. In fact, currently the only difference
  /// between this implementation and the DeltaROverlapTool is that this tool
  /// will optionally give precedence to bjets over taus.
  ///
  /// The procedure works as follows.
  ///   1. Remove non-btagged jets that overlap with taus.
  /// That's it (for now).
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class TauJetOverlapTool : public virtual IOverlapTool,
                            public BaseOverlapTool
  {

      /// Create proper constructor for Athena
      ASG_TOOL_CLASS(TauJetOverlapTool, IOverlapTool)

    public:

      /// Standalone constructor
      TauJetOverlapTool(const std::string& name);

      /// @brief Identify overlapping taus and jets.
      virtual StatusCode
      findOverlaps(columnar::Particle1Range cont1,
                   columnar::Particle2Range cont2,
                   columnar::EventContextId eventContext) const override;

      /// @brief Identify overlapping taus and jets.
      /// The above method calls this one.
      virtual StatusCode
      internalFindOverlaps(columnar::Particle1Range jets,
                           columnar::Particle2Range taus) const;

    protected:

      /// Initialize the tool
      virtual StatusCode initializeDerived() override;

    private:

      /// @name Configurable properties
      /// @{

      /// Input jet decoration which labels a bjet
      std::string m_bJetLabel;

      /// Flat delta-R cone for matching objects
      float m_dR;
      /// Calculate deltaR using rapidity
      bool m_useRapidity;

      /// @}

      /// @name Utilities
      /// @{

      /// Columnar accessors
      struct Accessors final : columnar::ColumnarTool<>
      {
        /// BJet helper
        columnar::Particle1Accessor<char> m_bJetAcc;
        using ColumnarTool::ColumnarTool;
      };
      std::unique_ptr<Accessors> m_accessors {std::make_unique<Accessors> (this)};

      /// Delta-R matcher
      std::unique_ptr<IParticleAssociator> m_dRMatcher;

      /// @}

  }; // class TauJetOverlapTool

} // namespace ORUtils

#endif
