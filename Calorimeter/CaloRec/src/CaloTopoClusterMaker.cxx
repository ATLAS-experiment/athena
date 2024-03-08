/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------
// File and Version Information:
// $Id: CaloTopoClusterMaker.cxx,v 1.44 2009-05-19 10:04:47 gunal Exp $
//
// Description: see CaloTopoClusterMaker.h
// 
// Environment:
//      Software developed for the ATLAS Detector at CERN LHC
//
// Author List:
//      Sven Menke
//
//-----------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "CaloTopoClusterMaker.h"
#include "CaloTopoTmpClusterCell.h"
#include "CaloTopoTmpHashCluster.h"
#include "CaloTopoTmpHashCell.h"
#include "CaloTopoTmpHashCellSort.h"
#include "CaloUtils/CaloBadCellHelper.h"
#include "CaloUtils/CaloClusterStoreHelper.h"
#include "CaloEvent/CaloCell.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloEvent/CaloPrefetch.h"
#include "AthAllocators/ArenaPoolAllocator.h"
#include "AthAllocators/ArenaHandle.h"
#include "GaudiKernel/StatusCode.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include <vector>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <memory>

#include "xAODCaloEvent/CaloClusterKineHelper.h"

#include "CaloProtoCluster.h"

using CLHEP::MeV;
using CLHEP::ns;

//#############################################################################

CaloTopoClusterMaker::CaloTopoClusterMaker(const std::string& type, 
					   const std::string& name,
					   const IInterface* parent)
  
  : AthAlgTool(type, name, parent),
    m_calo_id(nullptr),
    m_cellsKey(""),
    m_subcaloUsed(),
    m_cellThresholdOnEorAbsEinSigma    (    0.),
    m_neighborThresholdOnEorAbsEinSigma(    3.),
    m_seedThresholdOnEorAbsEinSigma    (    6.),
    m_seedThresholdOnTAbs              (  12.5*ns),
    m_timeCutUpperLimit                (    20.),
    m_xtalkDeltaT                      (  15.*ns),
    m_xtalk2Eratio1                    ( 4.),
    m_xtalk2Eratio2                    ( 25.),
    m_xtalk3Eratio                     ( 10.),
    m_xtalkEtaEratio                   (4.),
    m_xtalk2DEratio                    (4.),
    m_neighborOption                   ("super3D"),
    m_nOption                          (LArNeighbours::super3D),
    m_restrictHECIWandFCalNeighbors    (false),
    m_restrictPSNeighbors              (false),
    m_seedCutsInAbsE                   (false),
    m_neighborCutsInAbsE               (true),
    m_cellCutsInAbsE                   (true),
    m_clusterCutsInAbsE                (true),
    m_clusterEtorAbsEtCut              (    0.*MeV),
    m_twogaussiannoise                 (false),
    m_treatL1PredictedCellsAsGood      (true),
    m_seedCutsInT                      (false),
    m_cutOOTseed                       (false),
    m_useTimeCutUpperLimit             (false),
    m_xtalkEM2                         (false),
    m_xtalkEM2D                        (false),
    m_xtalkEM2n                        (false),
    m_xtalkEM3                         (false),
    m_xtalkEMEta                       (false),
    m_minSampling                      (0),
    m_maxSampling                      (0),
    m_hashMin                          (999999),
    m_hashMax                          (0),
    m_clusterSize                      ()
{
  declareInterface<CaloClusterCollectionProcessor> (this);
  // Name(s) of Cell Containers
  declareProperty("CellsName",m_cellsKey);
  
  // Name(s) of Calorimeters to consider
  declareProperty("CalorimeterNames",m_caloNames);

  // Name(s) of Calorimeter Samplings to consider for seeds
  declareProperty("SeedSamplingNames",m_samplingNames);

  // Energy thresholds (in units of noise Sigma) 
  declareProperty("SeedThresholdOnEorAbsEinSigma",
		  m_seedThresholdOnEorAbsEinSigma);
  declareProperty("NeighborThresholdOnEorAbsEinSigma",
		  m_neighborThresholdOnEorAbsEinSigma);
  declareProperty("CellThresholdOnEorAbsEinSigma",
		  m_cellThresholdOnEorAbsEinSigma);

  // Seed and cluster cuts are in E or Abs E
  declareProperty("SeedCutsInAbsE",m_seedCutsInAbsE);

  // Time thresholds (in abs. val.)
  declareProperty("SeedThresholdOnTAbs",m_seedThresholdOnTAbs);

  // Significance upper limit for applying time cut
  declareProperty("TimeCutUpperLimit",m_timeCutUpperLimit);

  //do Seed cuts on Time              
  declareProperty("SeedCutsInT",m_seedCutsInT);
  //exclude out-of-time seeds from neighbouring and cell stage              
  declareProperty("CutOOTseed",m_cutOOTseed);
  //do not apply time cut on cells of large significance
  declareProperty("UseTimeCutUpperLimit",m_useTimeCutUpperLimit);
  //relax time window (if timing is used) in EM2 when xTalk is present
  declareProperty("XTalkEM2",m_xtalkEM2);
  //relax time window (if timing is used) in EM2 when xTalk is present for all 2D neighbirs
  declareProperty("XTalkEM2D",m_xtalkEM2D);
  //relax time window (if timing is used) in EM2 Eta directionwhen xTalk is present
  declareProperty("XTalkEMEta",m_xtalkEMEta);
  //relax time window (if timing is used) in EM2 when xTalk is present also for 2nd phi neighbors (if XTalkEM2 is also set)
  declareProperty("XTalkEM2n",m_xtalkEM2n);
  //relax time window (if timing is used) in EM3 when xTalk is present for layer 3 neighbor of high energy layer 2 cells
  declareProperty("XTalkEM3",m_xtalkEM3);
  //delta T to add to upper time threshold for EM2 cells affected by xtalk
  declareProperty("XTalkDeltaT",m_xtalkDeltaT);
  //Eratio for first phi neighbor in layer2
  declareProperty("XTalk2Eratio1",m_xtalk2Eratio1);
  //Eratio for second phi neighbor in layer2
  declareProperty("XTalk2Eratio2",m_xtalk2Eratio2);
  //Eratio for previous sampling neighbor in layer 3
  declareProperty("XTalk3Eratio",m_xtalk3Eratio);
  //Eratio for cross-talk in eta layer 2
  declareProperty("XTalkEtaEratio",m_xtalkEtaEratio);
  //Eratio for cross-talk in all 2D layer 2 neighbors
  declareProperty("XTalk2DEratio",m_xtalk2DEratio);

  // Neighbor cuts are in E or Abs E
  declareProperty("NeighborCutsInAbsE",m_neighborCutsInAbsE);

  // Cell cuts are in E or Abs E
  declareProperty("CellCutsInAbsE",m_cellCutsInAbsE);

  // Neighbor Option
  declareProperty("NeighborOption",m_neighborOption);

  // Restrict HEC IW and FCal Neighbors
  declareProperty("RestrictHECIWandFCalNeighbors",m_restrictHECIWandFCalNeighbors);

  // Restrict PS Neighbors
  declareProperty("RestrictPSNeighbors",m_restrictPSNeighbors);

  //Cluster cuts are in E_t or Abs E_t 
  declareProperty("ClusterCutsInAbsEt",m_clusterCutsInAbsE);
  
  // Cluster E_t or Abs E_t cut
  declareProperty("ClusterEtorAbsEtCut",m_clusterEtorAbsEtCut);

  // use 2-gaussian noise for Tile
  declareProperty("TwoGaussianNoise",m_twogaussiannoise);

  // Treat bad cells with dead OTX if predicted from L1 as good
  declareProperty("TreatL1PredictedCellsAsGood",m_treatL1PredictedCellsAsGood);
}

//#############################################################################

StatusCode CaloTopoClusterMaker::initialize()
{
  ATH_MSG_INFO( "Initializing " << name()  );
  ATH_MSG_INFO( "Treat L1 Predicted Bad Cells as Good set to" << ((m_treatL1PredictedCellsAsGood) ? "true" : "false")  );
  ATH_MSG_INFO( "Two-Gaussian noise for Tile set to " << ((m_twogaussiannoise) ? "true" : "false")  );
  ATH_CHECK( m_cellsKey.initialize() );
  getClusterSize();

  ATH_CHECK( detStore()->retrieve (m_calo_id, "CaloCell_ID") );

  ATH_MSG_INFO( "Threshold choices:"
                << (m_seedCutsInAbsE?" SeedThresholdOnAbsEinSigma=":
                    " SeedThresholdOnEinSigma=")
                << m_seedThresholdOnEorAbsEinSigma
                << (m_neighborCutsInAbsE?", NeighborThresholdOnAbsEinSigma=":
                    ", NeighborThresholdOnEinSigma=")
                << m_neighborThresholdOnEorAbsEinSigma
                << (m_cellCutsInAbsE?", CellThresholdOnAbsEinSigma=":
                    ", CellThresholdOnEinSigma=")
                << m_cellThresholdOnEorAbsEinSigma
                );

  ATH_MSG_INFO( "Time cut option: " << ((!m_seedCutsInT) ? "None" : (m_cutOOTseed ? "Seed Extended" : "Seed")));
  ATH_MSG_INFO( "E/sigma veto on T cut: m_useTimeCutUpperLimit=" << (m_useTimeCutUpperLimit ? "true" : "false") << ", m_timeCutUpperLimit=" << m_timeCutUpperLimit);

  //--- set Neighbor Option

  if ( m_neighborOption == "all2D" ) 
    m_nOption = LArNeighbours::all2D;
  else if ( m_neighborOption == "all3D" ) 
    m_nOption = LArNeighbours::all3D;
  else if ( m_neighborOption == "super3D" ) 
    m_nOption = LArNeighbours::super3D;
  else {
    ATH_MSG_ERROR( "Invalid Neighbor Option "
                   << m_neighborOption << ", exiting ..."  );
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO( "Neighbor Option "
                << m_neighborOption << " is selected!"  );

  //--- check calorimeter names to use
  for (const std::string& caloName : m_caloNames) {
    if ( caloName == "LAREM" ) 
      m_subcaloUsed[CaloCell_ID::LAREM] = true;
    else if ( caloName == "LARHEC" )
      m_subcaloUsed[CaloCell_ID::LARHEC] = true;
    else if ( caloName == "LARFCAL" )
      m_subcaloUsed[CaloCell_ID::LARFCAL] = true;
    else if ( caloName == "TILE" )
      m_subcaloUsed[CaloCell_ID::TILE] = true;
    else 
      ATH_MSG_ERROR( "Calorimeter " << caloName
                     << " is not a valid Calorimeter name and will be ignored! "
                     << "Valid names are: LAREM, LARHEC, LARFCAL, and TILE."  );
  }

  //--- check sampling names to use for seeds
  for (const std::string& sampName : m_samplingNames) {
    if ( sampName == "PreSamplerB" ) 
      m_validSamplings.insert(CaloCell_ID::PreSamplerB);
    else if ( sampName == "EMB1" ) 
      m_validSamplings.insert(CaloCell_ID::EMB1);
    else if ( sampName == "EMB2" ) 
      m_validSamplings.insert(CaloCell_ID::EMB2);
    else if ( sampName == "EMB3" ) 
      m_validSamplings.insert(CaloCell_ID::EMB3);
    else if ( sampName == "PreSamplerE" ) 
      m_validSamplings.insert(CaloCell_ID::PreSamplerE);
    else if ( sampName == "EME1" ) 
      m_validSamplings.insert(CaloCell_ID::EME1);
    else if ( sampName == "EME2" ) 
      m_validSamplings.insert(CaloCell_ID::EME2);
    else if ( sampName == "EME3" ) 
      m_validSamplings.insert(CaloCell_ID::EME3);
    else if ( sampName == "HEC0" ) 
      m_validSamplings.insert(CaloCell_ID::HEC0);
    else if ( sampName == "HEC1" ) 
      m_validSamplings.insert(CaloCell_ID::HEC1);
    else if ( sampName == "HEC2" ) 
      m_validSamplings.insert(CaloCell_ID::HEC2);
    else if ( sampName == "HEC3" ) 
      m_validSamplings.insert(CaloCell_ID::HEC3);
    else if ( sampName == "TileBar0" ) 
      m_validSamplings.insert(CaloCell_ID::TileBar0);
    else if ( sampName == "TileBar1" ) 
      m_validSamplings.insert(CaloCell_ID::TileBar1);
    else if ( sampName == "TileBar2" ) 
      m_validSamplings.insert(CaloCell_ID::TileBar2);
    else if ( sampName == "TileGap1" ) 
      m_validSamplings.insert(CaloCell_ID::TileGap1);
    else if ( sampName == "TileGap2" ) 
      m_validSamplings.insert(CaloCell_ID::TileGap2);
    else if ( sampName == "TileGap3" ) 
      m_validSamplings.insert(CaloCell_ID::TileGap3);
    else if ( sampName == "TileExt0" ) 
      m_validSamplings.insert(CaloCell_ID::TileExt0);
    else if ( sampName == "TileExt1" ) 
      m_validSamplings.insert(CaloCell_ID::TileExt1);
    else if ( sampName == "TileExt2" ) 
      m_validSamplings.insert(CaloCell_ID::TileExt2);
    else if ( sampName == "FCAL0" ) 
      m_validSamplings.insert(CaloCell_ID::FCAL0);
    else if ( sampName == "FCAL1" ) 
      m_validSamplings.insert(CaloCell_ID::FCAL1);
    else if ( sampName == "FCAL2" ) 
      m_validSamplings.insert(CaloCell_ID::FCAL2);
    else 
      ATH_MSG_ERROR( "Calorimeter sampling" << sampName 
                     << " is not a valid Calorimeter sampling name and will be ignored! "
                     << "Valid names are: "
                     << "PreSamplerB, EMB1, EMB2, EMB3, "
                     << "PreSamplerE, EME1, EME2, EME3, "
                     << "HEC0, HEC1, HEC2, HEC3, "
                     << "TileBar0, TileBar1, TileBar2, "
                     << "TileGap1, TileGap2, TileGap3, "
                     << "TileExt0, TileExt1, TileExt2, "
                     << "FCAL0, FCAL1, FCAL2."  );
  }

  msg(MSG::INFO) << "Samplings to consider for seeds:";
  for (const std::string& sampName : m_samplingNames)
    msg() << " " << sampName;
  msg() << endmsg;

  m_minSampling=0;
  m_maxSampling=0;
  for (int s : m_validSamplings) {
    if ( s > m_maxSampling ) 
      m_maxSampling = s;
    if ( s < m_minSampling ) 
      m_minSampling = s;
  }

  m_useSampling.resize(m_maxSampling-m_minSampling+1,false);

  for (int s : m_validSamplings) {
    m_useSampling[s-m_minSampling] = true;
  }

  ATH_MSG_INFO( "CellCollection to use: " << m_cellsKey.key()  );

  msg(MSG::INFO) << "Calorimeters to consider:";
  for (const std::string& caloName : m_caloNames)
    msg() << " " << caloName;
  msg() << endmsg;

  //---- retrieve the noise CDO  ----------------
  
  ATH_CHECK(m_noiseCDOKey.initialize());

  ATH_MSG_INFO( (m_clusterCutsInAbsE?"ClusterAbsEtCut= ":"ClusterEtCut= ")
                << m_clusterEtorAbsEtCut << " MeV"  );

  m_hashMin = 999999;
  m_hashMax = 0;
  for (int subdet = 0; subdet < CaloCell_ID::NSUBCALO; ++subdet) {
    if (m_subcaloUsed[subdet]) {
      IdentifierHash thismin, thismax;
      m_calo_id->calo_cell_hash_range ((CaloCell_ID::SUBCALO)subdet,
                                       thismin, thismax);
      m_hashMin = std::min (m_hashMin, thismin);
      m_hashMax = std::max (m_hashMax, thismax);
    }
  }

  //ATH_CHECK( m_cablingKey.initialize() );

  return StatusCode::SUCCESS;
  
}

//#############################################################################

StatusCode
CaloTopoClusterMaker::execute(const EventContext& ctx,
                              xAOD::CaloClusterContainer* clusColl) const
{
  // minimal significance - should be > 0 in order to avoid 
  // throwing away of bad cells
  const float epsilon = 0.00001;

  //ATH_MSG_DEBUG( "Executing " << name());

  using HashCell = CaloTopoTmpHashCell<CaloTopoTmpClusterCell>;
  using HashCluster = CaloTopoTmpHashCluster;

  CaloPrefetch::ArenaHandlePrefetch<CaloTopoTmpClusterCell, SG::ArenaPoolAllocator, 2> tmpcell_pool;
  CaloPrefetch::ArenaHandlePrefetch<HashCluster,            SG::ArenaPoolAllocator, 2> tmpclus_pool;
  CaloTopoTmpHashCluster::pool_type                               tmplist_pool;

  // create cell list for cells above seed  cut (for seed growing algo)
  std::vector<HashCell> mySeedCells;
  mySeedCells.reserve (2000);
  // create initial cluster list (one cell per cluster)
  std::vector<HashCluster *> myHashClusters;
  myHashClusters.reserve (10000);

  // create vector to hold all cells.
  std::vector<HashCell> cellVector (m_hashMax - m_hashMin);
  HashCell* hashCells = cellVector.data() - m_hashMin;

  
  SG::ReadCondHandle<CaloNoise> noiseHdl{m_noiseCDOKey,ctx};
  const CaloNoise* noiseCDO=*noiseHdl;

  //---- Get the CellContainers ----------------

  //  for (const std::string& cellsName : m_cellsNames) {
  SG::ReadHandle<CaloCellContainer> cellColl(m_cellsKey, ctx);
  if( !cellColl.isValid()){ 
    ATH_MSG_ERROR( " Can not retrieve CaloCellContainer: "
                   << cellColl.name()  );
    return StatusCode::RECOVERABLE;      
  }    

  const DataLink<CaloCellContainer> cellCollLink (cellColl.name(),ctx);

  //ATH_MSG_DEBUG("CaloCell container: "<< cellsName 
  //		  <<" contains " << cellColl->size() << " cells");

  for (int isubdet = 0; isubdet < CaloCell_ID::NSUBCALO; ++isubdet) {
    CaloCell_ID::SUBCALO subdet = (CaloCell_ID::SUBCALO)isubdet;
    if (m_subcaloUsed[subdet] && cellColl->hasCalo(subdet)) {
      auto cellIter = cellColl->beginConstCalo (subdet);
      auto cellIterEnd = cellColl->endConstCalo (subdet);
      for (int iCell = cellColl->indexFirstCellCalo(subdet);
	   cellIter != cellIterEnd;
	   ++iCell, ++cellIter)
        {
          CaloPrefetch::nextDDE(cellIter, cellIterEnd, 2);
	  const CaloCell* pCell = *cellIter;
	  const float noiseSigma = m_twogaussiannoise ? \
	    noiseCDO->getEffectiveSigma(pCell->ID(),pCell->gain(),pCell->energy()) : \
	    noiseCDO->getNoise(pCell->ID(),pCell->gain());

	  float signedE = pCell->energy();
	  float signedEt = pCell->et();
	  float signedRatio = epsilon; // not 0 in order to keep bad cells 
	  if ( finite(noiseSigma) && noiseSigma > 0 && !CaloBadCellHelper::isBad(pCell,m_treatL1PredictedCellsAsGood) ) 
	    signedRatio = signedE/noiseSigma;

	  bool passedCellCut = (m_cellCutsInAbsE?std::abs(signedRatio):signedRatio) > m_cellThresholdOnEorAbsEinSigma;
	  bool passedNeighborCut = (m_neighborCutsInAbsE?std::abs(signedRatio):signedRatio) > m_neighborThresholdOnEorAbsEinSigma;
	  bool passedSeedCut = (m_seedCutsInAbsE?std::abs(signedRatio):signedRatio) > m_seedThresholdOnEorAbsEinSigma;

	  bool applyTimeCut = m_seedCutsInT && (!m_useTimeCutUpperLimit || signedRatio <= m_timeCutUpperLimit);
	  bool passTimeCut_seedCell = (!applyTimeCut || passCellTimeCut(pCell,cellColl.cptr()));
	  bool passedSeedAndTimeCut = (passedSeedCut && passTimeCut_seedCell);

	  bool passedNeighborAndTimeCut = passedNeighborCut;
	  if(m_cutOOTseed && passedSeedCut && !passTimeCut_seedCell) passedNeighborAndTimeCut=false; //exclude Out-Of-Time seeds from neighbouring stage as well (if required)

	  bool passedCellAndTimeCut = passedCellCut;
	  if(m_cutOOTseed && passedSeedCut && !passTimeCut_seedCell) passedCellAndTimeCut=false; //exclude Out-Of-Time seeds from cluster (if required)

	  if ( passedCellAndTimeCut || passedNeighborAndTimeCut || passedSeedAndTimeCut ) {
	    const CaloDetDescrElement* dde = pCell->caloDDE();
	    IdentifierHash hashid = dde ? dde->calo_hash() : m_calo_id->calo_cell_hash(pCell->ID());
	    CaloTopoTmpClusterCell *tmpClusterCell =
              new (tmpcell_pool.allocate())
              CaloTopoTmpClusterCell(hashid,subdet,iCell,signedRatio,signedEt);
#if 0
	    // some debug printout - can also be used to construct neighbor
	    // tables offline ...
	    if ( m_doALotOfPrintoutInFirstEvent ) {
	      ATH_MSG_DEBUG( " [ExtId|Id|SubDet|HashId|eta|phi|E/noise|Et]: "
			     << "[" << m_calo_id->show_to_string(pCell->ID(),0,'/')
			     << "|" << mypCell->ID().getString()
			     << "|" << subdet
			     << "|" << (unsigned int)hashid
			     << "|" << pCell->eta()
			     << "|" << pCell->phi()
			     << "|" << signedRatio
			     << "|" << signedEt
			     << "]");
	  
	    }
#endif
	    HashCell hashCell(tmpClusterCell);
	    if ( passedNeighborAndTimeCut || passedSeedAndTimeCut ) {
	      HashCluster *tmpCluster =
                new (tmpclus_pool.allocate()) HashCluster (tmplist_pool);
	      tmpClusterCell->setCaloTopoTmpHashCluster(tmpCluster);
	      tmpCluster->add(hashCell);
	      myHashClusters.push_back(tmpCluster);
	      int caloSample = dde ? dde->getSampling() : m_calo_id->calo_sample(pCell->ID());
	      if ( passedSeedAndTimeCut 
		   && caloSample >= m_minSampling 
		   && caloSample <= m_maxSampling 
		   && m_useSampling[caloSample-m_minSampling]) {
		tmpClusterCell->setUsed();
		mySeedCells.push_back(hashCell);
	      }
	    }
	    hashCells[hashid] = hashCell;
	  }
	}//end loop over cells
    }//end if use subcalo
  }//end loop over subcalos


  // sort initial seed cells to start with the cell of largest S/N
  // this makes the resulting clusters independent of the initial
  // ordering of the cells
  if ( m_useGPUCriteria) {
    if ( m_seedCutsInAbsE) {
      CaloTopoTmpHashCellSort::compareAbsWithIndex<CaloTopoTmpClusterCell> compareSoverN;
      std::sort(mySeedCells.begin(),mySeedCells.end(),compareSoverN); 
    }
    else {
      CaloTopoTmpHashCellSort::compareWithIndex<CaloTopoTmpClusterCell> compareSoverN;
      std::sort(mySeedCells.begin(),mySeedCells.end(),compareSoverN); 
    }
  }
  else {
    if ( m_seedCutsInAbsE) {
      CaloTopoTmpHashCellSort::compareAbs<CaloTopoTmpClusterCell> compareSoverN;
      std::sort(mySeedCells.begin(),mySeedCells.end(),compareSoverN); 
    }
    else {
      CaloTopoTmpHashCellSort::compare<CaloTopoTmpClusterCell> compareSoverN;
      std::sort(mySeedCells.begin(),mySeedCells.end(),compareSoverN); 
    }
  }

#if 1
  if (msgLvl(MSG::DEBUG)) {
    for (const HashCell& hc : mySeedCells) {
      ATH_MSG_DEBUG( " SeedCell [" 
		      << hc.getCaloTopoTmpClusterCell()->getSubDet() 
		      << "|" 
		      << (unsigned int)hc.getCaloTopoTmpClusterCell()->getID() 
		      << "] has S/N = " 
		      << hc.getCaloTopoTmpClusterCell()->getSignedRatio() 
                     );
    }
  }
#endif

  std::vector<HashCell> myNextCells;
  myNextCells.reserve (1000);

  std::vector<IdentifierHash> theNeighbors;
  theNeighbors.reserve(22);
#if 0
  std::vector<IdentifierHash> theNNeighbors;
  theNNeighbors.reserve(22);
#endif

  bool doRestrictHECIWandFCal = m_restrictHECIWandFCalNeighbors &&
    (m_nOption & LArNeighbours::nextInSamp);

  bool doRestrictPS = m_restrictPSNeighbors && 
    (m_nOption & LArNeighbours::nextInSamp);

  while ( !mySeedCells.empty() ) {
    // create cell list for next neighbor cells to consider
    myNextCells.clear();

    // loop over all current neighbor cells (for Seed Growing Algo)
    for (HashCell& hc : mySeedCells) {
      CaloTopoTmpClusterCell* pCell= hc.getCaloTopoTmpClusterCell();
      IdentifierHash hashid = pCell->getID();
      HashCluster *myCluster = pCell->getCaloTopoTmpHashCluster();
      CaloCell_ID::SUBCALO mySubDet = pCell->getSubDet();
      // in case we use all3d or super3D and the current cell is in the 
      // HEC IW or FCal2 & 3 or PS and we want to restrict their neighbors, 
      // use only next in sampling neighbors 
      LArNeighbours::neighbourOption opt = m_nOption;
      if (( mySubDet != CaloCell_ID::LAREM &&
	    doRestrictHECIWandFCal &&
	    ( ( mySubDet == CaloCell_ID::LARHEC  &&
		m_calo_id->region(m_calo_id->cell_id(hashid)) == 1 )  ||
	      ( mySubDet == CaloCell_ID::LARFCAL &&
		m_calo_id->sampling(m_calo_id->cell_id(hashid)) > 1 ) ) ) ||
	  ( doRestrictPS && 
	    ( ( mySubDet == CaloCell_ID::LAREM && 
		m_calo_id->sampling(m_calo_id->cell_id(hashid)) == 0 ) ) ) ) {
	opt = LArNeighbours::nextInSamp;
      }
      m_calo_id->get_neighbours(hashid,opt,theNeighbors);
      // loop over all neighbors of that cell (Seed Growing Algo)
      for (IdentifierHash nId : theNeighbors) {
        CaloCell_ID::SUBCALO otherSubDet =
          (CaloCell_ID::SUBCALO)m_calo_id->sub_calo(nId);
	if ( m_subcaloUsed[otherSubDet] ) {
	  HashCell neighborCell = hashCells[nId];
	  if ( neighborCell.getCaloTopoTmpClusterCell() ) {
	    CaloTopoTmpClusterCell* pNCell =
              neighborCell.getCaloTopoTmpClusterCell();
	    // check neighbor threshold only since seed cells are already in
	    // the original list 
	    bool isAboveNeighborThreshold = 
	      (m_neighborCutsInAbsE?std::abs(pNCell->getSignedRatio()):pNCell->getSignedRatio()) > m_neighborThresholdOnEorAbsEinSigma;
	    // checking the neighbors
	    if ( isAboveNeighborThreshold && !pNCell->getUsed() ) {
	      pNCell->setUsed();
	      myNextCells.push_back(neighborCell);
	    }
	    HashCluster *otherCluster = pNCell->getCaloTopoTmpHashCluster();
	    if ( myCluster != otherCluster ) {
	      HashCluster *toKill = nullptr;
	      HashCluster *toKeep = nullptr;
	      if ( !otherCluster || isAboveNeighborThreshold ) {
		
		auto compareClusters = [&](const auto & c1, const auto & c2) {
		  if (m_useGPUCriteria) {
		    //The seed cell with the largest SNR wins
		    if (m_seedCutsInAbsE) {
		      CaloTopoTmpHashCellSort::compareAbsWithIndex<CaloTopoTmpClusterCell> compare;
		      return compare(*(c1->begin()), *(c2->begin()));
		    }
		    else {
		      CaloTopoTmpHashCellSort::compareWithIndex<CaloTopoTmpClusterCell> compare;
		      return compare(*(c1->begin()), *(c2->begin()));
		    }
		  }
		  else {
		    //We merge the smallest cluster to the largest...
		    return c1->size() > c2->size();
		  }
		};
		
		if ( !otherCluster || compareClusters(myCluster, otherCluster) ) {
		  toKill = otherCluster;
		  toKeep = myCluster;
		}
		else {
		  toKill = myCluster;
		  toKeep = otherCluster;
		}
		if ( toKill ) {
                  for (auto *hc : *toKill)
                    hc->setCaloTopoTmpHashCluster(toKeep);
		  toKeep->add(*toKill);
		  toKill->removeAll();
		}
		else {
		  toKeep->add(neighborCell);
		  pNCell->setCaloTopoTmpHashCluster(toKeep);
		}
		myCluster = toKeep;
	      }
	    }
	  }
	}
      }
    }
    mySeedCells.swap (myNextCells);
  }


  //Create temporary list of proto-clusters 
  //Clusters below Et cut will be dropped. 
  //The remaining clusters will be sorted in E_t before storing 
  std::vector<std::unique_ptr<CaloProtoCluster> > sortClusters;
  sortClusters.reserve (myHashClusters.size());

  for (HashCluster* tmpCluster : myHashClusters) {
    bool addCluster(false);
    if ( tmpCluster->size() > 1 )
      addCluster = true;
    else if ( tmpCluster->size() == 1 ) {
      // need to check if seed cell was good
      HashCluster::iterator clusCellIter=tmpCluster->begin();
      if ( clusCellIter->getUsed() )
	addCluster = true;
    }
    if ( addCluster) {
      std::unique_ptr<CaloProtoCluster> myCluster = std::make_unique<CaloProtoCluster>(cellCollLink);
      //CaloProtoCluster* myCluster = new CaloProtoCluster(cellCollLink);
      myCluster->getCellLinks()->reserve(tmpCluster->size());

      for (CaloTopoTmpClusterCell* cell : *tmpCluster) {
	const size_t iCell = cell->getCaloCell();
	myCluster->addCell(iCell,1.);
      }
      const float cl_et = myCluster->et();
      if ( (m_clusterCutsInAbsE ? std::abs(cl_et) : cl_et) > m_clusterEtorAbsEtCut ) {
	sortClusters.push_back(std::move(myCluster));
      } 
    }
  }

  // Sort the clusters according to Et 
  std::sort(sortClusters.begin(),sortClusters.end(),[](const std::unique_ptr<CaloProtoCluster>& pc1, 
						       const std::unique_ptr<CaloProtoCluster>& pc2) {
      //As in CaloUtils/CaloClusterEtSort. 
      //assign to volatile to avoid excess precison on in FP unit on x386 machines
      volatile double et1(pc1->et());
      volatile double et2(pc2->et());
      //return (et1 < et2);  //This is the order we had from CaloRec-02-13-11 to  CaloRec-03-00-31
      return (et1 > et2); //This is the order we should have
    }
    );
 // add to cluster container
  clusColl->reserve(sortClusters.size());

  for (const auto& protoCluster: sortClusters) {
    xAOD::CaloCluster* xAODCluster=new xAOD::CaloCluster();
    clusColl->push_back(xAODCluster);
    xAODCluster->addCellLink(protoCluster->releaseCellLinks());//Hand over ownership to xAOD::CaloCluster
    xAODCluster->setClusterSize(m_clusterSize);
    CaloClusterKineHelper::calculateKine(xAODCluster,false,true, m_useGPUCriteria); //No weight at this point! 
  }
  
  tmpclus_pool.erase();
  tmpcell_pool.erase();

  return StatusCode::SUCCESS;
}

void CaloTopoClusterMaker::getClusterSize(){
  m_clusterSize = xAOD::CaloCluster::CSize_Unknown;

  if(m_seedThresholdOnEorAbsEinSigma==6. 
     && m_neighborThresholdOnEorAbsEinSigma==3. 
     && m_cellThresholdOnEorAbsEinSigma==3.){
    
    m_clusterSize = xAOD::CaloCluster::Topo_633;
    
  }else if(m_seedThresholdOnEorAbsEinSigma==4. 
	   && m_neighborThresholdOnEorAbsEinSigma==2. 
	   && m_cellThresholdOnEorAbsEinSigma==0.){
    
    m_clusterSize = xAOD::CaloCluster::Topo_420;    
  }
  ATH_MSG_DEBUG( "Cluster size = " << m_clusterSize);  
}


inline bool CaloTopoClusterMaker::passCellTimeCut(const CaloCell* pCell, const CaloCellContainer* cellColl) const {
  // get the cell time to cut on (the same as in CaloEvent/CaloCluster.h)                             
  bool isInTime = true; 
  // need sampling number already for time
  CaloSampling::CaloSample sam = pCell->caloDDE()->getSampling();
  // check for unknown sampling                                                 
  if (sam != CaloSampling::PreSamplerB && sam != CaloSampling::PreSamplerE && sam != CaloSampling::Unknown) {
    const unsigned pmask= pCell->caloDDE()->is_tile() ? 0x8080 : 0x2000; 
    //0x2000 is used to tell that time and quality information are available for this channel
    //(from TWiki: https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/CaloEventDataModel#The_Raw_Data_Model)	    
    // Is time defined?                                                                         
    if(pCell->provenance() & pmask) {
      isInTime = (std::abs(pCell->time())<m_seedThresholdOnTAbs);
      if ( m_xtalkEM2 && (!isInTime) && (pCell->energy() > 0 && (sam == CaloSampling::EMB2 || (sam == CaloSampling::EME2 && std::abs(pCell->eta()) < 2.5)))) {
	// relax time constraints in EMB2 and EME2_OW due to xTalk from direct phi neighbours
	// check if |E| is less than 0.25 times one of the |E| values of direct
	// phi-neighbours. In case that phi-neigbour is in-time, expand upper limit by m_xtalkDeltaT
	IdentifierHash hashid = pCell->caloDDE()->calo_hash();
	std::vector<IdentifierHash> theNeighbors;
	LArNeighbours::neighbourOption opt = (LArNeighbours::neighbourOption)(((int)LArNeighbours::prevInPhi)|((int)LArNeighbours::nextInPhi)); // shoud make a proper enum in LarNeighbours.h for this one ...
	m_calo_id->get_neighbours(hashid,opt,theNeighbors);
	// loop over all neighbors of that cell (Seed Growing Algo)
	for (IdentifierHash nId : theNeighbors) {
	  const CaloCell * pNCell = cellColl->findCell(nId);
	  if ( pNCell ) {
	    if ( pNCell->energy() > m_xtalk2Eratio1*pCell->energy() ) {
	      if ( (!(pNCell->provenance() & pmask)) || std::abs(pNCell->time()) < m_seedThresholdOnTAbs) {
		isInTime = ((pCell->time() > -m_seedThresholdOnTAbs) && (pCell->time() < m_seedThresholdOnTAbs + m_xtalkDeltaT));
		if ( isInTime ) {
		  // exit after first phi neighbour in case that already made
		  // the time-cut pass
		  break;
		}
	      }
	    }

            // check second neighbor
            if (m_xtalkEM2n) {
             std::vector<IdentifierHash> theNextNeighbors;
             m_calo_id->get_neighbours(nId,opt,theNextNeighbors);
             for (IdentifierHash n2Id : theNextNeighbors) {
              if (n2Id != hashid) {
                const CaloCell * p2NCell = cellColl->findCell(n2Id);
                if (p2NCell) {
                  if (p2NCell->energy() > m_xtalk2Eratio2*pCell->energy()) {
                      if ( (!(p2NCell->provenance() & pmask)) || std::abs(p2NCell->time()) < m_seedThresholdOnTAbs) {
                          isInTime = ((pCell->time() > -m_seedThresholdOnTAbs) && (pCell->time() < m_seedThresholdOnTAbs + m_xtalkDeltaT));
                          if (isInTime) break;
                      }
                  }
                }
              }
             }    // loop over 2nd neighbors
            }
	  }      // if (pNcell)
	}        // loop over first neighbors
      }          // special case for layer 2

      // check cross talk in eta
      if ( m_xtalkEMEta && (!isInTime) && (pCell->energy() > 0 && (sam == CaloSampling::EMB2 || (sam == CaloSampling::EME2 && std::abs(pCell->eta()) < 2.5)))) {
          IdentifierHash hashid = pCell->caloDDE()->calo_hash();
          std::vector<IdentifierHash> theNeighbors;
          LArNeighbours::neighbourOption opt = (LArNeighbours::neighbourOption)(((int)LArNeighbours::prevInEta)|((int)LArNeighbours::nextInEta));
          m_calo_id->get_neighbours(hashid,opt,theNeighbors);
          for (IdentifierHash nId : theNeighbors) {
            const CaloCell * pNCell = cellColl->findCell(nId);
            if ( pNCell ) {
                if ( pNCell->energy() > m_xtalkEtaEratio*pCell->energy() ) {
                 if ( (!(pNCell->provenance() & pmask)) || std::abs(pNCell->time()) < m_seedThresholdOnTAbs) {
                    isInTime = ((pCell->time() > -m_seedThresholdOnTAbs) && (pCell->time() < m_seedThresholdOnTAbs + m_xtalkDeltaT));
                    if ( isInTime ) {
                     // exit after first phi neighbour in case that already made
                     // the time-cut pass
                     break;
                    }
                 }
                }
            }
          }
      }

      // option for all2D
      if ( m_xtalkEM2D && (!isInTime) && (pCell->energy() > 0 && (sam == CaloSampling::EMB2 || (sam == CaloSampling::EME2 && std::abs(pCell->eta()) < 2.5)))) {
         IdentifierHash hashid = pCell->caloDDE()->calo_hash();
         std::vector<IdentifierHash> theNeighbors;
         LArNeighbours::neighbourOption opt =LArNeighbours::all2D;
         m_calo_id->get_neighbours(hashid,opt,theNeighbors);
         for (IdentifierHash nId : theNeighbors) {
           const CaloCell * pNCell = cellColl->findCell(nId);
           if ( pNCell ) {
               if ( pNCell->energy() > m_xtalk2DEratio*pCell->energy() ) {
                   if ( (!(pNCell->provenance() & pmask)) || std::abs(pNCell->time()) < m_seedThresholdOnTAbs) {
                        isInTime = ((pCell->time() > -m_seedThresholdOnTAbs) && (pCell->time() < m_seedThresholdOnTAbs + m_xtalkDeltaT));
                        if ( isInTime )  break;
                   }
               }
           }
         } 
      }

      // relax also time constraint for EMB3 and EME2_OW
      if ( m_xtalkEM3 && (!isInTime) && (pCell->energy() > 0 && (sam == CaloSampling::EMB3 || (sam == CaloSampling::EME3 && std::abs(pCell->eta()) < 2.5)))) {
         // check previous sampling cell, should be >10 times more (TBC)
         IdentifierHash hashid = pCell->caloDDE()->calo_hash();
         std::vector<IdentifierHash> theNeighbors;
         LArNeighbours::neighbourOption opt = LArNeighbours::prevInSamp;
         m_calo_id->get_neighbours(hashid,opt,theNeighbors);
         for (IdentifierHash nId : theNeighbors) {
           const CaloCell * pNCell = cellColl->findCell(nId);
           if ( pNCell ) {
            if ( pNCell->energy() > m_xtalk3Eratio*pCell->energy() ) {
              if ( (!(pNCell->provenance() & pmask)) || std::abs(pNCell->time()) < m_seedThresholdOnTAbs) {
                 isInTime = ((pCell->time() > -m_seedThresholdOnTAbs) && (pCell->time() < m_seedThresholdOnTAbs + m_xtalkDeltaT));
                 if (isInTime) break;
              }   
            }  // Eratio cut at 10
           }
         }     // loop over neighors
      }        // cell is layer 3 EM

    }
  }
  return isInTime;
}
