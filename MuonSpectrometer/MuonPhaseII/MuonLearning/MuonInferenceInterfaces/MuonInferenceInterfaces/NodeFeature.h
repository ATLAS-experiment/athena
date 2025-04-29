/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCEINTERACES_GRAPHFEATURE_H
#define MUONINFERENCEINTERACES_GRAPHFEATURE_H


#include <functional>
#include <string>

#include "MuonInferenceInterfaces/LayerBucket.h"

namespace MuonML {
    /** @brief The NodeFeature is the gluing instance to extract the information from the space point
     *         inside a MuonBucket and then to parse it to the ML inference framework. */
    class NodeFeature {
        public:
            /** @brief Abreviation of the Space point bucket type */
            using Bucket_t = LayerSpBucket;

            /** @brief Lambda function type to extract the feature from a bucket.
             *  @arg: Bucket_t of interest
             *  @arg: size_t Index of the space point to extract the bucket from */
            using Func_t = std::function<double(const Bucket_t&, size_t)>;

            /** @brief Standard constructor to build a feature
             *  @param featName: Name of the feature used to distinguish whether 
             *                   two feature lists are the same */
            NodeFeature(const std::string& featName,
                        const Func_t& extractFunc):
                m_name{featName}, m_func{extractFunc}{}
            /** @brief Standard move assignment & move constructor */
            NodeFeature(NodeFeature&& other) = default;

            /** @brief Returns the feature name */
            const std::string& name() const {
                return m_name;
            }
            /** @brief Extract the feature from a space point inside the bucket
             *  @param bucket: Reference to the space point bucket of interest
             *  @param spIndex: Index of the space point to extract the feature from */
            double eval(const Bucket_t& bucket, size_t spIndex) const {
                return m_func(bucket, spIndex);
            }
        private:
            std::string m_name{};
            Func_t m_func{[](const Bucket_t& , size_t) { return 0.; }};
    };   
}
#endif