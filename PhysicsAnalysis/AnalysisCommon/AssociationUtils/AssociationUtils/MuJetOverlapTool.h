/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_MUJETOVERLAPTOOL_H
#define ASSOCIATIONUTILS_MUJETOVERLAPTOOL_H

// Framework includes

// EDM includes
#include "xAODMuon/MuonContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/VertexContainer.h"

// Columnar includes
#include "ColumnarCore/ObjectColumn.h"
#include "ColumnarCore/VectorColumn.h"
#include "ColumnarJet/JetDef.h"
#include "ColumnarMuon/MuonDef.h"
#include "ColumnarTracking/TrackDef.h"

// Local includes
#include "AssociationUtils/IOverlapTool.h"
#include "AssociationUtils/BaseOverlapTool.h"
#include "AssociationUtils/IObjectAssociator.h"

namespace ORUtils
{

  /// @class MuJetOverlapTool
  /// @brief A tool implementing the recommended mu-jet overlap removal.
  ///
  /// This tool takes muons and jets and removes their overlaps based on
  /// various criteria including delta-R, jet track multiplicity, mu/jet PT
  /// ratio, and btagging results.
  ///
  /// The procedure works as follows.
  ///   1. Remove non-btagged jets that overlap with muons.
  ///        * Matching is done with ghost association or flat delta-R.
  ///        * Jets are then rejected if they have low track multiplicity or if
  ///          the muon has sufficient PT relative to the jet.
  ///   2. Remove muons from remaining non-pileup jets.
  ///        * Matching is done with either a flat delta-R or with the sliding
  ///          delta-R cone (c1 + c2/pt).
  ///
  /// To determine jet track multiplicity, the primary vertex is retrieved
  /// and used to query the NumTrkPt500 jet attribute.
  ///
  /// Some special configuration properties
  ///   UseSlidingDR     - activate sliding dR cone (c1 + c2/muPt)
  ///   SlidingDRC1      - c1 constant for sliding dR (default 0.04)
  ///   SlidingDRC2      - c2 constant for sliding dR (default 10GeV)
  ///   SlidingDRMaxCone - max cone size cutoff for sliding dR (default off)
  ///   NumJetTrk        - number of tracks required to give priorty to jet
  ///                      and remove the muon.
  ///
  /// @todo TODO document all the new properties!
  ///
  /// These properties are useful for specifying PV-related quantities so that
  /// the tool doesn't need to retrieve the PV:
  ///   JetNumTrackDecoration   - user-defined numTrk decoration
  ///   JetSumTrackPTDecoration - user-defined sumTrkPt decoration
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class MuJetOverlapTool : public virtual IOverlapTool,
                           public BaseOverlapTool
  {

      /// Create proper constructor for Athena
      ASG_TOOL_CLASS(MuJetOverlapTool, IOverlapTool)

    public:

      /// Standalone constructor
      MuJetOverlapTool(const std::string& name);

      /// @brief Identify overlapping muons and jets.
      /// First, low-track-multiplicity jets are flagged if they overlap with
      /// muons. Second, muons are flagged if they overlap with the remaining
      /// jets.
      virtual StatusCode
      findOverlaps(columnar::Particle1Range cont1,
                   columnar::Particle2Range cont2,
                   columnar::EventContextId eventContext) const override;

      /// @brief Identify overlapping muons and jets.
      /// The above method calls this one.
      virtual StatusCode
      internalFindOverlaps(columnar::Particle1Range muons,
                           columnar::Particle2Range jets,
                           columnar::EventContextId eventContext) const;

    protected:

      /// Initialize the tool
      virtual StatusCode initializeDerived() override;

    protected:

      /// Retrieve the primary vertex used to count jet tracks
      std::optional<std::size_t> getPrimVtxIdx(columnar::EventContextId eventContext) const;

      /// Get the number of tracks in a jet w.r.t. requested vertex
      int getNumTracks(columnar::Particle2Id jet, size_t vtxIdx) const;

      /// Get the sum trk pt in a jet w.r.t. requested vertex
      float getSumTrackPt(columnar::Particle2Id jet, size_t vtxIdx) const;

    private:

      /// @name Configurable properties
      /// @{

      /// Input jet decoration which labels a bjet
      std::string m_bJetLabel;

      /// Minimum number of jet tracks to prioritize the jet
      int m_numJetTrk;
      /// Toggle PT ratio criteria
      /// TODO: rename to m_applyPtRatio
      bool m_applyRelPt;
      /// PT ratio threshold for vetoing the jet.
      float m_muJetPtRatio;
      /// PT ratio threshold for vetoing the jet. Uses sum of jet track PT.
      float m_muJetTrkPtRatio;

      /// Optional user decoration for jet numTrack of type 'int'
      std::string m_jetNumTrkDec;
      /// Optional user decoration for jet sumTrackPT of type 'float'
      std::string m_jetSumTrkPtDec;

      /// Use ghost association in matching to remove jets
      bool m_useGhostAssociation;

      /// Inner dR cone within which jets get removed
      float m_innerDR;
      /// Outer dR cone within which muons get removed.
      /// Replaced by sliding dR if activated.
      float m_outerDR;

      /// Toggle sliding dR overlap removal for removing muons
      bool m_useSlidingDR;
      /// C1, the constant offset in sliding dR
      double m_slidingDRC1;
      /// C1, the inverse pt factor in sliding dR
      double m_slidingDRC2;
      /// MaxCone, the upper limit of the sliding cone
      double m_slidingDRMaxCone;

      /// Calculate deltaR using rapidity
      bool m_useRapidity;

      /// PV Container to use
      std::string m_PVContName;

      // Allow no PVs in the event
      bool m_allowNoPV;

      /// Columnar accessors
      struct Accessors final : columnar::ColumnarTool<>
      {
        columnar::Track0Accessor<columnar::ObjectColumn> m_track0Acc {*this, "InDetTrackParticles"};
        columnar::Track1Accessor<columnar::ObjectColumn> m_track1Acc {*this, "InDetForwardTrackParticles"};
        columnar::Particle1Accessor<float> m_muonPtAcc {*this, "pt"};
        columnar::Particle2Accessor<float> m_jetPtAcc {*this, "pt"};
        columnar::Particle2Accessor<int> m_jetNumTrkAcc;
        columnar::Particle2Accessor< std::vector<int> > m_jetNumTrkPt500Acc;
        columnar::Particle2Accessor<int> m_jetSumTrkPtAcc;
        columnar::Particle2Accessor< std::vector<float> > m_jetSumTrkPt500Acc;
        columnar::VertexAccessor<columnar::ObjectColumn> m_vtxContainerAcc;
        columnar::VertexAccessor<columnar::RetypeColumn<xAOD::VxType::VertexType,short>> m_vertexTypeAcc {*this, "vertexType"};
        using ColumnarTool::ColumnarTool;
      };
      std::unique_ptr<Accessors> m_accessors {std::make_unique<Accessors> (this)};

      /// @}

      /// @name Utilities
      /// @{

      /// BJet helper
      columnar::Particle2Accessor<char> m_bJetAcc;

      /// Delta-R matcher for the inner cone
      std::unique_ptr<IParticleAssociator> m_dRMatchCone1;
      /// Delta-R matcher for the outer cone
      std::unique_ptr<IParticleAssociator> m_dRMatchCone2;

      /// @}

  }; // class MuJetOverlapTool

} // namespace ORUtils

#endif
