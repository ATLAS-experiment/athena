/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//*****************************************************************************
//  Filename : TileCosmicsTrigger.cxx
//  Author   : jose.maneira@cern.ch
//  Created  : Nov 2005
//  Reviewed : March 2007 remove std.alone ntp, output to TileEvent/TileTrigger
//            May   2007 set 8 boards
//
//  DESCRIPTION:
//     Implement the algorithm
//
//  HISTORY:
//
//  BUGS:
//
//*****************************************************************************

//TileCalo include
#include "TileCosmicAlgs/TileCosmicsTrigger.h"
#include "TileIdentifier/TileTTL1Hash.h"

// Calo include
#include "CaloIdentifier/CaloLVL1_ID.h" 

// Athena includes
#include "AthenaKernel/errorcheck.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

//Gaudi includes
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"

const int TileCosmicsTrigger::m_NMaxTowers = NMAXTOWERS;

TileCosmicsTrigger::TileCosmicsTrigger(const std::string& name, ISvcLocator* pSvcLocator)
    : AthAlgorithm(name, pSvcLocator)
{
  for (int iboard = 0; iboard < m_NBOARDS; iboard++) {
    m_connectedDrawers[iboard].resize(m_NDRAWERSPERBOARD);
  }
}

TileCosmicsTrigger::~TileCosmicsTrigger() {
}

//****************************************************************************
//* Initialization
//****************************************************************************

StatusCode TileCosmicsTrigger::initialize() {


// retrieve TileID helper from det store

  CHECK( detStore()->retrieve(m_TT_ID) );


// convert string input into vector

  m_TTHash = new TileTTL1Hash();
  m_TTHash->initialize(m_TT_ID);
  for (int i = 0; i < m_NMaxTowers; i++)
    m_connectedTowers[i] = 0;

  for (int idrawer = 0; idrawer < m_NDRAWERSPERBOARD; idrawer++) {
    m_connectedDrawers[0][idrawer] = m_connectedDrawers1.value()[idrawer];
    m_connectedDrawers[1][idrawer] = m_connectedDrawers2.value()[idrawer];
    m_connectedDrawers[2][idrawer] = m_connectedDrawers3.value()[idrawer];
    m_connectedDrawers[3][idrawer] = m_connectedDrawers4.value()[idrawer];
    m_connectedDrawers[4][idrawer] = m_connectedDrawers5.value()[idrawer];
    m_connectedDrawers[5][idrawer] = m_connectedDrawers6.value()[idrawer];
    m_connectedDrawers[6][idrawer] = m_connectedDrawers7.value()[idrawer];
    m_connectedDrawers[7][idrawer] = m_connectedDrawers8.value()[idrawer];
  }
  for (int iboard = 0; iboard < m_NBOARDS; iboard++) {
    for (int idrawer = 0; idrawer < m_NDRAWERSPERBOARD; idrawer++) {
      if (m_connectedDrawers[iboard][idrawer] == "") continue;
      bool failure = 0;
      int section = 0;
      int side = 0;
      // get section (long or extended barrel)
      if (m_connectedDrawers[iboard][idrawer][0] == 'L'
          || m_connectedDrawers[iboard][idrawer][0] == 'l') {
        section = 1;
      } else if (m_connectedDrawers[iboard][idrawer][0] == 'E'
                || m_connectedDrawers[iboard][idrawer][0] == 'e') {
        section = 2;
      } else {
        failure = 1;
      }
      // get side (positive or negative)
      if (m_connectedDrawers[iboard][idrawer][2] == 'A'
          || m_connectedDrawers[iboard][idrawer][2] == 'a') {
        side = 1;
      } else if (m_connectedDrawers[iboard][idrawer][2] == 'C'
                 || m_connectedDrawers[iboard][idrawer][2] == 'c') {
        side = -1;
      } else {
        failure = 1;
      }
      int phi = -1 + atoi(m_connectedDrawers[iboard][idrawer].substr(3, m_connectedDrawers[iboard][idrawer].length() - 3).c_str());

      if (phi < 0 || phi > 63) failure = 1;

      if (failure) {
        ATH_MSG_ERROR( "Invalid Tile Drawer " << m_connectedDrawers[iboard][idrawer] );
        return StatusCode::FAILURE;
      }
      int mineta, maxeta;
      if (section == 1) {
        mineta = 0;
        maxeta = 7;
      } else {
        mineta = 9;
        maxeta = 14;
      }
      ATH_MSG_DEBUG( "Board " << iboard
                    << " Drawer " << idrawer
                    << " " << m_connectedDrawers[iboard][idrawer]
                    << " Section " << section
                    << " Side " << side
                    << " Phi " << phi );

      for (int ieta = mineta; ieta <= maxeta; ieta++) {
        Identifier ttid = m_TT_ID->tower_id(side, 1, 0, ieta, phi);
        int index = m_TTHash->operator()(m_TT_ID->tower_hash(ttid));
        m_connectedTowers[index] = 1;
        m_boardOfTower[index] = iboard;
      }
    }
  }

  int nt = 0;
  for (int i = 0; i < m_NMaxTowers; i++) {
    ATH_MSG_DEBUG( "tower " << i << " connected " << m_connectedTowers[i] );
    if (m_connectedTowers[i] == 1) nt++;
  }

  ATH_CHECK( m_ttl1ContainerKey.initialize() );
  ATH_CHECK( m_triggerContainerKey.initialize() );

  ATH_MSG_INFO( "Initialization completed, number of towers " << nt );

  return StatusCode::SUCCESS;
}

//****************************************************************************
//* Execution
//****************************************************************************

StatusCode TileCosmicsTrigger::execute(const EventContext& ctx) {

  // step1: read TileTTL1s from TDS
  SG::ReadHandle<TileTTL1Container> ttl1Container;
  ATH_CHECK( ttl1Container.isValid() );
  ATH_MSG_DEBUG( "Loaded TileTTL1Container: " <<  ttl1Container.key() );

  SG::WriteHandle<TileTriggerContainer> triggerContainer (m_triggerContainerKey, ctx);
  ATH_CHECK( triggerContainer.record(std::make_unique<TileTriggerContainer>()) );

  // step2: reset towers 
  for (int i = 0; i < m_NMaxTowers; i++) {
    m_firedTowers[i] = 0;
    m_towerSum[i] = 0.;
  }

  int nbacktowers = 0;
  int nchan = 0;

  Identifier p_maxTowerID;
  double p_maxTowerSum = 0;
  std::vector<Identifier> p_boardTowerID;
  std::vector<double> p_boardTowerSum;
  std::vector<Identifier> p_backTowerID;
  std::vector<double> p_backTowerSum;
  p_boardTowerID.resize(m_NBOARDS);
  p_boardTowerSum.resize(m_NBOARDS);
  p_backTowerID.resize(NMAXTOWERS);
  p_backTowerSum.resize(NMAXTOWERS);

  // step3: check which towers fired the discriminator
  TileTTL1Container::const_iterator it = ttl1Container->begin();
  TileTTL1Container::const_iterator end = ttl1Container->end();

  for (; it != end; ++it) {
    const TileTTL1 * cinp = (*it);
    Identifier id = cinp->TTL1_ID();

    std::vector<float> samples = cinp->fsamples();
    samples.resize(1);

    ATH_MSG_VERBOSE( " ichan=" << nchan
                    << " " << m_TT_ID->pos_neg_z(id)
                    << " /"<< m_TT_ID->eta(id)
                    << " /" << m_TT_ID->phi(id)
                    << " peak =" << samples[0] );


    if (samples[0] > m_TThreshold) {
      int index = m_TTHash->operator()(m_TT_ID->tower_hash(id));
      if (m_connectedTowers[index]) {
        m_firedTowers[index] = 1;
        m_towerSum[index] = samples[0];
        if (samples[0] > p_maxTowerSum) {
          p_maxTowerSum = samples[0];
          p_maxTowerID = id;
        }
      }
    }

    nchan++;
    if (nchan >= m_NMaxTowers) {
      ATH_MSG_DEBUG( "Number of ttl1s exceeds maximum (" << m_NMaxTowers << "), ignore all the rest" );
      break;
    }
  }

  // step4: compute tower coincidence logic
  int phi1, phi2;
  phi1 = phi2 = 0;

  Identifier id1;
  Identifier id2;
  // register maximum tower in each board
  for (int i = 0; i < m_NMaxTowers; i++) {
    if (m_firedTowers[i] == 0) continue;
    id1 = m_TT_ID->tower_id(m_TTHash->identifier(i));
    if (m_towerSum[i] > p_boardTowerSum[m_boardOfTower[i]]) {
      id1 = m_TT_ID->tower_id(m_TTHash->identifier(i));
      p_boardTowerID[m_boardOfTower[i]] = id1;
      p_boardTowerSum[m_boardOfTower[i]] = m_towerSum[i];
    }
  }

  // loop through all tower pairs for back-to-back coincidence
  for (int i = 0; i < m_NMaxTowers - 1; i++) {
    if (m_firedTowers[i] == 0) continue;
    id1 = m_TT_ID->tower_id(m_TTHash->identifier(i));
    for (int j = i + 1; j < m_NMaxTowers; j++) {
      //  check back-to-back trigger
      if (m_firedTowers[j] == 0) continue;
      id2 = m_TT_ID->tower_id(m_TTHash->identifier(j));
      phi1 = m_TT_ID->phi(id1);
      phi2 = m_TT_ID->phi(id2);
      if ((std::max(phi1, phi2) - std::min(phi1, phi2)) != 32) continue;
      if (m_TT_ID->pos_neg_z(id1) == m_TT_ID->pos_neg_z(id2)) continue;
      p_backTowerID[nbacktowers] = id1;
      p_backTowerSum[nbacktowers] = m_towerSum[i];
      nbacktowers++;
      p_backTowerID[nbacktowers] = id2;
      p_backTowerSum[nbacktowers] = m_towerSum[j];
      nbacktowers++;
    }
  }

  p_backTowerID.resize(nbacktowers);
  p_backTowerSum.resize(nbacktowers);

  triggerContainer->push_back(std::make_unique<TileTrigger>(p_maxTowerID, p_maxTowerSum,
                                                            p_boardTowerID, p_boardTowerSum,
                                                            p_backTowerID, p_backTowerSum));

  ATH_MSG_DEBUG( "TileTriggerContainer registered successfully (" << triggerContainer.key() << ")" );
  // Execution completed.
  ATH_MSG_DEBUG( "execute() completed successfully" );

  return StatusCode::SUCCESS;
}

//****************************************************************************
//* Finalize
//****************************************************************************

StatusCode TileCosmicsTrigger::finalize() {

  ATH_MSG_INFO( "finalize() completed successfully" );

  return StatusCode::SUCCESS;
}

