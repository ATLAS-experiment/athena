/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITkStripCablingAlg_H
#define ITkStripCablingAlg_H
/**   
 *   @file ITkStripCablingAlg.h
 *
 *   @brief Fills an ITkStripCablingData  object and records it in Storegate (based on ITkPixelCabling package)
 *
 *   @author Edson Carquin
 *   @date September 2024
 */

//Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "ITkStripCabling/ITkStripCablingData.h"
#include "StoreGate/WriteCondHandleKey.h"

//Gaudi includes
#include "GaudiKernel/ServiceHandle.h"

//STL includes
#include <string>

//Forward declarations
class SCT_ID;

/**
 *    @class ITkStripCablingAlg
 *    @brief Conditions algorithm which fills the ITkStripCablingData from plain text (a file).
 *
 */

class ITkStripCablingAlg: public AthReentrantAlgorithm {
 public:
  ITkStripCablingAlg(const std::string& name, ISvcLocator* svc);  
  virtual ~ITkStripCablingAlg() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  /** Make this algorithm clonable. */
  virtual bool isClonable() const override { return true; };
  
private:
  StringProperty m_source{this, "DataSource", "", "A data file for the ITkStrip cabling"};
  std::string m_configFilePath={};
  SG::WriteCondHandleKey<ITkStripCablingData> m_writeKey{this, "WriteKey", "ITkStripCablingData", "Key of output (derived) conditions data"};
  const SCT_ID* m_idHelper{nullptr};
};

#endif 
