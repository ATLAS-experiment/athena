/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   JetGhostMergingAlg
//
//   Ghost merger algorithm merges multiple collections of ghost
//   containers into one. This is useful for combining standard and
//   LRT ghost tracks into a single collection to pass to
//   downstream taggers.
///////////////////////////////////////////////////////////////////

/// @author Jackson Burzynski

#ifndef JET_ANALYSIS_ALGORITHMS__JET_GHOST_MERGING_ALG_H
#define JET_ANALYSIS_ALGORITHMS__JET_GHOST_MERGING_ALG_H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <xAODJet/JetContainer.h>

#include <AsgTools/PropertyWrapper.h>
#include <AsgDataHandles/WriteHandle.h>
#include <AsgDataHandles/WriteHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>

#include <string>
#include <vector>

namespace CP
{
  /// \brief an algorithm for combining multiple ghost collections into one
  class JetGhostMergingAlg final : public EL::AnaReentrantAlgorithm
  {
    /// \brief the standard constructor
  public:
    JetGhostMergingAlg (const std::string& name,
                        ISvcLocator* pSvcLocator);

  public:
    StatusCode initialize () override;

  public:
    StatusCode execute (const EventContext &ctx) const override;

  private:
    /// \brief the jet collection we run on
    SG::ReadHandleKey<xAOD::JetContainer> m_jetLocation {
      this, "JetCollection", "AntiKt4EMTopoJets"
    };
    /// \brief the name of the output ghost collection
    SG::WriteDecorHandleKey< xAOD::JetContainer > m_mergedGhostContainer {
      this, "MergedGhostName", "GhostTrackLRTMerged", "name of the output merged ghost container"
    };

  private:
    /// \brief internal vector to hold the ReadDecorHandles for the difference ghosts
    std::vector<SG::ReadDecorHandleKey<xAOD::JetContainer> > m_ghostTrackKeys;

  private:
    Gaudi::Property<std::vector<std::string> > m_inputGhostTrackNames {
      this, "InputGhostTrackNames", {"GhostTrack","GhostTrackLRT"}
    };

  };
}

#endif // JET_ANALYSIS_ALGORITHMS__JET_GHOST_MERGING_ALG_H
