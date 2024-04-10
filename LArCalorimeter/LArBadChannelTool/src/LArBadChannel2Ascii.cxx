/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArBadChannelTool/LArBadChannel2Ascii.h"
#include "LArRecConditions/LArBadFeb.h"
#include "LArRecConditions/LArBadChanBitPacking.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"

#include "LArIdentifier/LArOnlineID.h"
#include <fstream>

LArBadChannel2Ascii::LArBadChannel2Ascii(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm( name, pSvcLocator),
  m_BCKey("LArBadChannel"),
  m_BFKey("LArBadFeb"),
  m_cablingKey("LArOnOffIdMap")
{
  //declareProperty("BadChannelTool", m_BadChanTool, "public, shared BadChannelTool");
  declareProperty("BCKey",m_BCKey);
  declareProperty("BFKey",m_BFKey);
  declareProperty("LArOnOffIdMapKey",m_cablingKey);
  declareProperty("FileName",m_fileName="");
  declareProperty("WithMissing",m_wMissing=false);
  declareProperty("SkipDisconnected",m_skipDisconnected=false);
  declareProperty("ExecutiveSummaryFile",m_executiveSummaryFile="");
  declareProperty("SuperCell",m_isSC=false);
}

LArBadChannel2Ascii::~LArBadChannel2Ascii() = default;


StatusCode LArBadChannel2Ascii::initialize() {

  ATH_MSG_INFO ( "initialize()" );

  ATH_CHECK(m_BCKey.initialize());

  ATH_CHECK(m_BFKey.initialize(!m_executiveSummaryFile.empty()));

  ATH_CHECK(m_cablingKey.initialize(m_skipDisconnected));

  return StatusCode::SUCCESS;
}


StatusCode LArBadChannel2Ascii::execute() {

  const bool doExecSummary=(!m_executiveSummaryFile.empty());

  SG::ReadCondHandle<LArBadChannelCont> bch{m_BCKey};
  const LArBadChannelCont* badChannelCont{*bch};

  const LArOnOffIdMapping* cabling=nullptr;
  if (m_skipDisconnected) {
    SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey};
    cabling=(*cablingHdl);
  }

  const LArBadFebCont* badFebCont=nullptr;
  if (doExecSummary && !m_isSC) {
    SG::ReadCondHandle<LArBadFebCont> badFebHdl{m_BFKey};
    badFebCont=(*badFebHdl);
  }

  const LArOnlineID_Base* larOnlineID;
  if ( m_isSC ){
     const LArOnline_SuperCellID* ll;
     StatusCode sc = detStore()->retrieve(ll, "LArOnline_SuperCellID");
     if (sc.isFailure()) {
       ATH_MSG_ERROR( "Could not get LArOnlineID helper !" );
       return StatusCode::FAILURE;
     } else {
       larOnlineID = ll;
       ATH_MSG_DEBUG("Found the LArOnlineID helper");
     }
   } else { // m_isSC
     const LArOnlineID* ll;
     StatusCode sc = detStore()->retrieve(ll, "LArOnlineID");
     if (sc.isFailure()) {
       ATH_MSG_ERROR( "Could not get LArOnlineID helper !");
       return StatusCode::FAILURE;
     } else {
       larOnlineID = ll;
       ATH_MSG_DEBUG(" Found the LArOnlineID helper. ");
     }
  } 
  
  std::ostream *out = &(std::cout); 
  std::ofstream outfile;
  if (!m_fileName.empty()) {
    outfile.open(m_fileName.c_str(),std::ios::out);
    if (outfile.is_open()) {
      ATH_MSG_INFO ( "Writing to file " << m_fileName );
      out = &outfile;
    }
    else
      ATH_MSG_ERROR ( "Failed to open file " << m_fileName );
  }

  const LArBadChanBitPacking packing;
  const LArBadChanSCBitPacking SCpacking;

  // std::vector<std::vector<unsigned> > problemMatrix(nParts,std::vector<unsigned>(nProblemTypes) );
  std::vector<std::vector<unsigned> > problemMatrix(nProblemTypes, std::vector<unsigned>(nParts));

    std::vector<HWIdentifier>::const_iterator it = larOnlineID->channel_begin();
  std::vector<HWIdentifier>::const_iterator it_e = larOnlineID->channel_end();
  unsigned count = 0, nConnected = 0;
  for (; it != it_e; ++it) {
    const HWIdentifier chid = *it;
    // if (m_skipDisconnected && !m_larCablingSvc->isOnlineConnected(chid)) continue;
    if (cabling && !cabling->isOnlineConnected(chid))
      continue;
    ++nConnected;

    LArBadChannel bc = badChannelCont->status(chid);
  
    if (!bc.good()) {
      ++count;
      (*out) << larOnlineID->barrel_ec(chid) << " " << larOnlineID->pos_neg(chid) << " " << larOnlineID->feedthrough(chid) << " " << larOnlineID->slot(chid)
             << " " << larOnlineID->channel(chid) << " "
             << "0 ";  // Dummy 0 for calib-line
      if (m_isSC) {
        (*out) << SCpacking.stringStatus(bc);
      } else {
        (*out) << packing.stringStatus(bc);
      }

      (*out) << "  # 0x" << std::hex << chid.get_identifier32().get_compact();
      if (cabling) {
        Identifier offid = cabling->cnvToIdentifier(chid);
        (*out) << " -> 0x" << offid.get_identifier32().get_compact();
      }
      (*out) << std::dec << std::endl;
    }  // End if channel is not good (regular printout)

    if (doExecSummary) {
      HWIdentifier fid = larOnlineID->feb_Id(chid);
      LArBadFeb bf;
      if (!m_isSC) bf= badFebCont->status(fid);

      DetPart dp = EMB;
      if (larOnlineID->isEMECchannel(chid))
        dp = EMEC;
      else if (larOnlineID->isHECchannel(chid))
        dp = HEC;
      else if (larOnlineID->isFCALchannel(chid))
        dp = FCAL;

      
      if (bc.deadReadout() || bc.maskedOSUM())
        ++problemMatrix[DeadReadout][dp];
      if (bc.deadPhys())
         ++problemMatrix[DeadPhys][dp];
      if (bc.deadCalib())
        ++problemMatrix[DeadCalib][dp];
      if (bc.reallyNoisy())
        ++problemMatrix[Noisy][dp];
      if (bc.sporadicBurstNoise())
        ++problemMatrix[Sporadic][dp];

      if (bc.distorted() || bc.deformedTail() || bc.deformedPulse())
	++problemMatrix[Distorted][dp];

      if (bc.ADCJump() || bc.nonLinearRamp() || bc.SCAProblem() || bc.offOFCs() || bc.offAmplitude() || bc.offScale()) {
	++problemMatrix[PeakReco][dp];
      }
      
      if (bc.lowLightFibre() || bc.transmissionErrorFibre()) {
	++problemMatrix[Fibre][dp];
      }
      
      if (!m_isSC) { //problematic febs apply only to regular cells
        if (bf.deadAll() || bf.deadReadout() || bf.deactivatedInOKS())
          ++problemMatrix[DeadFEB][dp];
        if (bf.deadAll() || bf.deadReadout() || bf.deactivatedInOKS() || bc.deadReadout() || bc.deadPhys() || bc.reallyNoisy())
          ++problemMatrix[GrandTotalDead][dp];
      }
      else {
         if(bc.maskedOSUM() || bc.deadReadout() || bc.deadPhys() || bc.reallyNoisy())
          ++problemMatrix[GrandTotalDead][dp];
      }


    }  // end if executive Summary

  }  // end loop over channels;
  if (m_skipDisconnected)
    ATH_MSG_INFO ( "Found " << count << " entries in the bad-channel database. (Number of connected cells: " << nConnected << ")"  );
  else
    ATH_MSG_INFO ( "Found " << count << " entries in the bad-channel database. (Number of cells: " << nConnected  << ")");
  if (m_wMissing)
    ATH_MSG_INFO ( "Including missing FEBs" );
  else
    ATH_MSG_INFO ( "Without missing FEBs" );
  if (outfile.is_open())
    outfile.close();


  if (doExecSummary) {
    std::ofstream exeSum;
    exeSum.open(m_executiveSummaryFile.c_str(),std::ios::out);
    if (!exeSum.is_open()) {
      ATH_MSG_ERROR ( "Failed to open file " << m_executiveSummaryFile );
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO ( "Writing Executive Summary to file " << m_executiveSummaryFile );

    if (m_isSC) {
      exeSum << "LAr SuperCells  dead readout (incl masked OSUM)" << std::endl;
      writeSum(exeSum,problemMatrix[DeadReadout]);

      exeSum << "LAr SuperCells suffering from high noise:" << std::endl;
      writeSum(exeSum,problemMatrix[Noisy]);
    
      exeSum << "LAr SuperCells w/o calibration (constants from phi average of eta neighbours):" << std::endl;
      writeSum(exeSum,problemMatrix[DeadCalib]);

      exeSum << "LAr SuperCells with distorted pulse shape:" << std::endl;
      writeSum(exeSum,problemMatrix[Distorted]);

      exeSum << "LAr SuperCells having problems with the peak reco:" << std::endl;
      writeSum(exeSum,problemMatrix[PeakReco]);
      
      exeSum << "LAr SuperCells having problems with the optical transmission:" << std::endl;
      writeSum(exeSum,problemMatrix[Fibre]);

      exeSum << "LAr SuperCells not usable:" << std::endl;
      writeSum(exeSum,problemMatrix[GrandTotalDead]);
    }
    else {
      exeSum << "LAr dead readout channels:" << std::endl;
      writeSum(exeSum,problemMatrix[DeadReadout]);

      exeSum << "LAr permanently dead channels inside detector:" << std::endl;
      writeSum(exeSum,problemMatrix[DeadPhys]);

      exeSum << "LAr noisy readout channels (more than 10 sigma above phi average or unstable):" << std::endl;
      writeSum(exeSum,problemMatrix[Noisy]);
    
      exeSum << "LAr readout channels showing sporadic noise bursts:" << std::endl;
      writeSum(exeSum,problemMatrix[Sporadic]);

      exeSum << "LAr readout channels w/o calibration (constants from phi average of eta neighbours):" << std::endl;
      writeSum(exeSum,problemMatrix[DeadCalib]);

      exeSum << "LAr readout channels connected to inactive Front End Boards:" << std::endl;
      writeSum(exeSum,problemMatrix[DeadFEB]);
    
      exeSum << "LAr readout channels not usable:" << std::endl;
      writeSum(exeSum,problemMatrix[GrandTotalDead]);
    }
    
    exeSum.close();
  }

  return StatusCode::SUCCESS;
}


void LArBadChannel2Ascii::writeSum(std::ofstream& exeFile, const std::vector<unsigned>& probs) {
  const unsigned nEMB=109568, nEMEC=63744, nHEC=5632, nFCAL=3524;
  const unsigned nTot=nEMB+nEMEC+nHEC+nFCAL;
  unsigned nTotProb=0;
  for(size_t i=0;i<probs.size();++i) 
    nTotProb+=probs[i];

  exeFile << "    EMB: " <<  std::setw(5) << probs[EMB] << " of "  << nEMB << " (" << std::setprecision(3) <<  probs[EMB]*(100./nEMB) << "%)" << std::endl;
  exeFile << "   EMEC: " <<  std::setw(5) <<probs[EMEC] << " of "  << nEMEC << " (" << std::setprecision(3) << probs[EMEC]*(100./nEMEC) << "%)" << std::endl;
  exeFile << " EM tot: " <<  std::setw(5) << probs[EMEC]+probs[EMB]  << " of "  << nEMB+nEMEC 
          << " (" << std::setprecision(3) << (probs[EMEC]+probs[EMB])*(100./(nEMEC+nEMB)) << "%)" << std::endl;

  exeFile << "    HEC: " <<  std::setw(5) <<probs[HEC] << " of "  << nHEC << " (" << std::setprecision(3) << probs[HEC]*(100./nHEC) << "%)" << std::endl;
  exeFile << "   FCAL: " <<  std::setw(5) <<probs[FCAL] << " of "  << nFCAL << " (" << std::setprecision(3) << probs[FCAL]*(100./nFCAL) << "%)" << std::endl;    
  exeFile << "  Total: " <<  std::setw(5) << nTotProb << " of "  << nTot << " (" << std::setprecision(3) << nTotProb*(100./nTot) << "%)" << std::endl;
  exeFile << std::endl;

}

