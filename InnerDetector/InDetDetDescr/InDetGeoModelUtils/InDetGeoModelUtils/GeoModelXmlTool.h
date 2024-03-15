/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELSVC_GEOMODELXMLTOOL_H
#define GEOMODELSVC_GEOMODELXMLTOOL_H

#include <GeoModelInterfaces/IGeoDbTagSvc.h>
#include <GeoModelUtilities/GeoModelTool.h>
#include <RDBAccessSvc/IRDBAccessSvc.h>

namespace GeoModelIO{
  class ReadGeoModel;
}

class GeoPhysVol;
class GeoVPhysVol;
class GmxInterface;

class GeoModelXmlTool : public GeoModelTool
{

public:
  GeoModelXmlTool(const std::string& type, const std::string& name, const IInterface* parent);

protected:
  bool isAvailable(const std::string& versionNode, const std::string& tableNode) const;
  std::string getBlob(const std::string& versionNode, const std::string& tableNode) const;

  // method for derived classes to initialize the services needed here
  StatusCode createBaseTool();

  // method to check if we are using the Run4 geometry workflow, from a single sqlite file, and if so, return the reader
  // NB return can be null - used downstream to check if we are using sqlite inputs or not
  GeoModelIO::ReadGeoModel* getSqliteReader() const;

  // (optional) containingDetector/envelopeVolume can be used if the geometry tree needs to be further descended to add "topVolume"
  // (optional) sqlreader steers if using a pre-built sqlite input, or parsing the geometry xml files to build it here
  const GeoVPhysVol * createTopVolume(GeoPhysVol* worldVol, GmxInterface& interface, const std::string& versionNode, const std::string& tableNode, const std::string& containingDetector="", const std::string& envelopeName="", const GeoModelIO::ReadGeoModel* sqlreader=nullptr) const;

  Gaudi::Property<std::string> m_gmxFilename{this, "GmxFilename", "", "The name of the local file to read the geometry from"};
  Gaudi::Property<std::string> m_detectorName{this, "DetectorName", "ITkStrip", ""};
  ServiceHandle<IRDBAccessSvc> m_rdbAccessSvc{this, "RDBAccessSvc", "RDBAccessSvc", ""};
  ServiceHandle<IRDBAccessSvc> m_sqliteReadSvc{this, "SqliteReadSvc", "SqliteReadSvc", ""};
  ServiceHandle<IGeoDbTagSvc> m_geoDbTagSvc{this, "GeoDbTagSvc", "GeoDbTagSvc", ""};
  Gaudi::Property<std::string> m_dtdName{this, "OverrideDtdName", "", "Override standard .dtd file from GeoModelXml"};

  Gaudi::Property<bool> m_deduplicateLogVol{this, "enableLogVolDeduplication", false,
                                            "Remove duplications of equivalent logical volumes"};  
  Gaudi::Property<bool> m_deduplicatePhysVol{this, "enablePhysVolDeduplication", false,
                                            "Remove duplications of equivalent physical volumes"};
  Gaudi::Property<bool> m_deduplicateShape{this, "enableShapeDeduplication", false,
                                            "Remove duplications of equivalent shapes"};
  Gaudi::Property<bool> m_deduplicateTransf{this, "enableTransDeduplication", false,
                                            "Remove duplications of equivalent transform nodes"};


private:

  Gaudi::Property<std::string> m_clobOutputFileName{this, "ClobOutputName", "", "Name of file to dump CLOB content to"};
  //do the actual creation of the volume from the gmx files
  void createVolume(GeoPhysVol* worldVol, GmxInterface& interface, const std::string& versionNode, const std::string& tableNode) const;
};

#endif // GEOMODELSVC_GEOMODELXMLTOOL_H
