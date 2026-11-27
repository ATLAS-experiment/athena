/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef InDetDetDescrExample_TestTRT_Alignment_h
#define InDetDetDescrExample_TestTRT_Alignment_h

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GeoPrimitives/GeoPrimitives.h"

//#include "CLHEP/Geometry/Transform3D.h"

#include <vector>

class Identifier;
//class HepGeom::Transform3D;

namespace InDetDD{
  class TRT_DetectorManager;
  class TRT_BaseElement;
}

class TRT_ID;

/////////////////////////////////////////////////////////////////////////////

class TestTRT_Alignment : public AthAlgorithm {
public:
  using AthAlgorithm::AthAlgorithm;
  StatusCode initialize();
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();

private:
  
  void addShiftTop(const Amg::Transform3D & transform); 
  void addShiftModule(int bec, int phiMod, int layer, const Amg::Transform3D & transform);
  void addShift(int level, const Identifier & id, const Amg::Transform3D & transform); 
  void compareStraw(const InDetDD::TRT_BaseElement * element, int straw);
  void compareEndStraws(const Identifier &id);
  void printAlignmentShifts();
  bool testIdentity(const Amg::Transform3D & transform, double errRot=0, double errTrans=0);


  const InDetDD::TRT_DetectorManager * m_manager{nullptr};
  const TRT_ID * m_idHelper{nullptr};

  StringProperty m_managerName{this, "ManagerName", "TRT"};
  BooleanProperty m_longPrintOut{this, "LongPrintOut", false};
  BooleanProperty m_testAllStraws{this, "TestAllStraws", false};
  BooleanProperty m_testAllElements{this, "TestAllElements", false};
  DoubleProperty m_errRot{this, "ErrorRotation", 1e-15};      // For testing if alignment changed
  DoubleProperty m_errTrans{this, "ErrorTranslation", 1e-12}; // For testing if alignment changed
  BooleanProperty m_hardwiredShifts{this, "HardwiredShifts", false};
  IntegerProperty m_precision{this, "Precision", 6};
};

#endif // InDetDetDescrExample_TestTRT_Alignment_h
