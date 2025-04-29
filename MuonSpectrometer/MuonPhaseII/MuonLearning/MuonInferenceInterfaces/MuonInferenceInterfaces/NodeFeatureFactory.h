/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCEINTERACES_GRAPHFEATUREFACTORY_H
#define MUONINFERENCEINTERACES_GRAPHFEATUREFACTORY_H

#include "MuonInferenceInterfaces/NodeFeatureList.h"

class MsgStream;

namespace MuonML {
    namespace Factory {
      /** @brief Factory function that builds a NodeFeature from a predefined list of features
       *  @param featName: Name of the feature inside the list
       *  @param log: Refetrence to the message object for logging */
      NodeFeatureList::Feature_t makeFeature(const std::string& featName, MsgStream& log);
      /** @brief Factory function that builds a connector relation between two edges in the bucket.
       *  @param connName: Name of the connection function inside the predefined list
       *  @param log: Refetrence to the message object for logging */
      NodeFeatureList::Connector_t makeConnector(const std::string& connName, MsgStream& log);


    }
}

#endif