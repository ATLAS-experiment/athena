// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimGNNEdgeClassifierTool.h"

#include "FourMomUtils/P4Helpers.h"
#include <cmath>

///////////////////////////////////////////////////////////////////////////////
// AthAlgTool

FPGATrackSimGNNEdgeClassifierTool::FPGATrackSimGNNEdgeClassifierTool(const std::string& algname, const std::string &name, const IInterface *ifc) 
    : AthAlgTool(algname, name, ifc) {}

StatusCode FPGATrackSimGNNEdgeClassifierTool::initialize()
{
    ATH_CHECK( m_GNNInferenceTool.retrieve() );
    m_GNNInferenceTool->printModelInfo();
    assert(m_gnnFeatureNamesVec.size() == m_gnnFeatureScalesVec.size());
    assert(m_gnnFeatureNamesVec_pixelOnly.size() == m_gnnFeatureScalesVec_pixelOnly.size());
    
    return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// Functions

StatusCode FPGATrackSimGNNEdgeClassifierTool::scoreEdges(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits, std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges)
{
    std::vector<float> edge_scores;

    std::vector<float> gNodeFeatures = getNodeFeatures(hits);
    std::vector<int64_t> edgeList = getEdgeList(edges);
    std::vector<float> gEdgeFeatures = getEdgeFeatures(edges, gNodeFeatures);
    
    std::vector<Ort::Value> gInputTensor;
    ATH_CHECK( m_GNNInferenceTool->addInput(gInputTensor, gNodeFeatures, 0, hits.size()) );
    ATH_CHECK( m_GNNInferenceTool->addInput(gInputTensor, edgeList, 1, edges.size()) );
    ATH_CHECK( m_GNNInferenceTool->addInput(gInputTensor, gEdgeFeatures, 2, edges.size()) );

    std::vector<float> gOutputData;
    std::vector<Ort::Value> gOutputTensor;
    ATH_CHECK( m_GNNInferenceTool->addOutput(gOutputTensor, edge_scores, 0, edges.size()) );

    ATH_CHECK( m_GNNInferenceTool->inference(gInputTensor, gOutputTensor) );
    // apply sigmoid to the gnn output data
    for(auto& v : edge_scores) {
        v = 1.f / (1.f + std::exp(-v));
    };
    
    for (size_t i = 0; i < edges.size(); i++) {
        edges[i]->setEdgeScore(edge_scores[i]);
    }

    return StatusCode::SUCCESS;
}

std::vector<float> FPGATrackSimGNNEdgeClassifierTool::getNodeFeatures(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits)
{
    std::vector<float> gNodeFeatures;

    // For GNN model, we use phi-folding to [2pi/16,3pi/16], so all hit.phi needs to be rotated to the right region
    const float phi_binSize = M_PI / 16.0;
    int phiBin = m_regionNum & 0x1f;
    float regionMin = phi_binSize * phiBin;
    float referenceMin = 2.0 * M_PI / 16.0;
    float deltaPhi = referenceMin - regionMin;

    // For GNN model, I am going to assume eta symmetry for testing purposes, so negative eta should be flipped to positive eta
    int etaSide = (m_regionNum >> 5) & 0x1; // 1 is positive side, 0 negative side
    bool flipEta = (etaSide == 0);
    
    for(const auto& hit : hits) {
        std::map<std::string, float> features;
        features["r"] = hit->getR();
        features["phi"] = std::remainder(hit->getPhi() + deltaPhi, 2*M_PI); // Add the deltaPhi to shift the phi to the trained-region
        features["z"] = flipEta ? -hit->getZ() : hit->getZ();
        features["eta"] = flipEta ? -hit->getEta() : hit->getEta();
        if (m_doGNNPixelSeeding) { // Do not use cluster features for pixelOnly
            for(size_t i = 0; i < m_gnnFeatureNamesVec_pixelOnly.size(); i++){
                gNodeFeatures.push_back(
                features[m_gnnFeatureNamesVec_pixelOnly[i]] / m_gnnFeatureScalesVec_pixelOnly[i]);
            }
        }
        else { // Use cluster features and the standard feature vectors and scales
            features["cluster_r_1"] = hit->getCluster1R();
            features["cluster_phi_1"] = std::remainder(hit->getCluster1Phi() + deltaPhi, 2*M_PI);
            features["cluster_z_1"]   = flipEta ? -hit->getCluster1Z()   : hit->getCluster1Z();
            features["cluster_eta_1"] = flipEta ? -hit->getCluster1Eta() : hit->getCluster1Eta();
            features["cluster_r_2"] = hit->getCluster2R();
            features["cluster_phi_2"] = std::remainder(hit->getCluster2Phi() + deltaPhi, 2*M_PI);
            features["cluster_z_2"]   = flipEta ? -hit->getCluster2Z()   : hit->getCluster2Z();
            features["cluster_eta_2"] = flipEta ? -hit->getCluster2Eta() : hit->getCluster2Eta();

            for(size_t i = 0; i < m_gnnFeatureNamesVec.size(); i++){
                gNodeFeatures.push_back(
                features[m_gnnFeatureNamesVec[i]] / m_gnnFeatureScalesVec[i]);
            }
        }
    }
     
    return gNodeFeatures;
}

std::vector<int64_t> FPGATrackSimGNNEdgeClassifierTool::getEdgeList(const std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges)
{
    std::vector<int64_t> rowIndices;
    std::vector<int64_t> colIndices;
    std::vector<int64_t> edgesList(edges.size() * 2);

    for(const auto& edge : edges) {
        rowIndices.push_back(edge->getEdgeIndex1());
        colIndices.push_back(edge->getEdgeIndex2());    
    }

    std::copy(rowIndices.begin(), rowIndices.end(), edgesList.begin());
    std::copy(colIndices.begin(), colIndices.end(), edgesList.begin() + edges.size());

    return edgesList;
}

std::vector<float> FPGATrackSimGNNEdgeClassifierTool::getEdgeFeatures(std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges, const std::vector<float> & gNodeFeatures)
{
    std::vector<float> gEdgeFeatures;

    for (auto& edge : edges) {
        computeEdgeFeatures(edge, edge->getEdgeIndex1(), edge->getEdgeIndex2(), gNodeFeatures);

        gEdgeFeatures.push_back(edge->getEdgeDR());
        gEdgeFeatures.push_back(edge->getEdgeDPhi());
        gEdgeFeatures.push_back(edge->getEdgeDZ());
        gEdgeFeatures.push_back(edge->getEdgeDEta());
        gEdgeFeatures.push_back(edge->getEdgePhiSlope());
        gEdgeFeatures.push_back(edge->getEdgeRPhiSlope());
    }

    return gEdgeFeatures;
}

void FPGATrackSimGNNEdgeClassifierTool::computeEdgeFeatures(std::shared_ptr<FPGATrackSimGNNEdge>& edge,const int& hit1_index, const int& hit2_index,const std::vector<float>& gNodeFeatures)
{
    size_t num_features = m_doGNNPixelSeeding ? m_gnnFeatureNamesVec_pixelOnly.size() : m_gnnFeatureNamesVec.size();

    std::map<std::string, float> hit1_features;
    std::map<std::string, float> hit2_features;

    if (m_doGNNPixelSeeding) {
        // Fill only pixel-only features
        for (size_t i = 0; i < m_gnnFeatureNamesVec_pixelOnly.size(); i++) {
            hit1_features[m_gnnFeatureNamesVec_pixelOnly[i]] = gNodeFeatures[hit1_index * num_features + i];
            hit2_features[m_gnnFeatureNamesVec_pixelOnly[i]] = gNodeFeatures[hit2_index * num_features + i];
        }
    } else {
        // Fill full feature set (including clusters)
        for (size_t i = 0; i < m_gnnFeatureNamesVec.size(); i++) {
            hit1_features[m_gnnFeatureNamesVec[i]] = gNodeFeatures[hit1_index * num_features + i];
            hit2_features[m_gnnFeatureNamesVec[i]] = gNodeFeatures[hit2_index * num_features + i];
        }
    }

    // Now compute differences using only the common "physics" features
    float deta = hit2_features["eta"] - hit1_features["eta"];
    float dz = hit2_features["z"] - hit1_features["z"];
    float dr = hit2_features["r"] - hit1_features["r"];
    float dphi = P4Helpers::deltaPhi(hit2_features["phi"], hit1_features["phi"]);
    float phislope = dr == 0. ? 0. : dphi / dr;
    float rphislope = 0.5 * (hit2_features["r"] + hit1_features["r"]) * phislope;

    edge->setEdgeDR(dr);
    edge->setEdgeDPhi(dphi);
    edge->setEdgeDZ(dz);
    edge->setEdgeDEta(deta);
    edge->setEdgePhiSlope(phislope);
    edge->setEdgeRPhiSlope(rphislope);
}
