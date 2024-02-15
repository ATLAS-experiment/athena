/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXSysSim  -  description
//                              -------------------
//     begin                : 12 07 2019
//     email                : jacob.julian.kempster@cern.ch alison.elliot@cern.ch
//  ***************************************************************************/

#include "L1CaloFEXSim/eFEXSysSim.h"
#include "L1CaloFEXSim/eFEXSim.h"
#include "L1CaloFEXSim/eTower.h"
#include "L1CaloFEXSim/eFEXTOBxTOBMatching.h"
#include "L1CaloFEXSim/eTowerContainer.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"

#include "TrigConfData/L1Menu.h"

#include "xAODTrigger/eFexEMRoI.h"
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexEMRoIAuxContainer.h"

#include "xAODTrigger/eFexTauRoI.h"
#include "xAODTrigger/eFexTauRoIContainer.h"
#include "xAODTrigger/eFexTauRoIAuxContainer.h"


namespace LVL1 {
  
  
  // default constructor for persistency

  eFEXSysSim::eFEXSysSim(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent)
  {
    declareInterface<IeFEXSysSim>(this);

  }

    
  /** Destructor */
  //eFEXSysSim::~eFEXSysSim()
  //{
  //}

  //---------------- Initialisation -------------------------------------------------

  StatusCode eFEXSysSim::initialize()
  {
    
    ATH_CHECK( m_eTowerContainerSGKey.initialize() );

    ATH_CHECK( m_eFEXSimTool.retrieve() );

    ATH_CHECK( m_eFexOutKey.initialize() );
    ATH_CHECK( m_eFexEMxTOBOutKey.initialize() );

    ATH_CHECK( m_eFexTauActiveOutKey.initialize() );
    ATH_CHECK( m_eFexTauActivexTOBOutKey.initialize() );
    ATH_CHECK( m_eFexTauAltOutKey.initialize(SG::AllowEmpty) );
    ATH_CHECK( m_eFexTauAltxTOBOutKey.initialize(SG::AllowEmpty) );

    ATH_CHECK( m_eFEXFPGATowerIdProviderTool.retrieve() );

    ATH_CHECK( m_eFEXFPGATool.retrieve() );
    
    ATH_CHECK(m_l1MenuKey.initialize());

    return StatusCode::SUCCESS;
  }

  //---------------- Finalisation -------------------------------------------------

  StatusCode eFEXSysSim::finalize()
  {
    return StatusCode::SUCCESS;
  }

  
  void eFEXSysSim::init()  {

  }

  void eFEXSysSim::cleanup()  {

    m_eFEXCollection.clear();
  }


  int eFEXSysSim::calcTowerID(int eta, int phi, int mod) const {

    return ((64*eta) + phi + mod);
  }

  StatusCode eFEXSysSim::execute(eFEXOutputCollection* inputOutputCollection)  {    

    SG::ReadHandle<LVL1::eTowerContainer> this_eTowerContainer(m_eTowerContainerSGKey/*,ctx*/);
    if(!this_eTowerContainer.isValid()){
      ATH_MSG_FATAL("Could not retrieve eTowerContainer " << m_eTowerContainerSGKey.key());
      return StatusCode::FAILURE;
    }

    // remove TOBs of the previous events from the map
    m_allEmTobObjects.clear();
    m_allTauHeuristicTobObjects.clear();
    m_allTauBDTTobObjects.clear();

    // do mapping with preloaded csv file if it is available
    if (m_eFEXFPGATowerIdProviderTool->ifhaveinputfile()) {
      int tmp_eTowersIDs_subset_eFEX[10][18];
      for (int i_efex{ 0 }; i_efex < 24; i_efex++) {
          ATH_CHECK(m_eFEXFPGATowerIdProviderTool->getRankedTowerIDineFEX(i_efex, tmp_eTowersIDs_subset_eFEX));
          m_eFEXSimTool->init(i_efex);
          ATH_CHECK(m_eFEXSimTool->NewExecute(tmp_eTowersIDs_subset_eFEX, inputOutputCollection));
          // Get TOBs from this eFEX
          m_allEmTobObjects.insert( std::map<int, std::vector<std::unique_ptr<eFEXegTOB>> >::value_type(i_efex, m_eFEXSimTool->getEmTOBs() ));
          m_allTauHeuristicTobObjects.insert( std::map<int, std::vector<std::unique_ptr<eFEXtauTOB>> >::value_type(i_efex, m_eFEXSimTool->getTauHeuristicTOBs() ));
          m_allTauBDTTobObjects.insert( std::map<int, std::vector<std::unique_ptr<eFEXtauTOB>> >::value_type(i_efex, m_eFEXSimTool->getTauBDTTOBs() ));
          m_eFEXSimTool->reset();
      }
    } else {
    // We need to split the towers into 3 blocks in eta and 8 blocks in phi.

    // boundaries in eta: -2.5, -0.8, 0.8, 2.5
    // REAL boundaries in eta (overlaps must occur for sliding window algorithms!): -2.5, -0.7, -0.9, 0.9, 0.7, 2.5
    // Written explicitly:
    // -2.5 -> -0.7
    // -0.9 -> 0.9
    // 0.7 -> 2.5

    // boundaries in phi: 0.2, 1.0, 1.8, 2.6, 3.4, 4.2, 5.0, 5.8
    // Written explicitly with REAL boundaries in phi (overlaps must occur for sliding window algorithms!)
    // 0.1 -> 1.1
    // 0.9 -> 1.9
    // 1.7 -> 2.7
    // 2.5 -> 3.5
    // 3.3 -> 4.3
    // 4.1 -> 5.1
    // 4.9 -> 5.9
    // 5.7 -> 0.3

    // C-SIDE NEGATIVE EFEXs
    // DO THE LEFT-MOST (NEGATIVE ETA) EFEXs FIRST
    int fexcounter = 0;
    // Example values for eFEX 0
    //id_modifier + phi + (64 * eta)
    int emecEta = 24; int emecPhi = 1; int emecMod = 500000;
    int initialEMEC = calcTowerID(emecEta,emecPhi,emecMod); //501537;
    int transEta = 14; int transPhi = 1; int transMod = 300000;
    int initialTRANS = calcTowerID(transEta,transPhi,transMod); //300897;
    int embEta = 13; int embPhi = 1; int embMod = 100000;
    int initialEMB = calcTowerID(embEta,embPhi,embMod); //100833;
    int eFEXa = 0;

    for (int thisEFEX=eFEXa; thisEFEX<=21; thisEFEX+=3){
      
      if(fexcounter > 0){ initialEMEC += 8; initialTRANS += 8; initialEMB += 8; } // TODO // SOMEHOW REMOVE HARD-CODING?

      // decide which subset of towers (and therefore supercells) should go to the eFEX
      std::map<int,eTower> tmp_eTowersColl_subset;
      
      // let's try doing this with an array initially just containing tower IDs.
      int tmp_eTowersIDs_subset [10][18];
      
      int rows = sizeof tmp_eTowersIDs_subset / sizeof tmp_eTowersIDs_subset[0];
      int cols = sizeof tmp_eTowersIDs_subset[0] / sizeof tmp_eTowersIDs_subset[0][0];

      // set the EMEC part
      for(int thisCol=0; thisCol<10; thisCol++){
	for(int thisRow=0; thisRow<rows; thisRow++){

	  int towerid = initialEMEC - (thisCol * 64) + thisRow;

	  if( (thisEFEX == 21) && (thisRow >= 7)){ towerid -= 64; };
	  
	  tmp_eTowersIDs_subset[thisRow][thisCol] = towerid;
	  tmp_eTowersColl_subset.insert( std::map<int, eTower>::value_type(towerid,  *(this_eTowerContainer->findTower(towerid))));

	}
      }

      // set the TRANS part
      for(int thisRow = 0; thisRow < rows; thisRow++){

	int towerid = initialTRANS + thisRow;

	if( (thisEFEX == 21) && (thisRow >= 7)){ towerid -= 64; };

	tmp_eTowersIDs_subset[thisRow][10] = towerid;
	tmp_eTowersColl_subset.insert( std::map<int, eTower>::value_type(towerid,  *(this_eTowerContainer->findTower(towerid))));

      }

      // set the EMB part
      for(int thisCol = 11; thisCol < cols; thisCol++){
	for(int thisRow=0; thisRow<rows; thisRow++){

	  int towerid = initialEMB - ( (thisCol-11) * 64) + thisRow;

	  if( (thisEFEX == 21) && (thisRow >= 7)){ towerid -= 64; };

	  tmp_eTowersIDs_subset[thisRow][thisCol] = towerid;
	  tmp_eTowersColl_subset.insert( std::map<int, eTower>::value_type(towerid,  *(this_eTowerContainer->findTower(towerid))));

	}
      }
      

      if(false){
	ATH_MSG_DEBUG("CONTENTS OF eFEX " << thisEFEX << " :");
        for (int thisRow=rows-1; thisRow>=0; thisRow--){
          for (int thisCol=0; thisCol<cols; thisCol++){
            int tmptowerid = tmp_eTowersIDs_subset[thisRow][thisCol];
            const float tmptowereta = this_eTowerContainer->findTower(tmptowerid)->eta();
            const float tmptowerphi = this_eTowerContainer->findTower(tmptowerid)->phi();
            if(thisCol != cols-1){ ATH_MSG_DEBUG("|  " << tmptowerid << "([" << tmptowerphi << "][" << tmptowereta << "])  "); }
            else { ATH_MSG_DEBUG("|  " << tmptowerid << "([" << tmptowereta << "][" << tmptowerphi << "])  |"); }
          }
        }
      }


      m_eFEXSimTool->init(thisEFEX);
      ATH_CHECK(m_eFEXSimTool->NewExecute(tmp_eTowersIDs_subset, inputOutputCollection));
      m_allEmTobObjects.insert( std::map<int, std::vector<std::unique_ptr<eFEXegTOB>> >::value_type(thisEFEX, (m_eFEXSimTool->getEmTOBs() ) ));
      m_allTauHeuristicTobObjects.insert( std::map<int, std::vector<std::unique_ptr<eFEXtauTOB>> >::value_type(thisEFEX, (m_eFEXSimTool->getTauHeuristicTOBs() ) ));
      m_allTauBDTTobObjects.insert( std::map<int, std::vector<std::unique_ptr<eFEXtauTOB>> >::value_type(thisEFEX, (m_eFEXSimTool->getTauBDTTOBs() ) ));
      m_eFEXSimTool->reset();

      fexcounter++;
    }

    // CENTRAL EFEXs
    // DO THE CENTRAL EFEXs SECOND
    fexcounter = 0;
    int embnegEta = 8; int embnegPhi = 1; int embnegMod = 100000;
    int initialEMB_neg = calcTowerID(embnegEta,embnegPhi,embnegMod); //100513;
    int embposEta = 0; int embposPhi = 1; int embposMod = 200000;
    int initialEMB_pos = calcTowerID(embposEta,embposPhi,embposMod); //200001;
    int eFEXb = 1;

    for (int thisEFEX=eFEXb; thisEFEX<=22; thisEFEX+=3){

      if(fexcounter > 0){  initialEMB_neg += 8; initialEMB_pos += 8; }
      
      // decide which subset of towers (and therefore supercells) should go to the eFEX
      std::map<int,eTower> tmp_eTowersColl_subset;
      
      // doing this with an array initially just containing tower IDs.
      int tmp_eTowersIDs_subset [10][18];

      int rows = sizeof tmp_eTowersIDs_subset / sizeof tmp_eTowersIDs_subset[0];
      int cols = sizeof tmp_eTowersIDs_subset[0] / sizeof tmp_eTowersIDs_subset[0][0];

      // set the EMB part
      for(int thisCol = 0; thisCol < cols; thisCol++){
        for(int thisRow=0; thisRow<rows; thisRow++){
	  int towerid = -1;

	  int tmp_initEMB = initialEMB_neg;

	  if(thisCol < 9){
	    towerid = tmp_initEMB - ( (thisCol) * 64) + thisRow;
	  }
	  else{
	    tmp_initEMB = initialEMB_pos;
	    towerid = tmp_initEMB + ( (thisCol-9) * 64) + thisRow;
	  }

	  if( (thisEFEX == 22) && (thisRow >= 7)){ towerid -= 64; };

          tmp_eTowersIDs_subset[thisRow][thisCol] = towerid;

          tmp_eTowersColl_subset.insert( std::map<int, eTower>::value_type(towerid,  *(this_eTowerContainer->findTower(towerid))));
        }
      }
      

      if(false){
	      ATH_MSG_DEBUG("CONTENTS OF eFEX " << thisEFEX << " :");
        for (int thisRow=rows-1; thisRow>=0; thisRow--){
          for (int thisCol=0; thisCol<cols; thisCol++){
            int tmptowerid = tmp_eTowersIDs_subset[thisRow][thisCol];
            const float tmptowereta = this_eTowerContainer->findTower(tmptowerid)->eta();
            const float tmptowerphi = this_eTowerContainer->findTower(tmptowerid)->phi();
            if(thisCol != cols-1){ ATH_MSG_DEBUG("|  " << tmptowerid << "([" << tmptowereta << "][" << tmptowerphi << "])  "); }
            else { ATH_MSG_DEBUG("|  " << tmptowerid << "([" << tmptowereta << "][" << tmptowerphi << "])  |"); }
          }
        }
      }

      //tool use instead
      m_eFEXSimTool->init(thisEFEX);
      ATH_CHECK(m_eFEXSimTool->NewExecute(tmp_eTowersIDs_subset, inputOutputCollection));
      m_allEmTobObjects.insert( std::map<int, std::vector<std::unique_ptr<eFEXegTOB>> >::value_type(thisEFEX, (m_eFEXSimTool->getEmTOBs() ) ));
      m_allTauHeuristicTobObjects.insert( std::map<int, std::vector<std::unique_ptr<eFEXtauTOB>> >::value_type(thisEFEX, (m_eFEXSimTool->getTauHeuristicTOBs() ) ));
      m_allTauBDTTobObjects.insert( std::map<int, std::vector<std::unique_ptr<eFEXtauTOB>> >::value_type(thisEFEX, (m_eFEXSimTool->getTauBDTTOBs() ) ));
      m_eFEXSimTool->reset();

      fexcounter++;
    }

    // POSITIVE EFEXs
    // LET'S DO THE RIGHT-MOST (POSTITIVE ETA) EFEXs THIRD
    fexcounter = 0;
    // Example values for eFEX 0
    emecEta = 15; emecPhi = 1; emecMod = 600000;
    initialEMEC = calcTowerID(emecEta,emecPhi,emecMod); //600961;
    transEta = 14; transPhi = 1; transMod = 400000;
    initialTRANS = calcTowerID(transEta,transPhi,transMod); //400897;
    embEta = 7; embPhi = 1; embMod = 200000;
    initialEMB = calcTowerID(embEta,embPhi,embMod); //200449;
    int eFEXc = 2;

    for (int thisEFEX=eFEXc; thisEFEX<=23; thisEFEX+=3){

      if(fexcounter > 0){ initialEMEC += 8; initialTRANS += 8; initialEMB += 8; }

      // decide which subset of towers (and therefore supercells) should go to the eFEX
      std::map<int,eTower> tmp_eTowersColl_subset;

      // doing this with an array initially just containing tower IDs.
      int tmp_eTowersIDs_subset [10][18];

      int rows = sizeof tmp_eTowersIDs_subset / sizeof tmp_eTowersIDs_subset[0];
      int cols = sizeof tmp_eTowersIDs_subset[0] / sizeof tmp_eTowersIDs_subset[0][0];

      // set the EMB part
      for(int thisCol = 0; thisCol < 7; thisCol++){
        for(int thisRow=0; thisRow<rows; thisRow++){
          int towerid = initialEMB + ( (thisCol) * 64) + thisRow;

	  if( (thisEFEX == 23) && (thisRow >= 7)){ towerid -= 64; };

          tmp_eTowersIDs_subset[thisRow][thisCol] = towerid;
          tmp_eTowersColl_subset.insert( std::map<int, eTower>::value_type(towerid,  *(this_eTowerContainer->findTower(towerid))));
        }
      }
      // set the TRANS part
      for(int thisRow = 0; thisRow < rows; thisRow++){
        int towerid = initialTRANS + thisRow;

	if( (thisEFEX == 23) && (thisRow >= 7)){ towerid -= 64; };

        tmp_eTowersIDs_subset[thisRow][7] = towerid;
        tmp_eTowersColl_subset.insert( std::map<int, eTower>::value_type(towerid,  *(this_eTowerContainer->findTower(towerid))));
      }
      // set the EMEC part
      for(int thisCol=8; thisCol<cols; thisCol++){
        for(int thisRow=0; thisRow<rows; thisRow++){
          int towerid = initialEMEC + ( (thisCol-8) * 64) + thisRow;

	  if( (thisEFEX == 23) && (thisRow >= 7)){ towerid -= 64; };

          tmp_eTowersIDs_subset[thisRow][thisCol] = towerid;
          tmp_eTowersColl_subset.insert( std::map<int, eTower>::value_type(towerid,  *(this_eTowerContainer->findTower(towerid))));
        }
      }

      // Debug printout
      if(false){
	ATH_MSG_DEBUG("CONTENTS OF eFEX " << thisEFEX << " :");
	for (int thisRow=rows-1; thisRow>=0; thisRow--){
	  for (int thisCol=0; thisCol<cols; thisCol++){
	    int tmptowerid = tmp_eTowersIDs_subset[thisRow][thisCol];
	    const float tmptowereta = this_eTowerContainer->findTower(tmptowerid)->eta();
	    const float tmptowerphi = this_eTowerContainer->findTower(tmptowerid)->phi();
	    if(thisCol != cols-1){ ATH_MSG_DEBUG("|  " << tmptowerid << "([" << tmptowereta << "][" << tmptowerphi << "])  "); }
	    else { ATH_MSG_DEBUG("|  " << tmptowerid << "([" << tmptowereta << "][" << tmptowerphi << "])  |"); }
	  }
	}
      }

      //tool use instead
      m_eFEXSimTool->init(thisEFEX);
      ATH_CHECK(m_eFEXSimTool->NewExecute(tmp_eTowersIDs_subset, inputOutputCollection));
      m_allEmTobObjects.insert( std::map<int, std::vector<std::unique_ptr<eFEXegTOB>> >::value_type(thisEFEX, (m_eFEXSimTool->getEmTOBs() ) ));
      m_allTauHeuristicTobObjects.insert( std::map<int, std::vector<std::unique_ptr<eFEXtauTOB>> >::value_type(thisEFEX, (m_eFEXSimTool->getTauHeuristicTOBs() ) ));
      m_allTauBDTTobObjects.insert( std::map<int, std::vector<std::unique_ptr<eFEXtauTOB>> >::value_type(thisEFEX, (m_eFEXSimTool->getTauBDTTOBs() ) ));
      m_eFEXSimTool->reset();

      fexcounter++;
    }
    
    }//close the non-csv loop over eFEXes

    // EM TOBs and xTOBs

    // TOB Containers
    m_eContainer = std::make_unique<xAOD::eFexEMRoIContainer> ();
    m_eAuxContainer = std::make_unique<xAOD::eFexEMRoIAuxContainer> ();
    m_eContainer->setStore(m_eAuxContainer.get());

    // xTOB Containers
    m_xeContainer = std::make_unique<xAOD::eFexEMRoIContainer> ();
    m_xeAuxContainer = std::make_unique<xAOD::eFexEMRoIAuxContainer> ();
    m_xeContainer->setStore(m_xeAuxContainer.get());

    // iterate over all Em Tobs and fill xTOB EDM with them
    for( auto const& [efex, tobObjects] : m_allEmTobObjects ){
      for(auto &tobObject : tobObjects){
        m_eFEXFillEDMTool->fillEmEDM(m_xeContainer, efex, tobObject, true);
      }
    }

    // Form list of TOBs, sorted and truncated from each eFEX
    // Vector to store sorted TOBs from all eFEXes
    // Note that this step means moving TOBs from the all objects vector, which is why we do it last
    std::vector<std::unique_ptr<eFEXegTOB>> emTOBs;
    // Loop through eFEXes and sort TOBs from each
    auto iter = m_allEmTobObjects.begin();
    while (iter != m_allEmTobObjects.end()) {
        std::vector<std::unique_ptr<eFEXegTOB>> tobsSort = std::move(iter->second);
        // sort tobs by their et (last 12 bits of the 32 bit tob word)
        std::sort (tobsSort.begin(), tobsSort.end(), std::bind(TOBetSort<std::unique_ptr<eFEXegTOB>>, std::placeholders::_1, std::placeholders::_2, false));
        // Truncate at 6 TOBs per eFEX
        if (tobsSort.size() > 6) tobsSort.resize(6);
        // Append to system TOB list
        for (unsigned int t = 0; t < tobsSort.size(); ++t) emTOBs.push_back(std::move(tobsSort[t]));
        // Next eFEX
        ++iter;
    }

    // iterate over sorted eFEX EM TOBs and fill TOB EDM with them
    for(auto &tobObject : emTOBs){
        int efex = tobObject->geteFEXID();
        m_eFEXFillEDMTool->fillEmEDM(m_eContainer, efex, tobObject);
    }

    // Match xTOBs to TOBs and set isTOB flags if matched
    matchTOBs(m_eContainer, m_xeContainer);
   
    // Record EDMs in StoreGate   
    SG::WriteHandle<xAOD::eFexEMRoIContainer> outputeFexEMxTOBHandle(m_eFexEMxTOBOutKey/*, ctx*/);
    ATH_MSG_DEBUG("  write: " << outputeFexEMxTOBHandle.key() << " = " << "..." );
    ATH_CHECK(outputeFexEMxTOBHandle.record(std::move(m_xeContainer),std::move(m_xeAuxContainer)));

    SG::WriteHandle<xAOD::eFexEMRoIContainer> outputeFexHandle(m_eFexOutKey/*, ctx*/);
    ATH_MSG_DEBUG("  write: " << outputeFexHandle.key() << " = " << "..." );
    ATH_CHECK(outputeFexHandle.record(std::move(m_eContainer),std::move(m_eAuxContainer)));

    SG::ReadHandle<TrigConf::L1Menu> l1Menu (m_l1MenuKey/*, ctx*/);
    ATH_CHECK(l1Menu.isValid());

    auto & thr_eTAU = l1Menu->thrExtraInfo().eTAU();
    int activeAlgo = thr_eTAU.algoVersion() == 0 ? xAOD::eFexTauRoI_v1::Heuristic : xAOD::eFexTauRoI_v1::BDT;
    bool omitAltTauContainer = m_eFexTauAltxTOBOutKey.empty() || m_eFexTauAltOutKey.empty();

    // Repeat for Tau TOBs and xTOBs
    if (activeAlgo == xAOD::eFexTauRoI_v1::Heuristic) {
        ATH_CHECK(StoreTauTOBs(m_allTauHeuristicTobObjects, m_eFexTauActivexTOBOutKey, m_eFexTauActiveOutKey));
        if (!omitAltTauContainer) {
            ATH_CHECK(StoreTauTOBs(m_allTauBDTTobObjects, m_eFexTauAltxTOBOutKey, m_eFexTauAltOutKey));
        }
    } else if (activeAlgo == xAOD::eFexTauRoI_v1::BDT) {
        ATH_CHECK(StoreTauTOBs(m_allTauBDTTobObjects, m_eFexTauActivexTOBOutKey, m_eFexTauActiveOutKey));
        if (!omitAltTauContainer) {
            ATH_CHECK(StoreTauTOBs(m_allTauHeuristicTobObjects, m_eFexTauAltxTOBOutKey, m_eFexTauAltOutKey));
        }
    }

    //Send TOBs to bytestream?
    // ToDo
    // To implement
    // {--Implement--}

    return StatusCode::SUCCESS;

  }

 StatusCode eFEXSysSim::StoreTauTOBs(std::map<int, std::vector<std::unique_ptr<eFEXtauTOB>> >& allTauTobObjects,
		 SG::WriteHandleKey< xAOD::eFexTauRoIContainer >& eFexTauxTOBOutKey,
		 SG::WriteHandleKey< xAOD::eFexTauRoIContainer >& eFexTauOutKey)
 {
    std::unique_ptr< xAOD::eFexTauRoIContainer > tauContainer;
    std::unique_ptr< xAOD::eFexTauRoIAuxContainer > tauAuxContainer;
    std::unique_ptr< xAOD::eFexTauRoIContainer > xtauContainer;
    std::unique_ptr< xAOD::eFexTauRoIAuxContainer > xtauAuxContainer;

    tauContainer = std::make_unique<xAOD::eFexTauRoIContainer> ();
    tauAuxContainer = std::make_unique<xAOD::eFexTauRoIAuxContainer> ();
    tauContainer->setStore(tauAuxContainer.get());

    xtauContainer = std::make_unique<xAOD::eFexTauRoIContainer> ();
    xtauAuxContainer = std::make_unique<xAOD::eFexTauRoIAuxContainer> ();
    xtauContainer->setStore(xtauAuxContainer.get());

    // iterate over all tau TOBs and fill xTOB EDM with them
    for( auto const& [efex, tobObjects] : allTauTobObjects ){
      for( auto &tobObject: tobObjects ){
        m_eFEXFillEDMTool->fillTauEDM(xtauContainer, efex, tobObject, true);
      }
    }

    // Form list of TOBs, sorted and truncated from each eFEX
    // Vector to store sorted TOBs from all eFEXes
    // Note that this step means moving TOBs from the all objects vector, which is why we do it last
    std::vector<std::unique_ptr<eFEXtauTOB>> tauTOBs;
    // Loop through eFEXes and sort TOBs from each
    auto iterTau = allTauTobObjects.begin();
    while (iterTau != allTauTobObjects.end()) {
        std::vector<std::unique_ptr<eFEXtauTOB>> tobsSort = std::move(iterTau->second);
        // sort tobs by their et (last 12 bits of the 32 bit tob word)
        std::sort (tobsSort.begin(), tobsSort.end(), std::bind(TOBetSort<std::unique_ptr<eFEXtauTOB>>, std::placeholders::_1, std::placeholders::_2, true));
        // Truncate at 6 TOBs per eFEX
        if (tobsSort.size() > 6) tobsSort.resize(6);
        // Append to system TOB list
        for (unsigned int t = 0; t < tobsSort.size(); ++t) tauTOBs.push_back(std::move(tobsSort[t]));
        // Next eFEX
        ++iterTau;
    }

    // iterate over sorted eFEX Tau TOBs and fill TOB EDM with them
    for(auto &tobObject : tauTOBs){
        int efex = tobObject->geteFEXID();
        m_eFEXFillEDMTool->fillTauEDM(tauContainer, efex, tobObject);
    }

    // Match xTOBs and TOBs and set isTOB flags if matched
    matchTOBs(tauContainer, xtauContainer);

    // Record containers in StoreGate
    SG::WriteHandle<xAOD::eFexTauRoIContainer> outputeFexTauxTOBHandle(eFexTauxTOBOutKey/*, ctx*/);
    ATH_MSG_DEBUG(" write: " << outputeFexTauxTOBHandle.key() << " = " << "..." );
    ATH_CHECK(outputeFexTauxTOBHandle.record(std::move(xtauContainer), std::move(xtauAuxContainer)));

    SG::WriteHandle<xAOD::eFexTauRoIContainer> outputeFexTauHandle(eFexTauOutKey/*, ctx*/);
    ATH_MSG_DEBUG(" write: " << outputeFexTauHandle.key() << " = " << "..." );
    ATH_CHECK(outputeFexTauHandle.record(std::move(tauContainer), std::move(tauAuxContainer)));

    return StatusCode::SUCCESS;

  }
} // end of namespace bracket
