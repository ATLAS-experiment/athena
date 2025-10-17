/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUGNN_H
#define TAURECTOOLS_TAUGNN_H


#include "xAODTau/TauJet.h"
#include "xAODCaloEvent/CaloVertexedTopoCluster.h"
#include "tauRecTools/TauGNNDataLoader.h"
#include "FlavorTagInference/SaltModel.h"
#include "FlavorTagInference/SaltModelGraphConfig.h"

#include "AsgMessaging/AsgMessaging.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <string>
#include <map>

namespace FlavorTagInference{
    class SaltModel;
}

/**
 * @brief Wrapper around SaltModel to compute the output score of a model
 *
 *   Configures the network and computes the network outputs given the input
 *   objects. Retrieval of input variables is handled internally.
 *
 * @author N.M. Tamir
 *
 */
class TauGNN : public asg::AsgMessaging {
public:
    std::shared_ptr<const FlavorTagInference::SaltModel> m_saltModel;
    TauGNNDataLoader m_dataloader;
public:

    TauGNN(
        const TauGNNDataLoader::Config &config
    );
    ~TauGNN() = default;

    // Output the SaltModel tuple 
    std::tuple<
        std::map<std::string, float>,
        std::map<std::string, std::vector<char>>,
        std::map<std::string, std::vector<float>> > 
    compute(const xAOD::TauJet &tau) const;
    //Make the output config transparent to external tools
    FlavorTagInference::OutputConfig gnn_output_config;
};

#endif // TAURECTOOLS_TAUGNN_H