//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef READLARDIGITS_H
#define READLARDIGITS_H
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "TBEvent/TBLArDigitContainer.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/SmartDataPtr.h"
//#include "LArDetDescr/LArDetDescrManager.h"
#include <fstream>
#include "LArIdentifier/LArOnlineID.h"
#include "StoreGate/ReadCondHandleKey.h"

class ReadTBLArDigits : public AthAlgorithm
{
 public:
  ReadTBLArDigits(const std::string & name, ISvcLocator * pSvcLocator);

  ~ReadTBLArDigits();

  //standart algorithm methods
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;

 private:
  int m_count{0};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey
    {this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
  const LArEM_ID*   m_emId{nullptr};
  const LArFCAL_ID* m_fcalId{nullptr};
  const LArHEC_ID*  m_hecId{nullptr};
  const LArOnlineID* m_onlineHelper{nullptr};
  std::ofstream m_outfile;
  Gaudi::Property<std::string> m_containerKey{this, "ContainerKey", "", "SG key of the TBLArDigitContainer (empty: retrieve default)"};
  Gaudi::Property<std::string> m_dumpFile{this, "DumpFile", "", "Name of the text dump file (empty: no dump)"};
  Gaudi::Property<bool> m_printCellLoc{this, "PrintCellLocation", false, "Print cell layer/eta/phi in the dump file"};
  Gaudi::Property<bool> m_printFebChan{this, "PrintFebChan", true, "Print FEB ID and channel in the dump file"};
  //std::string m_NTupleLoc;
  
  NTuple::Tuple* m_ntuplePtr{nullptr};
  //NTuple::Item<long> m_cellIndex;
  NTuple::Item<long> m_cellIndex;
  NTuple::Array<long> m_layer, m_eta, m_phi, m_gain;
  NTuple::Array<long> m_barrel_ec, m_pos_neg, m_FT, m_slot, m_channel;
  NTuple::Matrix<long> m_samples; 
  NTuple::Item<long>  m_Nsamples;
  //NTuple::Array<long>* m_samples;
  //int m_NSamples;
  
  class SortDigits
    {
     public:
      SortDigits(const LArOnlineID* onlineHelper);
      inline bool operator()(LArDigit* a, LArDigit* b);
     private:
      const LArOnlineID* m_onlineHelper;
    };
};


inline bool  ReadTBLArDigits::SortDigits::operator()(LArDigit* a, LArDigit* b)
{const HWIdentifier chid_a=a->channelID();
 const HWIdentifier chid_b=b->channelID();
 if (m_onlineHelper->feb_Id(chid_a).get_compact() == m_onlineHelper->feb_Id(chid_b).get_compact())
   return (m_onlineHelper->channel(chid_a) <  m_onlineHelper->channel(chid_b));
 else
   return (m_onlineHelper->feb_Id(chid_a).get_compact() < m_onlineHelper->feb_Id(chid_b).get_compact());
}


#endif
