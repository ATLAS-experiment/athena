/*
+  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// here lives some common stuff between GNNTool and MultifoldGNNTool

#ifndef GNN_TOOLIFIERS_H
#define GNN_TOOLIFIERS_H

#include <string>
#include <map>
#include <cmath>

namespace asg {
  class AsgTool;
}

namespace FlavorTagInference {

  struct GNNOptions;

  struct GNNToolProperties {
    std::string flipTagConfig;
    std::map<std::string,std::string> variableRemapping;
    std::string trackLinkType;
    float default_output_value = NAN;
    std::map<std::string, double> default_output_values; // hack [1]
    bool default_zero_tracks = false;
  };

  void propify(asg::AsgTool& tool, GNNToolProperties* props);
  GNNOptions getOptions(const GNNToolProperties&);
}

#endif

// [1]: Gaudi doesn't like std::map<std::string, float> as a property,
// so we use std::map<std::string, double> instead. This is converted
// to a float before it's used.
