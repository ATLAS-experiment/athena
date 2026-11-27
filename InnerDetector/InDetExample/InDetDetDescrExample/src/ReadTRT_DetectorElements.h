/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ReadTRTDetectorElements.h
 * Header file for ReadTRT_DetectorElements class
 * @author Grant Gorfine
 * updated for Eigen (matrix library) migration Nick Styles, Shaun Roe
**/
#ifndef InDetDetDescrExample_ReadTRT_DetectorElements_h
#define InDetDetDescrExample_ReadTRT_DetectorElements_h

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GeoPrimitives/GeoPrimitives.h"


#include <string>

class Identifier;

namespace InDetDD{
  class TRT_DetectorManager;
  class TRT_BaseElement;
  class TRT_BarrelElement;
  class TRT_EndcapElement;
}

class TRT_ID;

/**
 * @brief Algorithm to demonstrate access to TRT detector elements, 
 * printing out parameters of specific straws
 * 
**/

class ReadTRT_DetectorElements : public AthAlgorithm {
public:
  //! Default Algorithm constructor with parameters
  using AthAlgorithm::AthAlgorithm;
  //@name Athena Algorithm methods
  //@{
  StatusCode initialize();
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();
  //@}
  
  void printAllElements();

  void printBarrelStraw(const InDetDD::TRT_BarrelElement * element, const unsigned int strawNum);
  void printEndcapStraw(const InDetDD::TRT_EndcapElement * element, const unsigned int strawNum);
  void printStraw(const InDetDD::TRT_BaseElement * element, const unsigned int strawNum);

  void compareBarrelStraw(const InDetDD::TRT_BarrelElement * element, const unsigned int strawNum);
  void compareEndcapStraw(const InDetDD::TRT_EndcapElement * element, const unsigned int strawNum);
  bool comparePos(const Amg::Vector3D & p1, const Amg::Vector3D & p2, const double err);
 
private:
  StringProperty m_managerName{this, "ManagerName", "TRT"};
  BooleanProperty m_doInit{this, "DoInitialize", true};
  BooleanProperty m_doExec{this, "DoExecute", false};

  const InDetDD::TRT_DetectorManager * m_manager{nullptr};
  const TRT_ID * m_idHelper{nullptr};

  double m_maxdiff{0};
  bool m_first{true};
};

#endif // InDetDetDescrExample_ReadTRT_DetectorElements_h
