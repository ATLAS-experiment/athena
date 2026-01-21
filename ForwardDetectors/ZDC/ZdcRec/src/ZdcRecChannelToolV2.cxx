/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
 * ZdcRecChannelTool.cxx
 *
 *  Created on: Nov 24, 2009
 *      Author: steinber
 */


#include <iostream>
#include <fstream>

#include <cmath>
#include <map>
#include <vector>

#include "TMath.h"

#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/MsgStream.h"

#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include "xAODTrigL1Calo/TriggerTowerAuxContainer.h"
#include "xAODForward/ZdcModuleToString.h"
#include "xAODForward/ZdcModuleContainer.h"
#include "xAODForward/ZdcModuleAuxContainer.h"
#include "ZdcRec/ZdcRecChannelToolV2.h"
#include "ZdcRec/ZdcSignalSinc.h"
#include "ZdcByteStream/ZdcToString.h"


#include "ZdcIdentifier/ZdcID.h"
#include "ZdcConditions/ZdcCablingService.h"
#include "AthContainers/Accessor.h"

const int      slink2ppmChannel[64] =
  {0,  4,  8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,
   3,  7, 11,  15,  19,  23,  27,  31,  35,  39,  43,  47,  51,  55,  59,  63,
   1,  5,  9,  13,  17,  21,  25,  29,  33,  37,  41,  45,  49,  53,  57,  61,
   2,  6, 10,  14,  18,  22,  26,  30,  34,  38,  42,  46,  50,  54,  58,  62 };


//==================================================================================================
ZdcRecChannelToolV2::ZdcRecChannelToolV2(const std::string& name):
  AsgTool(name)
{
	//Declare properties here...

	declareInterface<ZdcRecChannelToolV2>(this);
	declareProperty("ZeroSuppress", m_zeroSupress = 0,"Supress channels with only 0");
	declareProperty("DeltaPeak", m_delta = 5,"Minimum difference between min and max to be considered a signal");
	declareProperty("SaturationADC",m_saturation = 1000,"ADC value above which a HG channel is considered saturated");
	declareProperty("NSamples",m_nsamples=5,"Number of samples in PPM raw data");
	declareProperty("UseDelay",m_useDelay=0,"If true, then use delayed channels");
	declareProperty("SampleTime",m_sample_time=12.5,"Sample time in ns (25. = 40 MHz, 12.5 = 80 MHz)");
	declareProperty("GainFactor",m_gainFactor=10.,"High/lo gain factor");
	declareProperty("PedestalValue",m_pedestalValue=100.,"Pedestal value");
}
//==================================================================================================


void ZdcRecChannelToolV2::handle( const Incident& inc )
{
  if ( inc.type() == IncidentType::EndEvent) {

  }
}


//==================================================================================================
StatusCode ZdcRecChannelToolV2::initialize()
{
	msg(MSG::INFO) << "Initializing " << name() << endmsg;

	//Get the pedestal information for the channels.
	//For now, this is a file; later on it will be moved to a database

	// NONE
	
	const ZdcID* zdcId = nullptr;
	if (detStore()->retrieve( zdcId ).isFailure() ) {
	    msg(MSG::ERROR) << "execute: Could not retrieve ZdcID object from the detector store" << endmsg;
	    return StatusCode::FAILURE;
	}
	else {
	    msg(MSG::DEBUG) << "execute: retrieved ZdcID" << endmsg;
	}
	m_zdcId = zdcId;

	msg(MSG::DEBUG) << "--> ZDC : END OF MODIFICATION 0" << endmsg ;
	return StatusCode::SUCCESS;

}
//==================================================================================================

//==================================================================================================
StatusCode ZdcRecChannelToolV2::finalize()
{
	msg(MSG::INFO) << "Finalizing " << name() << endmsg;

	return StatusCode::SUCCESS;
}
//==================================================================================================

int ZdcRecChannelToolV2::convertTT2ZM(const xAOD::TriggerTowerContainer* ttCollection, xAOD::ZdcModuleContainer* zdcModules, xAOD::ZdcModuleContainer* zdcSums ) const
{

  //typedef std::map<uint64_t,xAOD::ZdcModule*> hashmapType;
  typedef std::map<uint32_t,xAOD::ZdcModule*> hashmapType;
  hashmapType digits_map;
  Identifier chan_id;
  
  //std::cout << "Zdc TT's have " << ttCollection->size() << " towers" << std::endl;

  for (const xAOD::TriggerTower* tt : *ttCollection)
    {
      //std::cout << "ZdcTT coolId = " << tt->coolId() << std::endl;
      //std::cout << ZdcToString(*tt) << std::endl;

      uint32_t coolId = tt->coolId();
      uint32_t pin = (coolId>>8) & 0xf;
      uint32_t asic = coolId & 0xf;
      uint32_t slinkChannel = asic*16 + pin;
      uint32_t ppmChannel = slink2ppmChannel[slinkChannel];

      uint32_t module = (coolId>>16) & 0xf;

      ATH_MSG_DEBUG( "--> ZCS: " << ZdcCablingService::getInstance() << " mod=" << module << " slinkC=" << slinkChannel << " ppmC=" << ppmChannel );

      chan_id = ZdcCablingService::getInstance()->h2s_channel_id(module, ppmChannel);

      const uint32_t chan_hash = chan_id.get_identifier32().get_compact();

      int gain  = ZdcCablingService::getInstance()->hwid2gain(module,ppmChannel);
      int delay = ZdcCablingService::getInstance()->hwid2delay(module,ppmChannel);

      int side = m_zdcId->side(chan_id);
      int mod = m_zdcId->module(chan_id);
      int type = m_zdcId->type(chan_id);
      int channel = m_zdcId->channel(chan_id);

      ATH_MSG_DEBUG( "Trying to set data of " << std::hex << chan_hash << std::dec << " side=" << side << " mod=" << mod << " type=" << type << " channel=" << channel << " gain=" << gain << " delay=" << delay);
      
      hashmapType::iterator iter = digits_map.find(chan_hash);
      if (iter == digits_map.end())
	{

	  xAOD::ZdcModule* new_mod = new xAOD::ZdcModule();
	  ATH_MSG_DEBUG("new module for " << chan_hash << std::hex << " new_mod=" << new_mod);

	  zdcModules->push_back(xAOD::ZdcModuleContainer::unique_type(new_mod));

	  new_mod->setZdcId(chan_hash);
	  new_mod->setZdcSide(side);
	  new_mod->setZdcModule(mod);
	  new_mod->setZdcType(type);
	  new_mod->setZdcChannel(channel);

	  digits_map.insert(std::pair<uint32_t,xAOD::ZdcModule*>(chan_hash,new_mod));
	  iter = digits_map.find(chan_hash);
	}

      if (iter != digits_map.end())
	{
	  ATH_MSG_DEBUG("adding data to " << std::hex << (*iter).first << " p=" << (*iter).second << " from tt=" << tt << " zdcModule=" << (*iter).second->zdcModule());
          static const SG::Accessor<std::vector<uint16_t> > g0d1dataAcc ("g0d1data");
          static const SG::Accessor<std::vector<uint16_t> > g1d1dataAcc ("g1d1data");
          static const SG::Accessor<std::vector<uint16_t> > g0d0dataAcc ("g0d0data");
          static const SG::Accessor<std::vector<uint16_t> > g1d0dataAcc ("g1d0data");
	  if (gain==0&&delay==0) g0d0dataAcc(*iter->second) = tt->adc() ;
	  if (gain==0&&delay==1) g0d1dataAcc(*iter->second) = tt->adc() ;
	  if (gain==1&&delay==0) g1d0dataAcc(*iter->second) = tt->adc() ;
	  if (gain==1&&delay==1) g1d1dataAcc(*iter->second) = tt->adc() ;
	  ATH_MSG_DEBUG("added data to " << std::hex << (*iter).first << " p=" << (*iter).second << " from tt=" << tt);
	}      
    }

  ATH_MSG_INFO("adding side data");

  // create ZDC sides
  for (int iside = 0;iside<2;iside++)
    {
      xAOD::ZdcModule* new_sum = new xAOD::ZdcModule();
      zdcSums->push_back(xAOD::ZdcModuleContainer::unique_type(new_sum));
      new_sum->setZdcSide(iside ? 1 : -1);
    }

  return 0;


}


