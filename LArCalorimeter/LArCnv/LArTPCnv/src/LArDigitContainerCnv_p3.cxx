/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArTPCnv/LArDigitContainerCnv_p3.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "Identifier/Identifier.h"
#include "LArIdentifier/LArOnlineID_Base.h"
#include "GaudiKernel/GaudiException.h"
#include "StoreGate/StoreGateSvc.h"

#include "AthAllocators/DataPool.h"

#include <algorithm>
#include <cstdlib>

namespace {

  void writeTwoBits(std::vector<unsigned char>& vec, const unsigned idx, const unsigned char val) {
    const auto qr=std::div(idx,4);
    const unsigned char maskval=(val&0x3) << 2*qr.rem;
    vec[qr.quot]|=maskval;
    return;
  }

  unsigned char readTwoBits(const std::vector<unsigned char>& vec, const unsigned idx) {
    const auto qr=std::div(idx,4);
    const unsigned char& v=vec[qr.quot];
    return (v>>2*qr.rem) & 0x3;
  }
}

LArDigitContainerCnv_p3::LArDigitContainerCnv_p3(const LArOnlineID_Base* idHelper,
						 const LArOnlineID_Base* idSCHelper,
						 const StoreGateSvc* storeGateSvc) :
  m_idHelper(idHelper),
  m_idSCHelper(idSCHelper),
  m_storeGateSvc(storeGateSvc)
{}

void
LArDigitContainerCnv_p3::persToTrans(const LArDigitContainer_p3* pers,
				     LArDigitContainer* trans, MsgStream &/*log*/) const
{

  DataPool<LArDigit> dataItems;
  if (pers->m_nSamples==0 || pers->m_samples.size()==0) {
    //No data
    return;
  }

  const unsigned nMaxChannels=4*(pers->m_gain.size());
  trans->clear(SG::VIEW_ELEMENTS);
  size_t numElements = pers->m_samples.size()/pers->m_nSamples;
  //
  dataItems.reserve(numElements);
  trans->reserve(numElements);

  const LArOnlineID_Base* idHelper = nullptr;
  if (!pers->m_this_is_slar) idHelper = m_idHelper;
  else idHelper = m_idSCHelper;

  std::vector<short>::const_iterator samplesIt=pers->m_samples.begin();
  for(unsigned idx=0;idx<nMaxChannels;idx++) {
    const unsigned char gain=readTwoBits(pers->m_gain,idx);
    if (gain!=0) {
      const HWIdentifier hwid=idHelper->channel_Id(IdentifierHash(idx));
      std::vector<short>::const_iterator samplesIt_e=samplesIt+pers->m_nSamples;
      std::vector<short> samples(samplesIt,samplesIt_e);
      LArDigit* lardigi = dataItems.nextElementPtr();
      (*lardigi) =
          LArDigit(hwid, (CaloGain::CaloGain)(gain - 1), std::move(samples));
      trans->push_back(lardigi);
      samplesIt = samplesIt_e;
    }//end if gain!=0, eg channel present
  }//end loop over all hashes
}


void
LArDigitContainerCnv_p3::transToPers(const LArDigitContainer* trans,
				     LArDigitContainer_p3* pers, MsgStream &log) const {

  const unsigned int nChannels=trans->size();
  if (nChannels==0) {
    pers->m_nSamples=0; //Avoid undefined value in output file
    return;
  }

  //Store the number of ADC samples from first digit
  pers->m_nSamples=(*(trans->begin()))->samples().size();

  //Determine whether we are dealing with standard cells or supercells
  const LArOnlineID_Base* idHelper = nullptr;
  const SG::DataProxy* proxy = m_storeGateSvc->proxy(trans);
  std::string containerName = proxy->name();
  if (containerName.find("SC")==std::string::npos) {
    idHelper = m_idHelper;
    pers->m_this_is_slar = false;
  }
  else {
    idHelper = m_idSCHelper;
    pers->m_this_is_slar = true;
  }

  const unsigned hashMax=idHelper->channelHashMax();

  //Resize persistant storage
  pers->m_gain.assign(hashMax/4,0);
  pers->m_samples.assign(pers->m_nSamples*nChannels,0);

  //Copy pointer to digits into a hash-indexed vector, nullptr for non-existing digits
  std::vector<const LArDigit*> digitsvsHash(hashMax,nullptr);
  for (const LArDigit* transDigit : *trans) {
    const IdentifierHash h=idHelper->channel_Hash(transDigit->hardwareID());
    if (transDigit->samples().size()!=pers->m_nSamples) [[unlikely]] {
      log << MSG::ERROR << "Encountered LArDigit Container with varying numbers of ADC samples" << endmsg;
      throw GaudiException("Encountered LArDigit Container with varying numbers of ADC samples",
			   "LArDigitContainerCnv_p3",StatusCode::FAILURE);
    }
    digitsvsHash[h]=transDigit;
  }

  unsigned idx=0;
  std::vector<short>::iterator writeIt=pers->m_samples.begin();
  std::vector<const LArDigit*>::const_iterator it=digitsvsHash.begin();
  std::vector<const LArDigit*>::const_iterator it_e=digitsvsHash.end();
  for (;it!=it_e;++it,++idx) {
    const LArDigit* dig=*it;
    if (dig!=nullptr) {
      //A gain value of 0 means no data for this hash-value
      //Store gain+1
      writeTwoBits(pers->m_gain,idx,1+dig->gain());
      //Copy ADC samples
      writeIt=std::copy(dig->samples().begin(),dig->samples().end(),writeIt);
    }
  }
}
