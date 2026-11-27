/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef InDetDetDescrExample_PrintSiElements_h
#define InDetDetDescrExample_PrintSiElements_h

#include "AthenaBaseComps/AthAlgorithm.h"

#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "GaudiKernel/ServiceHandle.h"

#include <fstream>

/////////////////////////////////////////////////////////////////////////////

class PrintSiElements : public AthAlgorithm {
public:
  using AthAlgorithm::AthAlgorithm;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;

  StatusCode printElements(const std::string & managerName);

  std::string printTransform(const Amg::Transform3D & trans) const;
  void extractAlphaBetaGamma(const Amg::Transform3D & trans, double& alpha, double& beta, double &gamma) const;

private:
  std::ofstream m_fileout;
  bool m_firstEvent{true};
  // Alg properties
  BooleanProperty m_modulesOnly{this, "ModulesOnly", true, "Print transforms of modules"};
  BooleanProperty m_expandId{this, "ExpandId", true, "Print fields of identifier"};
  BooleanProperty m_nominal{this, "NominalPosition", true, "Print out nominal geometry"};
  BooleanProperty m_aligned{this, "AlignedPosition", false, "Print out aligned geometry"};
  BooleanProperty m_fullRotationMatrix{this, "FullRotationMatrix", false, "If true prints the 9 elements of the rotation matrix"};
  StringProperty m_outputFileName{this, "OutputFile", "geometry.dat", "Output file name"};
  ServiceHandle<IGeoModelSvc> m_geoModelSvc{this, "GeoModelSvc", "GeoModelSvc"};
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey{this, "PixelDetEleCollKey", "PixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_SCTDetEleCollKey{this, "SCTDetEleCollKey", "SCT_DetectorElementCollection", "Key of SiDetectorElementCollection for SCT"};
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_ITkPixelDetEleCollKey{this, "ITkPixelDetEleCollKey", "ITkPixelDetectorElementCollection", "Key of SiDetectorElementCollection for ITkPixel"};
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_ITkStripDetEleCollKey{this, "ITkStripDetEleCollKey", "ITkStripDetectorElementCollection", "Key of SiDetectorElementCollection for ITkStrip"};
  StringArrayProperty m_detManagerNames{this, "DetectorManagerNames", {"Pixel","SCT"}};
};

#endif // InDetDetDescrExample_PrintSiElements_h
