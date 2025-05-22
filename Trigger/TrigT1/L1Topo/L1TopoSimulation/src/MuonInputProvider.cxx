/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonInputProvider.h"

#include <math.h>

#include "L1TopoEvent/TopoInputEvent.h"
#include "L1TopoSimulationUtils/Conversions.h"
#include "TrigT1Interfaces/RecMuonRoI.h"
#include "TrigT1Interfaces/MuCTPIL1Topo.h"
#include "TrigT1Interfaces/MuCTPIL1TopoCandidate.h"

#include "TrigT1Result/MuCTPIRoI.h"
#include "TrigT1Result/Header.h"
#include "TrigT1Result/Trailer.h"

#include "TrigConfData/L1Menu.h"

#include "xAODTrigger/MuonRoI.h"

#include "TrigT1MuctpiBits/HelpersPhase1.h"

using namespace LVL1;
using namespace xAOD;

MuonInputProvider::MuonInputProvider( const std::string& type, const std::string& name, 
                                      const IInterface* parent) :
   base_class(type, name, parent)
{
   declareInterface<LVL1::IInputTOBConverter>( this );
}

StatusCode
MuonInputProvider::initialize() {

   // Get the RPC and TGC RecRoI tool
   ATH_CHECK( m_recRPCRoiTool.retrieve() );
   ATH_CHECK( m_recTGCRoiTool.retrieve() );

   //This is a bit ugly but I've done it so the job options can be used to determine 
   //use of storegate
   CHECK(m_MuCTPItoL1TopoLocation.initialize(!m_MuCTPItoL1TopoLocation.key().empty()));
   
   if(!m_MuCTPItoL1TopoLocationPlusOne.key().empty())
     {m_MuCTPItoL1TopoLocationPlusOne = m_MuCTPItoL1TopoLocation.key()+std::to_string(1);}
   
   CHECK(m_MuCTPItoL1TopoLocationPlusOne.initialize(!m_MuCTPItoL1TopoLocationPlusOne.key().empty()));

   // MuCTPIL1Topo from muon RoI
   if (!m_MuonL1RoILocation.key().empty())
     {m_MuonL1RoILocation = m_MuonL1RoILocation.key();}
   CHECK(m_MuonL1RoILocation.initialize(!m_MuonL1RoILocation.key().empty()));


  //LateMuon from muonRoI
  if(!m_MuonL1RoILocationPlusOne.key().empty()){
     m_MuonL1RoILocationPlusOne = m_MuonL1RoILocationPlusOne.key();
  }
  CHECK(m_MuonL1RoILocationPlusOne.initialize(!m_MuonL1RoILocationPlusOne.key().empty()));

   
   if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());

   return StatusCode::SUCCESS;
}

TCS::MuonTOB MuonInputProvider::createMuonTOB(const xAOD::MuonRoI & muonRoI, const std::vector<unsigned int> & rpcPtValues, const std::vector<unsigned int> & tgcPtValues) const{


   float et;
   float fEta = muonRoI.eta();
   float fPhi = muonRoI.phi();

   int etaTopo = TSU::toTopoEta(fEta);
   unsigned int phiTopo = TSU::toTopoPhi(fPhi);

   // WARNING:: 
   // This uses a mapping for thrNumber : thrValue , where the thresholds
   // can change per run, and so the menu used might be different.
   // This should be changed to read from threshold value as soon
   // as it is available.
   // See: https://its.cern.ch/jira/browse/ATR-26165

   int thrNumber = muonRoI.getThrNumber();

   if (muonRoI.getSource() == xAOD::MuonRoI::RoISource::Barrel) { //RPC
      et = rpcPtValues[thrNumber]; //map is in GeV
    } else {
      et = tgcPtValues[thrNumber]; //map is in GeV
    }

   unsigned int EtTopo = et*10;
   

   ATH_MSG_DEBUG("Muon ROI: " << muonRoI.getSource() << " thrvalue = " << thrNumber << " eta = " << etaTopo << " phi = " << phiTopo << " BW2or3 " << muonRoI.getBW3Coincidence() << " Good MF " << muonRoI.getGoodMF() << " Inner Coincidence " << muonRoI.getInnerCoincidence() << " Charge " << muonRoI.getCharge() << ", w   = " << MSG::hex << std::setw( 8 ) << muonRoI.getRoI() << MSG::dec);
   
   TCS::MuonTOB muon( EtTopo, 0, etaTopo, phiTopo, muonRoI.getRoI() );
   muon.setEtDouble(static_cast<double>(EtTopo/10.));
   muon.setEtaDouble(static_cast<double>(etaTopo/40.));
   muon.setPhiDouble(static_cast<double>(phiTopo/20.));

   // Muon flags
   if ( muonRoI.getSource() != xAOD::MuonRoI::RoISource::Barrel) { // TGC ( endcap (E) + forward (F) )
      muon.setBW2or3( topoFlag(muonRoI.getBW3Coincidence()) ); //Needs checking if this is the right flag
      muon.setInnerCoin( topoFlag(muonRoI.getInnerCoincidence()) );
      muon.setGoodMF( topoFlag(muonRoI.getGoodMF()) );
      muon.setCharge( topoFlag(muonRoI.getCharge()) );
      muon.setIs2cand( 0 );
      muon.setIsTGC( 1 );
   }
   else { // RPC ( barrel (B) )
      muon.setBW2or3( 0 );
      muon.setInnerCoin( 0 );
      muon.setGoodMF( 0 );
      muon.setCharge( 0 );
      muon.setIs2cand( topoFlag(muonRoI.isMoreCandInRoI()) );  //Needs checking if this is the right flag
      muon.setIsTGC( 0 );
   }

   auto mon_hPt = Monitored::Scalar("MuonTOBPt", muon.EtDouble());
   auto mon_hPtTGC = Monitored::Scalar("MuonTOBPtTGC", muon.EtDouble());
   auto mon_hPtRPC = Monitored::Scalar("MuonTOBPtRPC", muon.EtDouble());
   auto mon_hEta = Monitored::Scalar("MuonTOBEta",muon.eta());
   auto mon_hPhi = Monitored::Scalar("MuonTOBPhi",muon.phi());
   auto mon_hBW2or3 = Monitored::Scalar("MuonTOBBW2or3",muon.bw2or3());
   auto mon_hInnerCoin = Monitored::Scalar("MuonTOBInnerCoin",muon.innerCoin());
   auto mon_hGoodMF = Monitored::Scalar("MuonTOBGoodMF",muon.goodMF());
   auto mon_hCharge = Monitored::Scalar("MuonTOBCharge",muon.charge());
   auto mon_hIs2cand = Monitored::Scalar("MuonTOBIs2cand",muon.is2cand());
   auto mon_hIsTGC = Monitored::Scalar("MuonTOBIsTGC",muon.isTGC());
   Monitored::Group(m_monTool, mon_hPt, mon_hEta, mon_hPhi, mon_hBW2or3, mon_hInnerCoin, mon_hGoodMF, mon_hCharge, mon_hIs2cand, mon_hIsTGC);
   if ( muon.isTGC() ) { Monitored::Group(m_monTool, mon_hPtTGC); }
   else                { Monitored::Group(m_monTool, mon_hPtRPC); }

   return muon;
}

TCS::MuonTOB
MuonInputProvider::createMuonTOB(const MuCTPIL1TopoCandidate & roi) const {
   ATH_MSG_DEBUG("Muon ROI (MuCTPiToTopo): thr ID = " << roi.getptThresholdID() << " eta = " << roi.geteta() << " phi = " << roi.getphi()  
                  << ", w   = " << MSG::hex << std::setw( 8 ) << roi.getRoiID() << MSG::dec );
   ATH_MSG_DEBUG("                            Oct = " << roi.getMioctID() << " etacode=" <<  roi.getetacode() << " phicode= " <<  
                  roi.getphicode()<< ", Sector="<< roi.getSectorName() );

   // roi.geteta() and roi.getphi() return the the exact geometrical coordinates of the trigger chambers
   // L1Topo granularities are 0.025 for eta (=> inverse = 40) and 0.05 for phi (=> inverse = 20)
   // L1Topo simulation uses positive phi (from 0 to 2pi) => transform phiTopo
   float fEta = roi.geteta();
   float fPhi = roi.getphi();
   
   int etaTopo = TSU::toTopoEta(fEta);
   unsigned int phiTopo = TSU::toTopoPhi(fPhi);
  
   unsigned int EtTopo = roi.getptValue()*10;
   

   
   TCS::MuonTOB muon( EtTopo, 0, etaTopo, phiTopo, roi.getRoiID() );
   muon.setEtDouble(static_cast<double>(EtTopo/10.));
   muon.setEtaDouble(static_cast<double>(etaTopo/40.));
   muon.setPhiDouble(static_cast<double>(phiTopo/20.));

   // Muon flags
   if ( roi.getSectorName().at(0) != 'B' ) { // TGC ( endcap (E) + forward (F) )
      muon.setBW2or3( topoFlag(roi.getbw2or3()) );
      muon.setInnerCoin( topoFlag(roi.getinnerCoin()) );
      muon.setGoodMF( topoFlag(roi.getgoodMF()) );
      muon.setCharge( topoFlag(roi.getcharge()) );
      muon.setIs2cand( 0 );
      muon.setIsTGC( 1 );
   }
   else { // RPC ( barrel (B) )
      muon.setBW2or3( 0 );
      muon.setInnerCoin( 0 );
      muon.setGoodMF( 0 );
      muon.setCharge( 0 );
      muon.setIs2cand( topoFlag(roi.getis2cand()) );
      muon.setIsTGC( 0 );
   }

   auto mon_hPt = Monitored::Scalar("MuonTOBPt", muon.EtDouble());
   auto mon_hPtTGC = Monitored::Scalar("MuonTOBPtTGC", muon.EtDouble());
   auto mon_hPtRPC = Monitored::Scalar("MuonTOBPtRPC", muon.EtDouble());
   auto mon_hEta = Monitored::Scalar("MuonTOBEta",muon.eta());
   auto mon_hPhi = Monitored::Scalar("MuonTOBPhi",muon.phi());
   auto mon_hBW2or3 = Monitored::Scalar("MuonTOBBW2or3",muon.bw2or3());
   auto mon_hInnerCoin = Monitored::Scalar("MuonTOBInnerCoin",muon.innerCoin());
   auto mon_hGoodMF = Monitored::Scalar("MuonTOBGoodMF",muon.goodMF());
   auto mon_hCharge = Monitored::Scalar("MuonTOBCharge",muon.charge());
   auto mon_hIs2cand = Monitored::Scalar("MuonTOBIs2cand",muon.is2cand());
   auto mon_hIsTGC = Monitored::Scalar("MuonTOBIsTGC",muon.isTGC());
   Monitored::Group(m_monTool, mon_hPt, mon_hEta, mon_hPhi, mon_hBW2or3, mon_hInnerCoin, mon_hGoodMF, mon_hCharge, mon_hIs2cand, mon_hIsTGC);
   if ( muon.isTGC() ) { Monitored::Group(m_monTool, mon_hPtTGC); }
   else                { Monitored::Group(m_monTool, mon_hPtRPC); }

   return muon;
}


TCS::LateMuonTOB MuonInputProvider::createLateMuonTOB(const xAOD::MuonRoI & muonRoI, const std::vector<unsigned int> & rpcPtValues, const std::vector<unsigned int> & tgcPtValues) const {


   float et;
   float eta = muonRoI.eta();
   float phi = muonRoI.phi();

   int etaTopo = TSU::toTopoEta(eta);
   unsigned int phiTopo = TSU::toTopoPhi(phi);

   // WARNING:: 
   // Also uses mapping for thrNumber : thrValue , see above.
   int thrNumber = muonRoI.getThrNumber();

   if (muonRoI.getSource() == xAOD::MuonRoI::RoISource::Barrel) { //RPC
      et = rpcPtValues[thrNumber]; //map is in GeV
    } else {
      et = tgcPtValues[thrNumber]; //map is in GeV
    }

   unsigned int EtTopo = et*10;

   ATH_MSG_DEBUG("LateMuon ROI: " << muonRoI.getSource() << " thrvalue = " << thrNumber << " eta = " << etaTopo << " phi = " << phiTopo << " BW2or3 " << muonRoI.getBW3Coincidence() << " Good MF " << muonRoI.getGoodMF() << " Inner Coincidence " << muonRoI.getInnerCoincidence() << " Charge " << muonRoI.getCharge() << ", w   = " << MSG::hex << std::setw( 8 ) << muonRoI.getRoI() << MSG::dec);

   TCS::LateMuonTOB muon( EtTopo, 0, etaTopo, static_cast<unsigned int>(phiTopo), muonRoI.getRoI() );
   muon.setEtDouble(static_cast<double>(EtTopo/10.));
   muon.setEtaDouble(static_cast<double>(etaTopo/40.));
   muon.setPhiDouble(static_cast<double>(phiTopo/20.));

   // Muon flags
   if ( muonRoI.getSource() != xAOD::MuonRoI::RoISource::Barrel) { // TGC ( endcap (E) + forward (F) )
      muon.setBW2or3( topoFlag(muonRoI.getBW3Coincidence()) ); //Needs checking if this is the right flag
      muon.setInnerCoin( topoFlag(muonRoI.getInnerCoincidence()) );
      muon.setGoodMF( topoFlag(muonRoI.getGoodMF()) );
      muon.setCharge( topoFlag(muonRoI.getCharge()) );
      muon.setIs2cand( 0 );
      muon.setIsTGC( 1 );
   }
   else { // RPC ( barrel (B) )
      muon.setBW2or3( 0 );
      muon.setInnerCoin( 0 );
      muon.setGoodMF( 0 );
      muon.setCharge( 0 );
      muon.setIs2cand( topoFlag(muonRoI.isMoreCandInRoI()) );  //Needs checking if this is the right flag
      muon.setIsTGC( 0 );
   }

   auto mon_hLateMuonPt = Monitored::Scalar("LateMuonTOBPt", muon.EtDouble());
   auto mon_hLateMuonPtTGC = Monitored::Scalar("LateMuonTOBPtTGC", muon.EtDouble());
   auto mon_hLateMuonPtRPC = Monitored::Scalar("LateMuonTOBPtRPC", muon.EtDouble());
   auto mon_hLateMuonEta = Monitored::Scalar("LateMuonTOBEta",muon.eta());
   auto mon_hLateMuonPhi = Monitored::Scalar("LateMuonTOBPhi",muon.phi());
   auto mon_hLateMuonBW2or3 = Monitored::Scalar("LateMuonTOBBW2or3",muon.bw2or3());
   auto mon_hLateMuonInnerCoin = Monitored::Scalar("LateMuonTOBInnerCoin",muon.innerCoin());
   auto mon_hLateMuonGoodMF = Monitored::Scalar("LateMuonTOBGoodMF",muon.goodMF());
   auto mon_hLateMuonCharge = Monitored::Scalar("LateMuonTOBCharge",muon.charge());
   auto mon_hLateMuonIs2cand = Monitored::Scalar("LateMuonTOBIs2cand",muon.is2cand());
   auto mon_hLateMuonIsTGC = Monitored::Scalar("LateMuonTOBIsTGC",muon.isTGC());
   Monitored::Group(m_monTool, mon_hLateMuonPt, mon_hLateMuonEta, mon_hLateMuonPhi, mon_hLateMuonBW2or3, mon_hLateMuonInnerCoin, mon_hLateMuonGoodMF, mon_hLateMuonCharge, mon_hLateMuonIs2cand, mon_hLateMuonIsTGC);
   if ( muon.isTGC() ) { Monitored::Group(m_monTool, mon_hLateMuonPtTGC); }
   else                { Monitored::Group(m_monTool, mon_hLateMuonPtRPC); }

   ATH_MSG_DEBUG("LateMuon created");
   return muon;

}


TCS::LateMuonTOB
MuonInputProvider::createLateMuonTOB(const MuCTPIL1TopoCandidate & roi) const {


   ATH_MSG_DEBUG("Late Muon ROI (MuCTPiToTopo):bcid=1 thr pt = " << roi.getptThresholdID() << " eta = " << roi.geteta() << " phi = " << roi.getphi() << ", w   = " << MSG::hex << std::setw( 8 ) << roi.getRoiID() << MSG::dec);

   float fEta = roi.geteta();
   float fPhi = roi.getphi();
   
   int etaTopo = TSU::toTopoEta(fEta);
   unsigned int phiTopo = TSU::toTopoPhi(fPhi);

   unsigned int EtTopo = roi.getptValue()*10;

 
   TCS::LateMuonTOB muon( EtTopo, 0, etaTopo, phiTopo, roi.getRoiID() );

   muon.setEtDouble(static_cast<double>(EtTopo/10.));
   muon.setEtaDouble(static_cast<double>(etaTopo/40.));
   muon.setPhiDouble(static_cast<double>(phiTopo/20.));

   // Muon flags
   if ( roi.getSectorName().at(0) != 'B' ) { // TGC ( endcap (E) + forward (F) )
      muon.setBW2or3( topoFlag(roi.getbw2or3()) );
      muon.setInnerCoin( topoFlag(roi.getinnerCoin()) );
      muon.setGoodMF( topoFlag(roi.getgoodMF()) );
      muon.setCharge( topoFlag(roi.getcharge()) );
      muon.setIs2cand( 0 );
      muon.setIsTGC( 1 );
   }
   else { // RPC ( barrel (B) )
      muon.setBW2or3( 0 );
      muon.setInnerCoin( 0 );
      muon.setGoodMF( 0 );
      muon.setCharge( 0 );
      muon.setIs2cand( topoFlag(roi.getis2cand()) );
      muon.setIsTGC( 0 );
   }

   auto mon_hLateMuonPt = Monitored::Scalar("LateMuonTOBPt", muon.EtDouble());
   auto mon_hLateMuonPtTGC = Monitored::Scalar("LateMuonTOBPtTGC", muon.EtDouble());
   auto mon_hLateMuonPtRPC = Monitored::Scalar("LateMuonTOBPtRPC", muon.EtDouble());
   auto mon_hLateMuonEta = Monitored::Scalar("LateMuonTOBEta",muon.eta());
   auto mon_hLateMuonPhi = Monitored::Scalar("LateMuonTOBPhi",muon.phi());
   auto mon_hLateMuonBW2or3 = Monitored::Scalar("LateMuonTOBBW2or3",muon.bw2or3());
   auto mon_hLateMuonInnerCoin = Monitored::Scalar("LateMuonTOBInnerCoin",muon.innerCoin());
   auto mon_hLateMuonGoodMF = Monitored::Scalar("LateMuonTOBGoodMF",muon.goodMF());
   auto mon_hLateMuonCharge = Monitored::Scalar("LateMuonTOBCharge",muon.charge());
   auto mon_hLateMuonIs2cand = Monitored::Scalar("LateMuonTOBIs2cand",muon.is2cand());
   auto mon_hLateMuonIsTGC = Monitored::Scalar("LateMuonTOBIsTGC",muon.isTGC());
   Monitored::Group(m_monTool, mon_hLateMuonPt, mon_hLateMuonEta, mon_hLateMuonPhi, mon_hLateMuonBW2or3, mon_hLateMuonInnerCoin, mon_hLateMuonGoodMF, mon_hLateMuonCharge, mon_hLateMuonIs2cand, mon_hLateMuonIsTGC);
   if ( muon.isTGC() ) { Monitored::Group(m_monTool, mon_hLateMuonPtTGC); }
   else                { Monitored::Group(m_monTool, mon_hLateMuonPtRPC); }

   ATH_MSG_DEBUG("LateMuon created");
   return muon;
}

int
MuonInputProvider::topoFlag(bool flag) const {
  if ( flag == true ) { return 1; }
  else { return -1; }
}

StatusCode
MuonInputProvider::fillTopoInputEvent(TCS::TopoInputEvent& inputEvent) const {

  if (!m_MuonL1RoILocation.key().empty()) {

        ATH_MSG_DEBUG("Using muon inputs from L1 RoI");

        const TrigConf::L1Menu * l1menu = nullptr;
        ATH_CHECK( detStore()->retrieve(l1menu) );
          
        //Read mapping from menu
        const auto & exMU = l1menu->thrExtraInfo().MU();
        auto rpcPtValues = exMU.knownRpcPtValues();
        auto tgcPtValues = exMU.knownTgcPtValues();

        SG::ReadHandle<xAOD::MuonRoIContainer> muonROIs (m_MuonL1RoILocation);
        for (auto muonRoi : *muonROIs) {

            inputEvent.addMuon( MuonInputProvider::createMuonTOB( *muonRoi, rpcPtValues, tgcPtValues) );

         }

         // Also add LateMuons:
         if(not m_MuonL1RoILocationPlusOne.key().empty()) {

            const EventContext& ctx = Gaudi::Hive::currentContext();
            SG::ReadHandle<xAOD::MuonRoIContainer> latemuonROIs (m_MuonL1RoILocationPlusOne, ctx);

            if( latemuonROIs.isValid() ) {

               ATH_MSG_DEBUG( "Contains L1Topo LateMuons L1Muctpi object from StoreGate!" );

               for(const auto muonRoi : *latemuonROIs) {
                  inputEvent.addLateMuon( MuonInputProvider::createLateMuonTOB( *muonRoi, rpcPtValues, tgcPtValues) );
               }
            }
         }
      } else{

      ATH_MSG_DEBUG("Use MuCTPiToTopo granularity Muon ROIs.");

      // first see if L1Muctpi simulation already ran and object is in storegate, if not throw an error

      const LVL1::MuCTPIL1Topo* l1topo  {nullptr};

      if(m_MuCTPItoL1TopoLocation.key().empty()==false){
         SG::ReadHandle<LVL1::MuCTPIL1Topo> l1topoh(m_MuCTPItoL1TopoLocation);
         if( l1topoh.isValid() ) l1topo = l1topoh.cptr();
      }

      if( l1topo ) {
         ATH_MSG_DEBUG("Use MuCTPiToTopo granularity Muon ROIs: retrieve from SG");

         const std::vector<MuCTPIL1TopoCandidate> & candList = l1topo->getCandidates();
         for( const MuCTPIL1TopoCandidate & muCand : candList) {
            inputEvent.addMuon( MuonInputProvider::createMuonTOB( muCand ) );
            if(muCand.moreThan2CandidatesOverflow()){
               inputEvent.setOverflowFromMuonInput(true);
               ATH_MSG_DEBUG("setOverflowFromMuonInput : true (MuCTPIL1TopoCandidate from SG)");
            }
         }
      } else {
 	 ATH_MSG_ERROR("Couldn't retrieve L1Topo inputs from StoreGate");
	 return StatusCode::FAILURE;
      }

      //BC+1 ... this can only come from simulation, in data taking this is collected by the L1Topo at its input
      // so no need to do anything else here
      if(m_MuCTPItoL1TopoLocationPlusOne.key().empty()==false) {
         SG::ReadHandle<LVL1::MuCTPIL1Topo> l1topoBC1(m_MuCTPItoL1TopoLocationPlusOne);
         if( l1topoBC1.isValid() ) {
            ATH_MSG_DEBUG( "Contains L1Topo LateMuons L1Muctpi object from StoreGate!" );
            const std::vector<MuCTPIL1TopoCandidate> & candList = l1topoBC1->getCandidates();
            for( const MuCTPIL1TopoCandidate& muCand : candList)
            {
               ATH_MSG_DEBUG("MuonInputProvider addLateMuon ");
               inputEvent.addLateMuon( MuonInputProvider::createLateMuonTOB( muCand ) );	   
            }
         }
      }
   }
   return StatusCode::SUCCESS;
}
