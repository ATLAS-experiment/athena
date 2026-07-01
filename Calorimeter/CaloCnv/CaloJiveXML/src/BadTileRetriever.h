/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_BADTILERETRIEVER_H
#define JIVEXML_BADTILERETRIEVER_H

#include "CaloEvent/CaloCellContainer.h"

#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "JiveXML/DataType.h" //DataMap typedef

#include <string>

class CaloCell_ID;
class Identifier;

namespace JiveXML{
  
  /**
   * @class BadTileRetriever
   * @brief Retrieves all @c Tile Calo Cell @c objects 
   *
   *  - @b Properties
   *    - StoreGateKeyTile: default is 'AllCalo'. Don't change.
   *	- CallThreshold: default is 50 MeV
   *	- RetrieveTile: activate retriever, default is true
   *	- CellEnergyPrec: output precision, default is 3 digits
   *	- DoBadTile: write Tile bad cell, default is false 
   *
   *   
   *  - @b Retrieved @b Data
   *    - location in phi and eta
   *    - identifier and energy of each cell 
   */
  class BadTileRetriever : public extends<AthAlgTool,IDataRetriever> {
    
    public:
      using base_class::base_class;
      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool) override;
      const DataMap getBadTileData(const CaloCellContainer* cellContainer);

      /// Return the name of the data type
      virtual std::string dataTypeName() const override{ return "BadTILE"; };
	
      ///Default AthAlgTool methods
      virtual StatusCode initialize() override;

    private:
      void calcTILELayerSub(Identifier&);
      const CaloCell_ID*   m_calocell_id{};
    
      SG::ReadHandleKey<CaloCellContainer> m_sgKey{this, "StoreGateKey", "AllCalo", "Name of the CaloCellContainer"};
      Gaudi::Property<double> m_cellThreshold{this, "CellThreshold", 50.};
      Gaudi::Property<int> m_cellEnergyPrec{this, "CellEnergyPrec", 3};
      Gaudi::Property<bool> m_tile{this, "RetrieveTILE", true};
      Gaudi::Property<bool> m_doBadTile{this, "DoBadTile", false};

      DataVect m_sub;
  };
}
#endif
