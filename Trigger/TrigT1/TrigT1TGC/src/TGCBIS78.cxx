/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1TGC/TGCBIS78.h"
#include "TrigT1TGC/BIS78TrigOut.h"

#include "AthenaBaseComps/AthCheckMacros.h"


namespace LVL1TGC {

TGCBIS78::TGCBIS78()
: AthMessaging("LVL1TGC::TGCBIS78") {
  for (int PadBoard=0; PadBoard < kNPadBoards; PadBoard++) {
    m_buffer[PadBoard] = std::make_shared<BIS78TrigOut>(std::vector<int>{PadBoard});
  }
}


StatusCode TGCBIS78::retrieve(const SG::ReadHandleKey<Muon::RpcBis78_TrigRawDataContainer>& key,
                              const EventContext& ctx) {
  ATH_MSG_DEBUG("retrieve");

  this->eraseOutput();

  //The following part will be available when RPC BIS78 Trigger Output is available.
  SG::ReadHandle<Muon::RpcBis78_TrigRawDataContainer> readBIS78_TrigRawDataContainer(key, ctx);
  ATH_CHECK( readBIS78_TrigRawDataContainer.isValid() );

  for(const Muon::RpcBis78_TrigRawData* bis78_sector : *readBIS78_TrigRawDataContainer){
    if ( bis78_sector->sideId() != 1 ) continue; // BIS78 is only in A side!
    for(const Muon::RpcBis78_TrigRawDataSegment* bis78_hit : *bis78_sector){
      this->setOutput(bis78_sector->sectorId(),
                      bis78_hit->etaIndex(),      // Eta-index
                      bis78_hit->phiIndex(),    // Phi-index
                      bis78_hit->deltaEta(), // Delta eta
                      bis78_hit->deltaPhi(), // Delta phi
                      bis78_hit->flag3over3Eta(), //
                      bis78_hit->flag3over3Phi() );
    }
  }

  return StatusCode::SUCCESS;
}


std::shared_ptr<const BIS78TrigOut> TGCBIS78::getOutput(int TGC_TriggerSector) const
{
  auto trigBIS78_output = std::make_shared<BIS78TrigOut>();
  if ( TGC_TriggerSector<0 || TGC_TriggerSector>47 ) return 0;
  int BIS78_TriggerSect = (TGC_TriggerSector - 2)/6;
  if(TGC_TriggerSector < 2) BIS78_TriggerSect=7;
  if(BIS78_TriggerSect>=0 && BIS78_TriggerSect<=7){
      *trigBIS78_output+=*m_buffer[BIS78_TriggerSect];
  }

  return trigBIS78_output;
}

void TGCBIS78::setOutput(unsigned int BIS78PadBoard, uint8_t BIS78eta_6bit, uint8_t BIS78phi_6bit, uint8_t BIS78Deta_3bit, uint8_t BIS78Dphi_3bit, uint8_t BIS78flag3over3eta_1bit, uint8_t BIS78flag3over3phi_1bit) {

  if (BIS78PadBoard >= kNPadBoards) return;
  m_buffer[BIS78PadBoard]->setBIS78PadBoard(BIS78PadBoard);
  m_buffer[BIS78PadBoard]->setEta(BIS78eta_6bit);
  m_buffer[BIS78PadBoard]->setPhi(BIS78phi_6bit);
  m_buffer[BIS78PadBoard]->setDeta(BIS78Deta_3bit);
  m_buffer[BIS78PadBoard]->setDphi(BIS78Dphi_3bit);
  m_buffer[BIS78PadBoard]->setflag3over3eta(BIS78flag3over3eta_1bit);
  m_buffer[BIS78PadBoard]->setflag3over3phi(BIS78flag3over3phi_1bit);
}


void TGCBIS78::eraseOutput()
{
  for (size_t idx=0; idx < kNPadBoards; idx++){
    m_buffer[idx]->clear(); 
  }
}

void TGCBIS78::print() const
{
  for (size_t idx=0; idx < kNPadBoards; idx++){
    m_buffer[idx]->print(); 
  }
}
  

}   // end of namespace
