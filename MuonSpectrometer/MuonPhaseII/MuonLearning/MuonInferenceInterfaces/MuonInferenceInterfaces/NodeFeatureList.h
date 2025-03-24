/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCEINTERACES_GRAPHFEATURELIST_H
#define MUONINFERENCEINTERACES_GRAPHFEATURELIST_H

#include "MuonInferenceInterfaces/NodeFeature.h"
#include "MuonInferenceInterfaces/NodeConnector.h"

class MsgStream;

namespace MuonML {
    struct GraphRawData;
    class NodeFeatureList {
        public:
            
            using Feature_t = std::shared_ptr<const NodeFeature>;
            using Connector_t = std::shared_ptr<const NodeConnector>;

            using Bucket_t = NodeFeature::Bucket_t;
            /** @brief Empty standard constructor */
            NodeFeatureList() = default;
            /** @brief Returns true if the features have pairwise
             *         the same name */
            bool operator==(const NodeFeatureList& other) const;
            /** @brief Returns whether the NodeFeatureList is complete,
             *         i.e. it must have at least one feature and the node
             *         connector */
            bool isValid() const;
            /** @brief Returns the number of features in the list */
            size_t numFeatures() const;
            /** @brief Returns the name of the features in the list */
            std::vector<std::string> featureNames() const;
            /** @brief  */
            void fillInData(const Bucket_t& bucket, 
                            GraphRawData& graphData) const;
            /** @brief Tries to add a new feature to the list using the predefined
             *          list of features in the GraphFeatureFactory
             *  @param featName: Name of the feature in the factory
             *  @param msg: Reference to the message stream object for logging. */
            bool addFeature(const std::string& featName, MsgStream& msg);
            /** @brief Tries to add a particular feature to the list.
             *  @param featPtr: Pointer to the instantiated feature
             *  @param msg: Reference to the message stream object for logging. */
            bool addFeature(const Feature_t& featPtr,  MsgStream& msg);

            /** @brief Tries to set the graph connector based on the connector name.
             *  @param conName: Name of the connector to extract from the factory
             *  @param msg: Reference to the message stream object for logging. */
            bool setConnector(const std::string& conName, MsgStream& msg);
            /** @brief Sets the  */
            void setConnector(const std::string& conName, NodeConnector::Evaluator_t evalFunc);

            
        private:
            std::vector<Feature_t> m_features{};
            Connector_t m_connector{};
            
    };
}


#endif