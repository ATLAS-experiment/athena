/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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

  
  Gaudi::Property<std::string> m_SimHitCollectionName{this, "SimHitCollection", "ALFA_HitCollection", "Name of the input ALFA Sim Hit Collection of simulated hits"};
  Gaudi::Property<std::string> m_SimODHitCollectionName{this, "SimODHitCollection", "ALFA_ODHitCollection", "Name of the input ALFA Sim OD Hit Collection of simulated hits"};
  Gaudi::Property<std::string> m_key_DigitCollection{this, "ALFA_DigitCollection", "ALFA_DigitCollection", "Name of the Collection to hold the output from the ALFA main detector digitization"};
  Gaudi::Property<std::string> m_key_ODDigitCollection{this, "ALFA_ODDigitCollection", "ALFA_ODDigitCollection", "Name of the Collection to hold the output from the ALFA OD digitization"};
  
  ALFA_DigitCollection* m_digitCollection{};
  ALFA_ODDigitCollection* m_ODdigitCollection{};
 
  
  Gaudi::Property<double> m_sigma0{this, "sigma0", 0.1};
  Gaudi::Property<double> m_sigma1{this, "sigma1", 1.0};
  Gaudi::Property<double> m_meanE_dep{this, "meanE_dep", 0.0863}; //MeV
  Gaudi::Property<double> m_meanN_photo{this, "meanN_photo", 4.11};
  Gaudi::Property<double> m_mean{this, "mean", 0.0};
  Gaudi::Property<double> m_stdDev{this, "stdDev", 1.0};
  Gaudi::Property<double> m_AmplitudeCut{this, "AmplitudeCut", 0.24};
 
  ALFA_HitCollection      *m_mergedALFA_HitList{};
  ALFA_ODHitCollection    *m_mergedALFA_ODHitList{};
  
  double m_cross_talk[8][127]{};
  int m_fibres[8][20][64]{};
		
  
};
 
#endif
