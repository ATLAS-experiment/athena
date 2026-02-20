// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file BunchCrossingIntensityCondAlg.h
 * @author Lorenzo Rossini <lorenzo.rossini@cern.ch>
 * @date July 2024
 * @brief Conditions algorithm to fill BunchCrossingIntensityCondData
 */


#ifndef LUMIBLOCKCOMPS_BUNCHCROSSINGINTENSITYCONDALG_H
#define LUMIBLOCKCOMPS_BUNCHCROSSINGINTENSITYCONDALG_H


#include "CoolLumiUtilities/FillParamsCondData.h"
#include "LumiBlockData/BunchCrossingIntensityCondData.h"
#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "TrigConfInterfaces/ILVL1ConfigSvc.h"
#include "LumiBlockData/LuminosityCondData.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"


/**
 * @brief Conditions algorithm to unpack fill parameters from COOL.
 */
class BunchCrossingIntensityCondAlg : public AthCondAlgorithm {

public:
  /// Forward base class ctor.
  using AthCondAlgorithm::AthCondAlgorithm;
  // typedef BunchCrossingIntensityCondData::bunchTrain_t bunchTrain_t;


  /// Gaudi initialize method.
  virtual StatusCode initialize() override;


  /// Algorithm execute method.
  virtual StatusCode execute (const EventContext& ctx) const override;



private:
  /// Input conditions object.
  SG::ReadCondHandleKey<CondAttrListCollection> m_fillParamsFolderKey{ this, "FillParamsFolderKey", "/TDAQ/OLC/LHC/BUNCHDATA", "" }; 
  SG::ReadCondHandleKey<LuminosityCondData> m_lumiCondDataKey{this, "LumiCondData", "LuminosityCondData", "Lumi cond data key"};
  

  /// Output conditions object.
  SG::WriteCondHandleKey<BunchCrossingIntensityCondData> m_outputKey{this, "OutputKey", "BunchCrossingIntensityData", "Key of output CDO" };

  // Service handle
  const ServiceHandle<TrigConf::ILVL1ConfigSvc> m_trigConfigSvc{this, "TrigConfigSvc", "", "Trig Config Svc"};

  ///internal methods:
  std::vector<float> tokenize(const std::string& pattern) const;

  //Algorithm properties
  
  // Properties of the two different possible channels
  Gaudi::Property<unsigned long> m_BPTX{ this, "BunchDevice", 0, "Channel assignments: 0=BPTX, 1=fast BCT." };
  Gaudi::Property<unsigned long> m_fBCT{ this, "BunchDeviceFast", 1, "Channel assignments: 0=BPTX, 1=fast BCT." };

  // general properties
  Gaudi::Property<unsigned> m_maxBunchSpacing{this, "MaxBunchSpacing",5, "Maximal bunch-spacing to be considered a 'bunch train'"};
  Gaudi::Property<unsigned> m_minBunchesPerTrain{this, "MinBunchesPerTrain",32, "Minimal number of bunches to be considerd a 'bunch train'"};
  Gaudi::Property<bool> m_isRun1{this,"Run1",false,"Assume run-1 database"};
  Gaudi::Property<int> m_mode{this, "Mode", 1, "Alg mode (COOL FILLPARAMS = 0, MC = 1, TrigConf = 2, Luminosity = 3)"};
};

#endif
