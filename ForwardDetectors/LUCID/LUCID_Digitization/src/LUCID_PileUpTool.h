/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LUCID_DIGITIZATION_TOOL_H
#define LUCID_DIGITIZATION_TOOL_H

#include "PileUpTools/PileUpToolBase.h"

#include "LUCID_DigitizationToolBox.h"

#include "LUCID_SimEvent/LUCID_SimHitCollection.h"
#include "LUCID_RawEvent/LUCID_DigitContainer.h"

#include "xAODEventInfo/EventInfo.h"   // SubEventIterator
#include "xAODEventInfo/EventAuxInfo.h"// SubEventIterator

#include "AthenaKernel/IAthRNGSvc.h"
#include "PileUpTools/PileUpMergeSvc.h"

#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"

#include <vector>
#include <utility> /* pair */

namespace CLHEP {
  class HepRandomEngine;
}

class LUCID_PileUpTool: public PileUpToolBase, public LUCID_DigitizationToolBox {

public:

  using PileUpToolBase::PileUpToolBase;

  virtual StatusCode initialize() override final;
  virtual StatusCode finalize() override final;

  /// called before the subevts loop. Not (necessarily) able to access SubEvents
  virtual StatusCode prepareEvent(const EventContext& ctx, const unsigned int nInputEvents) override final;

  /// called for each active bunch-crossing to process current SubEvents bunchXing is in ns
  virtual StatusCode processBunchXing(int bunchXing,
                                      SubEventIterator bSubEvents,
                                      SubEventIterator eSubEvents) override final;
  /// return false if not interested in  certain xing times (in ns)
  /// implemented by default in PileUpToolBase as FirstXing<=bunchXing<=LastXing
  //  virtual bool toProcess(int bunchXing) const override final;

  /// called at the end of the subevts loop. Not (necessarily) able to access SubEvents
  virtual StatusCode mergeEvent(const EventContext& ctx) override final;

  /// perform the digitization - used by LUCID_DigiTop::execute
  virtual StatusCode processAllSubEvents(const EventContext& ctx) override final;

private:

  ServiceHandle<PileUpMergeSvc> m_mergeSvc{this, "mergeSvc", "PileUpMergeSvc", ""};
  ServiceHandle<IAthRNGSvc> m_randomSvc{this, "RndmSvc", "AthRNGSvc", ""};
  Gaudi::Property<std::string> m_randomStreamName{this, "RandomStreamName", "LUCIDRndEng", ""};

  std::string m_dataObjectName;
  Gaudi::Property<std::string> m_SimHitCollectionName{this, "SimHitCollection", "LucidSimHitsVector", "Name of the input Collection of simulated hits"};
  Gaudi::Property<std::string> m_digitsContainerName{this, "LucidDigitsContainer", "LucidDigitsCnt", "Name of the Container to hold the output from the digitization"};
  std::string m_key_digitCnt{"Lucid_Digits"};

  // Digitization parameters, passed to LUCID_DigitizationToolBox in initialize()
  IntegerProperty m_numTubesProp{this, "numTubes", 40};
  DoubleProperty m_qdcChannelsPerPEProp{this, "qdcChannelsPerPE", 15.0};
  DoubleProperty m_qdcPedestalProp{this, "qdcPedestal", 150.0};
  DoubleProperty m_qdcFedNoiseFactorProp{this, "qdcFedNoiseFactor", 3.0};
  DoubleProperty m_tdcPmtNoiseFactorProp{this, "tdcPmtNoiseFactor", 2.8};
  DoubleProperty m_tdcFedNoiseFactorProp{this, "tdcFedNoiseFactor", 0.8};
  DoubleProperty m_TotalPMTgainProp{this, "TotalPMTgain", 1.25e6};
  DoubleProperty m_AmpFactorProp{this, "AmpFactor", 7.5};
  DoubleProperty m_Q1binProp{this, "Q1bin", 0.1e-12};
  IntegerProperty m_NoiseChargeProp{this, "NoiseCharge", 541666};
  IntegerProperty m_numDyinodesProp{this, "numDyinodes", 10};
  DoubleProperty m_dynodeGammaFactorProp{this, "dynodeGammaFactor", 1.356};
  DoubleArrayProperty m_pmtSmearingProp{this, "pmtSmearing", {0.000001}};
  DoubleArrayProperty m_pmtScalingProp{this, "pmtScaling", {0.000001}};
  DoubleArrayProperty m_gasScalingProp{this, "gasScaling", {0.000001}};
  DoubleArrayProperty m_npeThresholdProp{this, "npeThreshold", {0.000001,0.000001,0.000001,0.000001,0.000001,
                                                                 0.000001,0.000001,0.000001,0.000001,0.000001,
                                                                 0.000001,0.000001,0.000001,0.000001,0.000001}};
  BooleanProperty m_fillRootTreeProp{this, "fillRootTree", false};

  LUCID_SimHitCollection* m_mergedhitList{};
};

#endif
