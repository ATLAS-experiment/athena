/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_CALOFCALRETRIEVER_H
#define JIVEXML_CALOFCALRETRIEVER_H

#include "CaloEvent/CaloCellContainer.h"//readhandle template param

#include "LArCabling/LArOnOffIdMapping.h"//readhandle template param
#include "LArRawConditions/LArADC2MeV.h"//readhandle template param

#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <string>
#include <vector>
//
class IToolSvc;
class CaloCell_ID;
class Identifier;

namespace JiveXML{
  
  /**
   * @class CaloFCalRetriever
   * @brief Retrieves all @c Calo Cluster @c objects 
   *
   *  - @b Properties
   *    - StoreGateKey: default is 'AllCalo'. Don't change.
   *	- FCallCellThreshold: default is 50 (MeV)
   *	- RetrieveFCal: general flag, default is true
   *    - DoFCalCellDetails: default is false
   *    - CellConditionCut: default is false
   *    - LArChannelsToIgnoreM5: default is empty (none ignored). Input: vector of cells
   *   	- DoMaskLArChannelsM5: default is false (none masked)
   *   	- CellEnergyPrec: precision in int, default is 3 digits
   *   	- CellTimePrec: precision in int, default is 3 digits
   *   
   *  - @b Retrieved @b Data
   *    - location in x, y
   *    - numCells: number of cells in each cluster
   *    - cells: identifier and adc counts of each cell 
   */
  class CaloFCalRetriever : public extends<AthAlgTool,IDataRetriever> {
    
    public:

      using base_class::base_class;
      
      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool) override;
      const DataMap getFCalData(const CaloCellContainer* cellContainer);

      /// Return the name of the data type
      virtual std::string dataTypeName() const override { return "FCAL"; };

      ///Default AthAlgTool methods
      virtual StatusCode initialize() override;

    private:
      const CaloCell_ID*   m_calocell_id{};

      SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey
	{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
      SG::ReadCondHandleKey<LArADC2MeV> m_adc2mevKey
        { this, "ADC2MeVKey", "LArADC2MeV", "SG Key of the LArADC2MeV CDO" };
      SG::ReadHandleKey<CaloCellContainer> m_sgKey
	{this, "StoreGateKey", "AllCalo", "Name of the CaloCellContainer"};

      /// for properties
      Gaudi::Property<double> m_cellThreshold{this, "FCallCellThreshold", 50.};
      Gaudi::Property<int> m_cellEnergyPrec{this, "CellEnergyPrec", 3};
      Gaudi::Property<int> m_cellTimePrec{this, "CellTimePrec", 3};
      Gaudi::Property<bool> m_fcal{this, "RetrieveFCal", true};
      Gaudi::Property<bool> m_doFCalCellDetails{this, "DoFCalCellDetails", false};
      Gaudi::Property<bool> m_cellConditionCut{this, "CellConditionCut", false};
      Gaudi::Property<std::vector<Identifier::value_type>> m_LArChannelsToIgnoreM5{this, "LArChannelsToIgnoreM5", {}};
      Gaudi::Property<bool> m_doMaskLArChannelsM5{this, "DoMaskLArChannelsM5", false};
      Gaudi::Property<bool> m_doBadFCal{this, "DoBadFCal", false};

  };
}
#endif
