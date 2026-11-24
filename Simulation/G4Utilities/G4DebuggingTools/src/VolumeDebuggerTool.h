/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4DEBUGGINGTOOLS_G4UA__VOLUMEDEBUGGERTOOL_H
#define G4DEBUGGINGTOOLS_G4UA__VOLUMEDEBUGGERTOOL_H

#include "G4AtlasTools/UserActionToolBase.h"
#include "./VolumeDebugger.h"

namespace G4UA
{

  /// @class VolumeDebuggerTool
  /// @brief Tool which manages the VolumeDebugger action.
  ///
  /// Create the VolumeDebugger  for each worker thread
  ///
  /// @author Andrea Di Simone
  ///
  class VolumeDebuggerTool : public UserActionToolBase<VolumeDebugger>
  {
    public:
      using UserActionToolBase<VolumeDebugger>::UserActionToolBase;
    protected:
      /// builds the action for a thread
      virtual std::unique_ptr<VolumeDebugger>
      makeAndFillAction(G4AtlasUserActions&) override final;
    private:
      /// holds the configuration received from the python side
      Gaudi::Property<std::string> m_path{this, "OutputPath", "./test_volume.gdml"};
      Gaudi::Property<std::string> m_targetVolume{this, "TargetVolume", "", "By default targetVolume is empty => dump the whole Atlas::Atlas geometry"};
      Gaudi::Property<int> m_res{this, "Resolution", 1000000};
      Gaudi::Property<float> m_tol{this, "Tolerance", 0.000001f};
      Gaudi::Property<bool> m_verbose{this, "Verbose", true};
      Gaudi::Property<bool> m_volumeCheck{this, "RunGeoTest", false};
      Gaudi::Property<double> m_targetMaxCopiesToCheck{this, "MaxCopiesToCheck", 1.e6};
      Gaudi::Property<bool> m_dumpGDML{this, "DumpGDML", true};
      Gaudi::Property<bool> m_dumpPhysicsRegions{this, "DumpPhysicsRegions", false};
      Gaudi::Property<bool> m_printGeo{this, "PrintGeometry", false};
    }; // class VolumeDebuggerTool

} // namespace G4UA

#endif
