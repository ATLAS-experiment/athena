/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARL1SIM_LARTTL1MAKER_H
#define LARL1SIM_LARTTL1MAKER_H
// +======================================================================+
// +                                                                      +
// + Author ........: F. Ledroit                                          +
// + Institut ......: ISN Grenoble                                        +
// + Creation date .: 09/01/2003                                          +
// +                                                                      +
// +======================================================================+
//
// ....... include
//

#include "AthenaBaseComps/AthAlgorithm.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"

#include "LArDigitization/LArHitEMap.h"
#include "LArElecCalib/ILArfSampl.h"

#include "AthenaKernel/IAthRNGSvc.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "LArSimEvent/LArHitContainer.h"
#include "LArRawEvent/LArTTL1Container.h"

#include "CaloTriggerTool/CaloTriggerTowerService.h"

// trigger time
#include "AthenaKernel/ITriggerTime.h"

#include <vector>
#include <array>

class CaloLVL1_ID;
class LArEM_ID;
class LArHEC_ID;
class LArFCAL_ID;
namespace CLHEP
{
  class HepRandomEngine;
}

/**
   @brief The aim of this algorithm is the simulation of the LAr analogue trigger tower sums.

   It includes correct allocation of cells to towers, pulse profile as a function of time, saturation, appropriate noise addition, pile-up. <br>
   The resulting signals are an input to the Preprocessor (which in turn performs digitization, filtering, bunch crossing id., noise suppression,...) <br>
   Since it needs hits, the simulation only takes "simul" datasets as input, NOT digitized datasets.

   @warning although the output is not digitized, LArTTL1Maker is part of the digitization simulation.

   @author F. Ledroit (LPSC-Grenoble)
*/

class LArTTL1Maker : public AthAlgorithm,
                     public IIncidentListener
{
  //
  // >>>>>>>> public methods
  //
public:
  // delegate Constructor
  using AthAlgorithm::AthAlgorithm;

  //
  // ..... Gaudi algorithm hooks
  //
  /**  Read ascii files for auxiliary data (puslse shapes, noise, etc...) */
  virtual StatusCode initialize() override;
  /**       Create  LArTTL1  object
            save in TES (2 containers: 1 EM, 1 hadronic)
  */
  virtual StatusCode execute() override;

  virtual StatusCode finalize() override;
  virtual void handle(const Incident&) override;

  virtual bool isClonable() const override final { return true; }

private:

  //
  // >>>>>>>> private algorithm parts
  //

  /** initialize hit map
   */

  //virtual StatusCode initHitMap();

  std::vector<float> computeSignal(const Identifier towerId, const int Ieta, const int specialCase,
                                   std::vector<float> visEnergy, const int refTime) const;

  std::vector<float> computeNoise(const Identifier towerId, const int Ieta,
                                  std::vector<float>& inputV, CLHEP::HepRandomEngine* rndmEngine);

  /** method called at the begining of execute() to fill the hit map */
  //StatusCode fillEMap(int& totHit) ;

  /** method called at initialization to read auxiliary data from ascii files */
  StatusCode readAuxiliary();

  int decodeInverse(int region, int eta);

  // RNGWrapper hacks
  void setSeed(const std::string& algName, const EventContext& ctx);
  void setSeed(const std::string& algName, uint64_t ev, uint64_t run);
  void setSeed(size_t seed);

  //
  // >>>>>>>> private data parts
  //

  IChronoStatSvc*              m_chronSvc{};
  ServiceHandle<IAthRNGSvc> m_RandomSvc{this, "RndmSvc", "AthRNGSvc", ""};
  Gaudi::Property<std::string> m_randomStreamName{this, "RandomStreamName", "LArTTL1Maker", ""};
  Gaudi::Property<uint32_t> m_randomSeedOffset{this, "RandomSeedOffset", 2, ""};
  Gaudi::Property<bool> m_useLegacyRandomSeeds{this, "UseLegacyRandomSeeds", false,
    "Use MC16-style random number seeding"};

  /** Alorithm property: use trigger time or not*/
  Gaudi::Property<bool> m_useTriggerTime{this, "UseTriggerTime", false};
  /** Alorithm property: name of the TriggerTimeTool*/
  PublicToolHandle<ITriggerTime> m_triggerTimeTool{this, "TriggerTimeToolName", ""}; //"CosmicTriggerTimeTool"

  int m_BeginRunPriority{100};

  PublicToolHandle<CaloTriggerTowerService>  m_ttSvc{this, "CaloTriggerTowerService", "CaloTriggerTowerService"}; // FIXME Naming!!
  /** pointer to the offline TT helper */
  const CaloLVL1_ID*           m_lvl1Helper{};
  /** pointer to the offline EM helper */
  const LArEM_ID*              m_emHelper{};
  /** pointer to the offline HEC helper */
  const LArHEC_ID*             m_hecHelper{};
  /** pointer to the offline FCAL helper */
  const LArFCAL_ID*            m_fcalHelper{};
  /** pointer to the offline id helper  */
  const CaloCell_ID*           m_OflHelper{};
  /** Sampling fractions retrieved from DB */
  //const DataHandle<ILArfSampl>    m_dd_fSampl;
  SG::ReadCondHandleKey<ILArfSampl> m_fSamplKey{this, "LArfSamplKey", "LArfSamplSym"};

  /** number of sampling (in depth) */
  static const short s_NBDEPTHS = 4 ;
  /** number of samples in TTL1s */
  static const short s_NBSAMPLES = 7 ;
  /** max number of samples in pulse shape */
  static const short s_MAXSAMPLES = 24 ;
  /** peak position  */
  static const short s_PEAKPOS = 3 ;
  /** number of eta bins */
  static const short s_NBETABINS = 15 ;
  /** number of energies at which saturation is described (em) */
  static const short s_NBENERGIES = 12 ;

  /** auxiliary EM data: reference energies for saturation simulation */
  std::vector<float> m_refEnergyEm ;
  /** auxiliary EM data: pulse shapes */
  std::vector<std::vector<float> > m_pulseShapeEm ;
  /** auxiliary EM data: pulse shape derivative */
  std::vector<std::vector<float> > m_pulseShapeDerEm ;
  /** auxiliary EM data: auto-correlation matrix */
  std::vector<float> m_autoCorrEm ;

  /** auxiliary EMBarrel data: sin(theta) */
  std::vector<float> m_sinThetaEmb ;
  /** auxiliary EMBarrel data: calibration coefficient */
  Gaudi::Property<std::vector<float>> m_calibCoeffEmb{this, "EmBarrelCalibrationCoeffs", std::vector<float>(s_NBETABINS, 1.)};
  /** auxiliary EMBarrel data:  noise rms */
  std::vector<float> m_noiseRmsEmb ;
  // if later we want to disentangle between pre-sum and summing electronic noise...
  //  std::vector<std::vector<float> > m_noiseRmsEmb ;

  /** auxiliary EMEC data: sin(theta)  */
  std::vector<float> m_sinThetaEmec ;
  /** auxiliary EMEC data: calibration coeeficient */
  Gaudi::Property<std::vector<float>> m_calibCoeffEmec{this, "EmEndCapCalibrationCoeffs", std::vector<float>(s_NBETABINS, 1.)};
  /** auxiliary EMEC data: noise rms */
  std::vector<float> m_noiseRmsEmec ;
  // if later we want to disentangle between pre-sum and summing electronic noise...
  //  std::vector<std::vector<float> > m_noiseRmsEmec ;

  /** auxiliary HEC data: pulse shape */
  std::vector<float> m_pulseShapeHec ;
  /** auxiliary HEC data: pulse shape derivative */
  std::vector<float> m_pulseShapeDerHec ;
  /** auxiliary HEC data: calibration coefficients */
  Gaudi::Property<std::vector<float>> m_calibCoeffHec{this, "HECCalibrationCoeffs", std::vector<float>(s_NBETABINS, 1.)};
  /** auxiliary HEC data: sin(theta) */
  std::vector<float> m_sinThetaHec ;
  /** auxiliary HEC data: saturation energy */
  std::vector<float> m_satEnergyHec ;
  /** auxiliary HEC data: noise rms */
  std::vector<float> m_noiseRmsHec ;
  /** auxiliary HEC data: auto-correlation matrix */
  std::vector< std::vector<float> > m_autoCorrHec ;

  /** auxiliary FCAL data: relative gains */
  std::vector<float> m_cellRelGainFcal ;
  /** auxiliary FCAL data: pulse shapes */
  std::vector< std::vector<float> > m_pulseShapeFcal ;
  /** auxiliary FCAL data: pulse shape derivatives */
  std::vector< std::vector<float> > m_pulseShapeDerFcal ;
  /** auxiliary FCAL data: calibration coefficients */
  std::vector< std::vector<float> > m_calibCoeffFcal ;
  static constexpr int s_nEta = 4;
  /** auxiliary FCAL data: calibration coefficients */
  Gaudi::Property<std::vector<float>> m_calibCoeffFcalEm{this, "EmFcalCalibrationCoeffs", std::vector<float>(s_nEta, .03)};
  /** auxiliary FCAL data: calibration coefficients */
  Gaudi::Property<std::vector<float>> m_calibCoeffFcalHad{this, "HadFcalCalibrationCoeffs", std::vector<float>(s_nEta, .03)};
  /** auxiliary FCAL data: noise rms */
  //  std::vector<float> m_noiseRmsFcal ;
  std::vector< std::vector<float> > m_noiseRmsFcal ;
  /** auxiliary FCAL data: auto-correlation matrix */
  std::vector<float> m_autoCorrFcal ;

  /** hit map */
  SG::ReadHandleKey<LArHitEMap> m_hitMapKey{this,"LArHitEMapKey","LArHitEMap"};

  /** algorithm property: container name for the EM TTL1s */
  SG::WriteHandleKey<LArTTL1Container> m_EmTTL1ContainerName{this, "EmTTL1ContainerName", "LArTTL1EM"};
  /** algorithm property: container name for the HAD TTL1s */
  SG::WriteHandleKey<LArTTL1Container> m_HadTTL1ContainerName{this, "HadTTL1ContainerName", "LArTTL1HAD"};

  std::array<SG::ReadHandleKey<LArHitContainer>,4> m_xxxHitContainerName{{
      {this, "EmBarrelHitContainerName", "LArHitEMB"},
      {this, "EmEndCapHitContainerName", "LArHitEMEC"},
      {this, "HecHitContainerName", "LArHitHEC"},
      {this, "ForWardHitContainerName", "LArHitFCAL"}
    }};

  /** algorithm property: noise (in all sub-detectors) is on if true         */
  Gaudi::Property<bool> m_NoiseOnOff{this, "NoiseOnOff", true};
  /** algorithm property: pile up or not */
  Gaudi::Property<bool> m_PileUp{this, "PileUp", false};
  /** algorithm property: no calibration mode for EM towers */
  Gaudi::Property<bool> m_noEmCalibMode{this, "NoEmCalibrationMode", false};
  /** algorithm property: no calibration mode for had towers */
  Gaudi::Property<bool> m_noHadCalibMode{this, "NoHadCalibrationMode", false};
  /** algorithm property: debug threshold */
  Gaudi::Property<float> m_debugThresh{this, "DebugThreshold", 5000.};
  /** algorithm property: switch chrono on */
  Gaudi::Property<bool> m_chronoTest{this, "ChronoTest", false};

  /** key for saving truth */
  Gaudi::Property<std::string> m_truthHitsContainer{this, "TruthHitsContainer", "",
    "Specify a value to get a pair of LArTTL1 containers with the truth hits in them"};

};

#endif
