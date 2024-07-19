/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_CALOMBTSRETRIEVER_H
#define JIVEXML_CALOMBTSRETRIEVER_H



#include "CaloEvent/CaloCellContainer.h"
#include "TileEvent/TileCellContainer.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileEvent/TileDigitsContainer.h"
#include "TileConditions/TileCondToolEmscale.h"
#include "TileConditions/TileCondToolTiming.h"

#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include <string>
#include <vector>
#include <cstddef>
#include <map>

class IToolSvc;
class Identifier;
class TileTBID;

namespace JiveXML{
  
  /**
   * @class CaloMBTSRetriever
   * @brief Retrieves all @c Calo Cluster @c objects 
   *
   *  - @b Properties
   *    - StoreGateKeyMBTS: default is 'MBTSContainer'. Don't change.
   *	- MBTSThreshold: default is 0.05 (geV)
   *	- RetrieveMBTS: activate retriever, default is true
   *	- DoMBTSDigits: write MBTS digits (ADC), default is false
   *   
   *  - @b Retrieved @b Data
   *    - location in phi and eta
   *    - numCells: number of cells in each cluster
   *    - cells: identifier and adc counts of each cell 
   */
  class CaloMBTSRetriever : virtual public IDataRetriever,
                                   public AthAlgTool {
    
    public:
      
      /// Standard Constructor
      CaloMBTSRetriever(const std::string& type,const std::string& name,const IInterface* parent);
      
      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool); 
      const DataMap getMBTSData(const TileCellContainer* tileMBTSCellContainer);

      /// Return the name of the data type
      virtual std::string dataTypeName() const { return "MBTS"; };
	
      ///Default AthAlgTool methods
      StatusCode initialize();

    private:
      ToolHandle<TileCondToolTiming> m_tileToolTiming{this,
          "TileCondToolTiming", "TileCondToolTiming", "Tile timing tool"};

      ToolHandle<TileCondToolEmscale> m_tileToolEmscale{this,
          "TileCondToolEmscale", "TileCondToolEmscale", "Tile EM scale calibration tool"};

      const TileTBID*    m_tileTBID;

      ///properties:
      SG::ReadHandleKey<TileCellContainer> m_sgKeyMBTS{this, "StoreGateKey", "MBTSContainer", "Name of the TileCellContainer"};
      SG::ReadHandleKey<TileDigitsContainer> m_sgKeyTileDigits{this, "TileDigitsContainerKey", "", "Name of the TileDigitsContainer"};
      SG::ReadHandleKey<TileRawChannelContainer> m_sgKeyTileRawChannel{this, "TileRawChannelContainerKey", "", "Name of the TileRawChannelContainer"};
      double m_mbtsThreshold;
      bool m_mbts;
      bool m_mbtsdigit;
      bool m_mbtsCellDetails;
  };
}
#endif
