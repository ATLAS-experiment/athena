/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
  class CaloMBTSRetriever : public extends<AthAlgTool,IDataRetriever> {
    
    public:
      using base_class::base_class;
      
      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool) override;
      const DataMap getMBTSData(const TileCellContainer* tileMBTSCellContainer);

      /// Return the name of the data type
      virtual std::string dataTypeName() const override { return "MBTS"; };
	
      ///Default AthAlgTool methods
      virtual StatusCode initialize() override;

    private:
      ToolHandle<TileCondToolTiming> m_tileToolTiming{this,
          "TileCondToolTiming", "TileCondToolTiming", "Tile timing tool"};

      ToolHandle<TileCondToolEmscale> m_tileToolEmscale{this,
          "TileCondToolEmscale", "TileCondToolEmscale", "Tile EM scale calibration tool"};

      const TileTBID*    m_tileTBID{};

      SG::ReadHandleKey<TileCellContainer> m_sgKeyMBTS{this, "StoreGateKey", "MBTSContainer", "Name of the TileCellContainer"};
      SG::ReadHandleKey<TileDigitsContainer> m_sgKeyTileDigits{this, "TileDigitsContainer", ""
	, "Input collection to retrieve Tile raw channels, used when DoMBTSCellDetails is True"};
      SG::ReadHandleKey<TileRawChannelContainer> m_sgKeyTileRawChannel{this, "TileRawChannelContainer", ""
	, "Input collection to retrieve Tile digits, used when doTileDigit is True"};

      Gaudi::Property<double> m_mbtsThreshold{this, "MBTSThreshold", 0.05};
      Gaudi::Property<bool> m_mbts{this, "RetrieveMBTS", true};
      Gaudi::Property<bool> m_mbtsdigit{this, "DoMBTSDigits", false};
      Gaudi::Property<bool> m_mbtsCellDetails{this, "DoMBTSCellDetails", false};
  };
}
#endif
