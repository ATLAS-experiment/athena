/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_DIGITIZATION_TOOL_H
#define ZDC_DIGITIZATION_TOOL_H

#include "PileUpTools/PileUpToolBase.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"

#include "AthenaKernel/IAthRNGSvc.h"
#include "ZDC_SimEvent/ZDC_SimFiberHit.h"
#include "ZDC_SimEvent/ZDC_SimFiberHit_Collection.h"
#include "xAODForward/ZdcModuleContainer.h"
#include "xAODForward/ZdcModuleAuxContainer.h"
#include "HitManagement/TimedHitCollection.h"
#include "PileUpTools/PileUpMergeSvc.h"

#include <vector>
#include <string>
#include <memory>


class ZdcID;
class ZDCWaveformSampler;

namespace CLHEP {
  class HepRandomEngine;
}

typedef PileUpMergeSvc::TimedList<ZDC_SimFiberHit_Collection>::type TimedFiberHitCollList;

class ZDC_PileUpTool: public PileUpToolBase {

public:

  ZDC_PileUpTool(const std::string& type,
                 const std::string& name,
                 const IInterface* parent);

  virtual StatusCode initialize() override final;
  void initializePbPb2015();
  void initializeLHCf2022();
  void initializePbPb2023();
  virtual StatusCode finalize() override final { return StatusCode::SUCCESS; }

  /// called before the subevts loop. Not (necessarily) able to access SubEvents
  virtual StatusCode prepareEvent(const EventContext& ctx,const unsigned int nInputEvents) override final;

  /// called for each active bunch-crossing to process current SubEvents bunchXing is in ns
  virtual StatusCode processBunchXing( int bunchXing,
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

  void fillContainer(TimedHitCollection<ZDC_SimFiberHit>&, CLHEP::HepRandomEngine*, xAOD::ZdcModuleContainer*);
  void fillContainer(const ZDC_SimFiberHit_Collection*, CLHEP::HepRandomEngine*, xAOD::ZdcModuleContainer*);
  TimedHitCollection<ZDC_SimFiberHit> doZDClightGuideCuts(const ZDC_SimFiberHit_Collection* hitCollection);
  void createAndStoreWaveform(const ZDC_SimFiberHit &hit, CLHEP::HepRandomEngine*, xAOD::ZdcModuleContainer*);
  void addEmptyWaveforms(xAOD::ZdcModuleContainer *zdcModuleContainer, CLHEP::HepRandomEngine* rndEngine);
  std::vector<short unsigned int> generateWaveform(std::shared_ptr<ZDCWaveformSampler> wfSampler, float amplitude, float t0);
  
  void SetDumps(bool, bool);
  
  ZDC_SimFiberHit_Collection *m_mergedFiberHitList{};
  const ZdcID* m_ZdcID{};
  std::unique_ptr<xAOD::ZdcModuleContainer> m_ZdcModuleContainer;
  std::unique_ptr<xAOD::ZdcModuleAuxContainer> m_ZdcModuleAuxContainer;

  ServiceHandle<PileUpMergeSvc> m_mergeSvc{this, "mergeSvc", "PileUpMergeSvc", ""};
  ServiceHandle<IAthRNGSvc> m_randomSvc{this, "RndmSvc", "AthRNGSvc", ""};
  Gaudi::Property<std::string> m_randomStreamName{this, "RandomStreamName", "ZDCRndEng", ""};

  SG::ReadHandleKey<ZDC_SimFiberHit_Collection> m_SimFiberHitCollectionKey{this, "ZDC_SimFiberHit_CollectionName", "ZDC_SimFiberHit_Collection"};
  SG::WriteHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleContainerName{this, "ZdcModuleContainerName", "ZdcModules"};
  SG::WriteHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumsContainerName{this, "ZdcSumsContainerName", "ZdcSums"};

  Gaudi::Property<std::string> m_HitCollectionName{this, "HitCollectionName" , "ZDC_SimFiberHit_Collection",
      "Name of the input Collection of the simulation Hits"};
  Gaudi::Property<std::string> m_outputContainerName{this, "OutputContainerName" , "ZDC_SimModuleContainer",
      "Name of the output ZDC module container"};

  Gaudi::Property<std::string> m_configuration      {this, "configuration"      , "PbPb2023", "Named configuration to be used. Overwrites other properties if used" };
  Gaudi::Property<int   >      m_Pedestal           {this, "Pedestal"           , 100       , "DC offset of the pulse in ADC"                                       };
  Gaudi::Property<int   >      m_numTimeBins        {this, "MaxTimeBin"         , 7         , "The number of time-slices after digitization(Typically 5 or 7)"      };
  Gaudi::Property<double>      m_freqMHz            {this, "freqMHz"            , 40        , "Digitizer frequence in MHz"                                          };
  Gaudi::Property<double>      m_zdct0              {this, "zdct0"              , 40        , "Start time of the pulse in the digitization window"                  };
  Gaudi::Property<double>      m_rpdt0              {this, "rpdt0"              , 40        , "Start time of the pulse in the digitization window"                  };
  Gaudi::Property<double>      m_zdcRiseTime        {this, "zdcRiseTime"        , 4         , "Rise time of the ZDC pulses"                                         };
  Gaudi::Property<double>      m_zdcFallTime        {this, "zdcFallTime"        , 0.5       , "Fall time of the ZDC pulses"                                         };
  Gaudi::Property<double>      m_rpdRiseTime        {this, "rpdRiseTime"        , 4         , "Rise time of the RPD pulses"                                         };
  Gaudi::Property<double>      m_rpdFallTime        {this, "rpdFallTime"        , 0.5       , "Fall time of the RPD pulses"                                         };
  Gaudi::Property<double>      m_qsfRiseTime        {this, "qsfRiseTime"        , 4         , "Rise time of the RPD pulses"                                         };
  Gaudi::Property<double>      m_qsfFallTime        {this, "qsfFallTime"        , 4         , "Rise time of the RPD pulses"                                         };
  Gaudi::Property<double>      m_qsfFilter          {this, "qsfFilter"          , 4         , "Rise time of the RPD pulses"                                         };
  Gaudi::Property<float >      m_zdcAdcPerPhoton    {this, "zdcAdcPerPhoton"    , 0.000498  , "ADC counts per detected photon in the ZDCs"                          };
  Gaudi::Property<bool  >      m_LTQuadStepFilt     {this, "LTQuadStepFilt"     , false     , "Use LT Quad Step Filter waveform for ZDC channels"                   };
  Gaudi::Property<bool  >      m_delayChannels      {this, "delayChannels"      , false     , "Include delayed channels in the output"                              };
  Gaudi::Property<bool  >      m_doRPD              {this, "doRPD"              , false     , "Include RPD channels in the output"                                  };
  Gaudi::Property<bool  >      m_doBRAN             {this, "doBRAN"             , false     , "Include BRAN channels in the output"                                 };
  Gaudi::Property<std::vector< float> > m_rpdAdcPerPhoton {this, "rpdAdcPerPhoton", {5.25 , 1.7}, "ADC counts per detected photon in the RPDs"};
  
};

#endif
