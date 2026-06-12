/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CalibrationDataInterfaceTester.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ANALYSISCALIBRATIONDATAINTERFACETESTER_H
#define ANALYSISCALIBRATIONDATAINTERFACETESTER_H

// Gaudi includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "CalibrationDataInterface/CalibrationDataInterfaceTool.h"
#include <StoreGate/ReadHandleKey.h>
#include <string>

class AtlasDetectorID;
class Identifier;

namespace Analysis 
{

  /** @class CalibrationDataInterfaceTester

      Minimal test of the CalibrationDataInterfaceTool functionality.
      
      @author  Frank Filthaut <F.Filthaut@science.ru.nl>
  */  

  class CalibrationDataInterfaceTester : public AthAlgorithm
    {
    public:

       /** Standard Athena-Algorithm Constructor */
       CalibrationDataInterfaceTester(const std::string& name, ISvcLocator* pSvcLocator);
       /** Default Destructor */
       virtual ~CalibrationDataInterfaceTester() = default;

       /** standard Athena-Algorithm method */
       StatusCode          initialize() override;
       /** standard Athena-Algorithm method */
       StatusCode          execute(const EventContext& ctx) override;

    private:
      
       /** @brief jet collection name */
       Gaudi::Property  m_jetCollection;

       /** @brief tagger name */
       Gaudi::Property  m_tagger;

       /** @brief tagger operating point */
       Gaudi::Property  m_operatingPoint;

       /** @brief calibration uncertainty result */
       std::string  m_calibrationUncertainty;
       CalibrationDataInterfaceTool::Uncertainty m_uncertaintyType;

       /** @brief pointer to the CalibrationDataInterfaceTool */
       ToolHandle<ICalibrationDataInterfaceTool>  m_calibrationInterface {this, "ICalibrationDataInterfaceTool", "", "CalibrationDataInterface Tool"};

       /** @brief I/O handlers */
       SG::ReadHandleKey<JetCollection> m_JetCollectionKey {this, "JetCollectionKey", "", "read jet collection"};
    }; 
} // end of namespace

#endif 
