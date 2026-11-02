//Dear emacs, this is -*-c++-*-
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOTOPOCLUSTERMAKER_H
#define CALOTOPOCLUSTERMAKER_H
/**
 * @class CaloTopoClusterMaker
 * @version \$Id: CaloTopoClusterMaker.h,v 1.24 2009-05-15 12:09:48 artamono Exp $
 * @author Sven Menke <menke@mppmu.mpg.de>
 * @date 23-December-2003
 * @brief topological cluster maker.
 *
 * Make clusters from CaloCells based on topological neighboring and
 * noise motivated thresholds in units of sigma applied to the
 * absolute value of the energy for each cell.  The clusters will have
 * at least one cell with \f$ E > N_{\rm seed}\,\sigma \f$ (or \f$|E|
 * > N_{\rm seed}\,\sigma \f$ if SeedCutsInAbsE is true).  Cells with
 * \f$ |E| > N_{\rm neighbor}\,\sigma \f$ will expand the cluster by
 * means of their neighbors.  All cells in the cluster must fulfill
 * \f$ |E| > N_{\rm cell}\,\sigma \f$.  The actual clustering is
 * performed by expanding around seed cells. The neighboring cells
 * above neighbor threshold (if not used so far) are stored in a list
 * and used in the next expansion step. The algorithm terminates if no
 * more unused neighbor cells are found.  The final clusters are
 * copied to the cluster collection if they pass the \f$E_\perp\f$ (or
 * \f$|E|_\perp\f$ if SeedCutsInAbsE is true) cut.
 *
 * Like all other cluster maker tools this class derives from
 * CaloClusterCollectionProcessor.  */

#include "AthenaBaseComps/AthAlgTool.h"

#include "CaloIdentifier/CaloCell_ID.h"
#include "Identifier/IdContext.h"
#include "Identifier/IdentifierHash.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloConditions/CaloNoise.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CLHEP/Units/SystemOfUnits.h"

class Identifier; 
class CaloDetDescrElement;


class CaloTopoClusterMaker: public AthAlgTool, virtual public CaloClusterCollectionProcessor {
public:    
  
  CaloTopoClusterMaker(const std::string& type, const std::string& name,
		       const IInterface* parent);

  using CaloClusterCollectionProcessor::execute;
  virtual StatusCode execute(const EventContext& ctx,
                             xAOD::CaloClusterContainer* theClusters) const override;
  virtual StatusCode initialize() override;

  void getClusterSize();

private: 
  
  inline bool passCellTimeCut(const CaloCell*, const CaloCellContainer*) const;
  
  const CaloCell_ID* m_calo_id{nullptr};
  
  /** 
   * @brief vector of names of the cell containers to use as input.
   */
  SG::ReadHandleKey<CaloCellContainer> m_cellsKey{this, "CellsName", ""};

  /** 
   * @brief vector of names of the calorimeters to consider.
   *
   * The default is to use all calorimeters (i.e. LAREM, LARHEC,
   * LARFCAL, TILE).  Cells which belong to one of the input cell
   * containers and to one of the calorimeters in this vector are used
   * as input for the cluster maker.  This property is used in order
   * to ignore a certain subsystem (e.g. for LAREM only clusters
   * specify only LAREM in the jobOptions).  */
  Gaudi::Property<std::vector<std::string>> m_caloNames{this, "CalorimeterNames", {}};

  /** 
   * @brief Flag which subdetectors are to be used.
   *
   * This is initialized according to the names given in the property
   * m_caloNames.  */
  bool m_subcaloUsed[CaloCell_ID::NSUBCALO]{};

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
  Gaudi::Property<float> m_cellThresholdOnEorAbsEinSigma{this, "CellThresholdOnEorAbsEinSigma", 0.};

  /** 
   * @brief cells with \f$|E| > N_{\rm neighbor}\,\sigma\f$ extend the cluster
   *
   * This cut determines how many cells are asked for their neighbors
   * to expand the cluster. The smaller this cut is the more cells
   * will be asked for their neighbors. If a cell passing this cut is
   * neighbor of two other cells passing this cut from different
   * clusters, the two clusters are merged. The neighbor cut should be
   * lower or equal to the seed cut.  */
  Gaudi::Property<float> m_neighborThresholdOnEorAbsEinSigma{this, "NeighborThresholdOnEorAbsEinSigma", 3.};

  /** 
   * @brief cells with \f$|E| > N_{\rm seed}\,\sigma\f$ start a cluster
   *
   * This cut determines how many clusters are formed initially.  The
   * smaller this cut is the more clusters will be created.  During
   * the accumulation of cells inside the clusters it can happen that
   * clusters are merged if a cell passing the neighbor threshold
   * would be included in both clusters.  */
  Gaudi::Property<float> m_seedThresholdOnEorAbsEinSigma{this, "SeedThresholdOnEorAbsEinSigma", 6.};


  /**                                                                                                             
   * threshold used for timing cut on seed cells. Implemented as |seed_cell_time|<m_seedThresholdOnTAbs. No such cut on neighbouring cells.*/
  Gaudi::Property<float> m_seedThresholdOnTAbs{this, "SeedThresholdOnTAbs", 12.5*CLHEP::ns};
  
  /**                                                                                                             
   * upper limit on the energy significance, for applying the cell time cut */
  Gaudi::Property<float> m_timeCutUpperLimit{this, "TimeCutUpperLimit", 20.};


  /**                                                                                                             
   * additional max. delta t added to the upper limit time window in case of xtalk in EM2 should be accounted for */
  Gaudi::Property<float> m_xtalkDeltaT{this, "XTalkDeltaT", 15.*CLHEP::ns};

   /**
    *  cut on Eneighbor/E to revover out of time cell close to energetic first phi neighbor cell */
   Gaudi::Property<float> m_xtalk2Eratio1{this, "XTalk2Eratio1", 4.};

   /**
    *  cut on Eneighbor/E to revover out of time cell close to energetic second phi neighbor cell */
   Gaudi::Property<float> m_xtalk2Eratio2{this, "XTalk2Eratio2", 25.};

   /**
    *  cut on Eneighbor/E to revover out of time layer 3cell close to energetic previous sampling neighbor */
   Gaudi::Property<float> m_xtalk3Eratio{this, "XTalk3Eratio", 10.};

   /**
    *  cut on Eneighbor/E to revover out of time layer 2 cell close in eta to energetic neighor cell */
   Gaudi::Property<float> m_xtalkEtaEratio{this, "XTalkEtaEratio", 4.};

   /**
     * cut on Eneighbor/E to remove out of time layer layer2 all 2D neighbors */
   Gaudi::Property<float> m_xtalk2DEratio{this, "XTalk2DEratio", 4.};


  /** @brief Key of the CaloNoise Conditions data object. Typical values 
      are '"electronicNoise', 'pileupNoise', or '"totalNoise' (default) */

  SG::ReadCondHandleKey<CaloNoise> m_noiseCDOKey{this,"CaloNoiseKey","totalNoise","SG Key of CaloNoise data object"};


  //SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};

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
  Gaudi::Property<std::string> m_neighborOption{this, "NeighborOption", "super3D"};
  LArNeighbours::neighbourOption m_nOption{LArNeighbours::super3D};

  /**
   * @brief if set to true limit the neighbors in HEC IW and FCal2&3.
   *
   * The cells in HEC IW and FCal2&3 get very large in terms of eta
   * and phi.  Since this might pose problems on certain jet
   * algorithms one might need to avoid expansion in eta and phi for
   * those cells. If this property is set to true the 2d neighbors of
   * these cells are not used - only the next sampling neighbors are
   * probed. */
  Gaudi::Property<bool> m_restrictHECIWandFCalNeighbors{this, "RestrictHECIWandFCalNeighbors", false};
                                                 
  /**
   * @brief if set to true limit the neighbors in presampler Barrel and Endcap.
   *
   * The presampler cells add a lot of PileUp in the Hilum
   * samples. With this option set to true the presampler cells do not
   * expand the cluster in the presampler layer.  Only the next
   * sampling is used as valid neighbor source. */
  Gaudi::Property<bool> m_restrictPSNeighbors{this, "RestrictPSNeighbors", false};
                                                 
  /**
   * @brief if set to true seed cuts are on \f$|E|\f$ and \f$|E|_\perp\f$.
   *
   * The seed cuts and the \f$E_\perp\f$ cut on the final clusters
   * before insertion to the CaloClusterContainer will be on absolute
   * energy and absolute transverse energy if this is set to true. If
   * set to false the cuts will be on energy and transverse energy
   * instead.  */
  Gaudi::Property<bool> m_seedCutsInAbsE{this, "SeedCutsInAbsE", false};
                                                 
  /**
   * @brief if set to true neighbor cuts are on \f$|E|\f$ and \f$|E|_\perp\f$.
   *
   * The neighbor cuts will be on absolute energy and absolute
   * transverse energy if this is set to true. If set to false the
   * cuts will be on energy and transverse energy instead.  */
  Gaudi::Property<bool> m_neighborCutsInAbsE{this, "NeighborCutsInAbsE", true};
                                                 
  /**
   * @brief if set to true cell cuts are on \f$|E|\f$ and \f$|E|_\perp\f$.
   *
   * The cell cuts will be on absolute energy and absolute transverse
   * energy if this is set to true. If set to false the cuts will be
   * on energy and transverse energy instead.  */
  Gaudi::Property<bool> m_cellCutsInAbsE{this, "CellCutsInAbsE", true};
                                                 
  /**
   * @brief if set to true final cluster cuts are on \f$|E|_\perp\f$. 
   *
   * The final cluster cuts will be on absolute transverse energy
   * if this is set to true. If set to false the cuts will be on
   * transverse energy instead.  */
  Gaudi::Property<bool> m_clusterCutsInAbsE{this, "ClusterCutsInAbsEt", true};
  
  /**
   * @brief \f$E_\perp\f$ cut on the final cluster. 
   *
   * The final cluster has to pass this cut (which is on \f$E_\perp\f$
   * or \f$|E|_\perp\f$ of the cluster depending on the above switch)
   * in order to be inserted into the CaloClusterContainer.  */
  Gaudi::Property<float> m_clusterEtorAbsEtCut{this, "ClusterEtorAbsEtCut", 0.*CLHEP::MeV};
                                                 
  /**
   * @brief if set to true use 2-gaussian noise description for 
   * TileCal  */
  Gaudi::Property<bool> m_twogaussiannoise{this, "TwoGaussianNoise", false};
                                                 
  /**
   * @brief if set to true treat cells with a dead OTX which can be
   * predicted by L1 trigger info as good instead of bad cells */
  Gaudi::Property<bool> m_treatL1PredictedCellsAsGood{this, "TreatL1PredictedCellsAsGood", true};

  /**                                                                                              
   * if set to true, time cut is applied to seed cells, no cut otherwise 
   */
  Gaudi::Property<bool> m_seedCutsInT{this, "SeedCutsInT", false};

  /**                                                                                              
   * if set to true, seed cells failing the time cut are also excluded from cluster at all 
   */
  Gaudi::Property<bool> m_cutOOTseed{this, "CutOOTseed", false};

  /**                                                                                              
   * if set to true, the time cut is not applied on cell of large significance
   */
  Gaudi::Property<bool> m_useTimeCutUpperLimit{this, "UseTimeCutUpperLimit", false};

  /**                                                                                              
   * if set to true, the time window is softened in the EMB2 and EME2_OW due to xtalk from direct neighbour cells in phi
   */
  Gaudi::Property<bool> m_xtalkEM2{this, "XTalkEM2", false};

  /**
    * if set to true, the time window is softened in the EMB2 and EME2_OW due to xtalk from  all 2D neighors
    */
   Gaudi::Property<bool> m_xtalkEM2D{this, "XTalkEM2D", false};

  /**                                                                                              
   * if set to true (together with m_xtalkEM2) we also extend the time window for 2nd phi neighbors
   */
  Gaudi::Property<bool> m_xtalkEM2n{this, "XTalkEM2n", false};

  /**                                                                                              
   * if set to true  we extend the time window for direct layer 3 neighbors of high energy layer 2 cells
   */
  Gaudi::Property<bool> m_xtalkEM3{this, "XTalkEM3", false};

  /**
   
    * if set to true, the time window is softened in the EMB2 and EME2_OW due to xtalk from direct neighbour cells in eta
    */
  Gaudi::Property<bool> m_xtalkEMEta{this, "XTalkEMEta", false};


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
  Gaudi::Property<std::vector<std::string>> m_samplingNames{this, "SeedSamplingNames", {}};

  /** 
   * @brief actual set of samplings to be used for seeds
   *
   * This set is created according to the names given in the property
   * m_samplingNames.  */
  std::set<int> m_validSamplings;                    

  /** 
   * @brief smallest valid seed sampling found 
   *
   * This is needed to adjust the range of the vector<bool> for a
   * quick lookup if a cell belongs to a valid sampling for seeds or
   * not. */
  int m_minSampling{0};

  /** 
   * @brief largest valid seed sampling found 
   *
   * This is needed to adjust the range of the vector<bool> for a
   * quick lookup if a cell belongs to a valid sampling for seeds or
   * not. */
  int m_maxSampling{0};

  /** 
   * @brief flag for all samplings - true for used ones, false for excluded ones
   *
   * This vector serves as a quick lookup table to find out if a cell
   * belongs to a sampling that should be used for seeds. */
  std::vector<bool> m_useSampling;

  IdentifierHash m_hashMin{999999};
  IdentifierHash m_hashMax{0};


  /// Cluster size enum. Set based on energy cut jobO
  xAOD::CaloCluster::ClusterSize m_clusterSize{};
  
  Gaudi::Property<bool> m_useGPUCriteria {this, "UseGPUCriteria", false, "Adopt a set of criteria that is consistent with the GPU implementation."};
};

#endif // CALOTOPOCLUSTERMAKER_HH
