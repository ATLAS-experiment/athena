/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file MuonD3PDMaker/src/MuonTruthHitsFillerTool.cxx
 * @author scott snyder <snyder@bnl.gov>, from code by Niels van Eldik.
 * @date Aug, 2013
 * @brief Fill truth hit information for muons.
 */

#include "MuonTruthHitsFillerTool.h"

#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "AthenaKernel/errorcheck.h"
#include "AtlasHepMC/GenParticle.h"
#include "TruthUtils/MagicNumbers.h"
#include <format>


using namespace Muon::MuonStationIndex;
namespace{
  constexpr int chIdxMMS = toInt(ChIndex::ChIndexMax);
  constexpr int chIdxMML = chIdxMMS + 1;
}

namespace D3PD {
/**
 * @brief Standard Gaudi tool constructor.
 * @param type The name of the tool type.
 * @param name The tool name.
 * @param parent The tool's Gaudi parent.
 */
MuonTruthHitsFillerTool::MuonTruthHitsFillerTool (const std::string& type,
                                                  const std::string& name,
                                                  const IInterface* parent)
  : Base (type, name, parent)
{
  declareProperty("PRD_TruthNames",          m_PRD_TruthNames);
  m_PRD_TruthNames.emplace_back("CSC_TruthMap");
  m_PRD_TruthNames.emplace_back("RPC_TruthMap");
  m_PRD_TruthNames.emplace_back("TGC_TruthMap");
  m_PRD_TruthNames.emplace_back("MDT_TruthMap");
  m_PRD_TruthNames.emplace_back("MM_TruthMap");
  m_PRD_TruthNames.emplace_back("STGC_TruthMap");


}


/**
 * @brief Standard Gaudi initialize method.
 */
StatusCode MuonTruthHitsFillerTool::initialize()
{
  ATH_CHECK(book());
  ATH_CHECK( m_idHelperSvc.retrieve() );
  return StatusCode::SUCCESS;
}


/**
 * @brief Book variables for this block.
 */
StatusCode MuonTruthHitsFillerTool::book() {
  

  ATH_CHECK( addVariable ("nprecLayers",     m_nprecLayers )  );
  ATH_CHECK( addVariable ("nphiLayers",      m_nphiLayers )  );
  ATH_CHECK( addVariable ("ntrigEtaLayers",  m_ntrigEtaLayers )  );

  m_nprecHitsPerChamberLayer.resize( toInt(ChIndex::ChIndexMax)+2);
  for( unsigned int i=0; i<toInt(ChIndex::ChIndexMax); ++i) {
    ATH_CHECK( addVariable ( std::format("nprecHits{:}", chName(static_cast<ChIndex>(i))),
                         m_nprecHitsPerChamberLayer[i] )  );
  }

  ATH_CHECK( addVariable("nprecHitsMMS", m_nprecHitsPerChamberLayer[chIdxMMS] )  );
  ATH_CHECK( addVariable("nprecHitsMML", m_nprecHitsPerChamberLayer[chIdxMML] )  );

  m_nphiHitsPerChamberLayer.resize(toInt(PhiIndex::PhiIndexMax));
  m_ntrigEtaHitsPerChamberLayer.resize(toInt(PhiIndex::PhiIndexMax));
  for( unsigned int i=0; i<toInt(PhiIndex::PhiIndexMax); ++i) {
    ATH_CHECK(addVariable(std::format("nphiHits{:}", phiName(static_cast<PhiIndex>(i))), m_nphiHitsPerChamberLayer[i]));
    ATH_CHECK(addVariable(std::format("ntrigEtaHits{:}", phiName(static_cast<PhiIndex>(i))), m_ntrigEtaHitsPerChamberLayer[i]));
  }

  return StatusCode::SUCCESS;
}


/**
 * @brief Fill one block --- type-safe version.
 */
StatusCode MuonTruthHitsFillerTool::fill (const TrackRecord& trackRecord)
{
  ATH_CHECK( fillHitCounts (HepMC::uniqueID(trackRecord)) );
  return StatusCode::SUCCESS;
}


/**
 * @brief Fill one block --- type-safe version.
 */
StatusCode MuonTruthHitsFillerTool::fill (const xAOD::TruthParticle& p)
{
  ATH_CHECK( fillHitCounts (HepMC::uniqueID(p)) );
  return StatusCode::SUCCESS;
}


StatusCode MuonTruthHitsFillerTool::fillHitCounts (int uniqueID)
{

  bool found = false;
  for (const std::string& key : m_PRD_TruthNames) {
    if (!evtStore()->contains<PRD_MultiTruthCollection>(key)) {
      ATH_MSG_DEBUG(  "PRD_MultiTruthCollection " << key << " NOT found");
      continue;
    }
    const PRD_MultiTruthCollection* collection = nullptr;
    ATH_CHECK( evtStore()->retrieve(collection, key) );
    for (const PRD_MultiTruthCollection::value_type& mc : *collection) {
      // check if gen particle same as input
      // TODO Here barcode is being used purely as a unique
      // identifier, so we can use HepMC::uniqueID once TrackRecord
      // and xAOD::TruthParticle and PRD_MultiTruthCollection support
      // it.
      if ( !HepMC::is_same_particle(mc.second,uniqueID)) continue;
      found = true;
      const Identifier& id = mc.first;
      ATH_MSG_VERBOSE("found matching hit " << m_idHelperSvc->toString(id) );
      bool measPhi   = m_idHelperSvc->measuresPhi(id);

      if( m_idHelperSvc->issTgc(id) ) {
        int index = toInt(m_idHelperSvc->phiIndex(id));
        if( measPhi ) ++*(m_nphiHitsPerChamberLayer[index]);
        else          ++*(m_ntrigEtaHitsPerChamberLayer[index]);
      }
      else if( m_idHelperSvc->isMM(id) ) {
        int index = m_idHelperSvc->isSmallChamber(id) ? chIdxMMS:  chIdxMML;
        ++*(m_nprecHitsPerChamberLayer[index]);
      }
      else if( m_idHelperSvc->isTrigger(id) ) {
        const int index = toInt(m_idHelperSvc->phiIndex(id));
        if( measPhi ) ++*(m_nphiHitsPerChamberLayer[index]);
        else          ++*(m_ntrigEtaHitsPerChamberLayer[index]);
      }
      else {
        if( measPhi ) {
          ++*(m_nphiHitsPerChamberLayer[toInt(m_idHelperSvc->phiIndex(id))]);
        }else{
          ++*(m_nprecHitsPerChamberLayer[toInt(m_idHelperSvc->chamberIndex(id))]);
        }
      }
    }
  }

  if( !found ) {
    ATH_MSG_VERBOSE("No hits associated to muon ");
  }
  else {
    // sum precision hits
    const std::vector<int*>& nprec = m_nprecHitsPerChamberLayer;
    const std::vector<int*>& nphi  = m_nphiHitsPerChamberLayer;
    const std::vector<int*>& ntrig = m_ntrigEtaHitsPerChamberLayer;

    if( *nprec[toInt(ChIndex::BIS)] + *nprec[toInt(ChIndex::BIL)] > 3 ) ++*m_nprecLayers;
    if( *nprec[toInt(ChIndex::BMS)] + *nprec[toInt(ChIndex::BML)] > 2 ) ++*m_nprecLayers;
    if( *nprec[toInt(ChIndex::BOS)] + *nprec[toInt(ChIndex::BOL)] > 2 ) ++*m_nprecLayers;
    if( *nprec[toInt(ChIndex::EIS)] + *nprec[toInt(ChIndex::EIL)] > 3 ) ++*m_nprecLayers;
    if( *nprec[toInt(ChIndex::EMS)] + *nprec[toInt(ChIndex::EML)] > 2 ) ++*m_nprecLayers;
    if( *nprec[toInt(ChIndex::EOS)] + *nprec[toInt(ChIndex::EOL)] > 2 ) ++*m_nprecLayers;
    if( *nprec[toInt(ChIndex::EES)] + *nprec[toInt(ChIndex::EEL)] > 3 ) ++*m_nprecLayers;
    if( *nprec[toInt(ChIndex::CSS)] + *nprec[toInt(ChIndex::CSL)] > 2 ) ++*m_nprecLayers;
    if( *nprec[toInt(ChIndex::BEE)] > 3 ) ++*m_nprecLayers;
    if( *nprec[chIdxMML] + *nprec[chIdxMMS] > 3 ) ++*m_nprecLayers;

    if( *nphi[toInt(PhiIndex::BM1)] > 0 )  ++*m_nphiLayers;
    if( *nphi[toInt(PhiIndex::BM2)] > 0 )  ++*m_nphiLayers;
    if( *nphi[toInt(PhiIndex::BO1)] > 0 )  ++*m_nphiLayers;
    if( *nphi[toInt(PhiIndex::T1)]  > 0 )  ++*m_nphiLayers;
    if( *nphi[toInt(PhiIndex::T2)]  > 0 )  ++*m_nphiLayers;
    if( *nphi[toInt(PhiIndex::T3)]  > 0 )  ++*m_nphiLayers;
    if( *nphi[toInt(PhiIndex::T4)]  > 0 )  ++*m_nphiLayers;
    if( *nphi[toInt(PhiIndex::CSC)] > 2 )  ++*m_nphiLayers;
    if( *nphi[toInt(PhiIndex::STGC1)] + *nphi[toInt(PhiIndex::STGC2)] > 3 )  ++*m_nphiLayers;

    if( *ntrig[toInt(PhiIndex::BM1)] > 0 )  ++*m_ntrigEtaLayers;
    if( *ntrig[toInt(PhiIndex::BM2)] > 0 )  ++*m_ntrigEtaLayers;
    if( *ntrig[toInt(PhiIndex::BO1)] > 0 )  ++*m_ntrigEtaLayers;
    if( *ntrig[toInt(PhiIndex::T1)]  > 0 )  ++*m_ntrigEtaLayers;
    if( *ntrig[toInt(PhiIndex::T2)]  > 0 )  ++*m_ntrigEtaLayers;
    if( *ntrig[toInt(PhiIndex::T3)]  > 0 )  ++*m_ntrigEtaLayers;
    if( *ntrig[toInt(PhiIndex::T4)]  > 0 )  ++*m_ntrigEtaLayers;
    if( *ntrig[toInt(PhiIndex::CSC)] > 2 )  ++*m_ntrigEtaLayers;
    if( *ntrig[toInt(PhiIndex::STGC1)] +
        *ntrig[toInt(PhiIndex::STGC2)] > 3 )  ++*m_ntrigEtaLayers;

    ATH_MSG_DEBUG("Muon hits: prec " << *m_nprecLayers <<
                  " phi " << *m_nphiLayers
                  << " trig eta " << *m_ntrigEtaLayers );
  }
  return StatusCode::SUCCESS;
}


} // namespace D3PD
