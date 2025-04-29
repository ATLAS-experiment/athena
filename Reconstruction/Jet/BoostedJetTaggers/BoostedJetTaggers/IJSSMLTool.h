// IJSSMLTool.h

#ifndef IJSSMLTool_H
#define IJSSMLTool_H

#include "AsgTools/IAsgTool.h"

// System include(s).
#include <memory>
#include <string>
#include <iostream> 
#include <fstream>
#include <arpa/inet.h>
#include <numeric>

// root
#include "TH2.h"

// xAOD
#include "xAODJet/JetContainer.h"
#include "xAODPFlow/TrackCaloClusterContainer.h"

namespace AthONNX {

  class IJSSMLTool : virtual public asg::IAsgTool {
    ASG_TOOL_INTERFACE(IJSSMLTool)

      public:

    /// Function initialising the tool                                                                           
    virtual StatusCode initialize() = 0;
    
    /// Function executing the tool for a single event                                                           
    virtual double retrieveConstituentsScore(std::vector<TH2D> Images) const = 0;
    virtual double retrieveConstituentsScore(std::vector<std::vector<float>> constituents) const = 0;
    virtual double retrieveConstituentsScore(std::vector<std::vector<float>> constituents, std::vector<std::vector<std::vector<float>>> interactions) const = 0;
    virtual double retrieveHighLevelScore(std::map<std::string, double> JSSVars) const = 0;

    virtual StatusCode SetScaler(std::map<std::string, std::vector<double>> scaler) = 0;

  };

} // end name space AthONNX

#endif
