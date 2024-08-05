/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
  class BadTileRetriever : virtual public IDataRetriever,
                                   public AthAlgTool {
    
    public:
      
      /// Standard Constructor
      BadTileRetriever(const std::string& type,const std::string& name,const IInterface* parent);
      
      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool); 
      const DataMap getBadTileData(const CaloCellContainer* cellContainer);

      /// Return the name of the data type
      virtual std::string dataTypeName() const { return "BadTILE"; };
	
      ///Default AthAlgTool methods
      StatusCode initialize();

    private:
      void calcTILELayerSub(Identifier&);
      const CaloCell_ID*   m_calocell_id;
    
      SG::ReadHandleKey<CaloCellContainer> m_sgKey{this, "StoreGateKey", "AllCalo", "Name of the CaloCellContainer"};
      double m_cellThreshold;
      int m_cellEnergyPrec;
      bool m_tile;
      bool m_doBadTile;

      DataVect m_sub;
  };
}
#endif