/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelRDORetriever.h"

#include "InDetIdentifier/PixelID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
//#include "TrkEventPrimitives/LocalPosition.h"
//#include "GaudiKernel/SystemOfUnits.h"

#include "JiveXML/IFormatTool.h"

namespace JiveXML {

  /**
   * Standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  PixelRDORetriever::PixelRDORetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent)
  {
    //Declare the interface
    declareInterface<IDataRetriever>(this);

    //And properties
    declareProperty("PixelRDOContainer"  , m_PixelRDOContainerName = "PixelRDOs");
  }

  /**
   * Implementation of DataRetriever interface
   * - for each pixel raw data object obtain the identifier
   * - find the corresponding detector element and get its coordinates
   * - fill in all information in the data map
   * @param FormatTool the tool that will create formated output from the DataMap
   **/
  StatusCode PixelRDORetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
  
    //be verbose
    ATH_MSG_DEBUG( "Retrieving " << dataTypeName() ); 

    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle(m_pixelDetEleCollKey);
    const InDetDD::SiDetectorElementCollection* elements(*pixelDetEleHandle);
    if (not pixelDetEleHandle.isValid() or elements==nullptr) {
      ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
      return StatusCode::FAILURE;
    }

    //retrieve the PixelRDO container
    SG::ReadHandle<PixelRDO_Container> rdoContainer(m_PixelRDOContainerName);
    if (!rdoContainer.isValid()) {
      ATH_MSG_DEBUG( "Unable to retrieve PixelRDO_Container with name " << m_PixelRDOContainerName.key() );
      return StatusCode::RECOVERABLE;
    }

    //Define the data vectors we want to fill
    DataVect idVec,xVec,yVec,zVec,phiModuleVec,etaModuleVec;

    //Loop over pixel RDO container
    for (const auto rdoCollection : *rdoContainer) {
      
      //Loop over raw data objects
      for (const auto rdoData : *rdoCollection) {

        //Get the pixel identifier
        const Identifier id = rdoData->identify();
        const Identifier waferID = m_pixelID->wafer_id(id);
        IdentifierHash wafer_hash = m_pixelID->wafer_hash(waferID);

        //Get the detector element
        const InDetDD::SiDetectorElement *element = elements->getDetectorElement(wafer_hash);

        //Make sure we got the detector element
        if (element == nullptr){
          ATH_MSG_WARNING( "Unable to obtain detector element for PixelRDO hit with id " << id );
          continue;
        }

        //Get the global position from the local position
        Amg::Vector2D localPos = element->rawLocalPositionOfCell(id);
        localPos[Trk::distPhi] += m_lorentzAngleTool->getLorentzShift(element->identifyHash(), Gaudi::Hive::currentContext());
	      Amg::Vector3D globalPos = element->globalPosition(localPos);

        //Fill in all the data in our data vectors
	      idVec.push_back(DataType( id.get_compact() ));
        xVec.push_back(DataType(globalPos.x()*CLHEP::mm/CLHEP::cm));
        yVec.push_back(DataType(globalPos.y()*CLHEP::mm/CLHEP::cm));
        zVec.push_back(DataType(globalPos.z()*CLHEP::mm/CLHEP::cm));
        phiModuleVec.push_back(DataType(m_pixelID->phi_module(waferID)));
        etaModuleVec.push_back(DataType(m_pixelID->eta_module(waferID)));
      }
    }

    //be verbose about the amount of data we retrieved
    ATH_MSG_DEBUG( "Retrieved " << idVec.size() << " PixelRDO objects" );

    //Create a data map
    DataMap dataMap;
    //Add the data vectors to our map
    dataMap["id"]=idVec;
    dataMap["x"]=xVec;
    dataMap["y"]=yVec;
    dataMap["z"]=zVec;
    dataMap["phiModule"]=phiModuleVec;
    dataMap["etaModule"]=etaModuleVec;

    /**
     * NOTE: This is a temporary fix. Using rdoContainer.key() results in a segfault!
     **/
    //Forward data to formatting tool
    if ( FormatTool->AddToEvent(dataTypeName(), "PixelRDOs", &dataMap).isFailure())
      return StatusCode::RECOVERABLE;

    //Clean up and exit
    ATH_MSG_DEBUG( dataTypeName() << " retrieved" );
    
    return StatusCode::SUCCESS;
  }


  StatusCode PixelRDORetriever::initialize() {
    ATH_CHECK( m_lorentzAngleTool.retrieve() );
    ATH_CHECK(m_pixelDetEleCollKey.initialize());
    ATH_CHECK(m_PixelRDOContainerName.initialize());
    ATH_CHECK(detStore()->retrieve(m_pixelID, "PixelID"));

    return StatusCode::SUCCESS;
  }
}
