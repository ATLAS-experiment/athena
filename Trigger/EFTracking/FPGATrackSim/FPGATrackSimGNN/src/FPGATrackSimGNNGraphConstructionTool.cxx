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
        else if(m_moduleMapType == "triplet") {
            loadTripletModuleMap(); // Load the triplet module map and store entry branches in vectors
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

    unsigned mid1_value = 0;
    unsigned mid2_value = 0;
    float z0min_12_value = 0.0;
    float dphimin_12_value = 0.0;
    float phiSlopemin_12_value = 0.0;
    float detamin_12_value = 0.0;
    float z0max_12_value = 0.0;
    float dphimax_12_value = 0.0;
    float phiSlopemax_12_value = 0.0;
    float detamax_12_value = 0.0;

    tree->SetBranchAddress("Module1", &mid1_value);
    tree->SetBranchAddress("Module2", &mid2_value);
    tree->SetBranchAddress("z0min_12", &z0min_12_value);
    tree->SetBranchAddress("dphimin_12", &dphimin_12_value);
    tree->SetBranchAddress("phiSlopemin_12", &phiSlopemin_12_value);
    tree->SetBranchAddress("detamin_12", &detamin_12_value);
    tree->SetBranchAddress("z0max_12", &z0max_12_value);
    tree->SetBranchAddress("dphimax_12", &dphimax_12_value);
    tree->SetBranchAddress("phiSlopemax_12", &phiSlopemax_12_value);
    tree->SetBranchAddress("detamax_12", &detamax_12_value);

    int64_t nEntries = tree->GetEntries();
    m_cfgs.reserve(nEntries);
    for (int64_t i = 0; i < nEntries; ++i) {
        tree->GetEntry(i);

        ModuleMapConfig cfg;

        cfg.mid1 = mid1_value;
        cfg.mid2 = mid2_value;

        cfg.doubletCuts[0].z0.min = z0min_12_value;
        cfg.doubletCuts[0].z0.max = z0max_12_value;
        cfg.doubletCuts[0].dphi.min = dphimin_12_value;
        cfg.doubletCuts[0].dphi.max = dphimax_12_value;
        cfg.doubletCuts[0].deta.min = detamin_12_value;
        cfg.doubletCuts[0].deta.max = detamax_12_value;
        cfg.doubletCuts[0].phiSlope.min = phiSlopemin_12_value;
        cfg.doubletCuts[0].phiSlope.max = phiSlopemax_12_value;

        m_cfgs.emplace_back(std::move(cfg));
    }
}

void FPGATrackSimGNNGraphConstructionTool::loadTripletModuleMap()
{
    std::unique_ptr<TFile> file(TFile::Open(m_moduleMapPath.c_str()));
    std::unique_ptr<TTree> tree(static_cast<TTree*>(file->Get("TreeModuleTriplet")));

    unsigned mid1_value = 0;
    unsigned mid2_value = 0;
    unsigned mid3_value = 0;
    unsigned occurence_value = 0;

    float z0min_12_value = 0.0;
    float z0max_12_value = 0.0;
    float z0sum_12_value = 0.0;
    float z0sumSq_12_value = 0.0;
    float z0mean_12_value = 0.0;
    float z0rms_12_value = 0.0;
    float z0min_23_value = 0.0;
    float z0max_23_value = 0.0;
    float z0sum_23_value = 0.0;
    float z0sumSq_23_value = 0.0;
    float z0mean_23_value = 0.0;
    float z0rms_23_value = 0.0;

    float dphimin_12_value = 0.0;
    float dphimax_12_value = 0.0;
    float dphisum_12_value = 0.0;
    float dphisumSq_12_value = 0.0;
    float dphimean_12_value = 0.0;
    float dphirms_12_value = 0.0;
    float dphimin_23_value = 0.0;
    float dphimax_23_value = 0.0;
    float dphisum_23_value = 0.0;
    float dphisumSq_23_value = 0.0;
    float dphimean_23_value = 0.0;
    float dphirms_23_value = 0.0;

    float phiSlopemin_12_value = 0.0;
    float phiSlopemax_12_value = 0.0;
    float phiSlopesum_12_value = 0.0;
    float phiSlopesumSq_12_value = 0.0;
    float phiSlopemean_12_value = 0.0;
    float phiSloperms_12_value = 0.0;
    float phiSlopemin_23_value = 0.0;
    float phiSlopemax_23_value = 0.0;
    float phiSlopesum_23_value = 0.0;
    float phiSlopesumSq_23_value = 0.0;
    float phiSlopemean_23_value = 0.0;
    float phiSloperms_23_value = 0.0;

    float detamin_12_value = 0.0;
    float detamax_12_value = 0.0;
    float detasum_12_value = 0.0;
    float detasumSq_12_value = 0.0;
    float detamean_12_value = 0.0;
    float detarms_12_value = 0.0;
    float detamin_23_value = 0.0;
    float detamax_23_value = 0.0;
    float detasum_23_value = 0.0;
    float detasumSq_23_value = 0.0;
    float detamean_23_value = 0.0;
    float detarms_23_value = 0.0;

    float diff_dydx_min_value = 0.0;
    float diff_dydx_max_value = 0.0;
    float diff_dydx_sum_value = 0.0;
    float diff_dydx_sumSq_value = 0.0;
    float diff_dydx_mean_value = 0.0;
    float diff_dydx_rms_value = 0.0;

    float diff_dzdr_min_value = 0.0;
    float diff_dzdr_max_value = 0.0;
    float diff_dzdr_sum_value = 0.0;
    float diff_dzdr_sumSq_value = 0.0;
    float diff_dzdr_mean_value = 0.0;
    float diff_dzdr_rms_value = 0.0;
    
    tree->SetBranchAddress("Module1", &mid1_value);
    tree->SetBranchAddress("Module2", &mid2_value);
    tree->SetBranchAddress("Module3", &mid3_value);
    tree->SetBranchAddress("Occurence", &occurence_value);

    tree->SetBranchAddress("z0min_12", &z0min_12_value);
    tree->SetBranchAddress("z0max_12", &z0max_12_value);
    tree->SetBranchAddress("z0sum_12", &z0sum_12_value);
    tree->SetBranchAddress("z0sumSq_12", &z0sumSq_12_value);
    tree->SetBranchAddress("z0_12_mean", &z0mean_12_value);
    tree->SetBranchAddress("z0_12_rms", &z0rms_12_value);
    tree->SetBranchAddress("z0min_23", &z0min_23_value);
    tree->SetBranchAddress("z0max_23", &z0max_23_value);
    tree->SetBranchAddress("z0sum_23", &z0sum_23_value);
    tree->SetBranchAddress("z0sumSq_23", &z0sumSq_23_value);
    tree->SetBranchAddress("z0_23_mean", &z0mean_23_value);
    tree->SetBranchAddress("z0_23_rms", &z0rms_23_value);

    tree->SetBranchAddress("dphimin_12", &dphimin_12_value);
    tree->SetBranchAddress("dphimax_12", &dphimax_12_value);
    tree->SetBranchAddress("dphisum_12", &dphisum_12_value);
    tree->SetBranchAddress("dphisumSq_12", &dphisumSq_12_value);
    tree->SetBranchAddress("dphi_12_mean", &dphimean_12_value);
    tree->SetBranchAddress("dphi_12_rms", &dphirms_12_value);
    tree->SetBranchAddress("dphimin_23", &dphimin_23_value);
    tree->SetBranchAddress("dphimax_23", &dphimax_23_value);
    tree->SetBranchAddress("dphisum_23", &dphisum_23_value);
    tree->SetBranchAddress("dphisumSq_23", &dphisumSq_23_value);
    tree->SetBranchAddress("dphi_23_mean", &dphimean_23_value);
    tree->SetBranchAddress("dphi_23_rms", &dphirms_23_value);

    tree->SetBranchAddress("phiSlopemin_12", &phiSlopemin_12_value);
    tree->SetBranchAddress("phiSlopemax_12", &phiSlopemax_12_value);
    tree->SetBranchAddress("phiSlopesum_12", &phiSlopesum_12_value);
    tree->SetBranchAddress("phiSlopesumSq_12", &phiSlopesumSq_12_value);
    tree->SetBranchAddress("phiSlope_12_mean", &phiSlopemean_12_value);
    tree->SetBranchAddress("phiSlope_12_rms", &phiSloperms_12_value);
    tree->SetBranchAddress("phiSlopemin_23", &phiSlopemin_23_value);
    tree->SetBranchAddress("phiSlopemax_23", &phiSlopemax_23_value);
    tree->SetBranchAddress("phiSlopesum_23", &phiSlopesum_23_value);
    tree->SetBranchAddress("phiSlopesumSq_23", &phiSlopesumSq_23_value);
    tree->SetBranchAddress("phiSlope_23_mean", &phiSlopemean_23_value);
    tree->SetBranchAddress("phiSlope_23_rms", &phiSloperms_23_value);

    tree->SetBranchAddress("detamin_12", &detamin_12_value);
    tree->SetBranchAddress("detamax_12", &detamax_12_value);
    tree->SetBranchAddress("detasum_12", &detasum_12_value);
    tree->SetBranchAddress("detasumSq_12", &detasumSq_12_value);
    tree->SetBranchAddress("deta_12_mean", &detamean_12_value);
    tree->SetBranchAddress("deta_12_rms", &detarms_12_value);
    tree->SetBranchAddress("detamin_23", &detamin_23_value);
    tree->SetBranchAddress("detamax_23", &detamax_23_value);
    tree->SetBranchAddress("detasum_23", &detasum_23_value);
    tree->SetBranchAddress("detasumSq_23", &detasumSq_23_value);
    tree->SetBranchAddress("deta_23_mean", &detamean_23_value);
    tree->SetBranchAddress("deta_23_rms", &detarms_23_value);
    
    tree->SetBranchAddress("diff_dzdr_min", &diff_dzdr_min_value);
    tree->SetBranchAddress("diff_dzdr_max", &diff_dzdr_max_value);
    tree->SetBranchAddress("diff_dzdr_sum", &diff_dzdr_sum_value);
    tree->SetBranchAddress("diff_dzdr_sumSq", &diff_dzdr_sumSq_value);
    tree->SetBranchAddress("diff_dzdr_mean", &diff_dzdr_mean_value);
    tree->SetBranchAddress("diff_dzdr_rms", &diff_dzdr_rms_value);

    tree->SetBranchAddress("diff_dydx_min", &diff_dydx_min_value);
    tree->SetBranchAddress("diff_dydx_max", &diff_dydx_max_value);
    tree->SetBranchAddress("diff_dydx_sum", &diff_dydx_sum_value);
    tree->SetBranchAddress("diff_dydx_sumSq", &diff_dydx_sumSq_value);
    tree->SetBranchAddress("diff_dydx_mean", &diff_dydx_mean_value);
    tree->SetBranchAddress("diff_dydx_rms", &diff_dydx_rms_value);

    int64_t nEntries = tree->GetEntries();
    m_cfgs.reserve(nEntries);
    for (int64_t i = 0; i < nEntries; ++i) {
        tree->GetEntry(i);

        ModuleMapConfig cfg;

        cfg.mid1 = mid1_value;
        cfg.mid2 = mid2_value;
        cfg.mid3 = mid3_value;
        cfg.occurence = occurence_value;
        
        cfg.doubletCuts[0].z0.min = z0min_12_value;
        cfg.doubletCuts[0].z0.max = z0max_12_value;
        cfg.doubletCuts[0].z0.sum = z0sum_12_value;
        cfg.doubletCuts[0].z0.sumSq = z0sumSq_12_value;
        cfg.doubletCuts[0].z0.mean = z0mean_12_value;
        cfg.doubletCuts[0].z0.rms = z0rms_12_value;

        cfg.doubletCuts[1].z0.min = z0min_23_value;
        cfg.doubletCuts[1].z0.max = z0max_23_value;
        cfg.doubletCuts[1].z0.sum = z0sum_23_value;
        cfg.doubletCuts[1].z0.sumSq = z0sumSq_23_value;
        cfg.doubletCuts[1].z0.mean = z0mean_23_value;
        cfg.doubletCuts[1].z0.rms = z0rms_23_value;

        // dphi
        cfg.doubletCuts[0].dphi.min = dphimin_12_value;
        cfg.doubletCuts[0].dphi.max = dphimax_12_value;
        cfg.doubletCuts[0].dphi.sum = dphisum_12_value;
        cfg.doubletCuts[0].dphi.sumSq = dphisumSq_12_value;
        cfg.doubletCuts[0].dphi.mean = dphimean_12_value;
        cfg.doubletCuts[0].dphi.rms = dphirms_12_value;

        cfg.doubletCuts[1].dphi.min = dphimin_23_value;
        cfg.doubletCuts[1].dphi.max = dphimax_23_value;
        cfg.doubletCuts[1].dphi.sum = dphisum_23_value;
        cfg.doubletCuts[1].dphi.sumSq = dphisumSq_23_value;
        cfg.doubletCuts[1].dphi.mean = dphimean_23_value;
        cfg.doubletCuts[1].dphi.rms = dphirms_23_value;

        // eta
        cfg.doubletCuts[0].deta.min = detamin_12_value;
        cfg.doubletCuts[0].deta.max = detamax_12_value;
        cfg.doubletCuts[0].deta.sum = detasum_12_value;
        cfg.doubletCuts[0].deta.sumSq = detasumSq_12_value;
        cfg.doubletCuts[0].deta.mean = detamean_12_value;
        cfg.doubletCuts[0].deta.rms = detarms_12_value;

        cfg.doubletCuts[1].deta.min = detamin_23_value;
        cfg.doubletCuts[1].deta.max = detamax_23_value;
        cfg.doubletCuts[1].deta.sum = detasum_23_value;
        cfg.doubletCuts[1].deta.sumSq = detasumSq_23_value;
        cfg.doubletCuts[1].deta.mean = detamean_23_value;
        cfg.doubletCuts[1].deta.rms = detarms_23_value;

        // slope
        cfg.doubletCuts[0].phiSlope.min = phiSlopemin_12_value;
        cfg.doubletCuts[0].phiSlope.max = phiSlopemax_12_value;
        cfg.doubletCuts[0].phiSlope.sum = phiSlopesum_12_value;
        cfg.doubletCuts[0].phiSlope.sumSq = phiSlopesumSq_12_value;
        cfg.doubletCuts[0].phiSlope.mean = phiSlopemean_12_value;
        cfg.doubletCuts[0].phiSlope.rms = phiSloperms_12_value;

        cfg.doubletCuts[1].phiSlope.min = phiSlopemin_23_value;
        cfg.doubletCuts[1].phiSlope.max = phiSlopemax_23_value;
        cfg.doubletCuts[1].phiSlope.sum = phiSlopesum_23_value;
        cfg.doubletCuts[1].phiSlope.sumSq = phiSlopesumSq_23_value;
        cfg.doubletCuts[1].phiSlope.mean = phiSlopemean_23_value;
        cfg.doubletCuts[1].phiSlope.rms = phiSloperms_23_value;

        // extras
        cfg.tripletCuts.diff_dzdr.min = diff_dzdr_min_value;
        cfg.tripletCuts.diff_dzdr.max = diff_dzdr_max_value;
        cfg.tripletCuts.diff_dzdr.sum = diff_dzdr_sum_value;
        cfg.tripletCuts.diff_dzdr.sumSq = diff_dzdr_sumSq_value;
        cfg.tripletCuts.diff_dzdr.mean = diff_dzdr_mean_value;
        cfg.tripletCuts.diff_dzdr.rms = diff_dzdr_rms_value;

        cfg.tripletCuts.diff_dydx.min = diff_dydx_min_value;
        cfg.tripletCuts.diff_dydx.max = diff_dydx_max_value;
        cfg.tripletCuts.diff_dydx.sum = diff_dydx_sum_value;
        cfg.tripletCuts.diff_dydx.sumSq = diff_dydx_sumSq_value;
        cfg.tripletCuts.diff_dydx.mean = diff_dydx_mean_value;
        cfg.tripletCuts.diff_dydx.rms = diff_dydx_rms_value;

        m_cfgs.emplace_back(std::move(cfg));
        m_tripletMap.emplace(TripletKey{cfg.mid1, cfg.mid2, cfg.mid3}, &m_cfgs.back());
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
        getDoubletEdges(hits, edges, 0);
    }
    else if(m_moduleMapType == "triplet") {
        getTripletEdges(hits, edges);
    }
}

void FPGATrackSimGNNGraphConstructionTool::getTripletEdges(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits, std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges)
{
    std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> edges_12;
    std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> edges_23;

    getDoubletEdges(hits, edges_12, 0); // Build doublet edges between Module1 and Module2
    getDoubletEdges(hits, edges_23, 1); // Build doublet edges between Module2 and Module3

    std::unordered_map<int, std::vector<const FPGATrackSimGNNEdge*>> edge23_by_hit1;
    for (const auto& e : edges_23) edge23_by_hit1[e->getEdgeIndex1()].push_back(e.get());

    std::unordered_set<uint64_t> seen;
    auto pack = [](int a, int b) -> uint64_t { return (uint64_t(a) << 32) | uint32_t(b); };
    
    for (const auto& edge_12 : edges_12) {
        int hit1 = edge_12->getEdgeIndex1();
        int hit2 = edge_12->getEdgeIndex2();
        const auto& h1 = hits[hit1];
        const auto& h2 = hits[hit2];
        unsigned mid1 = h1->getIdentifierHash();
        unsigned mid2 = h2->getIdentifierHash();

        auto it = edge23_by_hit1.find(hit2);
        if (it == edge23_by_hit1.end()) continue;

        for (const auto* edge_23 : it->second) { // Loop over all edges_23 that have a hit1 that matches to hit2 from edges_12
            int hit3 = edge_23->getEdgeIndex2();
            const auto& h3 = hits[hit3];
            unsigned mid3 = h3->getIdentifierHash();

            TripletKey key{mid1, mid2, mid3};
            auto cfg_it = m_tripletMap.find(key);
            if (cfg_it == m_tripletMap.end()) continue;

            // Check doublet cuts with the cfg SPECIFIC to this triplet
            // for BOTH legs — not the pre-built edge lists
            const auto& cfg = *cfg_it->second;
            if (!applyDoubletCuts(h1, h2, cfg.doubletCuts[0])) continue;
            if (!applyDoubletCuts(h2, h3, cfg.doubletCuts[1])) continue;
            if (!applyTripletCuts(h1, h2, h3, cfg.tripletCuts)) continue;

            seen.insert(pack(hit1, hit2));
            seen.insert(pack(hit2, hit3));
        }
    }
    edges.clear();
    edges.reserve(seen.size());

    for (uint64_t packed : seen) {
        int index1 = static_cast<int>(packed >> 32);
        int index2 = static_cast<int>(packed & 0xFFFFFFFF);
        auto edge = std::make_shared<FPGATrackSimGNNEdge>();
        edge->setEdgeIndex1(index1);
        edge->setEdgeIndex2(index2);
        edges.emplace_back(std::move(edge));
    }
}

void FPGATrackSimGNNGraphConstructionTool::getDoubletEdges(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits, std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges, int cutIndex)
{
    // Take the list of hits and use the doublet module map to generate all the edges between hits that pass the doublet cuts

    std::unordered_map<unsigned, std::vector<int>> hits_by_module;

    for (size_t i = 0; i < hits.size(); i++) {
        hits_by_module[hits[i]->getIdentifierHash()].push_back(i);
    }

    for (const auto& cfg : m_cfgs) {
        unsigned midA, midB;
        const ModuleMapConfig::DoubletCuts& cuts = cfg.doubletCuts[cutIndex];
        
        if (cutIndex == 0) {
            midA = cfg.mid1;
            midB = cfg.mid2;
        }
        else if (cutIndex == 1) {
            midA = cfg.mid2;
            midB = cfg.mid3;
        }

        const auto& hit1_indices = hits_by_module[midA];
        const auto& hit2_indices = hits_by_module[midB];

        for (size_t h1 = 0; h1 < hit1_indices.size(); h1++) {
            for (size_t h2 = 0; h2 < hit2_indices.size(); h2++) {
                int i1 = hit1_indices[h1];
                int i2 = hit2_indices[h2];
                const auto& hit1 = hits[i1];
                const auto& hit2 = hits[i2];
                if (!applyDoubletCuts(hit1, hit2, cuts)) continue;
                auto edge = std::make_shared<FPGATrackSimGNNEdge>();
                edge->setEdgeIndex1(i1);
                edge->setEdgeIndex2(i2);
                edges.emplace_back(std::move(edge));
            }
        }
    }
}

bool FPGATrackSimGNNGraphConstructionTool::applyDoubletCuts(const std::shared_ptr<FPGATrackSimGNNHit> & hit1, const std::shared_ptr<FPGATrackSimGNNHit> & hit2, const ModuleMapConfig::DoubletCuts& cuts)
{
    // Four types of doublet cuts (dEta, z0, dPhi, phiSlope)
    // If an edge passes all four, then it is a valid edge and can be stored

    // delta_eta cuts
    float deta = hit2->getEta() - hit1->getEta();
    if(!doMask(deta, cuts.deta)) return false;

    // z0 cuts
    float dz = hit2->getZ() - hit1->getZ();
    float dr = hit2->getR() - hit1->getR();
    float z0 = dr==0. ? 0. : hit1->getZ() - (hit1->getR() * dz / dr);
    if(!doMask(z0, cuts.z0)) return false;

    // delta_phi cuts
    float dphi = P4Helpers::deltaPhi(hit2->getPhi(),hit1->getPhi()); // Look into this issue
    if(!doMask(dphi, cuts.dphi)) return false;

    // phislope cuts
    float phiSlope = dr==0. ? 0. : dphi / dr;
    if(!doMask(phiSlope, cuts.phiSlope)) return false;    

    return true;
}

bool FPGATrackSimGNNGraphConstructionTool::applyTripletCuts(const std::shared_ptr<FPGATrackSimGNNHit> & hit1, const std::shared_ptr<FPGATrackSimGNNHit> & hit2, const std::shared_ptr<FPGATrackSimGNNHit> & hit3, const ModuleMapConfig::TripletCuts& cuts)
{
    auto safeDiv = [&](float dA, float dB) { return (dB == 0.) ? 0.0f : (dA / dB); };

    // Diff dydx 
    float dy_12 = hit2->getY() - hit1->getY();
    float dy_23 = hit3->getY() - hit2->getY();
    float dx_12 = hit2->getX() - hit1->getX();
    float dx_23 = hit3->getX() - hit2->getX();

    float diff_dydx = safeDiv(dy_12, dx_12) - safeDiv(dy_23, dx_23);
    if(!doMask(diff_dydx, cuts.diff_dydx)) return false; // Fails the dydx cut

    // Diff dzdr
    float dz_12 = hit2->getZ() - hit1->getZ();
    float dz_23 = hit3->getZ() - hit2->getZ();
    float dr_12 = hit2->getR() - hit1->getR();
    float dr_23 = hit3->getR() - hit2->getR();

    float diff_dzdr = safeDiv(dz_12, dr_12) - safeDiv(dz_23, dr_23);
    if(!doMask(diff_dzdr, cuts.diff_dzdr)) return false; // Fails the drdz cut

    return true; // Passes both triplet cuts
}

bool FPGATrackSimGNNGraphConstructionTool::doMask(float val, const ModuleMapConfig::FeatureCuts& cuts)
{
    if(m_moduleMapFunc == "minmax") {
        return doMinMaxMask(val, cuts);
    }
    else if(m_moduleMapFunc == "meanrms") {
        return doMeanRMSMask(val, cuts);
    }
    else {
        ATH_MSG_ERROR("Chosen module map function is not minmax/meanrms which are the only types supported currently.");
        return false;
    }
}

bool FPGATrackSimGNNGraphConstructionTool::doMinMaxMask(float val, const ModuleMapConfig::FeatureCuts& cuts)
{
    return (val <= cuts.max * (1.0 + featureSign(cuts.max) * m_moduleMapTol)) && (val >= cuts.min * (1.0 - featureSign(cuts.min) * m_moduleMapTol));
}

bool FPGATrackSimGNNGraphConstructionTool::doMeanRMSMask(float val, const ModuleMapConfig::FeatureCuts& cuts)
{
    float min_rms = cuts.mean - cuts.rms * m_moduleMapRMSThresholdFactor;
    float max_rms = cuts.mean + cuts.rms * m_moduleMapRMSThresholdFactor;
    float tol_min = cuts.min * (1.0 - featureSign(cuts.min) * m_moduleMapTol);
    float tol_max = cuts.max * (1.0 + featureSign(cuts.max) * m_moduleMapTol);

    float capped_min = std::max(tol_min, min_rms);
    float capped_max = std::min(tol_max, max_rms);

    return (val <= capped_max) && (val >= capped_min);
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