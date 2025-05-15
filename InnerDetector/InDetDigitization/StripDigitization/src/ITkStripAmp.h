/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 *  Header file for ITkStripAmp Class  
 *  Dummy Amplifier for ITkStrips
 */

#ifndef STRIPDIGITIZATION_ITKSTRIPAMP_H
#define STRIPDIGITIZATION_ITKSTRIPAMP_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "SiDigitization/IAmplifier.h"

#include "InDetSimEvent/SiCharge.h"


class ITkStripAmp : public extends<AthAlgTool, IAmplifier> {
 public:
  /**  constructor */
  using base_class::base_class;
  /** Destructor */
  virtual ~ITkStripAmp() = default;
  /** AlgTool initialize */
  virtual StatusCode initialize() override;
  virtual float response(const list_t& Charges, const float timeOverThreshold) const override;
  virtual void response(const list_t& Charges, const float time, std::vector<float>& resp) const override;

  /** Neighbour strip cross talk response strip to a list of charges with times */
  virtual float crosstalk(const list_t& Charges, const float timeOverThreshold) const override;
  virtual void crosstalk(const list_t& Charges, const float timeOverThreshold, std::vector<float>& resp) const override;

private:


/** signal peak time */   
  FloatProperty m_PeakTime{this, "PeakTime", 25., "Front End Electronics peaking time"};
  float m_NormConstCentral{0.};
};

#endif // STRIPDIGITIZATION_ITKSTRIPAMP_H

