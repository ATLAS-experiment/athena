/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_CALOCLUSTERRETRIEVER_H
#define JIVEXML_CALOCLUSTERRETRIEVER_H


#include "CaloEvent/CaloClusterContainer.h"
#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "xAODCaloEvent/CaloClusterContainer.h"
#include "JiveXML/DataType.h" //DataMap typedef
#include <string>
#include <vector>
namespace JiveXML{
  
  /**
   * @class CaloClusterRetriever
   * @brief Retrieves all @c Calo Cluster @c objects 
   *
   *  - @b Properties
   *    - FavouriteJetCollection
   *    - OtherJetCollections
   *    - DoWriteHLT
   *
   *  - @b Retrieved @b Data
   *    - Usual four-vector: phi, eta, et
   *    - id: counter on clusters
   *    - Cells: numCells: number of cells in each cluster, and
   *             cells: compact identifier code of each cell 
   */
  class CaloClusterRetriever : public extends<AthAlgTool,IDataRetriever> {
    
    public:
      using base_class::base_class;
      
      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool) override;
      const DataMap getData(const xAOD::CaloClusterContainer*);

      /// Return the name of the data type
      virtual std::string dataTypeName() const override { return "Cluster"; };
	
      ///Default AthAlgTool methods
      virtual StatusCode initialize() override;

    private:
      SG::ReadHandleKey<xAOD::CaloClusterContainer> m_sgKeyFavourite{this
	  , "FavouriteClusterCollection", "egammaClusters", "Collection to be first in output, shown in Atlantis without switching"};
      Gaudi::Property<std::vector<std::string>> m_otherKeys{this
	, "OtherClusterCollections", {}, "Other collections to be retrieved. If list left empty, all available retrieved"};
      Gaudi::Property<bool> m_doWriteHLT{this
	, "DoWriteHLT", false, "Ignore HLTAutokey object by default"};
  };
}
#endif
