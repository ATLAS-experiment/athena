/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEO2G4_GDMLDetectorTool_H
#define GEO2G4_GDMLDetectorTool_H

// Base classes
#include "G4AtlasTools/DetectorGeometryBase.h"

#include "G4Transform3D.hh"

// Members

// STL library
#include <string>
#include <vector>

/** @class GDMLDetectorTool
 *
 *  Tool for building detectors out of a GDML description.
 *
 *  @author Andrea Dell'Acqua
 *  @date   2017-02-21
 */

class GDMLDetectorTool final : public DetectorGeometryBase
{
 public:
  // Basic constructor and destructor
  GDMLDetectorTool(const std::string& type, const std::string& name, const IInterface *parent);
  ~GDMLDetectorTool() {}

  /** Athena method. called at initialization time, being customized here */
  virtual StatusCode initialize() override final;

  /** virtual methods being implemented here */

  virtual void BuildGeometry() override final;

 private:
  // Internal methods
  bool IsTopTransform();
  void SetInitialTransformation();

  //Configurable Properties
  Gaudi::Property<std::string> m_GDMLFileName{this, "GDMLFileName", "", "Name of the GDML file to be used as input."};
  Gaudi::Property<std::string> m_geoDetectorName{this, "GeoDetectorName", "", "Name of the detector in GeoModel, if different from G4."};

  // Other member variables
  std::string m_builderName{""};
  bool m_blGetTopTransform{true};
  G4Transform3D m_topTransform; // initialized in the constructor
};

#endif // GEO2G4_GDMLDetectorTool_H
