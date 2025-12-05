/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_SHAREDTRKOVERLAPTOOL_H
#define ASSOCIATIONUTILS_SHAREDTRKOVERLAPTOOL_H

// Framework includes
#include "AsgTools/AsgTool.h"

// EDM includes
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "ColumnarEgamma/EgammaDef.h"
#include "ColumnarMuon/MuonDef.h"

// Columnar includes
#include "ColumnarCore/LinkColumn.h"
#include "ColumnarTracking/TrackDef.h"
#include "ColumnarVariant/VariantDef.h"
#include "ColumnarVariant/VariantLinkColumn.h"

// Local includes
#include "AssociationUtils/IOverlapTool.h"
#include "AssociationUtils/BaseOverlapTool.h"
#include "AssociationUtils/DeltaRMatcher.h"

namespace ORUtils
{

  /// @class EleMuSharedTrkOverlapTool
  /// @brief Tool for removing overlaps between electrons and muons that
  /// share a track or are DR matched.
  ///
  /// I don't yet know if it's straightforward to generalize to any kind of
  /// particles.
  ///
  /// The procedure works as follows.
  ///   1. Remove muons if they share a track with an electron, they are
  ///      calorimeter-tagged and removeCaloMuons is activated.
  ///   2. Remove electrons if they share a track with a muon or if they
  ///      are DR matched to a muon and useDRMatching is activated.
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class EleMuSharedTrkOverlapTool : public virtual IOverlapTool,
                                    public BaseOverlapTool
  {

      /// Create proper constructor for Athena
      ASG_TOOL_CLASS(EleMuSharedTrkOverlapTool, IOverlapTool)

    public:

      /// Standard constructor
      EleMuSharedTrkOverlapTool(const std::string& name);

      /// @brief Identify overlaps via shared ID track.
      /// Removes the electron from cont1
      virtual StatusCode
      findOverlaps(columnar::Particle1Range cont1,
                   columnar::Particle2Range cont2,
                   columnar::EventContextId eventContext) const override;

      /// Alternate method taking actual container types
      StatusCode
      internalFindOverlaps(columnar::Particle1Range electrons,
                           columnar::Particle2Range muons) const;

    protected:

      /// Initialize the tool
      virtual StatusCode initializeDerived() override;

    private:

      //
      // Configurable properties
      //

      /// Flag to remove calo-muons overlapping with electrons
      bool m_removeCaloMuons;

      /// Flag to remove electrons in a dR cone of muons (default: false)
      bool m_useDRMatching;
      /// Maximum dR between electrons and muons if m_useDRMatching is used
      float m_maxDR;
      /// Calculate delta-R using rapidity
      bool m_useRapidity;

      //
      // Utilities
      //

      /// IDTrack type
      using MyTrackDef = columnar::VariantContainerId<columnar::ContainerId::track0,columnar::ContainerId::track0, columnar::ContainerId::track1>;

      /// Columnar accessors
      struct Accessors final : columnar::ColumnarTool<>
      {
        columnar::Track0Accessor<columnar::ObjectColumn> m_track0Acc {*this, "InDetTrackParticles"};
        columnar::Track1Accessor<columnar::ObjectColumn> m_track1Acc {*this, "InDetForwardTrackParticles"};
        columnar::Track2Accessor<columnar::ObjectColumn> m_track2Acc {*this, "GSFTrackParticles"};
        columnar::Particle1Accessor<std::vector<columnar::OptTrack2Id>> m_eleTrackAcc {*this, "trackParticleLinks"};
        columnar::Particle2Accessor<columnar::ObjectLink<MyTrackDef>> m_muonTrkAcc {*this, "inDetTrackParticleLink"};
        columnar::Particle2Accessor<columnar::RetypeColumn<xAOD::Muon::MuonType,std::uint16_t>> m_muonTypeAcc {*this, "muonType"};
        columnar::Track2Accessor<columnar::ObjectLink<MyTrackDef>> m_gsfOriginalTrackAcc {*this, "originalTrackParticle"};
        using ColumnarTool::ColumnarTool;
      };
      std::unique_ptr<Accessors> m_accessors {std::make_unique<Accessors> (this)};

      [[nodiscard]] columnar::ObjectLink<MyTrackDef> getOriginalTrackParticle(columnar::Particle1Id electron) const;

      /// Delta-R matcher
      std::unique_ptr<DeltaRMatcher> m_dRMatcher;

  }; // class EleMuSharedTrkOverlapTool

} // namespace ORUtils

#endif
