#ifndef G4USERACTIONS_CELEROFFLOADTOOL_H
#define G4USERACTIONS_CELEROFFLOADTOOL_H

#include "G4AtlasTools/UserActionToolBase.h"

#include "CelerOffload.h"

#include "accel/SetupOptions.hh"

namespace G4UA {

  class CelerOffloadTool : public UserActionToolBase<CelerOffload>
  {
    public:
      CelerOffloadTool(const std::string& type, const std::string& name, const IInterface* parent);

      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;

    protected:

      virtual std::unique_ptr<CelerOffload> makeAndFillAction(G4AtlasUserActions&) override final;

    private:
      // Set configurable props here
      using SetupOptions=celeritas::SetupOptions;
      // - I/O options
      // geometry_file: probably never use as Athena/GeoModel provide the G4 geometry
      // output_file?
      // physics_output_file?
      // offload_output_file?
      Gaudi::Property<std::string> m_geometry_output_file{this, "geometry_output_file", {}, "Filename to dump a GDML file for debugging inside frameworks"};

      // - Stepper options
      Gaudi::Property<SetupOptions::size_type> m_max_num_tracks{this, "max_num_tracks", {}, "Number of track 'slots' to be transported simultaneously"};
      Gaudi::Property<SetupOptions::size_type> m_max_steps{this, "max_steps", SetupOptions::no_max_steps(), "Limit on number of steps per track before killing"};
      Gaudi::Property<SetupOptions::size_type> m_max_step_iters{this, "max_step_iters", SetupOptions::no_max_steps(), "Limit on number of step iterations before aborting"};
      Gaudi::Property<SetupOptions::size_type> m_initializer_capacity{this, "initializer_capacity", {}, "Maximum number of track initializers (primaries+secondaries)"};
      // secondary_stack_factor?
      // auto_flush?

      // - Track ordering options
      // track_order?
      // get_num_streams

      // - Stepping actions
      // make_along_step? This is a 'std::function<SPConstAction(AlongStepFactoryInput const&)>;'

      // - Field options
      // max_field_substeps

      // - Sensitive detector options
      // A nested struct in SetupOptions, need to see how Gaudi props handle this

      // - Physics Options
      Gaudi::Property<SetupOptions::VecString> m_ignore_processes{this, "ignore_processes", {}, "Do not use Celeritas physics for the given Geant4 process names"};
      // interpolation is another nested struct

      // - CUDA options
      Gaudi::Property<SetupOptions::size_type> m_cuda_stack_size{this, "cuda_stack_size", {}, "Per-thread stack size (may be needed for VecGeom)"};
      Gaudi::Property<SetupOptions::size_type> m_cuda_heap_size{this, "cuda_heap_size", {}, "Dynamic heap size (may be needed for VecGeom)"};
      Gaudi::Property<bool> m_action_times{this, "action_times", false, "Sync the GPU at every kernel for timing"};
  };
}
#endif

