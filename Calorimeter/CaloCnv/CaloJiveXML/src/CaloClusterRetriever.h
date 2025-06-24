/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_CALOCLUSTERRETRIEVER_H
#define JIVEXML_CALOCLUSTERRETRIEVER_H

#include <string>
#include <vector>
#include <map>

#include "CaloEvent/CaloClusterContainer.h"
#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "xAODCaloEvent/CaloClusterContainer.h"

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
  class CaloClusterRetriever : virtual public IDataRetriever,
                                   public AthAlgTool {
    
    public:
      
      /// Standard Constructor
      CaloClusterRetriever(const std::string& type,const std::string& name,const IInterface* parent);
      
      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool); 
      const DataMap getData(const xAOD::CaloClusterContainer*);

      /// Return the name of the data type
      virtual std::string dataTypeName() const { return "Cluster"; };
	
      ///Default AthAlgTool methods
      StatusCode initialize();

    private:
      SG::ReadHandleKey<xAOD::CaloClusterContainer> m_sgKeyFavourite{this, "StoreGateKey", "egammaClusters", "Name of the CaloClusterContainer"};
      std::vector<std::string> m_otherKeys;
      bool m_doWriteHLT;
  };
}
#endif
