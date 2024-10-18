/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SCT_DIGITIZATION_ITkStripFrontEnd_H
#define SCT_DIGITIZATION_ITkStripFrontEnd_H
// Inheritance
#include "AthenaBaseComps/AthAlgTool.h"
#include "SiDigitization/IFrontEnd.h"
// Athena
#include "SiDigitization/SiChargedDiodeCollection.h"
// Gaudi
#include "GaudiKernel/ToolHandle.h"
// STL
#include <string>

class IAmplifier;
class SCT_ID;

namespace InDetDD {
  class SCT_DetectorManager;
}

namespace CLHEP {
  class HepRandomEngine;
}
/**
 * @brief simulation of the ITk Strips front-end electronics
 * working as a SiPreDigitsProcessor
 * models response of ABCstar chip amplifiers to 
 * collected charges, also does cross-talk, offset
 * variation and gain variation, in a correlated way
 */

class  ITkStripFrontEnd : public extends<AthAlgTool, IFrontEnd> {
 public:
  /**  constructor */
  ITkStripFrontEnd(const std::string& type, const std::string& name, const IInterface* parent);
  /** Destructor */
  virtual ~ITkStripFrontEnd() = default;  
  /** AlgTool initialize */
  virtual StatusCode initialize() override;
  /**use the baseclass default finalize */
  //
  /**
   * process the collection of pre digits: needed to go through all single-strip pre-digits to calculate
   * the amplifier response add noise (this could be moved elsewhere later) apply threshold do clustering
   * stripMax is for benefit of ITkStrips which can have different numbers of strips for each module 
   */
  virtual void process(SiChargedDiodeCollection& collection, CLHEP::HepRandomEngine* rndmEngine) const override;
  
 private:
  //ToolHandle<IAmplifier> m_sct_amplifier{this, "SCT_Amp", "SCT_Amp", "Handle the Amplifier tool"}; //!< Handle the Amplifier tool
  //
  const InDetDD::SCT_DetectorManager* m_ITkStripMgr{nullptr}; //!< Handle to SCT detector manager, also valid for ITkStrips
  const SCT_ID* m_ITkStripId{nullptr}; //!< Handle to SCT ID helper  also valid for ITkStrips
  StringProperty m_detMgrName{this, "DetectorManager", "SCT", "Name of DetectorManager to retrieve"};
};

#endif //ITkStripFrontEnd_H