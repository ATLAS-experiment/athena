/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODTrigL1Calo/eFexTowerContainer.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"

#include "L1CaloFEXSim/eTower.h"
#include "L1CaloFEXSim/eTowerBuilder.h"
#include "L1CaloFEXSim/eTowerContainer.h"
#include "./eTowerMakerFromEfexTowers.h"
#include "L1CaloFEXSim/eFEXCompression.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"


namespace LVL1 {

  eTowerMakerFromEfexTowers::eTowerMakerFromEfexTowers(const std::string& name, ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  { 
  
  }


StatusCode eTowerMakerFromEfexTowers::initialize()
{
  ATH_CHECK( m_eTowerBuilderTool.retrieve() );
  ATH_CHECK( m_eFexTowerContainerSGKey.initialize() );
  ATH_CHECK( m_eFexTowerContainer2SGKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_eTowerContainerSGKey.initialize() );
  ATH_CHECK( m_noiseCutsKey.initialize(SG::AllowEmpty) );

  if (!m_noiseCutsKey.empty()) {
      // disable built-in noise cuts because will be loading them from conditions db instead
      eFEXCompression::s_disableNoiseCuts = true;
  }

  return StatusCode::SUCCESS;
}


  StatusCode eTowerMakerFromEfexTowers::execute(const EventContext& ctx) const
{
  // load noise cuts .. should really only need to do this at start of runs, not every event!
  std::map<std::pair<int, int>, int> noiseCutsMap; // key is [eta,layer]
  bool useHardcodedCuts = false;
  if(!m_noiseCutsKey.empty()) {
      // check timestamp of event is not *before* date when started using database
      if (ctx.eventID().time_stamp() < m_noiseCutBeginTimestamp) {
        useHardcodedCuts = true;
      } else {
        SG::ReadCondHandle <CondAttrListCollection> noiseCuts{m_noiseCutsKey, ctx};
        if (noiseCuts.isValid()) {
            if(msgLvl(MSG::DEBUG) && !m_printedNoiseCuts) {
                m_printedNoiseCuts = true;
                ATH_MSG_DEBUG("DB Noise cuts are:");
                noiseCuts->dump();
            }
            for (auto itr = noiseCuts->begin(); itr != noiseCuts->end(); ++itr) {
                if (itr->first >= 50) continue;
                noiseCutsMap[std::pair(itr->first, 0)] = itr->second["EmPS"].data<int>();
                noiseCutsMap[std::pair(itr->first, 1)] = itr->second["EmFR"].data<int>();
                noiseCutsMap[std::pair(itr->first, 2)] = itr->second["EmMD"].data<int>();
                noiseCutsMap[std::pair(itr->first, 3)] = itr->second["EmBK"].data<int>();
                noiseCutsMap[std::pair(itr->first, 4)] = (itr->first >= 10 && itr->first < 40)
                                                        ? itr->second["Tile"].data<int>()
                                                        : itr->second["HEC"].data<int>();
            }
        }
      }
  }

  // STEP 0 - Make a fresh local eTowerContainer
  std::unique_ptr<eTowerContainer> local_eTowerContainerRaw = std::make_unique<eTowerContainer>();

  // STEP 1 - Make some eTowers and fill the local container
  m_eTowerBuilderTool->init(local_eTowerContainerRaw);
  local_eTowerContainerRaw->clearContainerMap();
  local_eTowerContainerRaw->fillContainerMap();

  SG::ReadHandle<xAOD::eFexTowerContainer> eFexTowers(m_eFexTowerContainerSGKey, ctx);
  if((!eFexTowers.isValid() || eFexTowers->size() < m_minTowersRequired) && !m_eFexTowerContainer2SGKey.empty()) {
    eFexTowers = SG::ReadHandle<xAOD::eFexTowerContainer>(m_eFexTowerContainer2SGKey, ctx);
      // removing this to avoid breaking frozen tier0 policy
      // bug keeping commented out until sure we've replaced with a good alternative
      //const xAOD::EventInfo* ei = nullptr;
      //CHECK( evtStore()->retrieve(ei) );
      //ei->auxdecor<bool>("eTowerMakerFromEfexTowers_usedSecondary") = true;
  }

  // STEP 2 - Do the efexTower-tower mapping - put this information into the eTowerContainer
  for(auto eFexTower : *eFexTowers) {
      // need to ensure this eFexTower is a "core" tower in a module ... so that there aren't disconnected inputs
      // and also need to only do one tower per location, of course
      auto tower = local_eTowerContainerRaw->findTower(eFexTower->eFEXtowerID());
      auto counts = eFexTower->et_count();
      for(size_t i=0;i<counts.size();i++) {
          if (eFexTower->disconnectedCount(i)) continue;
          if (counts.at(i)==0 || counts.at(i)>1020) continue; // disconnected or masked channel
          // special case logic for reordering |eta|=2.5 and overlap
          // and l1 1.8-2.0 ... need to put the merged sc counts into slots that wont be split
          int layer; int cell=i;
          if(i<1 || (i==4 && std::abs(eFexTower->eta()+0.025)>2.4)) {layer = 0;cell=0;}
          else if(i<5) layer = 1;
          else if(i<9) layer = 2;
          else if(i<10) layer = 3;
          else layer = 4;

          // apply noise cut ... for runs up to 14th April 2023 was killing with <, then from run 449180 onwards kills with <=
          // since long-term behaviour is latter, will use that
          //if(useHardcodedCuts && !eFEXCompression::noiseCut(counts.at(i),layer,true)) continue;
          if(!useHardcodedCuts && counts.at(i) <= noiseCutsMap[std::pair( int( (eFexTower->eta() + 2.525)/0.1 ), layer)]) continue;

          // checking we haven't already filled this tower (happens when using efexDataTowers ... multiple towers per loc for different modules)
          // this is ugly as ...
          if(!tower->getET_float(layer,cell-(layer==1)*1-(layer==2)*5-(layer==3)*9-(layer==4)*10)) {
              // if tile then count is in steps of 500 MeV, not in latome multilinear encoding
              bool isTile = (std::abs(eFexTower->eta()+0.025)<1.5 && layer==4);
              tower->setET(cell,isTile ? (counts.at(i)*500.) : eFEXCompression::expand(counts.at(i)),layer + isTile, useHardcodedCuts);
          }
      }
  }



    // STEP 3 - Write the completed eTowerContainer into StoreGate (move the local copy in memory)
  SG::WriteHandle<LVL1::eTowerContainer> eTowerContainerSG(m_eTowerContainerSGKey, ctx);
  ATH_CHECK(eTowerContainerSG.record(std::move(/*my_eTowerContainerRaw*/local_eTowerContainerRaw)));

  // STEP 4 - Close and clean the event
  m_eTowerBuilderTool->reset();

  return StatusCode::SUCCESS;
}

  

} // end of LVL1 namespace
