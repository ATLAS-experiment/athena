/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef InDetDetDescrExample_TestSiAlignment_h
#define InDetDetDescrExample_TestSiAlignment_h

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "ReadoutGeometryBase/SiLocalPosition.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <vector>

class Identifier;

namespace InDetDD{
  class SiDetectorManager;
  class SiDetectorElement;
}

/////////////////////////////////////////////////////////////////////////////

class TestSiAlignment : public AthAlgorithm {
public:
  using AthAlgorithm::AthAlgorithm;
  StatusCode initialize();
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();

private:
  void printAlignmentShifts(const bool accessDuringInitialization);
  bool testIdentity(const Amg::Transform3D & transform, double errRot=0, double errTrans=0);
  void extractAlphaBetaGamma(const Amg::Transform3D & trans, double& alpha, double& beta, double &gamma) const; 
  const InDetDD::SiDetectorManager * m_manager{nullptr};
  StringProperty m_managerName{this, "ManagerName", ""};
  BooleanProperty m_longPrintOut{this, "LongPrintOut", false};
  DoubleProperty m_errRot{this, "ErrorRotation", 1e-15};      // For testing if alignment changed
  DoubleProperty m_errTrans{this, "ErrorTranslation", 1e-12}; // For testing if alignment changed
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_detEleCollKey{this, "DetEleCollKey", "SCT_DetectorElementCollection", "Key of SiDetectorElementCollection"};
};

#endif // InDetDetDescrExample_TestSiAlignment_h
