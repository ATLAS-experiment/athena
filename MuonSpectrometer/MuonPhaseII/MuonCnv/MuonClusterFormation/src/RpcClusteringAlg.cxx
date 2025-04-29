/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "RpcClusteringAlg.h"

#include <algorithm>
#include <xAODMuonPrepData/RpcStripAuxContainer.h>
#include <xAODMuonViews/ChamberViewer.h>
#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteDecorHandle.h>
#include <StoreGate/WriteHandle.h>
namespace MuonR4{

    StatusCode RpcClusteringAlg::initialize() {
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_linkKey.initialize());
        return StatusCode::SUCCESS;  
    }
    StatusCode RpcClusteringAlg::execute(const EventContext& ctx) const {
        const xAOD::RpcStripContainer* inContainer{nullptr};
        ATH_CHECK(SG::get(inContainer, m_readKey, ctx));

        SG::WriteHandle outContainer{m_writeKey, ctx};
        ATH_CHECK(outContainer.record(std::make_unique<xAOD::RpcStripContainer>(), 
                                      std::make_unique<xAOD::RpcStripAuxContainer>()));

        SG::WriteDecorHandle<xAOD::RpcStripContainer, LinkType> linkDecor{m_linkKey, ctx};
        
        auto createCluster = [&](std::vector<const xAOD::RpcStrip*>& constituents) {
            if (constituents.empty()) return;
            xAOD::RpcStrip* cluster = outContainer->push_back(std::make_unique<xAOD::RpcStrip>());
            (*cluster) = (*constituents[constituents.size()/2]);
            
            xAOD::MeasVector<1> locPos{ 0.5*(constituents[0]->localPosition<1>().x() +
                                        constituents[constituents.size() -1]->localPosition<1>().x())};
            xAOD::MeasMatrix<1> locCov{constituents.size() * cluster->localCovariance<1>()(0,0)};

            cluster->setMeasurement(cluster->identifierHash(), std::move(locPos), std::move(locCov));
            
            LinkType& links{linkDecor(*cluster)};
            float midTime{0.f}, midTimeCov{0.f};
            for (const xAOD::RpcStrip* strip: constituents){
                links.emplace_back(*inContainer, strip->index());
                midTime+= strip->time();
                midTimeCov+=strip->timeCovariance();
            }
            midTime /= constituents.size();
            midTimeCov /= constituents.size();
            float spread{0.f};
            for (const xAOD::RpcStrip* strip : constituents) {
                spread+=std::pow(midTime - strip->time(), 2);
            }
            spread /= std::pow(constituents.size(), 2);
            midTimeCov += spread;
            cluster->setTime(midTime);
            cluster->setTimeCovariance(midTimeCov);
            constituents.clear();
        };
        
        /** Helper object returning ranges per chamber */
        xAOD::ChamberViewer hitsPerChamber{*inContainer};
        std::size_t seenHits{};
        do {
            seenHits+=hitsPerChamber.size();
            /** Sort the hits per layer */
            std::vector<std::vector<const xAOD::RpcStrip*>> sortedHits{};
            for (const xAOD::RpcStrip* strip : hitsPerChamber) {
                const unsigned int layerHash = strip->layerHash();
                if (layerHash >= sortedHits.size()) {
                    sortedHits.resize(layerHash +1);
                }
                sortedHits[layerHash].push_back(strip);
            }
            /** Cluster the sorted hits */
            for (std::vector<const xAOD::RpcStrip*>& hitsInLay : sortedHits){
                if (hitsInLay.empty()) continue;               
                /// Sort the hits by channel
                std::sort(hitsInLay.begin(), hitsInLay.end(), 
                        [](const xAOD::RpcStrip*a, const xAOD::RpcStrip* b ){
                            return a->stripNumber() < b->stripNumber();
                        });
                uint16_t lastChannel = hitsInLay[0]->stripNumber();
                std::vector<const xAOD::RpcStrip*> groupedHits{};
                for (const xAOD::RpcStrip* strip : hitsInLay) {
                    if (strip->stripNumber() - lastChannel  -1u > m_maxHoles ||
                        (!groupedHits.empty() && 1u*(strip->stripNumber() - groupedHits[0]->stripNumber()) > m_maxSize)){
                        createCluster(groupedHits);
                    }
                    groupedHits.push_back(strip);
                }
                createCluster(groupedHits);
            }
        
        } while (hitsPerChamber.next());
        if (seenHits != inContainer->size()) {
            ATH_MSG_FATAL("Detected inconsistency between the hits in container "<<inContainer->size()<<
                          " and what's returned by the chamber viewer "<<seenHits);
            return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    }
}