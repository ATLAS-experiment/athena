// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimGNNGraphHitSelectorTool.h"

///////////////////////////////////////////////////////////////////////////////
// AthAlgTool

FPGATrackSimGNNGraphHitSelectorTool::FPGATrackSimGNNGraphHitSelectorTool(const std::string& algname, const std::string &name, const IInterface *ifc) 
    : AthAlgTool(algname, name, ifc) {}

///////////////////////////////////////////////////////////////////////
// Functions

StatusCode FPGATrackSimGNNGraphHitSelectorTool::selectHits(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & graph_hits)
{
    // Not all FPGATrackSimHits will be passed to our GNN Pipeline
    // So we need to select the hits that we want
    // For pixel, this is all the same
    // For strip, we only want to keep one spacepoint for the two clusters. However, we also need to keep the position of both clusters (necessary input for the GNN)

    for (unsigned long int i = 0; i < hits.size(); i++) {
        // For pixel, the hit = SP = cluster_1 = cluster_2
        if(hits[i]->isPixel() && to_string(hits[i]->getHitType()) == "clustered") {
            std::shared_ptr<FPGATrackSimGNNHit> graph_hit = std::make_shared<FPGATrackSimGNNHit>();
            graph_hit->setHitID(i);
            graph_hit->setIdentifier(hits[i]->getIdentifier());
            graph_hit->setX(hits[i]->getX());
            graph_hit->setY(hits[i]->getY());
            graph_hit->setZ(hits[i]->getZ());
            graph_hit->setR(hits[i]->getR());
            graph_hit->setR2(TMath::Power(hits[i]->getR(),2.0) + TMath::Power(hits[i]->getZ(),2.0));
            graph_hit->setPhi(hits[i]->getGPhi());
            graph_hit->setEta(getEta(hits[i]));
            graph_hit->setCluster1X(hits[i]->getX());
            graph_hit->setCluster1Y(hits[i]->getY());
            graph_hit->setCluster1Z(hits[i]->getZ());
            graph_hit->setCluster1R(hits[i]->getR());
            graph_hit->setCluster1Phi(hits[i]->getGPhi());
            graph_hit->setCluster1Eta(graph_hit->getEta());
            graph_hit->setCluster2X(hits[i]->getX());
            graph_hit->setCluster2Y(hits[i]->getY());
            graph_hit->setCluster2Z(hits[i]->getZ());
            graph_hit->setCluster2R(hits[i]->getR());
            graph_hit->setCluster2Phi(hits[i]->getGPhi());
            graph_hit->setCluster2Eta(graph_hit->getEta());
            graph_hits.push_back(graph_hit);
        }
        // For strip, two clusters -> one spacepoint -> duplicate SP record in the hit
        // This means we are ignoring single clusters in the strip that are labeled as hits (need to discuss if this is bad or not)
        else if(i+1 < hits.size() && hits[i]->isStrip() && hits[i+1]->isStrip() && to_string(hits[i]->getHitType()) == "spacepoint" && to_string(hits[i+1]->getHitType()) == "spacepoint" && hits[i]->getX() == hits[i+1]->getX()) {
            std::shared_ptr<FPGATrackSimGNNHit> graph_hit = std::make_shared<FPGATrackSimGNNHit>();
            std::shared_ptr<const FPGATrackSimHit> cluster1_hit = std::make_shared<FPGATrackSimHit>(hits[i]->getOriginalHit());
            std::shared_ptr<const FPGATrackSimHit> cluster2_hit = std::make_shared<FPGATrackSimHit>(hits[i+1]->getOriginalHit());
            graph_hit->setHitID(i);
            graph_hit->setIdentifier(hits[i]->getIdentifier());
            graph_hit->setX(hits[i]->getX());
            graph_hit->setY(hits[i]->getY());
            graph_hit->setZ(hits[i]->getZ());
            graph_hit->setR(hits[i]->getR());
            graph_hit->setPhi(hits[i]->getGPhi());
            graph_hit->setEta(getEta(hits[i]));
            graph_hit->setCluster1X(cluster1_hit->getX());
            graph_hit->setCluster1Y(cluster1_hit->getY());
            graph_hit->setCluster1Z(cluster1_hit->getZ());
            graph_hit->setCluster1R(cluster1_hit->getR());
            graph_hit->setCluster1Phi(cluster1_hit->getGPhi());
            graph_hit->setCluster1Eta(getEta(cluster1_hit));
            graph_hit->setCluster2X(cluster2_hit->getX());
            graph_hit->setCluster2Y(cluster2_hit->getY());
            graph_hit->setCluster2Z(cluster2_hit->getZ());
            graph_hit->setCluster2R(cluster2_hit->getR());
            graph_hit->setCluster2Phi(cluster2_hit->getGPhi());
            graph_hit->setCluster2Eta(getEta(cluster2_hit));
            graph_hits.push_back(graph_hit);
        }
    }

    return StatusCode::SUCCESS;
}

float FPGATrackSimGNNGraphHitSelectorTool::getEta(const std::shared_ptr<const FPGATrackSimHit> & hit)
{
    float r3 = TMath::Sqrt(hit->getR()*hit->getR() + hit->getZ()*hit->getZ());
    float theta = 0.5 * TMath::ACos(hit->getZ() / r3);
    float eta = -1.0 * TMath::Log(TMath::Tan(theta));
    return eta;
}