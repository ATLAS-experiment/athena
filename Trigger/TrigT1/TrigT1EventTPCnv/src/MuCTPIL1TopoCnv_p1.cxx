/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


// Gaudi/Athena include(s):
#include "GaudiKernel/MsgStream.h"

// TrigT1 inlcude(s):
#include "TrigT1Interfaces/MuCTPIL1TopoCandidate.h"

// Local include(s):
#include "TrigT1EventTPCnv/MuCTPIL1TopoCnv_p1.h"
#include "TrigT1EventTPCnv/MuCTPIL1TopoCandidate_p1.h"

/**
 * Function transferring the information from a persistent MuCTPIL1Topo_p1 object
 * to a transient MuCTPIL1Topo object.
 */
void MuCTPIL1TopoCnv_p1::persToTrans( const MuCTPIL1Topo_p1* persObj, LVL1::MuCTPIL1Topo* transObj,
				      MsgStream& log ) {

  if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Converting MuCTPIL1Topo from persistent state..." << endmsg;

  for (const MuCTPIL1TopoCandidate_p1 &cand : persObj->m_muonTopoCandidates) {
    LVL1::MuCTPIL1TopoCandidate muCand;
    muCand.setCandidateData(cand.m_sectorName,
			     cand.m_roiID,
			     cand.m_bcid,
			     cand.m_ptThresholdID,
			     cand.m_ptL1TopoCode,
			     cand.m_ptValue,
			     cand.m_eta,
			     cand.m_phi,
			     cand.m_etacode,
			     cand.m_phicode,
			     cand.m_etamin,
			     cand.m_etamax,
			     cand.m_phimin,
			     cand.m_phimax,
			     cand.m_mioctID,
			     cand.m_ieta,
			    cand.m_iphi);
    
    muCand.setTGCFlags(cand.m_bw2or3,
		       cand.m_innerCoin,
		       cand.m_goodMF,
		       cand.m_charge);
  
    muCand.setRPCFlags(cand.m_is2cand,
		       cand.m_phiOvl);
  		
    muCand.setRoiWord(cand.m_roiWord);
  
    transObj->addCandidate(muCand);
  }
  transObj->setBcidOffset (persObj->m_bcidOffset);

  if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Converting MuCTPIL1Topo from persistent state [OK]" << endmsg;

  return;
}

/**
 * Function transferring the information from a transient MuCTPIL1Topo object
 * to a persistent MuCTPIL1Topo_p1 object.
 */
void MuCTPIL1TopoCnv_p1::transToPers( const LVL1::MuCTPIL1Topo* transObj, MuCTPIL1Topo_p1* persObj,
				      MsgStream& log ) {

  if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Creating persistent state of MuCTPIL1Topo..." << endmsg;

  persObj->m_muonTopoCandidates.clear();
  for (const LVL1::MuCTPIL1TopoCandidate & cand : transObj->getCandidates()) {
    MuCTPIL1TopoCandidate_p1 muCand;
    muCand.m_sectorName       = cand.getSectorName();
    muCand.m_roiID            = cand.getRoiID();
    muCand.m_bcid 	      = cand.getbcid();
    muCand.m_ptThresholdID    = cand.getptThresholdID();
    muCand.m_ptL1TopoCode     = cand.getptL1TopoCode();
    muCand.m_ptValue	      = cand.getptValue();
    muCand.m_eta	      = cand.geteta();
    muCand.m_phi	      = cand.getphi();
    muCand.m_etacode	      = cand.getetacode();
    muCand.m_phicode	      = cand.getphicode();
    muCand.m_etamin	      = cand.getetamin();
    muCand.m_etamax	      = cand.getetamax();
    muCand.m_phimin	      = cand.getphimin();
    muCand.m_phimax	      = cand.getphimax();
    muCand.m_roiWord	      = cand.getRoiWord();
    muCand.m_mioctID	      = cand.getMioctID();
    muCand.m_ieta	      = cand.getieta();
    muCand.m_iphi	      = cand.getiphi();
    muCand.m_phiOvl	      = cand.getphiOvl();
    muCand.m_is2cand	      = cand.getis2cand();
    muCand.m_charge	      = cand.getcharge();
    muCand.m_bw2or3	      = cand.getbw2or3();
    muCand.m_innerCoin        = cand.getinnerCoin();
    muCand.m_goodMF           = cand.getgoodMF();   

    persObj->m_muonTopoCandidates.push_back(std::move(muCand));
  }
  persObj->m_bcidOffset         = transObj->getBcidOffset();

  if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Creating persistent state of MuCTPIL1Topo [OK]" << endmsg;

  return;
}
