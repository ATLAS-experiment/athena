/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_ALGOJIVEXML_H
#define JIVEXML_ALGOJIVEXML_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/MsgStream.h"
#include "JiveXML/IDataRetriever.h"
#include "xAODEventInfo/EventInfo.h"
#include <vector>

//Forward declarations
namespace JiveXML{
  //  class IDataRetriever;
  class IFormatTool;
  class IStreamTool;
}

namespace JiveXML {

  /**
   * Converts Reconstructed Event Data objects into XML files
   * which can be read by, for example, the Atlantis graphics package.
   *
   * @author N. Konstantinidis
   * @author S. Boeser
   */
  class AlgoJiveXML : public AthAlgorithm {

  public:

    //Constructor
    AlgoJiveXML (const std::string& name, ISvcLocator* pSvcLocator);

    //Default Athena algorithm methods
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();
  private:

    Gaudi::Property<std::string> m_AtlasRelease {this, "AtlasRelease", "unknown", "The Athena release number"};
    Gaudi::Property<std::vector<std::string>> m_dataTypes {this, "DataTypes", {}, "List of data retriever names to be run"};
    Gaudi::Property<bool> m_writeToFile {this, "WriteToFile", true, "Whether XML files shall be produced"};
    Gaudi::Property<bool> m_onlineMode {this, "OnlineMode", false, "Whether an XMLRPC server shall be started"};
    Gaudi::Property<bool> m_writeGeometry {this, "WriteGeometry", false, "Whether Geometry-XML files shall be produced"};
    Gaudi::Property<std::string> m_geometryVersionIn {this, "GeometryVersion", "default", "Geometry version as read from Athena"};
    Gaudi::Property<std::vector<std::string>> m_GeoWriterNames {this, "GeoWriterNames", {"JiveXML::GeometryWriter/GeometryWriter","JiveXML::MuonGeometryWriter/MuonGeometryWriter"}, "The names of the geometry-writer tools"};
    /**
     * The list of DataRetrievers. This is initialised using the list of names
     * supplied by the jobOptions. DataRetrievers are AlgTools residing in the
     * corresponding sub-detector packages (e.g. TrackRetriever in InDetJiveXML).
     **/
    ToolHandleArray<JiveXML::IDataRetriever> m_DataRetrievers;

    /**
     * Handle to the formatting tool, which is passed on to
     * the data retrievers and converts the data into XML
     **/
    ToolHandle<JiveXML::IFormatTool> m_FormatTool {this, "FormatTool", "JiveXML::XMLFormatTool/XMLFormatTool", "Format tool"};

    /**
     * Streaming tools that pass the formatted XML text
     * into a file, to a server, etc.
     **/
    ToolHandle<JiveXML::IStreamTool> m_StreamToFileTool {this, "StreamToFileTool", "JiveXML::StreamToFileTool/StreamToFileTool", "Stream to file tool"};
    ToolHandle<JiveXML::IStreamTool> m_StreamToServerTool; //only initialised if m_onlineMode is True

  };

}//namespace
#endif
