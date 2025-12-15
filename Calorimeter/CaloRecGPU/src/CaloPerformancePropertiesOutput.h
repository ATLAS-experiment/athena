//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_CALOPERFORMANCEPROPERTIESOUTPUT_H
#define CALORECGPU_CALOPERFORMANCEPROPERTIESOUTPUT_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloConditions/CaloNoise.h"

#include <string>
#include <mutex>
#include <vector>

class CaloCell_ID;

/**
 * @class CaloPerformancePropertiesOutput
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 17 September 2025
 * @brief Tool that outputs certain cell and cluster properties
 * with implications for performance to a text file
 */

class CaloPerformancePropertiesOutput :
  public extends<AthAlgTool, CaloClusterCollectionProcessor>
{
 public:

  CaloPerformancePropertiesOutput(const std::string & type, const std::string & name, const IInterface * parent);
  
  using CaloClusterCollectionProcessor::execute;
  
  virtual StatusCode initialize() override;
  
  virtual StatusCode execute (const EventContext& ctx, xAOD::CaloClusterContainer* cluster_collection) const override;

  virtual StatusCode finalize() override;
  
  virtual ~CaloPerformancePropertiesOutput() = default;

 private:

  /**
   * @brief The path specifying the folder to which the files should be saved.
     Default `"event_properties.txt"`
   */
  Gaudi::Property<std::string> m_fileName{this, "FileName", "event_properties.txt", "File to save the performance-related info."};

    /**
   * @brief vector of names of the cell containers to use as input.
   */
  SG::ReadHandleKey<CaloCellContainer> m_cellsKey {this, "CellsName", "", "Name(s) of Cell Containers"};
  
  /** @brief Key of the CaloNoise Conditions data object. Typical values
      are '"electronicNoise', 'pileupNoise', or '"totalNoise' (default)
      */
  SG::ReadCondHandleKey<CaloNoise> m_noiseCDOKey{this, "CaloNoiseKey", "totalNoise", "SG Key of CaloNoise data object"};
  
  /**
   * @brief if set to true use 2-gaussian noise description for TileCal
   */
  Gaudi::Property<bool> m_twoGaussianNoise{this, "TwoGaussianNoise", false, "Use 2-gaussian noise description for TileCal"};


  /** @brief Value to consider for the seed threshold. Should be consistent with the
   *  one used in Topological Clustering to ensure cell classification is correct.
   */
  Gaudi::Property<float> m_seedThreshold {this, "SeedThresholdOnEorAbsEinSigma", 4., "Seed threshold (in units of noise Sigma)"};
  
  /** @brief Value to consider for the seed threshold. Should be consistent with the
   *  one used in Topological Clustering to ensure cell classification is correct.
   */
  Gaudi::Property<float> m_growThreshold {this, "NeighborThresholdOnEorAbsEinSigma", 2., "Neighbor (grow) threshold (in units of noise Sigma)"};
  
  /** @brief Value to consider for the seed threshold. Should be consistent with the
   *  one used in Topological Clustering to ensure cell classification is correct.
   */
  Gaudi::Property<float> m_cellThreshold {this, "CellThresholdOnEorAbsEinSigma", 0., "Cell (terminal) threshold (in units of noise Sigma)"};
  
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
   * @brief type of neighbor relations to use for cluster growing.
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
  Gaudi::Property<std::string> m_growNeighborOptionString {this, "GrowingNeighborOption", "super3D",
                                                           "Neighbor option to be used for cell neighborhood relations during growing"};
  LArNeighbours::neighbourOption m_growNeighborOption;
  
  /**
   * @brief type of neighbor relations to use for cluster splitting.
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
  Gaudi::Property<std::string> m_splitNeighborOptionString {this, "SplittingNeighborOption", "super3D",
    "Neighbor option to be used for cell neighborhood relations during splitting"};
  LArNeighbours::neighbourOption m_splitNeighborOption;

  /**
   * @brief if set to true limit the neighbors in HEC IW and FCal2&3 during growing.
   *
   * The cells in HEC IW and FCal2&3 get very large in terms of eta
   * and phi.  Since this might pose problems on certain jet
   * algorithms one might need to avoid expansion in eta and phi for
   * those cells. If this property is set to true the 2d neighbors of
   * these cells are not used - only the next sampling neighbors are
   * probed. */
  Gaudi::Property<bool> m_growRestrictHECIWandFCalNeighbors {this, "GrowingRestrictHECIWandFCalNeighbors",
    false, "Limit the neighbors in HEC IW and FCal2&3 for growing"};
    
  /**
   * @brief if set to true limit the neighbors in HEC IW and FCal2&3 during splitting.
   *
   * The cells in HEC IW and FCal2&3 get very large in terms of eta
   * and phi.  Since this might pose problems on certain jet
   * algorithms one might need to avoid expansion in eta and phi for
   * those cells. If this property is set to true the 2d neighbors of
   * these cells are not used - only the next sampling neighbors are
   * probed. */
  Gaudi::Property<bool> m_splitRestrictHECIWandFCalNeighbors {this, "SplittingRestrictHECIWandFCalNeighbors",
    false, "Limit the neighbors in HEC IW and FCal2&3 for splitting"};

  /**
   * @brief if set to true limit the neighbors in presampler Barrel and Endcap during growing.
   *
   * The presampler cells add a lot of PileUp in the Hilum
   * samples. With this option set to true the presampler cells do not
   * expand the cluster in the presampler layer.  Only the next
   * sampling is used as valid neighbor source. */
  Gaudi::Property<bool> m_growRestrictPSNeighbors {this, "GrowingRestrictPSNeighbors",
    false, "Limit the neighbors in presampler Barrel and Endcap for growing"};
    
  /**
   * @brief if set to true limit the neighbors in presampler Barrel and Endcap during splitting.
   *
   * The presampler cells add a lot of PileUp in the Hilum
   * samples. With this option set to true the presampler cells do not
   * expand the cluster in the presampler layer.  Only the next
   * sampling is used as valid neighbor source. */
  Gaudi::Property<bool> m_splitRestrictPSNeighbors {this, "SplittingRestrictPSNeighbors",
    false, "Limit the neighbors in presampler Barrel and Endcap for splitting"};
    
    
  /**
   * @brief Pointer to Calo ID Helper
   */
  const CaloCell_ID* m_calo_id {nullptr};
  
  
  struct EventPerformanceInfo
  {
    int num_clusters = 0;
    
    int total_seed = 0;
    int total_grow = 0;
    int total_term = 0;
    int total_invalid = 0;
    
    int seed_in_cluster = 0;
    int grow_in_cluster = 0;
    int term_in_cluster = 0;
        
    struct Stats
    {      
      double min = 0., max = 0., avg = 0., stddev = 0.;
    };
    
    Stats cluster_size;
    Stats cluster_num_seed;
    Stats cluster_num_grow;
    Stats cluster_num_term;
    
    Stats cluster_seed_min_radius;
    Stats cluster_seed_max_radius;
    Stats cluster_first_max_radius;
  };
  
  
  /** @brief Mutex that is locked when recording info.
   */
  mutable std::mutex m_mutex;
  
  /** @brief Vector to hold the information.
   */
  mutable std::vector<EventPerformanceInfo> m_eventInfo ATLAS_THREAD_SAFE;
  //Mutexes should ensure no problems with thread safety.

  /** @brief Vector to hold the event numbers to be recorded if necessary.
   */
  mutable std::vector<size_t> m_eventNumbers ATLAS_THREAD_SAFE;
  //Mutexes should ensure no problems with thread safety.

};

#endif //CALORECGPU_CALOPERFORMANCEPROPERTIESOUTPUT_H
