/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * @author Masahiro Morii, Harvard University
 * Christos Anastopoulos move to using conditions Alg (MT)
 *
 * @brief Test that dumps the solenoid for a selected event
 * used as unit test see
 */

#ifndef SOLENOIDTEST_H
#define SOLENOIDTEST_H

// FrameWork includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "StoreGate/ReadHandleKey.h"
#include "GaudiKernel/ITHistSvc.h"
//
#include <atomic>
#include <string>


// forward declarations
class TTree;

namespace MagField {

class SolenoidTest : public AthAlgorithm {

 public:
  SolenoidTest(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override final;
  virtual StatusCode execute() override final;

 private:
  // Read Magnetic Field
  SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheKey{
      this, "AtlasFieldCacheCondObj", "fieldCondObj",
      "Magnetic Field conditions object key"};

  /// Histogram Service
  ServiceHandle<ITHistSvc> m_thistSvc;
  /// Variable to write out
  double m_xyzt[4]{};      //!< stores the current xyzt position
  double m_field[3]{};     //!< stores the field components
  double m_fieldZR[3]{};   //!< stores the 2d field components
  double m_deriv[9]{};     //!< stores derivatives
  double m_derivZR[9]{};   //!< stores derivatives
  /// the ROOT tree containing the output
  TTree* m_tree{nullptr};

  Gaudi::Property<std::string> m_histStream{
      this, "HistStreamName", "SolenoidTest",
      "Name of the THistSvc output stream"};
  Gaudi::Property<std::string> m_treeName{
      this, "ROOTTreeName", "field",
      "Name of the TTree object in the output file."};
  Gaudi::Property<double> m_minR{this, "MinimumR", 0, "minimum R"};
  Gaudi::Property<double> m_maxR{this, "MaximumR", 1075., "maximum R"};
  Gaudi::Property<double> m_minZ{this, "MinimumZ", -2820, "minimum Z"};
  Gaudi::Property<double> m_maxZ{this, "MaximumZ", 2820., "maximum Z"};
  Gaudi::Property<int> m_stepsR{this, "StepsR", 200,
                                "Number of steps along radius (granularity)"};
  Gaudi::Property<int> m_stepsZ{this, "StepsZ", 200,
                                "Number of steps along z (granularity)"};
  Gaudi::Property<int> m_stepsPhi{this, "StepsPhi", 200,
                                  "Number of steps along phi (granularity)"};
  Gaudi::Property<int> m_event{this, "eventTodump", 0,
                               "Event to dump the magnetic field for"};
  std::atomic<long int> m_eventsSeen{0};  //!< event counter

};  // end class SolenoidTest

}  // end namespace MagField

#endif

