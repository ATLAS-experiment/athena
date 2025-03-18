/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1TGC/TGCTMDB.h"

#include "AthenaBaseComps/AthCheckMacros.h"

namespace LVL1TGC {

TGCTMDB::TGCTMDB()
: AthMessaging("LVL1TGC::TGCTMDB")
{
  for (size_t side=0; side < TGCSide::kNSide; side++) {
    for (size_t mod=0; mod < kNTileModule; mod++) {
      m_buffer[side][mod] = std::make_shared<TGCTMDBOut>(side, mod);
    }
  }
}


StatusCode TGCTMDB::retrieve(const SG::ReadHandleKey<TileMuonReceiverContainer>& key,
                             const EventContext& ctx)
{
  ATH_MSG_DEBUG("fillTMDB");

  // clear TMDB
  this->eraseOutput();

  SG::ReadHandle<TileMuonReceiverContainer> readTileMuonReceiverContainer(key, ctx);
  ATH_CHECK( readTileMuonReceiverContainer.isValid() );

  // loop over all TileMuonReceiverObj in container
  TileMuonReceiverContainer::const_iterator tmItr = readTileMuonReceiverContainer->begin();

  const TileMuonReceiverObj * tmObj_Thresholds = *tmItr;
  if ( (tmObj_Thresholds->GetThresholds()).size() == 4) {
      float thresholds[4];
      for (size_t ip=0;ip<4;ip++){
        thresholds[ip] = (tmObj_Thresholds->GetThresholds()).at(ip);
      }
      ATH_MSG_DEBUG("thresholds[] :" << thresholds[0] << thresholds[1] << thresholds[2] << thresholds[3] );
      ATH_MSG_DEBUG("type of GetThreshold : " << typeid((tmObj_Thresholds->GetThresholds())).name()
                    << "  ID of GetThreshold : "
                    << tmObj_Thresholds->GetID() );
  }


  //clear tmobj_Threshols
  tmObj_Thresholds = 0;
  // m_id and decision , etc ... from
  ++tmItr;

  for ( ; tmItr != readTileMuonReceiverContainer->end(); ++tmItr) {

      const TileMuonReceiverObj * tmObj = *tmItr;
      // Tile Module
      unsigned int moduleID = static_cast<unsigned int>(tmObj->GetID());
      unsigned int sideID   = (moduleID & 0xf00) >> 8;
      unsigned int mod = (moduleID & 0x0ff);   // 0...63
      // TMDB decision
      bool tile2SL[4];
      //     [0]       [1]       [2]      [3]
      //     d5d6_hi   d5d6_lo   d6_hi    d6_lo
      for (size_t ip=0;ip<4;ip++){
        tile2SL[ip] = (tmObj->GetDecision()).at(ip);
      }
      //if ( moduleID < 300 || (moduleID > 363 && moduleID < 400) || moduleID > 463 ||
      if ( mod > 63 || (sideID !=3 && sideID !=4) ||
           ((tmObj->GetDecision()).size() != 4) ) {
        continue;
      } else {

        TGCSide side = TGCSide::kNSide;
        if (sideID == 3) {
          side = TGCSide::ASIDE;
        } else if (sideID == 4) {
          side = TGCSide::CSIDE;
        }
        // setOutput(side, mod, hit56, hit6) -> hit56, 6
        TGCTMDBOut::TileModuleHit hit56 = TGCTMDBOut::TM_NA;
        TGCTMDBOut::TileModuleHit hit6 = TGCTMDBOut::TM_NA;
        if (tile2SL[0] == true && tile2SL[1] == false) {
          hit56 = TGCTMDBOut::TM_HIGH;
        } else if (tile2SL[0] == false && tile2SL[1] == true) {
          hit56 = TGCTMDBOut::TM_LOW;
        } else if (tile2SL[0] == false && tile2SL[1] == false) {
          hit56 = TGCTMDBOut::TM_NOHIT;
        }

        if (tile2SL[2] == true && tile2SL[3] == false) {
          hit6 = TGCTMDBOut::TM_HIGH;
        } else if (tile2SL[2] == false && tile2SL[3] == true) {
          hit6 = TGCTMDBOut::TM_LOW;
        } else if (tile2SL[2] == false && tile2SL[3] == false) {
          hit6 = TGCTMDBOut::TM_NOHIT;
        }

        TGCTMDBOut::TileModuleHit prehit56 = this->getOutput(side, mod)->getHit56();
        TGCTMDBOut::TileModuleHit prehit6 = this->getOutput(side, mod)->getHit6();
        if(prehit56 != TGCTMDBOut::TM_NA && prehit56 > hit56) { hit56=prehit56; }
        if(prehit6 != TGCTMDBOut::TM_NA && prehit6 > hit6) { hit6=prehit6; }

        this->setOutput(side, mod, hit56, hit6);
      }
  }

  return StatusCode::SUCCESS;
}



std::shared_ptr<const TGCTMDBOut> TGCTMDB::getOutput(const TGCSide side, unsigned int mod) const {
  if (side >= TGCSide::kNSide) return 0;
  if (mod >= kNTileModule) return 0;
  return m_buffer[side][mod];
}

std::shared_ptr<const TGCTMDBOut> TGCTMDB::getOutput(const TGCSide side, int sector, unsigned int mod) const
{
  if (side >= TGCSide::kNSide) return 0;
  if ((sector<0)||(sector>47)) return 0;
  if (mod>3) return 0;
  int octant = sector / 6;
  int sec = sector % 6;
  int offset = 0;
  if      (sec==0) offset = -4;
  else if (sec==1) offset = -4; // same SL board as sec#0   
  else if (sec==2) offset =  0;
  else if (sec==3) offset =  0; // same SL board as sec#1
  else if (sec==4) offset =  2;
  else if (sec==5) offset =  2; // same SL board as sec#2
  int moduleID = (octant*(kNTileModule/8) + offset + kNTileModule) % kNTileModule;
  moduleID = (moduleID + mod) % kNTileModule;
  return m_buffer[side][moduleID];
}

void TGCTMDB::setOutput(const TGCSide side, const unsigned int module,
                        const TGCTMDBOut::TileModuleHit hit56,
                        const TGCTMDBOut::TileModuleHit hit6)
{
  if (module>=kNTileModule) return;
  m_buffer.at(side)[module]->setHit56(hit56);
  m_buffer.at(side)[module]->setHit6(hit6);
}

void TGCTMDB::eraseOutput() {
  for (auto &perside : m_buffer) {
    for (auto &mod : perside) mod->clear(); 
  }
}

int TGCTMDB::getInnerTileBits(const TGCSide side, int sector) const
{
  int inner_tile = 0;  

  for (int ii = 0; ii < 4; ii++) {
    TGCTMDBOut::TileModuleHit hit56 = getOutput(side, sector, ii)->getHit56();
    TGCTMDBOut::TileModuleHit hit6  = getOutput(side, sector, ii)->getHit6();

    int tmp_56 = (hit56 == TGCTMDBOut::TM_LOW || hit56 == TGCTMDBOut::TM_HIGH) ? 1 : 0;
    int tmp_6  = (hit6  == TGCTMDBOut::TM_LOW || hit6  == TGCTMDBOut::TM_HIGH) ? 1 : 0;
    
    int tmp_all = (tmp_6 << 1) | (tmp_56);

    inner_tile |= (tmp_all << (ii*2));
  }

  return inner_tile;
}

void TGCTMDB::print() const {
  for (auto &perside : m_buffer) {
    for (auto &mod : perside) mod->print();
  }
}
  

}   // end of namespace
