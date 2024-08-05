/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
  class BadLArRetriever : virtual public IDataRetriever,
                                   public AthAlgTool {
    
    public:
      
      /// Standard Constructor
      BadLArRetriever(const std::string& type,const std::string& name,const IInterface* parent);
      
      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool); 
      const DataMap getBadLArData(const CaloCellContainer* cellContainer);

      /// Return the name of the data type
      virtual std::string dataTypeName() const { return "BadLAr"; };

      ///Default AthAlgTool methods
      StatusCode initialize();

    private:
      const CaloCell_ID*   m_calocell_id;
      SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
    
      /// for properties
      SG::ReadHandleKey<CaloCellContainer> m_sgKey{this, "StoreGateKey", "AllCalo", "Name of the CaloCellContainer"};
      double m_cellThreshold;
      int m_cellEnergyPrec;
      bool m_lar;
      bool m_doBadLAr;
      bool m_cellConditionCut; 
  };
}
#endif