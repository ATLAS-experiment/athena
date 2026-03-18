/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Implementation of LArRodEncoder class
#include "LArByteStream/LArRodEncoder.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IToolSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "eformat/Version.h"
#include <cstdio>
#include <stdio.h>
#include <Identifier/HWIdentifier.h>


// constructor 
LArRodEncoder::LArRodEncoder (const LArOnlineID& onlineHelper,
                              const CaloDetDescrManager& calodd,
                              const LArOnOffIdMapping& onOffIdMapping,
                              LArRodBlockStructure* BlStruct)
  : m_onlineHelper (onlineHelper),
    m_CaloDetDescrManager (calodd),
    m_onOffIdMapping (onOffIdMapping),
    m_BlStruct (BlStruct)
{
}

// destructor 
LArRodEncoder::~LArRodEncoder()
{
  clear();
}

// Add LArRawChannel
void LArRodEncoder::add(const LArRawChannel* rc)
{
 uint32_t FEB_ID = (m_onlineHelper.feb_Id(rc->channelID()).get_identifier32().get_compact());
 if ( m_mFEB[FEB_ID].vLArRC.empty() ){
	m_mFEB[FEB_ID].vLArRC.resize(128,0);
 }
 uint32_t chan = m_BlStruct->FebToRodChannel(m_onlineHelper.channel( rc->channelID() ) );
 m_mFEB[FEB_ID].vLArRC[chan]=rc;
}

// Add free gain digits
void LArRodEncoder::add(const LArDigit* dg)
{
 uint32_t FEB_ID = (m_onlineHelper.feb_Id(dg->hardwareID()).get_identifier32().get_compact());
 m_mFEB[FEB_ID].vLArDigit.push_back(dg);
}
//Add fixed gain digits
void LArRodEncoder::add(const LArDigit* dg, const int gain)
{
 uint32_t FEB_ID = (m_onlineHelper.feb_Id(dg->hardwareID()).get_identifier32().get_compact());
 m_mFEB[FEB_ID].vLArDigitFixed[gain].push_back(dg);
}
//Add calibration digits
void LArRodEncoder::add(const LArCalibDigit* dg, const int gain)
{
 uint32_t FEB_ID = (m_onlineHelper.feb_Id(dg->hardwareID()).get_identifier32().get_compact());
 m_mFEB[FEB_ID].vLArCalibDigit[gain].push_back(dg);
}


void LArRodEncoder::clear()
{
  m_mFEB.clear();
}


// convert all LArRawChannels and LArDigits
// in the current list to a vector of 32bit words
void LArRodEncoder::fillROD(std::vector<uint32_t>& v, MsgStream& logstr, const CaloNoise& noise, double nsigma)
{
 if (!m_BlStruct)
  {logstr << MSG::ERROR << "No LArRodBlockStructure defined! Can't encode fragment!" << endmsg; 
   return;
  }
 m_BlStruct->initializeFragment(v); //Makes new Fragment or splits v into existing Feb-Blocks
 std::map<uint32_t,FebData_t>::iterator it=m_mFEB.begin();
 std::map<uint32_t,FebData_t>::iterator it_end=m_mFEB.end();
 // The sort alorithm for the vectors inside the data object of the map fails
 // when I use const_iterator at this point. Don't ask me why
 for(;it!=it_end;++it) {
    m_BlStruct->initializeFEB(it->first);                                     // Set FEB id
    // ************* Energy Block *************
    if (m_BlStruct->canSetEnergy() && it->second.vLArRC.size()>0) {
      // compute Ex, Ey for this FEB : initialisation phase
      {
	double Ex=0;
	double Ey=0;
	double Ez=0;
	double SumE=0;
	Identifier myofflineID;

        for (const LArRawChannel *theChannel : it->second.vLArRC) {
	  if ( theChannel != nullptr ){
	  int cId =  m_onlineHelper.channel(theChannel->hardwareID());

	  int e=theChannel->energy();
          uint32_t quality = theChannel->quality();
	  m_BlStruct->setNextEnergy(cId,e, theChannel->time(), 
				    quality,theChannel->gain());
	  
	  // you convert from hardwareID to offline channle ID hash (???)
	  myofflineID = m_onOffIdMapping.cnvToIdentifier(theChannel->hardwareID()) ;
          const CaloDetDescrElement* caloDDE = m_CaloDetDescrManager.get_element(myofflineID);
	  // This is probably NOT what one wants. You want the cell gain!
          double cellnoise = noise.getNoise(myofflineID,theChannel->gain());
          if( e > (nsigma*cellnoise) && (quality != 65535 ) ){
	    double aux=caloDDE->sinTh();
	    double aux_z=tanh(caloDDE->eta());
            Ex += e*aux*caloDDE->cosPhi(); 
	    Ey += e*aux*caloDDE->sinPhi();
	    Ez += e*aux_z;
	    SumE += e;
          }
	 } // End of if to check for the LArRawChannel existence
        } // end of chit loop
	// Set after the loop finishes
	m_BlStruct->setEx(Ex);
	m_BlStruct->setEy(Ey);
	m_BlStruct->setEz(Ez);
	m_BlStruct->setSumE(SumE);
      }
    }//End canSetEnergy
    // try to include samples together with data
    if (m_BlStruct->canIncludeRawData() && it->second.vLArDigit.size()>0) {
      // Order channels according to ROD numbering
      m_BlStruct->sortDataVector(it->second.vLArDigit);
      for (const LArDigit* digit : it->second.vLArDigit) {
        int cId =  m_onlineHelper.channel(digit->hardwareID());
        m_BlStruct->setRawData(cId, digit->samples(), digit->gain());
      } // end of for digits
    } // End of can Include Raw Data check
    // ************** Raw Data Block ***********
    if (m_BlStruct->canSetRawData() && it->second.vLArDigit.size()>0) {
      m_BlStruct->sortDataVector(it->second.vLArDigit);
      std::vector<const LArDigit*>::const_iterator digit_it = it->second.vLArDigit.begin();
      std::vector<const LArDigit*>::const_iterator digit_it_end=it->second.vLArDigit.end();
      if(digit_it!=digit_it_end) { //Container not empty
	m_BlStruct->setNumberOfSamples((*digit_it)->samples().size());
	for (;digit_it!=digit_it_end;++digit_it)  {
	  int cId =  m_onlineHelper.channel((*digit_it)->hardwareID());
	  m_BlStruct->setRawData(cId, (*digit_it)->samples(), (*digit_it)->gain());
	} 
      }// end container not empty
    }// end if can set raw data    

    // ************** Fixed Gain Data Block ***********
    if (m_BlStruct->canSetRawDataFixed()) {
      for (int i=0;i<3;i++) { //Loop over three gains
	m_BlStruct->sortDataVector(it->second.vLArDigitFixed[i]);
	std::vector<const LArDigit*>::const_iterator digit_it = it->second.vLArDigitFixed[i].begin();
	std::vector<const LArDigit*>::const_iterator digit_it_end=it->second.vLArDigitFixed[i].end();
	if(digit_it!=digit_it_end) {//Container not empty
	  m_BlStruct->setNumberOfSamples((*digit_it)->samples().size());
	  for (;digit_it!=digit_it_end;++digit_it)  {
	    int cId =  m_onlineHelper.channel((*digit_it)->hardwareID()); 
	    m_BlStruct->setRawDataFixed(cId, (*digit_it)->samples(), (*digit_it)->gain());
	  }
	} //end Container not empty
      } // end loop over three gains
    } // end can set raw data fixed
  
    // ************** Calibration Digit Block ***********
    if (m_BlStruct->canSetCalibration()) { 
        for (int i=0;i<3;i++) { //Loop over three gains
	  m_BlStruct->sortDataVector(it->second.vLArCalibDigit[i]);
	  std::vector<const LArCalibDigit*>::const_iterator digit_it = it->second.vLArCalibDigit[i].begin();
	  std::vector<const LArCalibDigit*>::const_iterator digit_it_end=it->second.vLArCalibDigit[i].end();
	  if(digit_it!=digit_it_end) {//Container not empty
	    m_BlStruct->setNumberOfSamples((*digit_it)->samples().size());
	    m_BlStruct->setDelay((*digit_it)->delay());
	    m_BlStruct->setDAC((*digit_it)->DAC());
	    for (;digit_it!=digit_it_end;++digit_it) { 
	      int cId =  m_onlineHelper.channel((*digit_it)->hardwareID()); 
	      m_BlStruct->setRawDataFixed(cId, (*digit_it)->samples(), (*digit_it)->gain());
	      if ((*digit_it)->isPulsed())
		m_BlStruct->setPulsed(cId);
	    }
	  } //End Container not empty
	}// end loop over three gains
    } //end can set calibration
    
    m_BlStruct->finalizeFEB();
 }// end iterating over FEB-map
 m_BlStruct->concatinateFEBs();
 return; 
}
