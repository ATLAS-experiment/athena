//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_CaloGPUClusterAndCellDataMonitor_H
#define CALORECGPU_CaloGPUClusterAndCellDataMonitor_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloRecGPU/CaloClusterGPUTransformers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/Helpers.h"
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <mutex>
#include <tuple>

#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

#include "CaloGPUClusterAndCellDataMonitorOptions.h"

class CaloCell_ID;

/**
 * @class CaloGPUClusterAndCellDataMonitor
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 18 March 2023
 * @brief Places (matched) cluster and cell properties in monitored variables
 *        to enable plotting with the Athena THistSvc instead
 *        of the custom solution that was being used previously.
 *        Its histograms can be configured as in a MonitoringTool.
 */

class CaloGPUClusterAndCellDataMonitor :
  public AthAlgTool, virtual public ICaloClusterGPUPlotter
{
 public:

  CaloGPUClusterAndCellDataMonitor(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode initialize() override;

  virtual ~CaloGPUClusterAndCellDataMonitor() = default;


  virtual StatusCode update_plots_start(const EventContext & ctx,
                                        const CaloRecGPU::ConstantDataHolder & constant_data,
                                        const xAOD::CaloClusterContainer * cluster_collection_ptr) const override;

  virtual StatusCode update_plots_end(const EventContext & ctx,
                                      const CaloRecGPU::ConstantDataHolder & constant_data,
                                      const xAOD::CaloClusterContainer * cluster_collection_ptr) const override;

  virtual StatusCode update_plots(const EventContext & ctx,
                                  const CaloRecGPU::ConstantDataHolder & constant_data,
                                  const xAOD::CaloClusterContainer * cluster_collection_ptr,
                                  const CaloClusterCollectionProcessor * tool) const override;

  virtual StatusCode update_plots(const EventContext & ctx,
                                  const CaloRecGPU::ConstantDataHolder & constant_data,
                                  const xAOD::CaloClusterContainer * cluster_collection_ptr,
                                  const CaloRecGPU::EventDataHolder & event_data,
                                  const ICaloClusterGPUInputTransformer * tool) const override;

  virtual StatusCode update_plots(const EventContext & ctx,
                                  const CaloRecGPU::ConstantDataHolder & constant_data,
                                  const xAOD::CaloClusterContainer * cluster_collection_ptr,
                                  const CaloRecGPU::EventDataHolder & event_data,
                                  const CaloClusterGPUProcessor * tool) const override;

  virtual StatusCode update_plots(const EventContext & ctx,
                                  const CaloRecGPU::ConstantDataHolder & constant_data,
                                  const xAOD::CaloClusterContainer * cluster_collection_ptr,
                                  const CaloRecGPU::EventDataHolder & event_data,
                                  const ICaloClusterGPUOutputTransformer * tool) const override;

  virtual StatusCode finalize_plots() const override;

 private:

  StatusCode initialize_plotted_variables();

  StatusCode add_data(const EventContext & ctx,
                      const CaloRecGPU::ConstantDataHolder & constant_data,
                      const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> & cell_info,
                      const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> & cell_state,
                      const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> & clusters,
                      const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterMomentsArr> & moments,
                      const std::string & tool_name) const;


  StatusCode add_combination(const EventContext & ctx,
                             const CaloRecGPU::ConstantDataHolder & constant_data,
                             const int index_1,
                             const int index_2,
                             const std::string & prefix,
                             const bool match_in_energy,
                             const bool match_without_shared,
                             const bool match_perfectly) const;

  /*! Returns @p true if this tool should be plotted for.
  */
  bool filter_tool_by_name(const std::string & tool_name) const;

  StatusCode convert_to_GPU_data_structures(const EventContext & ctx,
                                            const CaloRecGPU::ConstantDataHolder & constant_data,
                                            const xAOD::CaloClusterContainer * cluster_collection_ptr,
                                            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> & cell_info,
                                            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> & cell_state,
                                            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> & clusters,
                                            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterMomentsArr> & moments) const;

  ///Remove invalid clusters, reorder by ET and update the tags accordingly.
  StatusCode compactify_clusters(const EventContext & ctx,
                                 const CaloRecGPU::ConstantDataHolder & constant_data,
                                 const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> & cell_info,
                                 CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> & cell_state,
                                 CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> & clusters,
                                 CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterMomentsArr> & moments) const;


  struct sample_comparisons_holder
  {
    std::vector<int> r2t_table, t2r_table;
    std::vector<int> unmatched_ref_list, unmatched_test_list;

    ///Converts a cluster index from the reference tool (first) to the test tool (second).
    ///Returns `-1` in case the cluster has not been matched.
    int r2t(const int i) const
    {
      return r2t_table[i];
    }
    ///Converts a cluster index from the test tool (second) to the reference tool (first).
    ///Returns `-1` in case the cluster has not been matched.
    int t2r(const int i) const
    {
      return t2r_table[i];
    }
    ///Returns the number of unmatched clusters in the reference (first) tool.
    int ref_unmatched() const
    {
      return unmatched_ref_list.size();
    }
    ///Returns the number of unmatched clusters in the test (second) tool.
    int test_unmatched() const
    {
      return unmatched_test_list.size();
    }
  };

  StatusCode match_clusters(sample_comparisons_holder & sch,
                            const CaloRecGPU::ConstantDataHolder & constant_data,
                            const CaloRecGPU::CellInfoArr & cell_info,
                            const CaloRecGPU::CellStateArr & cell_state_1,
                            const CaloRecGPU::CellStateArr & cell_state_2,
                            const CaloRecGPU::ClusterInfoArr & cluster_info_1,
                            const CaloRecGPU::ClusterInfoArr & cluster_info_2,
                            const CaloRecGPU::ClusterMomentsArr & /*moments_1*/,
                            const CaloRecGPU::ClusterMomentsArr & /*moments_2*/,
                            const bool match_in_energy,
                            const bool match_without_shared) const;

  StatusCode match_clusters_perfectly(sample_comparisons_holder & sch,
                                      const CaloRecGPU::ConstantDataHolder & constant_data,
                                      const CaloRecGPU::CellInfoArr & cell_info,
                                      const CaloRecGPU::CellStateArr & cell_state_1,
                                      const CaloRecGPU::CellStateArr & cell_state_2,
                                      const CaloRecGPU::ClusterInfoArr & cluster_info_1,
                                      const CaloRecGPU::ClusterInfoArr & cluster_info_2,
                                      const CaloRecGPU::ClusterMomentsArr & /*moments_1*/,
                                      const CaloRecGPU::ClusterMomentsArr & /*moments_2*/,
                                      const bool match_without_shared) const;
                                    
  //--------------------------------------------------
  //
  //             GENERAL OPTIONS/SETTINGS
  //
  //--------------------------------------------------

  /**
   * @brief Cell (terminal) threshold to use for cluster matching.
   */
  Gaudi::Property<float> m_termThreshold {this, "CellThreshold", 0., "Cell (terminal) threshold (in units of noise Sigma)"};

  /**
   * @brief Neighbor (growing) threshold to use for cluster matching.
   */
  Gaudi::Property<float> m_growThreshold {this, "NeighborThreshold", 2., "Neighbor (grow) threshold (in units of noise Sigma)"};

  /**
   * @brief Seed threshold to use for cluster matching.
   */
  Gaudi::Property<float> m_seedThreshold {this, "SeedThreshold", 4., "Seed threshold (in units of noise Sigma)"};

  /**
  * @brief vector of names of the cell containers to use as input.
  */
  SG::ReadHandleKey<CaloCellContainer> m_cellsKey {this, "CellsName", "", "Name(s) of Cell Containers"};

  /** @brief Monitoring tool.
    */
  ToolHandle< GenericMonitoringTool > m_moniTool { this, "MonitoringTool", "", "Monitoring tool" };

  //--------------------------------------------------
  //
  //                    PLOT OPTIONS
  //
  //--------------------------------------------------

  /** @brief Tools to plot individually.

      @warning If a tool appears more than once with different identifiers, the last one is used.
  */
  Gaudi::Property<std::vector<SimpleSingleTool>> m_toolsToPlot
  {this, "ToolsToPlot", {}, "Tools to be plotted individually"};

  /** @brief Pairs of tools to compare.
  */
  Gaudi::Property< std::vector<SimpleToolPair> > m_pairsToPlot
  {this, "PairsToPlot", {}, "Pairs of tools to be compared and plotted"};

  /** @brief Option for adjusting the parameters for the cluster matching algorithm.
  */
  Gaudi::Property<MatchingOptions> m_matchingOptions
  {this, "ClusterMatchingParameters", {}, "Parameters for the cluster matching algorithm"};

  //--------------------------------------------------
  //
  //              OTHER MEMBER VARIABLES
  //
  //--------------------------------------------------

  /**
   * @brief Parameters for the cluster matching algorithm, for easier access.
   */
  double m_min_similarity = 0.5, m_seed_weight = 5000., m_grow_weight = 250., m_terminal_weight = 10.;

  /**
   * @brief Pointer to Calo ID Helper
   */
  const CaloCell_ID * m_calo_id {nullptr};

  /** @brief Map of the strings corresponding to all the tools
             that will be relevant for plotting (individually or in comparisons)
             to the index that will be used to identify the tool within the plotter.
             (Indices of -1 signal tools that are only plotted individually,
              no need to keep them.)
  */
  std::map<std::string, int> m_toolsToCheckFor;

  ///@brief Maps tools to their respective identifying prefix for variables.
  std::map<std::string, std::string> m_toolToIdMap;

  ///@brief The number of tools that will actually need to be kept in memory for combined plotting.
  int m_numToolsToKeep = 0;

  struct pair_to_plot
  {
    int index_ref = -1, index_test = -1;
    std::string prefix;
    bool match_in_energy = false;
    bool match_without_shared = false;
    bool match_perfectly = false;
  };

  std::vector<pair_to_plot> m_toolCombinations;

  ///@brief Counts the total number of clusters per tool.
  mutable std::map<std::string, std::atomic<size_t>> m_numClustersPerTool ATLAS_THREAD_SAFE;

  ///@brief Counts the number of events.
  size_t m_numEvents = 0;

  struct per_tool_storage
  {
    CaloRecGPU::CellInfoArr cell_info;
    CaloRecGPU::CellStateArr cell_state;
    CaloRecGPU::ClusterInfoArr clusters;
    CaloRecGPU::ClusterMomentsArr moments;
  };

  /** @brief Stores the intermediate results needed for tool-level matching.
  */
  mutable CaloRecGPU::Helpers::separate_thread_holder<std::vector<per_tool_storage>> m_storageHolder ATLAS_THREAD_SAFE;

  /** @brief Control which properties will actually be calculated and stored.
             In principle, should be automagically filled based on the booked histograms.
  */
  std::vector<bool> m_clusterPropertiesToDo, m_comparedClusterPropertiesToDo,
      m_extraComparedClusterPropertiesToDo, m_cellPropertiesToDo,
      m_comparedCellPropertiesToDo, m_cellTypesToDo,
      m_comparedCellTypesToDo, m_extraThingsToDo;

  /** @brief If no properties are asked for, skip the relevant loops entirely...
  */
  bool m_doCells = false, m_doClusters = false, m_doCombinedCells = false, m_doCombinedClusters = false;
  
  
  /** @brief A flag to signal that the variables to be monitored have been detected
             based on the booked histograms.
   */
  mutable std::atomic<bool> m_plottedVariablesInitialized;


  /** @brief This mutex is locked to ensure only one thread detects the monotired variables..
    */
  mutable std::mutex m_mutex;

};

#endif //CALORECGPU_CaloGPUClusterAndCellDataMonitor_H