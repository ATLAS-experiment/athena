/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Max Goblirsch



#ifndef ASG_ANALYSIS_ALGORITHMS__MUON_LRT_MERGING_ALGORITHM__H
#define ASG_ANALYSIS_ALGORITHMS__MUON_LRT_MERGING_ALGORITHM__H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODMuon/MuonAuxContainer.h>
#include <MuonAnalysisInterfaces/IMuonLRTOverlapRemovalTool.h>
#include <AsgTools/ToolHandle.h>

#include <AsgTools/PropertyWrapper.h>
#include <AsgDataHandles/WriteHandleKey.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteHandle.h>
#include <AsgDataHandles/ReadHandle.h>

namespace CP
{
  /// \brief this wraps the MCP LRT collection merger in a CP algorithm

  class MuonLRTMergingAlg final : public EL::AnaReentrantAlgorithm
  {
    /// \brief the standard constructor
  public:
    MuonLRTMergingAlg (const std::string& name,
                             ISvcLocator* pSvcLocator);
    StatusCode initialize () override;
    StatusCode execute (const EventContext &ctx) const  override;

  private:

      ///////////////////////////////////////////////////////////////////
      /** @brief Private data:                                       */
      ///////////////////////////////////////////////////////////////////

      /// handles for interacting with the event storage

      SG::ReadHandleKey<xAOD::MuonContainer>      m_promptMuonLocation{this, "PromptMuonLocation","Muons", "Prompt muons to merge"}; /** Vector of muon collections to be merged. */
      SG::ReadHandleKey<xAOD::MuonContainer>      m_lrtMuonLocation{this, "LRTMuonLocation","MuonsLRT", "LRT muons to merge"}; /** Vector of muon collections to be merged. */
      SG::WriteHandleKey<xAOD::MuonContainer>     m_outMuonLocation{this, "OutputMuonLocation", "StdWithLRTMuons", "name of the muon container to write"};   /** Combined muon collection.   */

      /// flag to create a view collection rather than building deep-copies (true by default)
      Gaudi::Property<bool>  m_createViewCollection{this, "CreateViewCollection", true};     //!< option to create a view collection and not deep-copy muons
      ToolHandle<CP::IMuonLRTOverlapRemovalTool>  m_overlapRemovalTool{this, "overlapRemovalTool", "", "tool to determine overlaps between regular and LRT muons"};

      /// allows to pass an overlap removal strategy to the underlying removal tool, without manually configuring said tool.
      /// Advantageous in certain analysis frameworks.
      Gaudi::Property<int>  m_ORstrategy{this, "overlapStrategy", CP::IMuonLRTOverlapRemovalTool::defaultStrategy, "Overlap removal strategy to use (0 = default, 1 = write all muons and decorate with overlap type)"};
      Gaudi::Property<bool>  m_useRun3WP{this, "UseRun3WP", false, "Switch to toggle the run 2 & run 3 geometry for the muon ID working points used in the prompt-LRT overlap resolution. Set to true for run 3 and false for run 2."};

      ///////////////////////////////////////////////////////////////////
      /** @brief Private methods:                                    */
      ///////////////////////////////////////////////////////////////////

      /** @brief A routine that merges the muon collections into a view container. */
      StatusCode mergeMuon(const xAOD::MuonContainer & muonCol,
                           const std::vector<bool> & muonIsGood,
                           ConstDataVector<xAOD::MuonContainer>* outputCol) const;

      /** @brief A routine that merges the muon collections. */
      StatusCode mergeMuon(const xAOD::MuonContainer & muonCol,
                           const std::vector<bool> & muonIsGood,
                           xAOD::MuonContainer* outputCol) const;


  };
}

#endif
