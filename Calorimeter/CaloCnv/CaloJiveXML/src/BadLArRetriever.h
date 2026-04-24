/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_BADLARRETRIEVER_H
#define JIVEXML_BADLARRETRIEVER_H



#include "CaloEvent/CaloCellContainer.h"
#include "LArCabling/LArOnOffIdMapping.h"

#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "JiveXML/DataType.h" //DataMap typedef

#include <string>
class CaloCell_ID;
namespace JiveXML{
  
  /**
   * @class BadLArRetriever
   * @brief Retrieves all @c Calo Cluster @c objects 
   *
   *  - @b Properties
   *    - StoreGateKey: default is 'AllCalo'. Don't change.
   *	- LArlCellThreshold: default is 50 (MeV)
   *	- RetrieveLAr: general flag, default is true
   *    - CellConditionCut: default is false
   *   	- CellEnergyPrec: precision in int, default is 3 digits
   *    - DoBadLAr: write LAr bad cell, default is false
   *   
   *  - @b Retrieved @b Data
   *    - location in phi and eta
   *    - identifier and energy of each cell 
   */
  class BadLArRetriever : public extends<AthAlgTool, IDataRetriever> {
    
    public:

      using base_class::base_class;
      
      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool) override; 
      const DataMap getBadLArData(const CaloCellContainer* cellContainer);

      /// Return the name of the data type
      virtual std::string dataTypeName() const override{ return "BadLAr"; };

      ///Default AthAlgTool methods
      virtual StatusCode initialize() override;

    private:
      const CaloCell_ID*   m_calocell_id{};
      SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
    
      /// for properties
      SG::ReadHandleKey<CaloCellContainer> m_sgKey{this, "StoreGateKey", "AllCalo", "Name of the CaloCellContainer"};
      Gaudi::Property<double> m_cellThreshold{this, "LArlCellThreshold", 50.};
      Gaudi::Property<int> m_cellEnergyPrec{this, "CellEnergyPrec", 3};
      Gaudi::Property<bool> m_lar{this, "RetrieveLAr", true};
      Gaudi::Property<bool> m_doBadLAr{this, "DoBadLAr", false};
      Gaudi::Property<bool> m_cellConditionCut{this, "CellConditionCut", false};
  };
}
#endif
