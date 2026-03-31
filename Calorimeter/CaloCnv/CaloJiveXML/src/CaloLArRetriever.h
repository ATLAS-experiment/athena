/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_CALOLARRETRIEVER_H
#define JIVEXML_CALOLARRETRIEVER_H


#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArRawConditions/LArADC2MeV.h"

#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "JiveXML/DataType.h" //DataMap

#include <string>
#include <vector>

class IToolSvc;

class Identifier;
class CaloCellContainer;

namespace JiveXML{
  
  /**
   * @class CaloLArRetriever
   * @brief Retrieves all @c Calo Cluster @c objects 
   *
   *  - @b Properties
   *    - StoreGateKey: default is 'AllCalo'. Don't change.
   *	- LArlCellThreshold: default is 50 (MeV)
   *	- RetrieveLAr: general flag, default is true
   *    - DoLArCellDetails: default is false
   *    - CellConditionCut: default is false
   *    - LArChannelsToIgnoreM5: default is empty (none ignored). Input: vector of cells
   *   	- DoMaskLArChannelsM5: default is false (none masked)
   *   	- CellEnergyPrec: precision in int, default is 3 digits
   *   	- CellTimePrec: precision in int, default is 3 digits
   *   
   *  - @b Retrieved @b Data
   *    - location in phi and eta
   *    - numCells: number of cells in each cluster
   *    - cells: identifier and adc counts of each cell 
   */
  class CaloLArRetriever : public extends<AthAlgTool,IDataRetriever> {
    
    public:
      using base_class::base_class;
      
      /// Retrieve all the data
      virtual StatusCode retrieve (ToolHandle<IFormatTool> &FormatTool) override;
      const DataMap getLArData(const CaloCellContainer* cellContainer);

      /// Return the name of the data type
      virtual std::string dataTypeName() const override { return "LAr"; };
	
      ///Default AthAlgTool methods
      virtual StatusCode initialize() override;

    private:
      const CaloCell_ID*   m_calocell_id{};
      SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey
	{this, "CablingKey", "LArOnOffIdMap", "SG Key of LArOnOffIdMapping object"};
    
      SG::ReadCondHandleKey<LArADC2MeV> m_adc2mevKey
        {this, "ADC2MeVKey", "LArADC2MeV", "SG Key of the LArADC2MeV CDO"};

      /// for properties
      SG::ReadHandleKey<CaloCellContainer> m_sgKey{this, "StoreGateKey", "AllCalo", "Name of the CaloCellContainer"};
      Gaudi::Property<double> m_cellThreshold{this, "LArlCellThreshold", 50.};
      Gaudi::Property<int> m_cellEnergyPrec{this, "CellEnergyPrec", 3};
      Gaudi::Property<int> m_cellTimePrec{this, "CellTimePrec", 3};
      Gaudi::Property<bool> m_lar{this, "RetrieveLAr", true};
      Gaudi::Property<bool> m_doLArCellDetails{this, "DoLArCellDetails", false};
      Gaudi::Property<bool> m_cellConditionCut{this, "CellConditionCut", false};
      Gaudi::Property<std::vector<Identifier::value_type>> m_LArChannelsToIgnoreM5{this, "LArChannelsToIgnoreM5", {}};
      Gaudi::Property<bool> m_doMaskLArChannelsM5{this, "DoMaskLArChannelsM5", false};
      Gaudi::Property<bool> m_doBadLAr{this, "DoBadLAr", false};
  };
}
#endif
