//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//


#ifndef CALORECGPU_TOPOAUTOMATONCLUSTERING_H
#define CALORECGPU_TOPOAUTOMATONCLUSTERING_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "CxxUtils/checker_macros.h"

#include "CaloRecGPU/CaloClusterGPUProcessor.h"
#include "CaloRecGPU/CaloGPUTimed.h"
#include "TopoAutomatonClusteringImpl.h"
#include <string>
#include <mutex>

#include "CLHEP/Units/SystemOfUnits.h"

#include "CaloRecGPU/IGPUKernelSizeOptimizerSvc.h"
#include "CaloRecGPU/CaloGPUCUDAInitialization.h"

#include "GaudiKernel/ServiceHandle.h"

/**
 * @class TopoAutomatonClustering
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 31 May 2022
 * @brief Topological cluster maker algorithm to be run on GPUs.
 */
 

class TopoAutomatonClustering :
  public extends<AthAlgTool, CaloClusterGPUProcessor>, public CaloGPUTimed, public CaloGPUCUDAInitialization
{
 public:

  TopoAutomatonClustering(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode initialize() override
  {
    return CaloGPUCUDAInitialization::initialize();
  }
  
  virtual StatusCode initialize_non_CUDA() override;
  
  virtual StatusCode initialize_CUDA() override;
  
  virtual StatusCode execute (const EventContext & ctx,
                              const CaloRecGPU::ConstantDataHolder & constant_data,
                              CaloRecGPU::EventDataHolder & event_data,
                              void * temporary_buffer) const override;

  virtual StatusCode finalize() override;
  
  virtual ~TopoAutomatonClustering() = default;
  
 private:

  /** 
   * @brief vector of names of the calorimeters to consider.
   *
   * The default is to use all calorimeters (i.e. LAREM, LARHEC,
   * LARFCAL, TILE).  Cells which belong to one of the input cell
   * containers and to one of the calorimeters in this vector are used
   * as input for the cluster maker.  This property is used in order
   * to ignore a certain subsystem (e.g. for LAREM only clusters
   * specify only LAREM in the jobOptions).  */
  Gaudi::Property<std::vector<std::string>>  m_caloNames {this, "CalorimeterNames", {}, "Name(s) of Calorimeters to use for clustering"};  

  /**
   * @brief vector of names of the calorimeter samplings to consider
   * for seeds.
   *
   * The default is to use all calorimeter samplings. Excluding a
   * sampling from this vector prevents the definition of a seed cell
   * in this sampling. Cells in those samplings are still used and
   * incorporated in the topo clusters (both on the neighbor and the
   * cell level) they can therefore even expand a cluster but not seed
   * one ...*/
  Gaudi::Property<std::vector<std::string>>  m_samplingNames {this, "SeedSamplingNames", {}, "Name(s) of Calorimeter Samplings to consider for seeds"};

  /**
   * @brief all cells have to satisfy \f$|E| > N_{\rm cell}\,\sigma\f$
   *
   * This cut determines how much the cluster will extend beyond the
   * last cell passing the neighbor threshold. The smaller this cut is
   * the more cells will be allowed in the tail of the cluster. It
   * should be smaller or equal to the neighbor threshold. If a cell
   * passing this cut is neighbor of two or more cells passing the
   * neighbor cut it will be inserted in the cluster which has the
   * neighbor cell that was asked first for its neighbors. Since the
   * original list of seed cells is ordered in descending order of
   * \f$E/\sigma\f$ (or \f$|E|/\sigma\f$) the distance of the cell (in
   * number of cell generations passing the neighbor cut until this
   * cell will be reached) usually determines in which cluster the
   * cell will end up in. The cell cut should be lower or equal to the
   * neighbor cut.  */
  Gaudi::Property<float> m_cellThresholdOnEorAbsEinSigma {this, "CellThresholdOnEorAbsEinSigma", 0., "Cell (terminal) threshold (in units of noise Sigma)"};

  /**
   * @brief cells with \f$|E| > N_{\rm neighbor}\,\sigma\f$ extend the cluster
   *
   * This cut determines how many cells are asked for their neighbors
   * to expand the cluster. The smaller this cut is the more cells
   * will be asked for their neighbors. If a cell passing this cut is
   * neighbor of two other cells passing this cut from different
   * clusters, the two clusters are merged. The neighbor cut should be
   * lower or equal to the seed cut.  */
  Gaudi::Property<float> m_neighborThresholdOnEorAbsEinSigma {this, "NeighborThresholdOnEorAbsEinSigma", 2., "Neighbor (grow) threshold (in units of noise Sigma)"};

  /**
   * @brief cells with \f$|E| > N_{\rm seed}\,\sigma\f$ start a cluster
   *
   * This cut determines how many clusters are formed initially.  The
   * smaller this cut is the more clusters will be created.  During
   * the accumulation of cells inside the clusters it can happen that
   * clusters are merged if a cell passing the neighbor threshold
   * would be included in both clusters.  */
  Gaudi::Property<float> m_seedThresholdOnEorAbsEinSigma {this, "SeedThresholdOnEorAbsEinSigma", 4., "Seed threshold (in units of noise Sigma)"};


  /**
   * @brief if set to true seed cuts are on \f$|E|\f$ and \f$|E|_\perp\f$.
   *
   * The seed cuts and the \f$E_\perp\f$ cut on the final clusters
   * before insertion to the CaloClusterContainer will be on absolute
   * energy and absolute transverse energy if this is set to true. If
   * set to false the cuts will be on energy and transverse energy
   * instead.  */
  Gaudi::Property<bool> m_seedCutsInAbsE {this, "SeedCutsInAbsE", true, "Seed cuts in Abs E instead of E"};

  /**
   * @brief if set to true neighbor cuts are on \f$|E|\f$ and \f$|E|_\perp\f$.
   *
   * The neighbor cuts will be on absolute energy and absolute
   * transverse energy if this is set to true. If set to false the
   * cuts will be on energy and transverse energy instead.  */
  Gaudi::Property<bool> m_neighborCutsInAbsE {this, "NeighborCutsInAbsE", true, "Neighbor (grow) cuts in Abs E instead of E"};

  /**
   * @brief if set to true cell cuts are on \f$|E|\f$ and \f$|E|_\perp\f$.
   *
   * The cell cuts will be on absolute energy and absolute transverse
   * energy if this is set to true. If set to false the cuts will be
   * on energy and transverse energy instead.  */
  Gaudi::Property<bool> m_cellCutsInAbsE {this, "CellCutsInAbsE", true, "Cell (terminal) cuts in Abs E instead of E"};

  /**
   * if set to true, time cut is applied to seed cells, no cut otherwise
   */
  Gaudi::Property<bool> m_cutCellsInTime {this, "SeedCutsInT", false, "Do seed cuts in time"};

  /**
   * threshold used for timing cut on seed cells. Implemented as |seed_cell_time|<m_timeThreshold. No such cut on neighbouring cells.*/
  Gaudi::Property<float> m_timeThreshold {this, "SeedThresholdOnTAbs", 12.5 * CLHEP::ns, "Time thresholds (in abs. val.)"};

  /**
   * upper limit on the energy significance, for applying the cell time cut */
  Gaudi::Property<float> m_thresholdForKeeping {this, "TimeCutUpperLimit", 20., "Significance upper limit for applying time cut"};


  /**
   * @brief if set to true treat cells with a dead OTX which can be
   * predicted by L1 trigger info as good instead of bad cells */
  Gaudi::Property<bool> m_treatL1PredictedCellsAsGood {this, "TreatL1PredictedCellsAsGood", true, "Treat bad cells with dead OTX if predicted from L1 as good"};

  /**
   * @brief if set to true, seed cells failing the time cut are also excluded from cluster at all
   */
  Gaudi::Property<bool> m_excludeCutSeedsFromClustering {this, "CutOOTseed", true, "Exclude out-of-time seeds from neighbouring and cell stage"};

  /**
   * @brief if set to true, the time cut is not applied on cell of large significance
   */
  Gaudi::Property<bool> m_keepSignificantCells {this, "UseTimeCutUpperLimit", false, "Do not apply time cut on cells of large significance"};

  /**
   * @brief if set to true use 2-gaussian noise description for
   * TileCal
   *
   * @warning Currently unsupported on the GPU side!
   */
  Gaudi::Property<bool> m_twoGaussianNoise{this, "TwoGaussianNoise", false, "Use 2-gaussian noise description for TileCal"};


  /**
   * @brief type of neighbor relations to use.
   *
   * The CaloIdentifier package defines different types of neighbors
   * for the calorimeter cells. Currently supported neighbor relations
   * for topological clustering are:
   *
   * @li "all2D" for all cells in the same layer (sampling or module)
   *      of one calorimeter subsystem. Note that endcap and barrel
   *      will be unconnected in this case even for the LAREM.
   *
   * @li "all3D" for all cells in the same calorimeter. This means all
   *      the "all2D" neighbors for each cell plus the cells in
   *      adjacent samplings overlapping at least partially in
   *      \f$\eta\f$ and \f$\phi\f$ with the cell. Note that endcap
   *      and barrel will be connected in this case for the LAREM.
   *
   * @li "super3D" for all cells. This means all the "all3D" neighbors
   *      for each cell plus the cells in adjacent samplings from
   *      other subsystems overlapping at least partially in
   *      \f$\eta\f$ and \f$\phi\f$ with the cell. All calorimeters
   *      are connected in this case.
   *
   * The default setting is "super3D".  */
  Gaudi::Property<std::string> m_neighborOptionString {this, "NeighborOption", "super3D",
                                                       "Neighbor option to be used for cell neighborhood relations"};

  /**
   * @brief if set to true limit the neighbors in HEC IW and FCal2&3.
   *
   * The cells in HEC IW and FCal2&3 get very large in terms of eta
   * and phi.  Since this might pose problems on certain jet
   * algorithms one might need to avoid expansion in eta and phi for
   * those cells. If this property is set to true the 2d neighbors of
   * these cells are not used - only the next sampling neighbors are
   * probed. */
  Gaudi::Property<bool> m_restrictHECIWandFCalNeighbors {this, "RestrictHECIWandFCalNeighbors",
                                                         false, "Limit the neighbors in HEC IW and FCal2&3"};

  /**
   * @brief if set to true limit the neighbors in presampler Barrel and Endcap.
   *
   * The presampler cells add a lot of PileUp in the Hilum
   * samples. With this option set to true the presampler cells do not
   * expand the cluster in the presampler layer.  Only the next
   * sampling is used as valid neighbor source. */
  Gaudi::Property<bool> m_restrictPSNeighbors {this, "RestrictPSNeighbors",
                                               false, "Limit the neighbors in presampler Barrel and Endcap"};

  
  /**
   * @brief If set to true, the time window is softened 
            in the EMB2 and EME2_OW due to crosstalk
            from direct neighbour cells in phi. */
  Gaudi::Property<bool> m_xtalkEM2 {this, "XTalkEM2",
                                    false, "Relax time window (if timing is used) in EM2 when xTalk is present"};

  /**
   * @brief Additional maximum delta t added to the upper limit time window
            in case crosstalk in EM2 should be accounted for.  */
  Gaudi::Property<float> m_xtalkDeltaT {this, "XTalkDeltaT",
                                        15 * CLHEP::ns, "Delta T to add to upper time threshold for EM2 cells affected by xtalk."};


  /** @brief Options for the algorithm, held in a GPU-friendly way.
  */
  TAGrowing::TACOptionsHolder m_options;
  
  /** @brief Handle to the CUDA kernel block and grid size optimization service. */
  ServiceHandle<IGPUKernelSizeOptimizerSvc> m_kernelSizeOptimizer { this, "KernelSizeOptimizer", "GPUKernelSizeOptimizerSvc", "CUDA kernel size optimization service." };

};

#endif //CALORECGPU_TOPOAUTOMATONCLUSTERING_H
