/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonInferenceInterfaces/NodeFeatureFactory.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"

#include <GaudiKernel/SystemOfUnits.h>
#include <set>

namespace MuonML {

    using Feature_t = NodeFeatureList::Feature_t;
    using Connector_t = NodeFeatureList::Connector_t;
    using Bucket_t = NodeFeature::Bucket_t;

    bool operator<(const std::string& a, const Feature_t & b) {
        return a < b->name();
    }
    bool operator<( const Feature_t & a, const std::string& b) {
        return a->name() < b;
    }
    bool operator<(const Feature_t& a, const Feature_t & b) {
        return a->name() < b->name();
    }

    bool operator<(const std::string& a, const Connector_t & b) {
        return a < b->name();
    }
    bool operator<( const Connector_t & a, const std::string& b) {
        return a->name() < b;
    }
    bool operator<(const Connector_t& a, const Connector_t & b) {
        return a->name() < b->name();
    }

    namespace Factory {
        Feature_t makeFeature(const std::string& featName, MsgStream& log) {
            using CovIdx = MuonR4::SpacePoint::CovIdx;
            static const std::set<Feature_t, std::less<>> featurePool{
                std::make_unique<NodeFeature>("localX", 
                    [](const Bucket_t& bucket, size_t index) {
                        return bucket[index]->localPosition().x();
                    }),
                std::make_unique<NodeFeature>("localY", 
                    [](const Bucket_t& bucket, size_t index) {
                        return bucket[index]->localPosition().y();
                        bucket[index]->localPosition().y();
                    }),
                std::make_unique<NodeFeature>("localZ", 
                    [](const Bucket_t& bucket, size_t index) {
                        return bucket[index]->localPosition().z();
                    }),
                std::make_unique<NodeFeature>("stationIndex", 
                    [](const Bucket_t& bucket, size_t index) {
                        return bucket[index]->msSector()->idHelperSvc()->stationName(bucket[index]->identify());
                    }),
                std::make_unique<NodeFeature>("stationPhi", 
                    [](const Bucket_t& bucket, size_t index) {
                        return bucket[index]->msSector()->idHelperSvc()->stationPhi(bucket[index]->identify());
                    }),
                std::make_unique<NodeFeature>("stationEta", 
                    [](const Bucket_t& bucket, size_t index) {
                        return bucket[index]->msSector()->idHelperSvc()->stationEta(bucket[index]->identify());
                    }),
                std::make_unique<NodeFeature>("driftR", 
                    [](const Bucket_t& bucket, size_t index) {
                        return bucket[index]->driftRadius();
                    }),
                std::make_unique<NodeFeature>("relative_layer", 
                    [](const Bucket_t& bucket, size_t index) {
                        const double relLayNum = (1 + 1.*bucket.layerNum(index)) / (bucket.nStripLayers() + bucket.nMdtLayers());
                        return relLayNum;
                    }),
                std::make_unique<NodeFeature>("neighbors", 
                    [](const Bucket_t& bucket, size_t index) {
                        constexpr double radCut2 =  (50.*Gaudi::Units::cm * 50.*Gaudi::Units::cm);
                        unsigned int n =0;
                        for (size_t other =0 ; other < bucket.size(); ++ other){
                            n+= index != other && (bucket[index]->localPosition() - bucket[other]->localPosition()).perp2() < radCut2;
                        }
                        return n;
                    }),
                std::make_unique<NodeFeature>("bucket_density", 
                    [](const Bucket_t& bucket, size_t /*index*/) {

                        return 1.*bucket.size() / std::max(bucket.coveredMax() - bucket.coveredMin(), 1. * Gaudi::Units::cm); 
                    }), 
                std::make_unique<NodeFeature>("isolation", 
                    [](const Bucket_t& bucket, size_t index) {
                        unsigned int  neighbors = 0;
                        constexpr double radCut2 =  (50.*Gaudi::Units::cm * 50.*Gaudi::Units::cm);
                        for (size_t other =0 ; other < bucket.size(); ++ other){
                            neighbors+= index != other && (bucket[index]->localPosition() - bucket[other]->localPosition()).perp2() < radCut2;
                        }
                    
                        float bucket_density = 1.f*bucket.size() / std::max(bucket.coveredMax() - bucket.coveredMin(), 1. * Gaudi::Units::cm);
                        return neighbors / bucket_density;
                    }), 

                std::make_unique<NodeFeature>("covX", 
                    [](const Bucket_t& bucket, size_t index) {
                        return bucket[index]->covariance()[Acts::toUnderlying(CovIdx::phiCov)];
                    }),
                std::make_unique<NodeFeature>("covY", 
                    [](const Bucket_t& bucket, size_t index) {
                        return bucket[index]->covariance()[Acts::toUnderlying(CovIdx::etaCov)];
                    }),              
            }; 
            const auto feat_itr = featurePool.find(featName);
            if(feat_itr != featurePool.end()){
                if (log.level() <= MSG::DEBUG) {
                    log<<MSG::DEBUG<<"Found graph feature "<<featName<<"."<<endmsg;
                }
                return *feat_itr;
            }
            std::stringstream available{};
            for (const Feature_t& known : featurePool) {
                available<<known->name()<<", ";
            }
            log<<MSG::ERROR<<"The feature "<<featName<<" is unknown to the feature factory. "
               <<" Please check for typos w.r.t "<<available.str()<<". Otherwise augment "
               <<__FILE__<<" with your desired feature "<<endmsg;
            return nullptr;
        }
        NodeFeatureList::Connector_t makeConnector(const std::string& connName, MsgStream& log) {

            static const std::set<Connector_t, std::less<>> connectorPool{
                std::make_unique<NodeConnector>("fullyConnected", 
                        [](const Bucket_t& , size_t , size_t ) {
                            return true;
                        }),
            };
            const auto feat_itr = connectorPool.find(connName);
            if(feat_itr != connectorPool.end()){
                if (log.level() <= MSG::DEBUG) {
                    log<<MSG::DEBUG<<"Found graph connector "<<connName<<"."<<endmsg;
                }
                return *feat_itr;
            }
            std::stringstream available{};
            for (const Connector_t& known : connectorPool) {
                available<<known->name()<<", ";
            }
            log<<MSG::ERROR<<"The graph connector "<<connName<<" is unknown to the factory. "
            <<" Please check for typos w.r.t "<<available.str()<<". Otherwise augment "
            <<__FILE__<<" with your desired connection function. "<<endmsg;
            return nullptr;
        }
    }

}