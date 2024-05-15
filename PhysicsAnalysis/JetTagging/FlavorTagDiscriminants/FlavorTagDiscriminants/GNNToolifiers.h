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

namespace FlavorTagDiscriminants {

  struct GNNOptions;

  struct GNNToolProperties {
    std::string flipTagConfig;
    std::map<std::string,std::string> variableRemapping;
    std::string trackLinkType;
    float default_output_value = NAN;
  };

  void propify(asg::AsgTool& tool, GNNToolProperties* props);
  GNNOptions getOptions(const GNNToolProperties&);
}

#endif
