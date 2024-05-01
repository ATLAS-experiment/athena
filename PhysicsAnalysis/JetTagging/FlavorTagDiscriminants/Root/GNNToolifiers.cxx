/*
+  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/GNNToolifiers.h"
#include "FlavorTagDiscriminants/FlipTagEnums.h"
#include "FlavorTagDiscriminants/GNNOptions.h"

#include "AsgTools/AsgTool.h"

namespace FlavorTagDiscriminants {
  void propify(asg::AsgTool& t, GNNToolProperties* props) {
    t.declareProperty("flipTagConfig", props->flipTagConfig,
      "flip configuration used for calibration");
    t.declareProperty("variableRemapping", props->variableRemapping,
      "user-defined mapping to rename the vars stored in the NN");
    t.declareProperty("trackLinkType", props->trackLinkType,
      "access tracks as IParticleContainer or as TrackParticleContainer");
    t.declareProperty("defaultOutputValue", props->default_output_value);
  }

  GNNOptions getOptions(const GNNToolProperties& props) {
    GNNOptions opts;
    if (props.flipTagConfig.size() > 0) {
      opts.flip_config = flipTagConfigFromString(props.flipTagConfig);
    }
    opts.variable_remapping = props.variableRemapping;
    if (props.trackLinkType.size() > 0) {
      opts.track_link_type = trackLinkTypeFromString(props.trackLinkType);
    }
    opts.default_output_value = props.default_output_value;
    return opts;
  }

}
