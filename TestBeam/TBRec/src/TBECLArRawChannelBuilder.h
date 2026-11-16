/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBECLARRAWCHANNELBUILDER_H
#define TBREC_TBECLARRAWCHANNELBUILDER_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "LArRawConditions/LArADC2MeV.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "LArElecCalib/ILArOFC.h"

#include "LArRawEvent/LArRawChannelContainer.h"

#include "LArIdentifier/LArOnlineID.h"

#include "CaloIdentifier/CaloCell_ID.h"
#include "StoreGate/ReadCondHandle.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArElecCalib/ILArHVScaleCorr.h"
#include "CLHEP/Units/SystemOfUnits.h"

class CaloCell_ID;

class TBECLArRawChannelBuilder : public AthAlgorithm
{
public:
  TBECLArRawChannelBuilder (const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;

private:
  //Services & Tools 
  const LArOnlineID* m_onlineHelper{nullptr};
  const CaloCell_ID* m_calo_id{nullptr};

  SG::ReadCondHandleKey<LArOnOffIdMapping>  m_cablingKey {this,"keyCabling", "LArOnOffIdMap", "Input key for Id mapping"} ;
  SG::ReadCondHandleKey<ILArHVScaleCorr> m_offlineHVScaleCorrKey{this, "keyOfflineHVCorr", "LArHVScaleCorrRecomputed","Key for LArHVScaleCorr"};

  //Algo-properties 
  Gaudi::Property<std::string> m_DataLocation{this, "DataLocation", "FREE", "Input LArDigitContainer key"};
  Gaudi::Property<std::string> m_ChannelContainerName{this, "LArRawChannelContainerName", "LArRawChannels", "Output LArRawChannelContainer key"};
  Gaudi::Property<bool> m_useTDC{this, "UseTDC", false, "Use TDC phase (TBPhase)"};
  Gaudi::Property<bool> m_useRamp{this, "UseRamp", true, "Use ramps from conditions"};
  Gaudi::Property<bool> m_useShape{this, "UseShape", true, "Use shape for quality factor"};
  Gaudi::Property<bool> m_ConvertADCToHighGain{this, "ConvertADCToHighGain", false, "Convert medium gain ADC to high gain"};
  Gaudi::Property<float> m_Ecut{this, "Ecut", 256*CLHEP::MeV, "Energy threshold for time and quality computation"};
  Gaudi::Property<int> m_initialTimeSampleShift{this, "InitialTimeSampleShift", 0, "Initial time sample shift"};
  Gaudi::Property<int> m_NOFCPhases{this, "NOFCPhases", 25, "Total number of available OFC sets"};
  Gaudi::Property<int> m_NOFCTimeBins{this, "NOFCTimeBins", 25, "Number of OFC time bins in a sampling periode"};
  Gaudi::Property<bool> m_useHighGainIntercept{this, "UseHighGainRampIntercept", false, "Use ramp intercept for high gain"};
  Gaudi::Property<bool> m_useMedGainIntercept{this, "UseMedGainRampIntercept", false, "Use ramp intercept for medium gain"};
  Gaudi::Property<bool> m_useLowGainIntercept{this, "UseLowGainRampIntercept", false, "Use ramp intercept for low gain"};
  bool m_useIntercept[4]{}; // filled from the UseXXXGainRampIntercept properties in initialize()
  Gaudi::Property<bool> m_useOFCPhase{this, "UseOFCPhase", false, "Use OFC phase"};
  Gaudi::Property<bool> m_phaseInv{this, "PhaseInversion", false, "Invert TDC phase"};
  float m_ramp_max[3]{};

  Gaudi::Property<double> m_SamplingPeriode{this, "SamplingPeriod", 1/(40.08*CLHEP::megahertz), "Sampling period"};
  Gaudi::Property<float> m_OFCTimeBin{this, "OFCTimeBin", 1/(40.08*CLHEP::megahertz)/25, "OFC time bin width"};
  Gaudi::Property<bool> m_binHalfOffset{this, "BinHalfOffset", false, "Shift OFC bins by half a bin"};
  Gaudi::Property<bool> m_allowTimeJump{this, "AllowTimeSampleJump", true, "Allow time sample jump"};
  Gaudi::Property<int> m_pedestalFallbackMode{this, "PedestalFallbackMode", 0, "0=only DB, 1=Only if missing, 2=Low, 3=Low+Medium, 4=All LAr"};
  Gaudi::Property<unsigned int> m_iPedestal{this, "PedestalSample", 0, "Time sample used as pedestal in fallback mode"};
  Gaudi::Property<unsigned int> m_shapeMode{this, "ShapeMode", 0, "Shape mode"};

  Gaudi::Property<unsigned> m_skipSaturCells{this, "SkipSaturCellsMode", 0, "0 = no skip ; 1 = skip saturating cells ; 2 = skipp all event"};
  Gaudi::Property<short> m_AdcMax{this, "ADCMax", 4095, "Saturation ADC value"};
  
  //Counters for errors & warnings per job:
  int m_noEnergy{0};   // Number of events with at least completly failed channel
  int m_noTime{0};     // Number of events with at least one channel without time info
  int m_noShape{0};    // Number of events with at least one channel without Shape (=with not quality factor);
  int m_noShapeDer{0}; // Number of events with at least one channel without ShapeDerivative (=not taken into accout for quality factor);
  int m_saturation{0}; // Number of events with at least one saturating channel
  
  //bool m_errorPrint;
  int m_lastNoEnergy{0};    // Number of completly failed channels in previous event
  int m_lastNoTime{0};      // Number of channels without time info in previous event
  int m_lastNoShape{0};     // Number of channels without Shape (=with not quality factor) in previous event
  int m_lastNoShapeDer{0};  // Number of channels without ShapeDerivative in previous event
  //int m_lastSaturCells;  // Number of saturating channels without in previous event (not used)
  
  float m_aveNoEnergy{0};   // Average number of completly failed channels per event
  float m_aveNoTime{0};     // Average number of channels without time info per event
  float m_aveNoShape{0};    // Average number of channels without Shape (=with not quality factor) per event
  float m_aveNoShapeDer{0}; // Average number of channels without ShapeDerivative per event
  float m_aveSaturCells{0}; // Average number of saturating channels per event
  
  int   m_nEvents{0};       // Total number of processed events ;
  float m_aveChannels{0};   // Average number of readout channels per event
  Gaudi::Property<bool> m_hvcorr{this, "HVcorr", false, "Apply HV scale correction"};
  
  //Time ranges for OFC application
  double m_SamplingPeriodeUpperLimit{0},m_SamplingPeriodeLowerLimit{0};
  
  //Private Member functions:
  //void sortChannels(LArRawChannelContainer* container) ;
 
  // to be used for detailed DEBUG output only
  const LArEM_ID* m_emId{nullptr};

  // For useRamp = False
  float m_adc2mev[30]{};

  /// Property: OFC coefficients (conditions input).
  SG::ReadCondHandleKey<ILArOFC> m_ofcKey
    {this, "OFCKey", "LArOFC", "SG Key of OFC conditions object" };

  SG::ReadCondHandleKey<LArADC2MeV> m_adc2mevKey
    { this, "ADC2MeVKey", "LArADC2MeV", "SG Key of the LArADC2MeV CDO" };
};

#endif
