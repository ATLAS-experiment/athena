/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ALFA_PILEUP_TOOL_H
#define ALFA_PILEUP_TOOL_H

#include "PileUpTools/PileUpToolBase.h"

#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include "HitManagement/TimedHitCollection.h"

#include "ALFA_SimEv/ALFA_HitCollection.h"
#include "ALFA_SimEv/ALFA_ODHitCollection.h"

#include "ALFA_RawEv/ALFA_DigitCollection.h"
#include "ALFA_RawEv/ALFA_ODDigitCollection.h"

#include "PileUpTools/PileUpMergeSvc.h"

#include <string>
#include <vector>

namespace CLHEP {
  class HepRandomEngine;
}

class ALFA_PileUpTool: public PileUpToolBase {

 public:
  
  ALFA_PileUpTool(const std::string& type,
		 const std::string& name,
		 const IInterface* parent);
  
  virtual StatusCode initialize() override final;
  virtual StatusCode finalize() override final;
  
  /// code taken from ZDC; author (highly) probably John Chapman
  /// called before the subevts loop. Not (necessarily) able to access SubEvents
  virtual StatusCode prepareEvent(const EventContext& ctx, const unsigned int nInputEvents) override final;
  
  /// called for each active bunch-crossing to process current SubEvents bunchXing is in ns
  virtual  StatusCode processBunchXing(
                                       int bunchXing,
                                       SubEventIterator bSubEvents,
                                       SubEventIterator eSubEvents
                                       ) override final;
  /// return false if not interested in  certain xing times (in ns)
  /// implemented by default in PileUpToolBase as FirstXing<=bunchXing<=LastXing
  //  virtual bool toProcess(int bunchXing) const;

  /// called at the end of the subevts loop. Not (necessarily) able to access SubEvents
  virtual StatusCode mergeEvent(const EventContext& ctx) override final;

  virtual StatusCode processAllSubEvents(const EventContext& ctx) override final;

 private:
   
   StatusCode recordCollection(ServiceHandle<StoreGateSvc>& evtStore, const std::string& key_digitCnt);
   
   void ALFA_MD_info(const ALFA_HitCollection*);
   void ALFA_MD_info(TimedHitCollection<ALFA_Hit>&);

   
   StatusCode fill_MD_DigitCollection(CLHEP::HepRandomEngine*);     

   StatusCode recordODCollection(ServiceHandle<StoreGateSvc>& evtStore, const std::string& key_ODdigitCnt);
   
   void ALFA_OD_info(const ALFA_ODHitCollection*);
   void ALFA_OD_info(TimedHitCollection<ALFA_ODHit>&);
 
   
   StatusCode fill_OD_DigitCollection(CLHEP::HepRandomEngine*);
   StatusCode XTalk(); 
 
  
  ServiceHandle<PileUpMergeSvc> m_mergeSvc{this, "mergeSvc", "PileUpMergeSvc", ""};
  ServiceHandle<IAthRNGSvc> m_randomSvc{this, "RndmSvc", "AthRNGSvc", ""};
  Gaudi::Property<std::string> m_randomStreamName{this, "RandomStreamName", "ALFARndEng", ""};

  double m_E_fib[8][20][64]{};
  double m_E_ODfib[8][2][3][30]{};

  
  std::string m_SimHitCollectionName;
  std::string m_SimODHitCollectionName;
  std::string m_key_DigitCollection;
  std::string m_key_ODDigitCollection;
  
  ALFA_DigitCollection* m_digitCollection{};
  ALFA_ODDigitCollection* m_ODdigitCollection{};
 
  
  double m_sigma0{};          
  double m_sigma1{};          
  double m_meanE_dep{};       
  double m_meanN_photo{};      
  double m_mean{};             
  double m_stdDev{};
  double m_AmplitudeCut{};   
 
  ALFA_HitCollection      *m_mergedALFA_HitList{};
  ALFA_ODHitCollection    *m_mergedALFA_ODHitList{};
  
  double m_cross_talk[8][127]{};
  int m_fibres[8][20][64]{};
		
  
};
 
#endif
