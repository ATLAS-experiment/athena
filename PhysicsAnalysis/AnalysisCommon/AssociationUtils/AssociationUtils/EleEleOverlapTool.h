/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_ELEELEOVERLAPTOOL_H
#define ASSOCIATIONUTILS_ELEELEOVERLAPTOOL_H

// EDM includes
#include "xAODEgamma/ElectronContainer.h"
#include "ColumnarEgamma/EgammaDef.h"

// Columnar includes
#include "ColumnarCore/ColumnAccessor.h"
#include "ColumnarCore/LinkColumn.h"
#include "ColumnarCore/VectorColumn.h"
#include "ColumnarCluster/ClusterHelpers.h"
#include "ColumnarTracking/TrackDef.h"

// Local includes
#include "AssociationUtils/IOverlapTool.h"
#include "AssociationUtils/BaseOverlapTool.h"

namespace ORUtils
{

  /// @class EleEleOverlapTool
  /// @brief A tool implementing the recommended ele-ele overlap removal.
  ///
  /// This tool flags electrons that match to other electrons under two
  /// possible criteria:
  ///   1. The electrons share a track (on by default)
  ///   2. The electron clusters are overlapping (off by default)
  ///
  /// The rejected electron is decided as follows:
  ///   - author 'Ambiguous' rejected from author 'Electron'
  ///   - otherwise, the softer electron is rejected (in PT)
  ///
  /// The implementation here comes from discussions on JIRA issue ATLASG-438.
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class EleEleOverlapTool : public virtual IOverlapTool,
                            public BaseOverlapTool
  {

      /// Create proper constructor for Athena
      ASG_TOOL_CLASS(EleEleOverlapTool, IOverlapTool)

    public:

      /// Standalone constructor
      EleEleOverlapTool(const std::string& name);

      /// @brief Identify overlapping electrons.
      /// Note that in this tool, the two containers should be the same.
      virtual StatusCode
      findOverlaps(columnar::Particle1Range<CMode> cont1,
                   columnar::Particle2Range<CMode> cont2,
                   columnar::EventContextId<CMode> eventContext) const override;

      /// @brief Identify overlapping electrons and jets.
      /// The above method calls this one.
      virtual StatusCode
      internalFindOverlaps(columnar::Particle1Range<CMode> electrons) const;

    protected:

      /// Initialize the tool
      virtual StatusCode initializeDerived() override;

    private:

      /// Helper method for matching electrons
      bool electronsMatch(columnar::Particle1Id<CMode> el1, columnar::Particle1Id<CMode> el2) const;

      /// Helper method to decide which electron to reject
      bool rejectFirst(columnar::Particle1Id<CMode> el1, columnar::Particle1Id<CMode> el2) const;

      /// @name Configurable properties
      /// @{

      /// Match electrons by shared track (on by default)
      bool m_useTrackMatch;

      /// Match electrons by cluster distance (off by default)
      bool m_useClusterMatch;

      /// Cluster-matching dEta
      double m_clusterDeltaEta;

      /// Cluster-matching dPhi
      double m_clusterDeltaPhi;

      /// @}

      /// Columnar accessors
      using MyTrackDef = columnar::VariantContainerId<columnar::Track0Def,columnar::Track0Def>;
      struct Accessors final : columnar::ColumnarTool<CMode>
      {
        columnar::ClusterAccessor<columnar::ObjectColumn,CMode> m_clusterContainerAcc;
        columnar::Track0Accessor<columnar::ObjectColumn,CMode> m_track0Acc;
        columnar::Particle1Accessor<float,CMode> m_ptAcc {*this, "pt"};
        columnar::Particle1Accessor<std::uint16_t,CMode> m_authorAcc {*this, "author"};
        columnar::Particle1Accessor<std::vector<columnar::OptClusterId<CMode>>,CMode> m_caloClusterAcc;
        columnar::Particle1Accessor<std::vector<columnar::OptTrackId<CMode>>,CMode> m_trackAcc;
        std::optional<columnar::ClusterHelpers::EtaBEAccessor<CMode>> m_etaBEAcc;
        std::optional<columnar::ClusterHelpers::PhiBEAccessor<CMode>> m_phiBEAcc;
        using ColumnarTool::ColumnarTool;
      };
      std::unique_ptr<Accessors> m_accessors {std::make_unique<Accessors> (this)};

  }; // class EleEleOverlapTool

} // namespace ORUtils

#endif
