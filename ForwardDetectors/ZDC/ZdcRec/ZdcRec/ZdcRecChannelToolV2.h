/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*
 * ZdcRecChannelToolV2.h
 *
 * Two steps:
 * 1. Convert the towers to ZdcModules
 * 2. Process the ZdcModules and add Energy/time info
 * Created on: September 27, 2015
 *      Author: steinber
 */

#ifndef ZDCRECCHANNELTOOLV2_H_
#define ZDCRECCHANNELTOOLV2_H_


#include <string>
#include <map>

//#include "AthenaBaseComps/AthAlgTool.h"
#include "AsgTools/AsgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "Gaudi/Property.h"

#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include "xAODTrigL1Calo/TriggerTowerAuxContainer.h"

#include "xAODForward/ZdcModuleContainer.h"
#include "xAODForward/ZdcModuleAuxContainer.h"

class StatusCode;
class Identifier;
class ZdcID ;

class ZdcRecChannelToolV2: public asg::AsgTool, virtual public IIncidentListener 
{
  ASG_TOOL_INTERFACE(ZdcRecChannelToolV2)
  ASG_TOOL_CLASS0(ZdcRecChannelToolV2)
 public:
  ZdcRecChannelToolV2(const std::string& name);
  virtual ~ZdcRecChannelToolV2() {};
  
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual void handle( const Incident& ) override;

  int convertTT2ZM(const xAOD::TriggerTowerContainer* ttCollection, xAOD::ZdcModuleContainer* zdcModules, xAOD::ZdcModuleContainer* zdcSums) const;
  int makeRawFromDigits(xAOD::ZdcModuleContainer& zdcModules) const; // NOT const -- we're going to modify the objects to add signal processing
  int makeWaveformFromDigits(xAOD::ZdcModule& module) const;
  static int splitWaveform(std::map<int,float>& waveform, std::vector<float>& times, std::vector<float>& adcs) ;
  int getPeakProperties(std::vector<float>& times, std::vector<float>& adcs, float& time, float& amp, float& qual, float& presamp) const;

private:

  // internal parameters, set in pythion
  Gaudi::Property<unsigned int> m_nsamples{this, "NSamples", 5, "Number of samples in PPM raw data"};
  Gaudi::Property<float> m_sample_time{this, "SampleTime", 12.5, "Sample time in ns (25. = 40 MHz, 12.5 = 80 MHz)"};
  Gaudi::Property<float> m_gainFactor{this, "GainFactor", 10., "High/lo gain factor"};
  Gaudi::Property<bool> m_useDelay{this, "UseDelay", false, "If true, then use delayed channels"};
  Gaudi::Property<int> m_delta{this, "DeltaPeak", 5, "Minimum difference between min and max to be considered a signal"}; // A number to check for signal
  Gaudi::Property<int> m_zeroSupress{this, "ZeroSuppress", 0, "Supress channels with only 0"};
  Gaudi::Property<int> m_saturation{this, "SaturationADC", 1000, "ADC value above which a HG channel is considered saturated"}; // saturation level (absolute ADC)
  Gaudi::Property<float> m_pedestalValue{this, "PedestalValue", 100., "Pedestal value"}; // pedestal value (set to 100)

  const ZdcID* m_zdcId{};

};


#endif /* ZDCRECCHANNELTOOLV2_H_ */
