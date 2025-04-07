/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArBadChannelTool/LArBadChannel2Ascii.h"
#include "LArRecConditions/LArBadFeb.h"
#include "LArRecConditions/LArBadChanBitPacking.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "LArIdentifier/LArOnlineID.h"

#include <format>
#include <fstream>
#include <algorithm>

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
    outfile.open(m_fileName.value().c_str(),std::ios::out);
    if (outfile.is_open()) {
      ATH_MSG_INFO ( "Writing to file " << m_fileName );
      out = &outfile;
    }
    else
      ATH_MSG_ERROR ( "Failed to open file " << m_fileName );
  }

  const LArBadChanBitPacking packing;
  const LArBadChanSCBitPacking SCpacking;

  std::vector<std::vector<unsigned> > problemMatrix(nProblemTypes, std::vector<unsigned>(nParts));

  std::vector<HWIdentifier>::const_iterator it = larOnlineID->channel_begin();
  std::vector<HWIdentifier>::const_iterator it_e = larOnlineID->channel_end();
  unsigned count = 0, nConnected = 0;
  std::vector<unsigned> nPerPart(nParts);

  for (; it != it_e; ++it) {
    const HWIdentifier chid = *it;
    if (cabling && !cabling->isOnlineConnected(chid))
      continue;
    ++nConnected;

    DetPart dp = EMB;
    if (larOnlineID->isHECchannel(chid))
      dp = HEC;
    else if (larOnlineID->isEMECchannel(chid))
      dp = EMEC;
    else if (larOnlineID->isFCALchannel(chid))
      dp = FCAL;

    ++nPerPart[dp];

    LArBadChannel bc = badChannelCont->status(chid);
  
    if (!bc.good()) {
      ++count;
      (*out) << std::format("{} {} {} {} {} 0 ",larOnlineID->barrel_ec(chid),larOnlineID->pos_neg(chid),larOnlineID->feedthrough(chid),larOnlineID->slot(chid),larOnlineID->channel(chid));
      // Dummy 0 for calib-line
      if (m_isSC) {
        (*out) << SCpacking.stringStatus(bc);
      } else {
        (*out) << packing.stringStatus(bc);
      }

      (*out) << std::format("  # {:#x}", chid.get_identifier32().get_compact());
      if (cabling) {
        Identifier offid = cabling->cnvToIdentifier(chid);
        (*out) << std::format(" -> {:#x}",offid.get_identifier32().get_compact());
      }
      (*out) << std::endl;
    }  // End if channel is not good (regular printout)

    if (doExecSummary) {
      HWIdentifier fid = larOnlineID->feb_Id(chid);
      LArBadFeb bf;
      if (!m_isSC) bf= badFebCont->status(fid);

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
    exeSum.open(m_executiveSummaryFile.value().c_str(),std::ios::out);
    if (!exeSum.is_open()) {
      ATH_MSG_ERROR ( "Failed to open file " << m_executiveSummaryFile );
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO ( "Writing Executive Summary to file " << m_executiveSummaryFile );

    if (m_isSC) {
      exeSum << "LAr SuperCells  dead readout (incl masked OSUM)" << std::endl;
      writeSum(exeSum,problemMatrix[DeadReadout],nPerPart);

      exeSum << "LAr SuperCells suffering from high noise:" << std::endl;
      writeSum(exeSum,problemMatrix[Noisy],nPerPart);
    
      exeSum << "LAr SuperCells w/o calibration (constants from phi average of eta neighbours):" << std::endl;
      writeSum(exeSum,problemMatrix[DeadCalib],nPerPart);

      exeSum << "LAr SuperCells with distorted pulse shape:" << std::endl;
      writeSum(exeSum,problemMatrix[Distorted],nPerPart);

      exeSum << "LAr SuperCells having problems with the peak reco:" << std::endl;
      writeSum(exeSum,problemMatrix[PeakReco],nPerPart);
      
      exeSum << "LAr SuperCells having problems with the optical transmission:" << std::endl;
      writeSum(exeSum,problemMatrix[Fibre],nPerPart);

      exeSum << "LAr SuperCells not usable:" << std::endl;
      writeSum(exeSum,problemMatrix[GrandTotalDead],nPerPart);
    }
    else {
      exeSum << "LAr dead readout channels:" << std::endl;
      writeSum(exeSum,problemMatrix[DeadReadout],nPerPart);

      exeSum << "LAr permanently dead channels inside detector:" << std::endl;
      writeSum(exeSum,problemMatrix[DeadPhys],nPerPart);

      exeSum << "LAr noisy readout channels (more than 10 sigma above phi average or unstable):" << std::endl;
      writeSum(exeSum,problemMatrix[Noisy],nPerPart);
    
      exeSum << "LAr readout channels showing sporadic noise bursts:" << std::endl;
      writeSum(exeSum,problemMatrix[Sporadic],nPerPart);

      exeSum << "LAr readout channels w/o calibration (constants from phi average of eta neighbours):" << std::endl;
      writeSum(exeSum,problemMatrix[DeadCalib],nPerPart);

      exeSum << "LAr readout channels connected to inactive Front End Boards:" << std::endl;
      writeSum(exeSum,problemMatrix[DeadFEB],nPerPart);
    
      exeSum << "LAr readout channels not usable:" << std::endl;
      writeSum(exeSum,problemMatrix[GrandTotalDead],nPerPart);
    }

    exeSum.close();
  }

  return StatusCode::SUCCESS;
}


void LArBadChannel2Ascii::writeSum(std::ofstream& exeFile, const std::vector<unsigned>& probs, const std::vector<unsigned>& nChans) const  {
  
  const unsigned nTot=std::accumulate(nChans.begin(),nChans.end(),0);
  const unsigned nTotProb=std::accumulate(probs.begin(),probs.end(),0);

  constexpr const char* fmt="{:>7}: {:>5} of {} ({:.3f}%)\n";
  //avoid divide by zero, return 0. instead
  auto f = [](unsigned den)->double{ 
    if (den > 0) return 100./den;
    return 0.;
  };
  exeFile << std::format(fmt, "EMB",probs[EMB], nChans[EMB], probs[EMB]*f(nChans[EMB]));
  exeFile << std::format(fmt, "EMEC",probs[EMEC], nChans[EMEC], probs[EMEC]*f(nChans[EMEC]));
  exeFile << std::format(fmt, "EM tot",probs[EMEC]+probs[EMB],nChans[EMB]+nChans[EMEC],(probs[EMEC]+probs[EMB])*f(nChans[EMEC]+nChans[EMB]));
  exeFile << std::format(fmt, "EMEC",probs[HEC], nChans[HEC], probs[HEC]*f(nChans[HEC]));
  exeFile << std::format(fmt, "EMEC",probs[FCAL], nChans[FCAL], probs[FCAL]*f(nChans[FCAL]));
  exeFile << std::format(fmt, "Total",nTotProb,nTot,nTotProb*f(nTot));

  exeFile << std::endl;

}

