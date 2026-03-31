/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_LARDIGITRETRIEVER_H
#define JIVEXML_LARDIGITRETRIEVER_H



#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArRawConditions/LArADC2MeV.h"

#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadCondHandleKey.h"
#include <string>
#include <vector>
#include <cstddef>
#include <map>
class IToolSvc;

class Identifier;
class CaloCellContainer;

namespace JiveXML{
  
  /**
   * @class LArDigitRetriever
   * @brief Retrieves all @c Tile Calo Cell @c objects 
   *
   *  - @b Properties
   *    - StoreGateKey: default is 'AllCalo'. Don't change.
   *	- CallThreshold: default is 50 MeV
   *	- RetrieveLAr: activate retriever, default is true
   *	- DoLarDigits: write LAr digits (ADC), default is false
   *	- RetrieveHEC: activate retriever, default is true
   *	- DoHECDigits: write HEC digits (ADC), default is false
   *	- RetrieveFCal: activate retriever, default is true
   *	- DoFCalDigits: write FCal digits (ADC), default is false
   *	- CellEnergyPrec: output precision, default is 3 digits
   *	- CellTimePrec: output precision, default is 3 digits
   *   
   *  - @b Retrieved @b Data
   *    - location in phi and eta
   *    - identifier and adc counts of each cell 
   *    - various pmt details
   */
  class LArDigitRetriever : public extends<AthAlgTool,IDataRetriever> {
    
    public:
      
      using base_class::base_class;
      
      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool) override;
      const DataMap getLArDigitData(const CaloCellContainer* cellContainer, 
                                    const std::string& datatype, 
                                    CaloCell_ID::SUBCALO calotype);

      /// Return the name of the data type
      virtual std::string dataTypeName() const override { return "LArDigit"; };
	
      ///Default AthAlgTool methods
      virtual StatusCode initialize() override;

    private:
      void calcEMLayerSub(Identifier&);
      void calcHECLayerSub(Identifier&);

      SG::ReadHandleKey<CaloCellContainer> m_sgKey{this, "StoreGateKey", "AllCalo", "Name of the CaloCellContainer"};
      SG::ReadHandleKey<LArDigitContainer> m_sgKeyLArDigit_raw{this, "StoreGateKey", "FREE", "Name of the LArDigitContainer"}; // can also be: "HIGH" (for raw data)
      SG::ReadHandleKey<LArDigitContainer> m_sgKeyLArDigit_esd{this, "StoreGateKey", "LArDigitContainer_Thinned", "Name of the LArDigitContainer"}; // used for DPD/ESD
      SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
    
      SG::ReadCondHandleKey<LArADC2MeV> m_adc2mevKey
        { this, "ADC2MeVKey", "LArADC2MeV", "SG Key of the LArADC2MeV CDO" };

      const CaloCell_ID*   m_calocell_id{};

      Gaudi::Property<bool> m_lar{this, "RetrieveLAr", true};
      Gaudi::Property<bool> m_hec{this, "RetrieveHEC", true};
      Gaudi::Property<bool> m_fcal{this, "RetrieveFCAL", true};
      Gaudi::Property<bool> m_doLArDigit{this, "DoLArDigit", false};
      Gaudi::Property<bool> m_doHECDigit{this, "DoHECDigit", false};
      Gaudi::Property<bool> m_doFCalDigit{this, "DoFCalDigit", false};
      Gaudi::Property<bool> m_cellConditionCut{this, "CellConditionCut", false};
      Gaudi::Property<int> m_cellEnergyPrec{this, "CellEnergyPrec", 3};
      Gaudi::Property<int> m_cellTimePrec{this, "CellTimePrec", 3};
      Gaudi::Property<double> m_cellThreshold{this, "CellThreshold", 50.};
      Gaudi::Property<std::vector<Identifier::value_type>> m_LArChannelsToIgnoreM5{this, "LArChannelsToIgnoreM5", {}};
      Gaudi::Property<bool> m_doMaskLArChannelsM5{this, "DoMaskLArChannelsM5", false};

      bool m_doDigit{false};
      bool m_inputdpd{false};
      DataVect m_sub;
  };
}
#endif
