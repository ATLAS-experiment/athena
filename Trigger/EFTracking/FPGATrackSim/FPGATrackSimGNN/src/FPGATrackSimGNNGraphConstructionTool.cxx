// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimGNNGraphConstructionTool.h"

#include <TFile.h>
#include <TTree.h>
#include "FourMomUtils/P4Helpers.h"

///////////////////////////////////////////////////////////////////////////////
// AthAlgTool

FPGATrackSimGNNGraphConstructionTool::FPGATrackSimGNNGraphConstructionTool(const std::string& algname, const std::string &name, const IInterface *ifc) 
    : AthAlgTool(algname, name, ifc) {}

StatusCode FPGATrackSimGNNGraphConstructionTool::initialize()
{
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());
    if(m_graphTool == "ModuleMap") {
        if (m_FPGATrackSimMapping->getGNNModuleMapString() != "") {
            m_moduleMapPath = m_FPGATrackSimMapping->getGNNModuleMapString();
        }
        else {
            ATH_MSG_ERROR("Path to 1st stage NN-based fake track removal ONNX file is empty! If you want to run this pipeline, you need to provide an input file.");
            return StatusCode::FAILURE;
        }

        if(m_moduleMapType == "doublet") {
            loadDoubletModuleMap(); // Load the doublet module map and store entry branches in vectors
        }
    }
    else if(m_graphTool == "MetricLearning") {
        ATH_CHECK( m_MLInferenceTool.retrieve() );
        m_MLInferenceTool->printModelInfo();
        assert(m_MLFeatureNamesVec.size() == m_MLFeatureScalesVec.size());
    }

    return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// Functions

StatusCode FPGATrackSimGNNGraphConstructionTool::getEdges(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits, std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges)
{
    if(m_graphTool == "ModuleMap") {
        doModuleMap(hits, edges);
    }
    else if(m_graphTool == "MetricLearning") {
        doMetricLearning(hits, edges);
    }

    return StatusCode::SUCCESS;
}

void FPGATrackSimGNNGraphConstructionTool::loadDoubletModuleMap()
{
    std::unique_ptr<TFile> file(TFile::Open(m_moduleMapPath.c_str()));
    std::unique_ptr<TTree> tree(static_cast<TTree*>(file->Get("TreeModuleDoublet")));

    unsigned int mid1_value = 0;
    unsigned int mid2_value = 0;
    float z0min_12_value = 0.0;
    float dphimin_12_value = 0.0;
    float phislopemin_12_value = 0.0;
    float detamin_12_value = 0.0;
    float z0max_12_value = 0.0;
    float dphimax_12_value = 0.0;
    float phislopemax_12_value = 0.0;
    float detamax_12_value = 0.0;

    tree->SetBranchAddress("Module1", &mid1_value);
    tree->SetBranchAddress("Module2", &mid2_value);
    tree->SetBranchAddress("z0min_12", &z0min_12_value);
    tree->SetBranchAddress("dphimin_12", &dphimin_12_value);
    tree->SetBranchAddress("phiSlopemin_12", &phislopemin_12_value);
    tree->SetBranchAddress("detamin_12", &detamin_12_value);
    tree->SetBranchAddress("z0max_12", &z0max_12_value);
    tree->SetBranchAddress("dphimax_12", &dphimax_12_value);
    tree->SetBranchAddress("phiSlopemax_12", &phislopemax_12_value);
    tree->SetBranchAddress("detamax_12", &detamax_12_value);

    int64_t nEntries = tree->GetEntries();
    for (int64_t i = 0; i < nEntries; ++i) {
        tree->GetEntry(i);
        m_mid1.emplace_back(mid1_value);
        m_mid2.emplace_back(mid2_value);
        m_z0min_12.emplace_back(z0min_12_value);
        m_dphimin_12.emplace_back(dphimin_12_value);
        m_phislopemin_12.emplace_back(phislopemin_12_value);
        m_detamin_12.emplace_back(detamin_12_value);
        m_z0max_12.emplace_back(z0max_12_value);
        m_dphimax_12.emplace_back(dphimax_12_value);
        m_phislopemax_12.emplace_back(phislopemax_12_value);
        m_detamax_12.emplace_back(detamax_12_value);
    }
}

void FPGATrackSimGNNGraphConstructionTool::doModuleMap(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits, std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges)
{
    // Use Module Map method for edge building
    // Two types of module maps: Doublet and Triplet
    // For each type of module map there is three functions: minmax, meanrms, and hybrid
    // Use the proper configuration set by the input script and passed as Gaudi::Property variables
    // Currently only Doublet Module Map with minmax cuts exist, but others can be implemented later on as desired

    if(m_moduleMapType == "doublet") {
        getDoubletEdges(hits, edges);
    }
}

void FPGATrackSimGNNGraphConstructionTool::getDoubletEdges(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits, std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges)
{
    // Take the list of hits and use the doublet module map to generate all the edges between hits that pass the doublet cuts

    for (size_t i = 0; i < m_mid2.size(); i++) {
        std::vector<std::shared_ptr<FPGATrackSimGNNHit>> hit1_matches;
        std::vector<std::shared_ptr<FPGATrackSimGNNHit>> hit2_matches;
    
        std::vector<int> hit1_indices;
        std::vector<int> hit2_indices;

        for (size_t j = 0; j < hits.size(); j++) {
            if (hits[j]->getIdentifier() == m_mid1[i]) {
                hit1_matches.emplace_back(hits[j]);
                hit1_indices.emplace_back(j);
            }
            if (hits[j]->getIdentifier() == m_mid2[i]) {
                hit2_matches.emplace_back(hits[j]);
                hit2_indices.emplace_back(j);
            }
        }

        for (size_t h1 = 0; h1 < hit1_matches.size(); h1++) {
            for (size_t h2 = 0; h2 < hit2_matches.size(); h2++) {
                applyDoubletCuts(hit1_matches[h1], hit2_matches[h2], edges, hit1_indices[h1], hit2_indices[h2], i);
            }
        }
    }
}

void FPGATrackSimGNNGraphConstructionTool::applyDoubletCuts(const std::shared_ptr<FPGATrackSimGNNHit> & hit1, const std::shared_ptr<FPGATrackSimGNNHit> & hit2, std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges, int hit1_index, int hit2_index, unsigned int modulemap_id)
{
    // Four types of doublet cuts (dEta, z0, dPhi, phiSlope)
    // If an edge passes all four, then it is a valid edge and can be stored

    // delta_eta cuts
    float deta = hit1->getEta() - hit2->getEta();
    if(!doMask(deta, m_detamin_12[modulemap_id], m_detamax_12[modulemap_id])) return;

    // z0 cuts
    float dz = hit2->getZ() - hit1->getZ();
    float dr = hit2->getR() - hit1->getR();
    float z0 = dr==0. ? 0. : hit1->getZ() - (hit1->getR() * dz / dr);
    if(!doMask(z0, m_z0min_12[modulemap_id], m_z0max_12[modulemap_id])) return;

    // delta_phi cuts
    float dphi = P4Helpers::deltaPhi(hit2->getPhi(),hit1->getPhi());
    if(!doMask(dphi, m_dphimin_12[modulemap_id], m_dphimax_12[modulemap_id])) return;

    // phislope cuts
    float phislope = dr==0. ? 0. : dphi / dr;
    if(!doMask(phislope, m_phislopemin_12[modulemap_id], m_phislopemax_12[modulemap_id])) return;

    // if pass all doublet cuts, then record the edge information
    std::shared_ptr<FPGATrackSimGNNEdge> edge = std::make_shared<FPGATrackSimGNNEdge>();
    edge->setEdgeIndex1(hit1_index);
    edge->setEdgeIndex2(hit2_index);
    edges.emplace_back(edge);
}

bool FPGATrackSimGNNGraphConstructionTool::doMask(float val, float min, float max)
{
    bool mask = false;
    if(m_moduleMapFunc == "minmax") {
        mask = doMinMaxMask(val, min, max);
    }

    return mask;
}

bool FPGATrackSimGNNGraphConstructionTool::doMinMaxMask(float val, float min, float max)
{
    bool mask = false;

    if((val <= max * (1.0 + featureSign(max) * m_moduleMapTol)) &&
       (val >= min * (1.0 - featureSign(min) * m_moduleMapTol))) {
        mask = true;
    }

    return mask;
}

float FPGATrackSimGNNGraphConstructionTool::featureSign(float feature)
{
    if(feature < 0.0) { return -1.0; }
    else if(feature > 0.0) { return 1.0; }
    else { return 0.0; }
}

void FPGATrackSimGNNGraphConstructionTool::doMetricLearning(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits, std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges)
{
    // Use Metric Learning for edge construction 
    // Clustering properties can be set in the input scripta as Gaudi::Property variables
    std::vector<float> gNodeFeatures = getNodeFeatures(hits);
    std::vector<float> gEmbedded = embed(hits);
    doClustering(hits, edges, gEmbedded);
}

std::vector<float> FPGATrackSimGNNGraphConstructionTool::getNodeFeatures(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits)
{
    std::vector<float> gNodeFeatures;
    
    for(auto hit : hits) {
        std::map<std::string, float> features;
        features["r"] = hit->getR();
        features["phi"] = hit->getPhi();
        features["z"] = hit->getZ();

        for(size_t i = 0; i < m_MLFeatureNamesVec.size(); i++){
            gNodeFeatures.push_back(
            features[m_MLFeatureNamesVec[i]] / m_MLFeatureScalesVec[i]);
        }
    }
    return gNodeFeatures;
}

std::vector<float> FPGATrackSimGNNGraphConstructionTool::embed(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits)
{
    // Use the ML network to embed the hits in a 12-dim latent space
    std::vector<float> gNodeFeatures = getNodeFeatures(hits);
    std::vector<float> gEmbedded;

    std::vector<Ort::Value> gInputTensor;
    StatusCode s = m_MLInferenceTool->addInput(gInputTensor, gNodeFeatures, 0, hits.size());
    std::vector<Ort::Value> gOutputTensor;
    s = m_MLInferenceTool->addOutput(gOutputTensor, gEmbedded, 0, hits.size());
    s = m_MLInferenceTool->inference(gInputTensor, gOutputTensor);

    return gEmbedded;
}

void FPGATrackSimGNNGraphConstructionTool::doClustering(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits, std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges, 
                                                        std::vector<float> & gEmbedded)
{
    // Create graph edges based on the hits distance in the latent space
    // Creates a directed graph
    int n_dim = 12;
    int size = hits.size();
    float r_squared = m_metricLearningR*m_metricLearningR; 
    int index1 = 0;
    int index2 = 0;
    int count = 0;
    float distance = 0.;
    std::vector<float> start(n_dim); 

    // Loop over all hits
    for(int k = 0; k < size; ++k){
        count = 0;
        // Setup current hit
        for(int j = 0; j < n_dim; ++j){
            start[j] = gEmbedded[k*n_dim + j];
        }
        // Loop over the hits not yet checked 
        for (int i = k + 1; i < size; ++i){
            distance = 0.;
            for(int d = 0; d < n_dim; ++d){
                distance += (start[d] - gEmbedded[i*n_dim + d]) * (start[d] - gEmbedded[i*n_dim + d]);
            }
            // Store edge if the distance between the hits meets is below the limit
            if(distance < r_squared){
                std::shared_ptr<FPGATrackSimGNNEdge> edge = std::make_shared<FPGATrackSimGNNEdge>();
                // Set order of edge indices to make a directed graph
                float d_i_sq = (hits[i]->getR() * hits[i]->getR()) + (hits[i]->getZ() * hits[i]->getZ());
                float d_k_sq = (hits[k]->getR() * hits[k]->getR()) + (hits[k]->getZ() * hits[k]->getZ());
                if (d_i_sq < d_k_sq){ 
                    index1 = i;
                    index2 = k;
                } else {
                    index1 = k;
                    index2 = i;
                }
                
                edge->setEdgeIndex1(index1);
                edge->setEdgeIndex2(index2);
                edges.emplace_back(edge);
                ++count;
            }
            // Upper limit for connections of the same hit
            if(count > m_metricLearningMaxN){
                break;
            }
        }

    }
}