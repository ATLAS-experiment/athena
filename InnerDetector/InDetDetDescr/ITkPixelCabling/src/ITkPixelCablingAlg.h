/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITkPixelCablingAlg_H
#define ITkPixelCablingAlg_H
/**   
 *   @file ITkPixelCablingAlg.h
 *
 *   @brief Fills an ITkPixelCablingData  object and records it in Storegate
 *
 *   @author Shaun Roe, Ondra Kovanda
 *   @date June 2024
 */

//Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "ITkPixelCabling/ITkPixelCablingData.h"

#include "StoreGate/WriteCondHandleKey.h"

//Gaudi includes
#include "GaudiKernel/ServiceHandle.h"

//STL includes
#include <string>

//Forward declarations
class PixelID;

namespace InDetDD{
    class PixelDetectorManager;
}


/**
 *    @class ITkPixelCablingAlg
 *    @brief Conditions algorithm which fills the ITkPixelCablingData from plain text (a file).
 *
 */

class ITkPixelCablingAlg: public AthReentrantAlgorithm {
 public:
  ITkPixelCablingAlg(const std::string& name, ISvcLocator* svc);
  virtual ~ITkPixelCablingAlg() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  /** Make this algorithm clonable. */
  virtual bool isClonable() const override { return true; };
  
private:

  StatusCode generateTestCabling(std::unique_ptr<ITkPixelCablingData>& cabling) const;

  StringProperty m_source{this, "DataSource", "ITkPixelCabling.dat", "a plain text file for the ITkPixel cabling"};
  Gaudi::Property<bool> m_useTestCabling{this, "UseTestCabling", false, "Generate a dummy cabling for testing/development purposes, as the online IDs don't exist yet"};
  SG::WriteCondHandleKey<ITkPixelCablingData> m_writeKey{this, "WriteKey", "ITkPixelCablingData", "Key of output (derived) conditions data"};
  const PixelID* m_idHelper{nullptr};
  const InDetDD::PixelDetectorManager* m_detManager{};

};

#endif 
