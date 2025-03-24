/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCEINTERACES_NODECONNECTOR_H
#define MUONINFERENCEINTERACES_NODECONNECTOR_H

#include <string>
#include <functional>

#include <MuonInferenceInterfaces/NodeFeature.h>
namespace MuonML{
    /** @brief The NodeConnector is indicating whether two space points inside a bucket,
     *         the graph nodes, shall have a connection in their graph neural net representation.
     *         In short terms, the node connector is std::function with the bucket & the two indices
     *         to the space point inside as input arguments and then returning a boolean decision 
     *         whether the connection shall be built.
     *     In order, to verify that two instances of the neural net graph are identical,
     *     the node connector also has a name as attribute. */
    class NodeConnector {
        public:
            using Bucket_t = NodeFeature::Bucket_t;
            /** @brief Function type to connect two space points in a bucket. The signature
             *         takes the reference to the bucket and then the two indices of the space
             *         points which shall be connected. The function needs to return true or false */
            using Evaluator_t = std::function<bool(const Bucket_t&, size_t, size_t)>;
            /** @brief Standard constructor taking the name  of the node connector & 
             *         a connector function definition
             *  @brief cName: Name of the connector function
             *  @brief conFunc: Definition of the connector function */
            NodeConnector(const std::string& cName, const Evaluator_t conFunc):
                            m_name{cName}, m_func{conFunc} {} 
            /** @brief Returns the name of the node connector */
            const std::string& name() const {
                return m_name;
            }
            /** @brief returns the decision of the connector function
             *  @param bucket: Reference to the space point bucket
             *  @param i: Index of the first space point to connect
             *  @param j: Index of the second space point to connect */
            bool connect(const Bucket_t& bucket, size_t i, size_t j) const {
                return m_func(bucket, i, j);
            }

        private:
            std::string m_name{};
            Evaluator_t m_func{[](const Bucket_t, size_t, size_t) { return false; }};

    };
}
#endif