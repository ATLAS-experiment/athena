/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelClusterRetriever.h"
#include "JiveXML/DataType.h"

#include "CLHEP/Geometry/Point3D.h"

#include "InDetPrepRawData/PixelCluster.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "ReadoutGeometryBase/SiLocalPosition.h"

#include "InDetIdentifier/PixelID.h"

namespace JiveXML {

 /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  PixelClusterRetriever::PixelClusterRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent)
  {

    //Only declare the interface
    declareInterface<IDataRetriever>(this);
    
    //And the properties
    declareProperty("PixelClusters"  , m_PixelClusterCollName = "PixelClusters");
    declareProperty("PixelTruthMap" , m_PixelTruthMapName = "PRD_MultiTruthPixel");
  }

  StatusCode PixelClusterRetriever::initialize() {
    ATH_CHECK(m_pixelDetEleCollKey.initialize());
    ATH_CHECK(m_PixelClusterCollName.initialize());
    m_usePixelTruthMap = !m_PixelTruthMapName.key().empty();
    ATH_CHECK(m_PixelTruthMapName.initialize(m_usePixelTruthMap));

    return m_geo.retrieve();
  }

  /**
   * Implementation of DataRetriever interface
   * - For each cluster get the coordinates
   * - Try to find associated truth cluster
   * - Store coordinates and id of truth particles
   * @param FormatTool the tool that will create formated output from the DataMap
   **/
  StatusCode PixelClusterRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    //be verbose
    ATH_MSG_DEBUG( "Retrieving " << dataTypeName() ); 
    
    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle(m_pixelDetEleCollKey);
    const InDetDD::SiDetectorElementCollection* elements(*pixelDetEleHandle);
    if (not pixelDetEleHandle.isValid() or elements==nullptr) {
      ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
      return StatusCode::FAILURE;
    }

    //Retrieve the cluster container
    SG::ReadHandle<InDet::SiClusterContainer> SiClusterCont(m_PixelClusterCollName);
    if (!SiClusterCont.isValid()) {
      ATH_MSG_WARNING( "Could not retrieve SiClusterContainer with name " << m_PixelClusterCollName.key() );
      return StatusCode::RECOVERABLE;
    }
   
    //Retrieve the truth collection
    SG::ReadHandle<PRD_MultiTruthCollection> simClusterMap;
    if (m_usePixelTruthMap) {
      simClusterMap = SG::makeHandle(m_PixelTruthMapName);
      if (!simClusterMap.isValid()) {
        //Just write out a warning if this fails
        ATH_MSG_WARNING( "Could not retrieve PRD_MultiTruthCollection with name " <<  m_PixelTruthMapName.key() );
      }
    }

    //Loop over all collections in the container and count the clusters
    unsigned long NClusterTotal = 0;
    for (const auto SiClusterColl : * SiClusterCont)
      NClusterTotal += SiClusterColl->size();
  
    //Now prepare the output data vectors
    DataVect x0; x0.reserve(NClusterTotal);
    DataVect y0; y0.reserve(NClusterTotal);
    DataVect z0; z0.reserve(NClusterTotal);
    DataVect widthx; widthx.reserve(NClusterTotal);
    DataVect widthy; widthy.reserve(NClusterTotal);
    DataVect eloss; eloss.reserve(NClusterTotal);
    DataVect ident; ident.reserve(NClusterTotal);

    //Usually less than one track per cluster - so reserving one should be okay
    DataVect numBarcodes; numBarcodes.reserve(NClusterTotal);
    DataVect barcodes; barcodes.reserve(2*NClusterTotal); 

    DataVect phiModule; phiModule.reserve(NClusterTotal);
    DataVect etaModule; etaModule.reserve(NClusterTotal);

    //Loop over all cluster collections in the container
    for (const auto SiClusterColl : * SiClusterCont) {

      //Only run on Pixel clusters
      if ( ! m_geo->PixelIDHelper()->is_pixel(SiClusterColl->identify())) continue ;

      //Now loop over all clusters in that collection 
      for (const auto sicluster : * SiClusterColl) {

        //Get the cluster
        const InDet::PixelCluster *cluster = dynamic_cast<const InDet::PixelCluster*>(sicluster);
        if (not cluster) continue;
        //and the detector element for that cluster via the id
        Identifier id = m_geo->PixelIDHelper()->wafer_id(cluster->identify());
        IdentifierHash wafer_hash = m_geo->PixelIDHelper()->wafer_hash(id);
        const InDetDD::SiDetectorElement* element = elements->getDetectorElement(wafer_hash);
        if (!element){
          ATH_MSG_DEBUG( "Could not obtain Detector Element with ID " << id );
          continue ;
        }

        //Now store all the information we've obtained so far
        x0.push_back(DataType(cluster->globalPosition().x() /10.));
        y0.push_back(DataType(cluster->globalPosition().y() /10.));
        z0.push_back(DataType(cluster->globalPosition().z() /10.));
        widthx.push_back(DataType(cluster->width().phiR()/10.0));
        widthy.push_back(DataType(cluster->width().z()/10.0));
	      eloss.push_back(DataType(cluster->energyLoss()));

        //Get the cluster id
        Identifier clusterId = cluster->identify();
        ident.push_back(DataType(clusterId.get_compact()));
        phiModule.push_back(DataType(m_geo->PixelIDHelper()->phi_module(clusterId)));
        etaModule.push_back(DataType(m_geo->PixelIDHelper()->eta_module(clusterId)));
        
        //Only process truth if its there
        if ( !m_usePixelTruthMap || !simClusterMap.isValid() ) continue;

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
    dataMap["widthx"] = std::move(widthx);
    dataMap["widthy"] = std::move(widthy);
    dataMap["eloss"] = std::move(eloss);
    dataMap["id"] = std::move(ident);
    dataMap["phiModule"] = std::move(phiModule);
    dataMap["etaModule"] = std::move(etaModule);

    //Only store truth association if we processed them
    if ( numBarcodes.size() > 0 ){
      //Add barcodes counter
      dataMap["numBarcodes"] = std::move(numBarcodes);
      //Calculate multiplicy for barcodes of truth tracks
      std::string bctag = "barcodes multiple=\""+DataType(barcodes.size()/double(numBarcodes.size())).toString()+"\"";
      dataMap[bctag] = std::move(barcodes);
    }

    //Be verbose
    ATH_MSG_DEBUG( " Retrieved " << dataTypeName() << ": " <<  x0.size() );
 
     //forward data to formating tool and return
    return FormatTool->AddToEvent(dataTypeName(), "", &dataMap);
  }

}
