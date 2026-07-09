/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "SiClusterRetriever.h"
#include "StoreGate/ReadCondHandle.h"
#include "JiveXML/DataType.h"

#include "CLHEP/Geometry/Point3D.h"


#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "ReadoutGeometryBase/SiLocalPosition.h"

#include "InDetIdentifier/SCT_ID.h"

namespace JiveXML {

 /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  SiClusterRetriever::SiClusterRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent)
  {

    //Only declare the interface
    declareInterface<IDataRetriever>(this);
    
    //And the properties
    declareProperty("SCTClusters"  , m_SiClusterCollName = "SCT_Clusters", "Collection name of SiClusterContainer for SCT");
    declareProperty("SCT_TruthMap" , m_SiTruthMapName = "PRD_MultiTruthSCT", "Collection name of PRD_MultiTruthCollection> for SCT");
  }

  /**
   * Implementation of DataRetriever interface
   * - For each cluster get the coordinates
   * - Try to find associated truth cluster
   * - Store coordinates and id of truth particles
   * @param FormatTool the tool that will create formated output from the DataMap
   **/
  StatusCode SiClusterRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    //be verbose
    ATH_MSG_DEBUG( "Retrieving " << dataTypeName() ); 

    // Get SCT_DetectorElementCollection
    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> sctDetEle(m_SCTDetEleCollKey);
    const InDetDD::SiDetectorElementCollection* elements(sctDetEle.retrieve());
    if (elements==nullptr) {
      ATH_MSG_FATAL(m_SCTDetEleCollKey.fullKey() << " could not be retrieved");
      return StatusCode::FAILURE;
    }

    //Retrieve the cluster container
    SG::ReadHandle<InDet::SiClusterContainer> SiClusterCont(m_SiClusterCollName);
    if (!SiClusterCont.isValid()) {
      ATH_MSG_DEBUG( "Could not retrieve SiClusterContainer with name " << m_SiClusterCollName.key() );
      return StatusCode::RECOVERABLE;
    }
   
    //Retrieve the truth collection
    SG::ReadHandle<PRD_MultiTruthCollection> simClusterMap;
    if (m_useSiTruthMap) {
      simClusterMap = SG::makeHandle(m_SiTruthMapName);
      if (!simClusterMap.isValid()) {
          //Just write out a warning if this fails
          ATH_MSG_DEBUG( "Could not retrieve PRD_MultiTruthCollection with name " <<  m_SiTruthMapName.key() );
      }
    }

    //Loop over all collections in the container and count the clusters
    unsigned long NClusterTotal = 0;
    for (const auto SiClusterColl : *SiClusterCont.cptr())
      NClusterTotal += SiClusterColl->size();
  
    //Now prepare the output data vectors
    DataVect x0; x0.reserve(NClusterTotal);
    DataVect y0; y0.reserve(NClusterTotal);
    DataVect z0; z0.reserve(NClusterTotal);
    DataVect x1; x1.reserve(NClusterTotal);
    DataVect y1; y1.reserve(NClusterTotal);
    DataVect z1; z1.reserve(NClusterTotal);
    DataVect width; width.reserve(NClusterTotal);
    DataVect ident; ident.reserve(NClusterTotal);

    //Usually less than one track per cluster - so reserving one should be okay
    DataVect numBarcodes; numBarcodes.reserve(NClusterTotal);
    DataVect barcodes; barcodes.reserve(NClusterTotal); 

    DataVect phiModule; phiModule.reserve(NClusterTotal);
    DataVect etaModule; etaModule.reserve(NClusterTotal);
    DataVect side; side.reserve(NClusterTotal);

    //Loop over all cluster collections in the container
    for (const auto SiClusterColl : *SiClusterCont.cptr()){

      //Only run on silicon (SCT) clusters
      if ( ! m_geo->SCTIDHelper()->is_sct(SiClusterColl->identify())) continue ;

      const IdentifierHash waferHash = SiClusterColl->identifyHash();

      //Now loop over all clusters in that collection 
      for (const auto cluster : *SiClusterColl){ 

        //and the detector element for that cluster via the id
        Identifier id = m_geo->SCTIDHelper()->wafer_id(cluster->identify());
        const InDetDD::SiDetectorElement* element = elements->getDetectorElement(waferHash);
        if (!element){
          ATH_MSG_DEBUG( "Could not obtain Detector Element with ID " << id );
          continue ;
        }
            
        //Get the local position of the cluster
        InDetDD::SiLocalPosition pos = element->localPosition(cluster->globalPosition());
        std::pair<Amg::Vector3D, Amg::Vector3D > ends = element->endsOfStrip(pos);
        
	      Amg::Vector3D a = ends.first;   // Top end, first cluster
	      Amg::Vector3D b = ends.second;  // Bottom end, first cluster
        
        //Now store all the infromation we've obtained so far
        x0.push_back(DataType(a.x() /10.));
        y0.push_back(DataType(a.y() /10.));
        z0.push_back(DataType(a.z() /10.));
        x1.push_back(DataType(b.x() /10.));
        y1.push_back(DataType(b.y() /10.));
        z1.push_back(DataType(b.z() /10.));
        width.push_back(DataType(cluster->width().phiR()/20.0));

        //Get the cluster id
        Identifier clusterId = cluster->identify();
        ident.push_back(DataType(clusterId.get_compact()));
        phiModule.push_back(DataType(m_geo->SCTIDHelper()->phi_module(clusterId)));
        etaModule.push_back(DataType(m_geo->SCTIDHelper()->eta_module(clusterId)));
        side.push_back(DataType(m_geo->SCTIDHelper()->side(clusterId)));
        
        //Only process truth if its there
        if ( !m_useSiTruthMap || !simClusterMap.isValid() ) continue;

        // Count the number of associated truth particles, and store their barcodes
        unsigned long countBarcodes=0;
        using iter = PRD_MultiTruthCollection::const_iterator;
        std::pair<iter,iter> range = simClusterMap->equal_range(clusterId);
        for (iter i = range.first; i != range.second; ++i) {
          ++countBarcodes;
          barcodes.push_back(DataType(i->second.barcode()));
        }
        numBarcodes.push_back(DataType(countBarcodes));

      } // loop over clusters
    } // loop over collections


    //Now generate a DataMap for the output
    DataMap dataMap;
    dataMap["x0"] = std::move(x0);
    dataMap["y0"] = std::move(y0);
    dataMap["z0"] = std::move(z0);
    dataMap["x1"] = std::move(x1);
    dataMap["y1"] = std::move(y1);
    dataMap["z1"] = std::move(z1);
    dataMap["width"] = std::move(width);
    dataMap["id"] = std::move(ident);
    dataMap["phiModule"] = std::move(phiModule);
    dataMap["etaModule"] = std::move(etaModule);
    dataMap["side"] = std::move(side);

    //Only store truth association if we processed them
    if ( numBarcodes.size() > 0 ){
      //Add barcodes counter
      dataMap["numBarcodes"] = std::move(numBarcodes);
      //Calculate multiplicy for barcodes of truth tracks
      std::string bctag = "barcodes multiple=\""+DataType(barcodes.size()/double(numBarcodes.size())).toString()+"\"";
      dataMap[bctag] = barcodes;
    }

    //Be verbose
    ATH_MSG_DEBUG( " Retrieved " << dataTypeName() << ": " <<  x0.size() );
 
     //forward data to formating tool and return
    return FormatTool->AddToEvent(dataTypeName(), "", &dataMap);
  }

  StatusCode SiClusterRetriever::initialize() {
    ATH_CHECK(m_SCTDetEleCollKey.initialize());
    ATH_CHECK(m_SiClusterCollName.initialize());
    m_useSiTruthMap = !m_SiTruthMapName.key().empty();
    ATH_CHECK(m_SiTruthMapName.initialize(m_useSiTruthMap));

    return m_geo.retrieve();
  }
}
 
     
    
