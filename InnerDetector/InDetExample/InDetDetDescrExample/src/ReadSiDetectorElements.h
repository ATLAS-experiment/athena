/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef InDetDetDescrExample_ReadSiDetectorElements_h
#define InDetDetDescrExample_ReadSiDetectorElements_h

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "InDetConditionsSummaryService/ISiliconConditionsTool.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"

#include <vector>

class Identifier;

namespace InDetDD{
  class SiDetectorManager;
  class SiDetectorElement;
}

class AtlasDetectorID;
class PixelID;
class SCT_ID;

/////////////////////////////////////////////////////////////////////////////

class ReadSiDetectorElements : public AthAlgorithm {
public:
  using AthAlgorithm::AthAlgorithm;
  StatusCode initialize();
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();

  void testElement(const Identifier & id, 
                   const std::vector<InDetDD::SiCellId> & cellIdVec,
                   const std::vector<Amg::Vector2D> & positionsVec,
                   const InDetDD::SiDetectorElementCollection* elements=nullptr) const;
  std::string printElementId(const InDetDD::SiDetectorElement * element) const;
  void printAllElements(const bool accessDuringInitialization);
  void printRandomAccess(const bool accessDuringInitialization);
  void printDifference() const;

 private:
  // Job properties
  StringProperty m_managerName{this, "ManagerName", "Pixel"};
  BooleanProperty m_doLoop{this, "LoopOverElements", true};
  BooleanProperty m_doInit{this, "DoInitialize", false};
  BooleanProperty m_doExec{this, "DoExecute", true};
  BooleanProperty m_useConditionsTools{this, "UseConditionsTools", false};
  ToolHandle<ISiliconConditionsTool> m_siConditionsTool{this, "SiConditionsTool", "SCT_SiliconConditionsTool", "Silicon conditions tool"};
  ToolHandle<ISiLorentzAngleTool> m_siLorentzAngleTool{this, "SiLorentzAngleTool", "SiLorentzAngleTool", "Silicon Lorentz anglet tool"};

  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_detEleCollKey{this, "DetEleCollKey", "SCT_DetectorElementCollection", "Key of SiDetectorElementCollection"};

  // Other
  const InDetDD::SiDetectorManager * m_manager{};
  const AtlasDetectorID * m_idHelper{};
  const PixelID * m_pixelIdHelper{};
  const SCT_ID * m_sctIdHelper{};

  bool m_first{true};
  BooleanProperty m_printProbePositions{this, "PrintProbePositions", true};
  BooleanProperty m_printTransforms{this, "PrintTransforms", true};
  BooleanProperty m_printDirections{this, "PrintDirections", true};

};

#endif // InDetDetDescrExample_ReadSiDetectorElements_h
