/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_TAU_ANTITAU_JET_OVERLAPTOOL_H
#define ASSOCIATIONUTILS_TAU_ANTITAU_JET_OVERLAPTOOL_H

// Framework includes
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"

// EDM includes
#include "AsgDataHandles/ReadHandleKey.h"
#include <xAODEventInfo/EventInfo.h>
#include "xAODTau/TauJetContainer.h"
#include "xAODJet/JetContainer.h"

// Columnar includes
#include "ColumnarCore/ObjectColumn.h"
#include "ColumnarEventInfo/EventInfoDef.h"
#include "ColumnarTau/TauJetDef.h"
#include "ColumnarJet/JetDef.h"

// Local includes
#include "AssociationUtils/IOverlapTool.h"
#include "AssociationUtils/BaseOverlapTool.h"
#include "AssociationUtils/IObjectAssociator.h"

namespace ORUtils
{

  /// @class TauAntiTauJetOverlapTool
  /// @brief A tool implementing a specialized tau-jet overlap removal.
  ///
  /// This tool was implemented by request for the h->bbtautau analysis which
  /// uses a loose-ish btag working point and also considers anti-tau ID.
  ///
  /// Object precedence:
  ///   ID-tau > bjet > anti-tau > light-jet
  ///
  /// The procedure works as follows.
  ///   1. Remove bjets overlapping with ID taus.
  ///   2. Remove anti-taus from remaining bjets.
  ///   3. Remove light jets from remaining ID-taus and anti-taus.
  ///
  /// To fully utilize the above procedure you have to set the following
  /// properties:
  ///
  /// * TauLabel: The user-set decoration name labeling IDed taus.
  ///   E.g., "isIDTau". Default is empty, which does not apply any selection.
  ///
  /// * AntiTauLabel: The user-set decoration name labeling anti-taus.
  ///   E.g., "isAntiTau". Default is empty, which disables anti-taus.
  ///
  /// * BJetLabel: The usual user-set decoration name labeling bjets.
  ///   E.g., "isBJet". Default is empty, which disables bjet OR.
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class TauAntiTauJetOverlapTool : public virtual IOverlapTool,
                                   public BaseOverlapTool
  {

      /// Create proper constructor for Athena
      ASG_TOOL_CLASS(TauAntiTauJetOverlapTool, IOverlapTool)

    public:

      /// Standalone constructor
      TauAntiTauJetOverlapTool(const std::string& name);

      /// @brief Identify overlapping taus and jets.
      virtual StatusCode
      findOverlaps(columnar::Particle1Range cont1,
                   columnar::Particle2Range cont2,
                   columnar::EventContextId eventContext) const override;

      /// @brief Identify overlapping taus and jets.
      /// The above method calls this one.
      virtual StatusCode
      internalFindOverlaps(columnar::Particle1Range jets,
                           columnar::Particle2Range taus,
                           columnar::EventContextId eventContext) const;

    protected:
      /// @name Helper methods
      /// @{

    protected:

      /// Initialize the tool
      virtual StatusCode initializeDerived() override;

      /// Is this jet a b-jet? Returns false if bjet ID not configured.
      /// Does not check if the jet is "surviving" OR.
      bool isBJet(columnar::Particle1Id jet) const;

      /// Is this an ID tau? Returns false if tau ID not configured.
      /// This one does check if the tau is "surviving" OR.
      bool isSurvivingTau(columnar::Particle2Id tau) const;

      /// Is this an anti-tau? Returns false if anti-tau ID not configured.
      /// This one does check if the tau is "surviving" OR.
      bool isSurvivingAntiTau(columnar::Particle2Id tau) const;

    private:
      /// @name Configurable properties
      /// @{

      /// Input jet decoration which labels a bjet
      std::string m_bJetLabel;

      /// Decoration labelling an IDed tau
      std::string m_tauLabel;

      /// Decoration labelling an anti-tau
      std::string m_antiTauLabel;

      /// Flat delta-R cone for matching objects
      float m_dR;
      /// Calculate deltaR using rapidity
      bool m_useRapidity;

      /// @}

    private:
      /// @name Utilities
      /// @{

      /// Columnar accessors
      struct Accessors final : columnar::ColumnarTool<>
      {
        columnar::EventInfoAccessor<std::uint64_t> m_eventNumberAcc {*this, "eventNumber"};
        columnar::Particle2Accessor<int> m_categoryAcc;

        /// Columnar accessors
        columnar::EventInfoAccessor<columnar::ObjectColumn> m_evtAcc;

        /// BJet helper
        columnar::Particle1Accessor<char> m_bJetAcc;

        using ColumnarTool::ColumnarTool;
      };
      std::unique_ptr<Accessors> m_accessors {std::make_unique<Accessors> (this)};

      /// Delta-R matcher
      std::unique_ptr<IParticleAssociator> m_dRMatcher;

      /// Decoration helper for the IDed taus
      std::unique_ptr<OverlapDecorationHelper<columnar::ContainerId::particle2>> m_tauDecHelper;

      /// Decoration helper for the anti-taus
      std::unique_ptr<OverlapDecorationHelper<columnar::ContainerId::particle2>> m_antiTauDecHelper;

      std::string m_antiTauEventCategoryDecorName;

      Gaudi::Property<std::string> m_evtKeyName{this, "EventInfoKey", "EventInfo", "xAOD::EventInfo ReadHandleKey"};

      /// @}

  }; // class TauAntiTauJetOverlapTool

} // namespace ORUtils

#endif
