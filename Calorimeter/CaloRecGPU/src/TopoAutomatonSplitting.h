//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_TOPOAUTOMATONSPLITTING_H
#define CALORECGPU_TOPOAUTOMATONSPLITTING_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloRecGPU/CaloClusterGPUProcessor.h"
#include "CaloRecGPU/CaloGPUTimed.h"
#include "CaloRecGPU/CaloGPUCUDAInitialization.h"
#include "TopoAutomatonSplittingImpl.h"
#include <string>

#include "GaudiKernel/ServiceHandle.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include "CaloRecGPU/IGPUKernelSizeOptimizerSvc.h"

/**
 * @class TopoAutomatonSplitting
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 25 August 2022
 * @brief Cluster splitter algorithm to be run on GPUs, using the same cellular automaton-based approach as Topo-Automaton Clustering.
 */


class TopoAutomatonSplitting :
  public AthAlgTool, virtual public CaloClusterGPUProcessor, public CaloGPUTimed, public CaloGPUCUDAInitialization
{
 public:

  TopoAutomatonSplitting(const std::string & type, const std::string & name, const IInterface * parent);

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

  virtual ~TopoAutomatonSplitting() = default;

 private:

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
  Gaudi::Property<std::vector<std::string>>  m_samplingNames {this, "SamplingNames", {}, "Name(s) of Calorimeter Samplings to consider for local maxima"};

  /**
   * @brief vector of names of the secondary calorimeter samplings to
   * consider.
   *
   * Samplings in this list will be considered for local maxima only
   * if no local max in the primary list is overlapping. By default this
   * list is empty  */
  Gaudi::Property<std::vector<std::string>> m_secondarySamplingNames {this, "SecondarySamplingNames", {}, "Name(s) of secondary Calorimeter Samplings to consider for local maxima"};

  /**
   * @brief local maxima need at least this number of neighbors to
   * become seeds
   *
   * each cell above the energy cut having at least this many
   * neighbors in the parent cluster and only neighbors with smaller
   * energy seed a split cluster. */
  Gaudi::Property<int> m_nCells {this, "NumberOfCellsCut", 4, "Local maxima need at least this number of neighbors to become seeds. Must be a non-negative number (obviously)."};

  /**
   * @brief local maxima need at least this energy content
   *
   * potential seed cells have to pass this cut on the energy
   * content. */
  Gaudi::Property<float> m_minEnergy {this, "EnergyCut", 500 * CLHEP::MeV, "Minimal energy for a local max"};


  /**
   * @brief share cells at the border between two local maxima
   *
   * this property needs to be set to true in order to treat cells
   * which would be included in 2 clusters (for more then 2 the 2 with
   * the largest E for the current seed cells are used) as shared
   * cells. Shared cells are first excluded from the clustering and
   * then clustered after all normal cells are clustered. The shared
   * clusters are added to the 2 clusters they neighbor with the
   * weights \f$w_1 = E_1/(E_1+r E_2)\f$ and \f$w_2 = 1-w_1\f$, where
   * \f$E_{1,2}\f$ are the current energies of the 2 neighboring
   * clusters without the shared cells and \f$r=\exp(d_1-d_2)\f$ is
   * the ratio of the expected dependencies on the distances \f$d_i\f$
   * (in units of a typical em shower scale) of each shared cell to
   * the cluster centers. If the property is set to false the border
   * cells are included in the normal clustering and the cluster with
   * the largest E for the current seed cells gets the current border
   * cell. */
  Gaudi::Property<bool> m_shareBorderCells {this, "ShareBorderCells", false, "Whether or not to share cells at the boundary between two clusters"};


  /**
   * @brief typical EM shower scale to use for distance criteria in
   * shared cells
   *
   * a shared cell is included in both clusters neighboring the cell
   * with weights depending on the cluster energies and the distance
   * of the shared cell to the cluster centroids. The distance is
   * measured in units of this property to roughly describe the
   * exponential slope of the energy density distribution for em
   * showers. The exact choice of this property is not critical but
   * should roughly match the Moliere radius in the LArEM since here
   * the sharing of cells has the biggest use case. */
  Gaudi::Property<float> m_emShowerScale {this, "EMShowerScale", 5 * CLHEP::cm, "Typical EM shower distance for which the energy density should drop to 1/e"};

  /**
   * @brief if set to true, splitter only looks at absolute
   * value of Energy in order to identify potential seed cells */
  Gaudi::Property<bool> m_absOpt {this, "WeightingOfNegClusters", false, "Should absolute value be used to identify potential seed cells"};

  /**
   * @brief if set to true treat cells with a dead OTX which can be
   * predicted by L1 trigger info as good instead of bad cells */
  Gaudi::Property<bool> m_treatL1PredictedCellsAsGood {this, "TreatL1PredictedCellsAsGood", true, "Treat bad cells with dead OTX if predicted from L1 as good"};

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

  //WARNING: the CPU version of the algorithm does not seem to have this option. Given the description,
  //         maybe it makes some sense to still allow this here? In our configuration we'll keep it disabled, but...

  /** @brief Options for the algorithm, held in a GPU-friendly way.
  */
  TASplitting::TASOptionsHolder m_options;
  
  /** @brief Handle to the CUDA kernel block and grid size optimization service. */
  ServiceHandle<IGPUKernelSizeOptimizerSvc> m_kernelSizeOptimizer { this, "KernelSizeOptimizer", "GPUKernelSizeOptimizerSvc", "CUDA kernel size optimization service." };
};

#endif //CALORECGPU_TOPOAUTOMATONSPLITTING_H