/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArSC2NtupleEB.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "Identifier/HWIdentifier.h"
#include "LArRawEvent/LArSCDigit.h"

LArSC2NtupleEB::LArSC2NtupleEB(const std::string& name, ISvcLocator* pSvcLocator):
  LArCond2NtupleBaseEB(name, pSvcLocator)
{
  m_isSC = true;
  m_ntTitle = "LAr SC Energies";
  m_ntpath  = "/NTUPLES/FILE1/LARSC";
}


StatusCode LArSC2NtupleEB::initialize()
{
  ATH_MSG_DEBUG( "in initialize" );

  ATH_CHECK( LArCond2NtupleBaseEB::initialize() );

  StatusCode sc = m_nt->addItem("IEvent",m_IEvent);
  if (sc!=StatusCode::SUCCESS) {
    ATH_MSG_ERROR( "addItem 'IEvent' failed" );
    return sc;
  }

  if(m_fillBCID){
    sc = m_nt->addItem("BCID",m_bcid);
    if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "addItem 'BCID' failed" );
      return sc;
    }
  }


  if(!m_sccontKey.empty()){ // SC_ET RawSCContainer
    sc = m_nt->addItem("energyVec_ET", m_SC, m_scNet, m_energyVec_ET);
    if (sc.isFailure()) { 
         ATH_MSG_ERROR( "addItem 'energyVec_ET' failed" );
         return sc;
      }

    sc = m_nt->addItem("bcidVec_ET", m_SC, m_scNet, m_bcidVec_ET);
    if (sc.isFailure()) {
         ATH_MSG_ERROR( "addItem 'bcidVec_ET_ID' failed" );
         return sc;
      }

    sc = m_nt->addItem("saturVec_ET", m_SC, m_scNet, m_saturVec_ET);
    if (sc.isFailure()) { 
         ATH_MSG_ERROR( "addItem 'saturVec_ET' failed" );
         return sc;
      }

    sc = m_nt->addItem("passVec_ET", m_SC, m_scNet, m_passVec_ET);
    if (sc.isFailure()) { 
         ATH_MSG_ERROR( "addItem 'passVec_ET' failed" );
         return sc;
      }
  }

  if(!m_reccontKey.empty()){ // SC Reco RawSCContainer
    sc = m_nt->addItem("energyVec_Reco", m_SC, m_recoNet, m_energyVec_Reco);
    if (sc.isFailure()) {
         ATH_MSG_ERROR( "addItem 'energyVec_Reco' failed" );
         return sc;
      }

    sc = m_nt->addItem("tauVec_Reco", m_SC, m_recoNet, m_tauVec_Reco);
    if (sc.isFailure()) {
         ATH_MSG_ERROR( "addItem 'tauVec_Reco' failed" );
         return sc;
      }

    sc = m_nt->addItem("bcidVec_Reco", m_SC, m_recoNet, m_bcidVec_Reco);
    if (sc.isFailure()) {
         ATH_MSG_ERROR( "addItem 'bcidVec_Reco' failed" );
         return sc;
      }

    sc = m_nt->addItem("passVec_Reco", m_SC, m_recoNet, m_passVec_Reco);
    if (sc.isFailure()) {
         ATH_MSG_ERROR( "addItem 'passVec_Reco' failed" );
         return sc;
      }

    sc = m_nt->addItem("saturVec_Reco", m_SC, m_recoNet, m_saturVec_Reco);
    if (sc.isFailure()) {
         ATH_MSG_ERROR( "addItem 'saturVec_Reco' failed" );
         return sc;
      }
  }

  if(!m_rawcontKey.empty()) {
    sc = m_nt->addItem("ROD_energy", m_SC,  16, m_ROD_energy);
    if (sc.isFailure()) {
         ATH_MSG_ERROR( "addItem 'ROD_energy' failed" );
         return sc;
      }

    sc = m_nt->addItem("ROD_time", m_SC,  16, m_ROD_time);
    if (sc.isFailure()) {
         ATH_MSG_ERROR( "addItem 'ROD_time' failed" );
         return sc;
      }
    sc = m_nt->addItem("ROD_id", m_SC,  16, m_ROD_id);
    if (sc.isFailure()) {
         ATH_MSG_ERROR( "addItem 'ROD_id' failed" );
         return sc;
      }


  }

  ATH_CHECK(m_sccontKey.initialize(!m_sccontKey.empty()) );
  ATH_CHECK(m_reccontKey.initialize(!m_reccontKey.empty()) );
  ATH_CHECK(m_rawcontKey.initialize(!m_rawcontKey.empty()) );
  ATH_CHECK(m_evtInfoKey.initialize() );
  ATH_CHECK(m_cablingKeyAdditional.initialize() );


  return StatusCode::SUCCESS;

}

StatusCode LArSC2NtupleEB::execute()
{

  StatusCode        sc;

  const EventContext& ctx = Gaudi::Hive::currentContext();

  ATH_MSG_DEBUG( "LArSC2NtupleEB in execute" );
  unsigned long long thisevent;
  unsigned long        thisbcid           = 0;

  SG::ReadHandle<xAOD::EventInfo>evt (m_evtInfoKey, ctx);
  thisevent           = evt->eventNumber();

  thisbcid           = evt->bcid();

  const LArRawSCContainer* etcontainer  = nullptr;
  if (!m_sccontKey.empty()) {
     SG::ReadHandle<LArRawSCContainer> hdlSc(m_sccontKey);

     if(!hdlSc.isValid()) {
       ATH_MSG_WARNING( "Unable to retrieve LArRawSCContainer with key " << m_sccontKey << " from DetectorStore. " );
       return StatusCode::SUCCESS;
     } else {
       ATH_MSG_DEBUG( "Got LArRAwSCContainer with key " << m_sccontKey.key() );
       etcontainer = &*hdlSc;
     }
  }

  const LArRawSCContainer* recocontainer   = nullptr;
  if (!m_reccontKey.empty()) {
     SG::ReadHandle<LArRawSCContainer> hdlReco(m_reccontKey);

     if(!hdlReco.isValid()) {
       ATH_MSG_WARNING( "Unable to retrieve LArRawSCContainer with key " << m_reccontKey << " from DetectorStore. " );
       return StatusCode::SUCCESS;
     } else {
       ATH_MSG_DEBUG( "Got LArRAwSCContainer with key " << m_reccontKey.key() );
       recocontainer = &*hdlReco;
     }
  }

  if(! (etcontainer || recocontainer)) {

       ATH_MSG_WARNING( "Do not have any containers to loop on. " );
       return StatusCode::SUCCESS;
  }

  const LArRawChannelContainer* rawcontainer   = nullptr;
  if (!m_rawcontKey.empty()) {
     SG::ReadHandle<LArRawChannelContainer> hdlRaw(m_rawcontKey);

     if(!hdlRaw.isValid()) {
       ATH_MSG_WARNING( "Unable to retrieve LArRawChannelContainer with key " << m_rawcontKey << " from DetectorStore. " );
       return StatusCode::SUCCESS;
     } else {
       ATH_MSG_DEBUG( "Got LArRAwChannelContainer with key " << m_rawcontKey.key() );
       rawcontainer = &*hdlRaw;
     }
  }

  unsigned cellCounter=0;

  if(m_fillBCID) m_bcid           = thisbcid;
  m_IEvent                 = thisevent;

  rawChanMap_t  rawChannelMap;
  rawSCMap_t  rawSuperCellMap;

  if(rawcontainer){
     for (const LArRawChannel& raw : *rawcontainer) {
       rawChannelMap.try_emplace( raw.channelID(), &raw );
    }

  }

  const LArOnOffIdMapping* cabling=nullptr;
  const LArOnOffIdMapping* cablingROD=nullptr;
  if(rawcontainer){
     SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{cablingKey()};
     cabling=*cablingHdl;
     if(!cabling) {
        ATH_MSG_ERROR( "Do not have cabling for SC!" );
        return StatusCode::FAILURE;
     }
     SG::ReadCondHandle<LArOnOffIdMapping> cablingHdlROD{m_cablingKeyAdditional};
     cablingROD=*cablingHdlROD;
     if(!cablingROD) {
        ATH_MSG_ERROR( "Do not have cabling for ROD!" );
        return StatusCode::FAILURE;
     }
  }


  const LArRawSCContainer *itercont = nullptr; 
  const LArRawSCContainer *secondcont = nullptr; 
  if(etcontainer) {
     itercont = etcontainer;
     if(recocontainer) {
        secondcont = recocontainer;
        auto cbiter=secondcont->cbegin();
        auto ceiter=secondcont->cend();
        for ( ; cbiter<ceiter; ++cbiter) {
          rawSuperCellMap.try_emplace( (*cbiter)->hardwareID(), *cbiter );
        }
     }
  } else {
     if(recocontainer) itercont = recocontainer;
  }


  auto cbsciter=itercont->cbegin();
  auto cesciter=itercont->cend();
  for( ; cbsciter<cesciter; ++cbsciter ){

     const LArRawSC *iterSC = *cbsciter;
     fillFromIdentifier(iterSC->hardwareID(), cellCounter);

     const LArRawSC* secondSC  = nullptr;
     if( secondcont ){ // find corresponding LArRawSC
        secondSC   = rawSuperCellMap[iterSC->hardwareID()];
        }

     unsigned int truenet = etcontainer ? m_scNet : m_recoNet;

     if(truenet > iterSC->energies().size()) truenet=iterSC->energies().size();
     for( unsigned i=0; i<truenet;++i){  
          if(fabs(iterSC->energies().at(i)) > m_eCut) { 
            if(etcontainer) {
               m_energyVec_ET[cellCounter][i]  = iterSC->energies().at(i);
               if(iterSC->bcids().size()) m_bcidVec_ET[cellCounter][i]  = iterSC->bcids().at(i);
               if(iterSC->passTauSelection().size()) m_passVec_ET[cellCounter][i]  = iterSC->passTauSelection().at(i);
               if(iterSC->satur().size()) m_saturVec_ET[cellCounter][i]  = iterSC->satur().at(i);
               m_scNet = truenet;
            } else {
               m_energyVec_Reco[cellCounter][i]  = iterSC->energies().at(i);
               if(m_energyVec_Reco[cellCounter][i] !=0) m_tauVec_Reco[cellCounter][i]  = iterSC->tauEnergies().at(i)/m_energyVec_Reco[cellCounter][i];
               m_passVec_Reco[cellCounter][i]  = iterSC->passTauSelection().at(i);
               m_saturVec_Reco[cellCounter][i]  = iterSC->satur().at(i);
               m_bcidVec_Reco[cellCounter][i]  = iterSC->bcids().at(i);
               m_recoNet = truenet;
            }
          } //m_eCut
     }

     if(etcontainer && recocontainer) {
        truenet = m_recoNet;
        if(truenet > secondSC->energies().size()) truenet=secondSC->energies().size();
        for( unsigned i=0; i<truenet;++i){  
          if(fabs(secondSC->energies().at(i)) > m_eCut) { 
            m_energyVec_Reco[cellCounter][i]  = secondSC->energies().at(i);
            if(m_energyVec_Reco[cellCounter][i] !=0) m_tauVec_Reco[cellCounter][i]  = secondSC->tauEnergies().at(i)/m_energyVec_Reco[cellCounter][i];
            m_passVec_Reco[cellCounter][i]  = secondSC->passTauSelection().at(i);
            m_saturVec_Reco[cellCounter][i]  = secondSC->satur().at(i);
            m_bcidVec_Reco[cellCounter][i]  = secondSC->bcids().at(i);
          }
        }
        m_recoNet = truenet;
     }


    if(rawcontainer) {
       fillRODEnergy(iterSC->hardwareID(), cellCounter, rawChannelMap, cabling, cablingROD);
    }

    cellCounter++;
  }// over iter+second container


  ATH_CHECK( ntupleSvc()->writeRecord(m_nt) );

  ATH_MSG_DEBUG( "LArSC2NtupleEB has finished, filled"<< cellCounter << " cells" );
  return StatusCode::SUCCESS;
}// end finalize-method.

void LArSC2NtupleEB::fillRODEnergy(HWIdentifier SCId, unsigned cell,
                       rawChanMap_t &rawChanMap, const LArOnOffIdMapping* cabling, const LArOnOffIdMapping* cablingROD)
{
 const Identifier offId = cabling->cnvToIdentifier(SCId);
 const std::vector<Identifier> cellIds = m_scidtool->superCellToOfflineID(offId);
 for(unsigned i=0; i<16; ++i) {
    m_ROD_energy[cell][i] = 0.;
    m_ROD_time[cell][i] =  0.;
    m_ROD_id[cell][i] =  0.;
 }
 for(unsigned i=0; i<cellIds.size(); ++i ) {
    const HWIdentifier hwcell=cablingROD->createSignalChannelID(cellIds[i]);
    if (hwcell.is_valid()  && (rawChanMap.count(hwcell) != 0) ) {
       m_ROD_energy[cell][i] = rawChanMap[hwcell]->energy();
       m_ROD_time[cell][i] = rawChanMap[hwcell]->time();
       m_ROD_id[cell][i] = rawChanMap[hwcell]->hardwareID().get_identifier32().get_compact();
    } else {
       ATH_MSG_DEBUG(i<<"-th cell invalid Id");
    }
 }

}
