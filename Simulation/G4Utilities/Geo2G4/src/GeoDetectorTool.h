/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEO2G4_GeoDetectorTool_H
#define GEO2G4_GeoDetectorTool_H

// Base classes
#include "G4AtlasTools/DetectorGeometryBase.h"
#include "G4AtlasInterfaces/IGeo2G4Svc.h"

#include "G4Transform3D.hh"

// Members

// STL library
#include <string>
#include <vector>

/** @class GeoDetectorTool GeoDetectorTool.h "G4AtlasTools/GeoDetectorTool.h"
 *
 *  Tool for building detectors out of a GeoModel description. Basically a
 *  carbon copy of GeoDetectorFacility in GeoDetectorPlugins which is supposed
 *  to replace.
 *
 *  @author Andrea Dell'Acqua
 *  @date   2015-03-10
 */

class GeoDetectorTool final : public DetectorGeometryBase
{
public:
  // Basic constructor and destructor
  GeoDetectorTool(const std::string& type, const std::string& name, const IInterface *parent);
  ~GeoDetectorTool() = default;

  /** Athena method. called at initialization time, being customized here */
  virtual StatusCode initialize() override final;

  /** virtual methods being implemented here */

  virtual void BuildGeometry() override final;

  virtual void PositionInParent() override final;

private:
  // Internal methods
  G4LogicalVolume* Convert();
  bool IsTopTransform();
  void SetInitialTransformation();

  //Configurable Properties
  Gaudi::Property<std::string> m_dumpGDMLFile{this, "GDMLFileOut", "", "File name where the GDML description for the detector will be dumped."};
  Gaudi::Property<std::string> m_geoDetectorName{this, "GeoDetectorName", "", "Name of the detector in GeoModel, if different from G4."};
  ServiceHandle<IGeo2G4Svc> m_geo2G4Svc{this, "Geo2G4Svc", "Geo2G4Svc", ""};

  // Other member variables
  std::string m_builderName{""};
  bool m_blParamOn{false};
  bool m_blGetTopTransform{true};
  G4Transform3D m_topTransform; // initialized in constructor
};

#endif
