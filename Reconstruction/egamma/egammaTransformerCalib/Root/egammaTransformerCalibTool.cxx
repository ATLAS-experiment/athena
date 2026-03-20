/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "egammaTransformerCalib/egammaTransformerCalibTool.h"

#include "egammaMVACalib/egammaMVAFunctions.h"

#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/Photon.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODCaloEvent/CaloCluster.h"

#include "PathResolver/PathResolver.h"
#include "CxxUtils/checker_macros.h"

#include "TFile.h"
#include "TMath.h"
#include "TObjString.h"
#include "TTree.h"
#include "TClass.h"

#include <cmath>
#include <format>

#ifndef XAOD_ANALYSIS
#include "GaudiKernel/SystemOfUnits.h"
using Gaudi::Units::GeV;
#else
#define GeV 1000
#endif

egammaTransformerCalibTool::egammaTransformerCalibTool(const std::string& name) :
  asg::AsgTool(name)
{
}

// Need to declare this out-of-line since the full type of m_funcs
// isn't available in the header.
egammaTransformerCalibTool::~egammaTransformerCalibTool()
{
}


StatusCode egammaTransformerCalibTool::initialize()
{
  if (m_particleType == xAOD::EgammaParameters::NumberOfEgammaTypes) {
    ATH_MSG_FATAL("Particle type not set: you have to set property ParticleType to a valid value");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Initializing with particle " << m_particleType);

  if (m_isMC) {
    ATH_MSG_DEBUG("Input is MC");
  } else {
    ATH_MSG_DEBUG("Input is data");
  }

  if (!m_isMC && m_useLayerCorrected) {
    ATH_MSG_DEBUG("Using layer-corrected energies as input to Transformer");
    //
    m_layerRecalibTool = std::make_unique<egammaLayerRecalibTool>(m_layerCalibTune, m_useSaccCorrection);
    m_layerRecalibTool->fixForMissingCells(m_useFixForMissingCells);
    m_layerRecalibTool->disable_LayerclEdecoration();
    // by default it will not apply timing cut fix, we apply the timing cut fix here by default 
  } else {
    ATH_MSG_DEBUG("Not using layer tool, Using raw layer energies as input to Transformer");
  }
  
  // get the Transformer models and initialize functions
  ATH_MSG_DEBUG("get Transformer ONNX models in folder: " << m_folder);
  switch (m_particleType) {
  case xAOD::EgammaParameters::electron:
    {
      m_num_cluster_features = 11;
      m_num_cell_features = 7;
      ATH_CHECK(setupTransformerModel(PathResolverFindCalibFile(m_folder + "/" + m_electronModelFile)));
    }
    break;
  case xAOD::EgammaParameters::unconvertedPhoton:
    {
      m_num_cluster_features = 11;
      m_num_cell_features = 7;
      ATH_CHECK(setupTransformerModel(PathResolverFindCalibFile(m_folder + "/" + m_unconvertedPhotonModelFile))); 
    }
    break;
  case xAOD::EgammaParameters::convertedPhoton:
    {
      m_num_cluster_features = 15;
      m_num_cell_features = 7;
      ATH_CHECK(setupTransformerModel(PathResolverFindCalibFile(m_folder + "/" + m_convertedPhotonModelFile)));
    }
    break;
  case xAOD::EgammaParameters::forwardelectron:
    {
      m_num_cluster_features = 11;
      m_num_cell_features = 7;
      // Forward electron is not implemented, will use model for electron
      ATH_MSG_WARNING("Forward electron Transformer model is not implemented, will use electron model instead");
      ATH_CHECK(setupTransformerModel(PathResolverFindCalibFile(m_folder + "/" + m_electronModelFile)));
    }
    break;
    
  default:
    ATH_MSG_FATAL("Particle type not set properly: " << m_particleType);
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


StatusCode egammaTransformerCalibTool::setupTransformerModel(const std::string& fileName)
{
  ATH_MSG_DEBUG("initialize() initialize salt model...");

  m_saltModel = std::make_unique<FlavorTagInference::SaltModel>(fileName);

  // return StatusCode::SUCCESS;

  // set up decorators using a dummy query of the onnx model
  std::map<std::string, FlavorTagInference::Inputs> gnn_input;

  ATH_MSG_DEBUG("initialize() initialize cluster-level features...");
  std::vector<float> cluster_feat(m_num_cluster_features, 0.);
  std::vector<int64_t> cluster_feat_dim = {1, static_cast<int64_t>(cluster_feat.size())};
  FlavorTagInference::Inputs elec_info(cluster_feat, cluster_feat_dim);
  gnn_input.insert({"cluster_features", elec_info}); // need to use the "jet_features" keyword as we are borrowing flavour tagging code

  ATH_MSG_DEBUG("initialize() initialize cell-level features...");
  std::vector<float> cell_feat(m_num_cell_features, 0.);
  std::vector<int64_t> cell_feat_dim = {1, m_num_cell_features};
  FlavorTagInference::Inputs track_info(cell_feat, cell_feat_dim);
  gnn_input.insert({"cell_features", track_info});

  ATH_MSG_DEBUG("initialize() initialize dummy evaluation...");
  auto [out_f, out_vc, out_vf] = m_saltModel->runInference(gnn_input); // the dummy evaluation

  ATH_MSG_DEBUG("initialize() finished dummy evaluation...");
  ATH_MSG_DEBUG("initialize() Output Float(s):");
  for (auto &singlefloat : out_f)
  {
    ATH_MSG_DEBUG("initialize() " << singlefloat.first << " = " << singlefloat.second);
  }
  ATH_MSG_DEBUG("initialize() Output vector char(s):");
  for (auto &vecchar : out_vc)
  {
    ATH_MSG_DEBUG("initialize() " << vecchar.first << " = ");
    for (auto &cc : vecchar.second)
    {
      ATH_MSG_DEBUG("initialize() " << cc);
    }
  }

  ATH_MSG_DEBUG("initialize() Output vector float(s):");
  for (auto &vecfloat : out_vf)
  {
    ATH_MSG_DEBUG("initialize() " << vecfloat.first << " = ");
    for (auto &ff : vecfloat.second)
    {
      ATH_MSG_DEBUG("initialize() " << ff);
    }
  }

  return StatusCode::SUCCESS;
}

float egammaTransformerCalibTool::getEnergy(const xAOD::CaloCluster& clus,
                                            const xAOD::Egamma* eg,
                                            const egammaMVACalib::GlobalEventInfo& gei) const
{
    // 0. Safety Checks
    if (!m_saltModel || !eg) {
        if (m_clusterEif0) {
            ATH_MSG_WARNING("Model not loaded or Egamma pointer is null, returning cluster energy");
            return clus.e();
        } else {
            ATH_MSG_FATAL("Model not loaded or Egamma pointer is null, and useClusterIf0 is false, cannot proceed");
            return 0.0f;
        }
    }

    // --- 1. Cell Recovery (Timing Cut Fix) ---
    IegammaCellRecoveryTool::Info recoveryInfo;
    bool recoverySucceeded = false;
    if (!m_egammaCellRecoveryTool.empty()) {
        if (m_egammaCellRecoveryTool->execute(clus, recoveryInfo).isFailure()) {
             ATH_MSG_WARNING("Cell Recovery Tool failed. Proceeding without recovered cells.");
        } else {
            recoverySucceeded = true;
        }
    }

    // --- 2. Apply Layer Calibration if needed ---
    const xAOD::CaloCluster* clusterForTransformer = eg->caloCluster();
    std::unique_ptr<xAOD::Egamma> temp_eg; // Manages the lifetime of the temporary object

    bool isForward = (m_particleType == xAOD::EgammaParameters::forwardelectron);
    auto array_layer_scales = std::array<double, 4>{1.0, 1.0, 1.0, 1.0}; // default scales

    if (m_layerRecalibTool && !m_isMC && !isForward) {
        ATH_MSG_DEBUG("Applying layer recalibration for GNN on data.");
        
        // A. Create a new object of the correct concrete type (Electron or Photon)
        // We use a switch based on the configured particle type.
        switch (m_particleType) {
            case xAOD::EgammaParameters::electron: {
                temp_eg = std::make_unique<xAOD::Electron>();
                temp_eg->makePrivateStore(*eg); // B. Copy data from the original object
                break;
            }
            case xAOD::EgammaParameters::unconvertedPhoton:
            case xAOD::EgammaParameters::convertedPhoton: {
                temp_eg = std::make_unique<xAOD::Photon>();
                temp_eg->makePrivateStore(*eg);   // B. Copy data from the original object
                break;
            }
            default:
                ATH_MSG_WARNING("Unknown particle type set in tool for layer calibration: " << m_particleType);
                temp_eg = nullptr;
                break;
        }
        
        // D. Apply correction to the new, non-const object
        if (temp_eg) {
            const xAOD::EventInfo* eventInfo = gei.eventInfo;
            array_layer_scales = m_layerRecalibTool->getLayerCorrections(*temp_eg, *eventInfo);
            // E. Get the calibrated cluster from the temporary object
            clusterForTransformer = temp_eg->caloCluster();
        }
    } 
    else
    {
        // Get the cluster from the (possibly calibrated) local object, if not using layer corrections, this will just be the original cluster
        clusterForTransformer = eg->caloCluster();
        ATH_MSG_DEBUG("Not using layer tool, Using raw layer energies as input to Transformer");
    }
    
    if ( m_useExtraLayerScales ) {
        ATH_MSG_DEBUG("Applying extra layer scales for systematic studies, normally this is for MC events.");
        if ( !m_isMC ) {
            ATH_MSG_WARNING("You are applying extra layer scales but the input is not MC! Are you sure this is intended?");
        }
        // extract scales from global event info
        for (std::size_t i = 0; i < 4; ++i)
          array_layer_scales[i] *= gei.scaleEs[i];
    }

    // --- 3. Calculate Scale Factors ---
    
    // Raw energies + Recovered Energy (Timing Fix)
    // double raw_Es0 = clus.energyBE(0);  // LG: not sure if this is still needed but keeping it here for consistency
    double raw_Es1 = clus.energyBE(1);
    double raw_Es2 = clus.energyBE(2) + (recoverySucceeded && m_useFixForMissingCells ? recoveryInfo.eCells[0] : 0.0);
    double raw_Es3 = clus.energyBE(3) + (recoverySucceeded && m_useFixForMissingCells ? recoveryInfo.eCells[1] : 0.0);

    // --- 4. Cell Gathering ---
    std::vector<float> cells_E, cells_eta, cells_phi, cells_x, cells_y, cells_z;
    std::vector<int> cells_layer;
    std::vector<Identifier> included_cells; // Track cells to avoid duplicates

    // Layer sums
    double sum_cell_E_L0 = 0.0, sum_cell_E_L1 = 0.0, sum_cell_E_L2 = 0.0, sum_cell_E_L3 = 0.0, sum_cell_E_Gap = 0.0;

    // A. Iterate over Standard Cluster Cells
    const CaloClusterCellLink* cellLinks = clus.getCellLinks();
    if (cellLinks) {
        for (const CaloCell* cell : *cellLinks) {
            if (!cell) continue;

            int sampling = cell->caloDDE()->getSampling();
            double scale_factor = 1.0;
            int layer_idx = -1;

            switch (sampling) {
            case CaloCell_ID::PreSamplerB: case CaloCell_ID::PreSamplerE:
                scale_factor = array_layer_scales[0]; layer_idx = 0; break;
            case CaloCell_ID::EMB1: case CaloCell_ID::EME1:
                scale_factor = array_layer_scales[1]; layer_idx = 1; break;
            case CaloCell_ID::EMB2: case CaloCell_ID::EME2:
                scale_factor = array_layer_scales[2]; layer_idx = 2; 
                // Track cells that might be already recovered (those with time > timing cut)
                if (cell->time() > m_timeCut) {  // Use your actual timing cut threshold
                    included_cells.push_back(cell->ID());
                }
                break;
            case CaloCell_ID::EMB3: case CaloCell_ID::EME3:
                scale_factor = array_layer_scales[3]; layer_idx = 3; 
                if (cell->time() > m_timeCut) {
                    included_cells.push_back(cell->ID());
                }
                break;
            case CaloCell_ID::TileGap3:
                scale_factor = 1.0;    layer_idx = 4; break;
            default: continue;
            }

            double final_E = cell->e() * scale_factor;
            
            cells_E.push_back(final_E);
            cells_eta.push_back(cell->eta());
            cells_phi.push_back(cell->phi());
            cells_x.push_back(cell->x());
            cells_y.push_back(cell->y());
            cells_z.push_back(cell->z());
            cells_layer.push_back(layer_idx);

            // Accumulate Sums
            switch(layer_idx) {
                case 0: sum_cell_E_L0 += final_E; break;
                case 1: sum_cell_E_L1 += final_E; break;
                case 2: sum_cell_E_L2 += final_E; break;
                case 3: sum_cell_E_L3 += final_E; break;
                case 4: sum_cell_E_Gap += final_E; break;
            }
        }
    }

    // B. Iterate over Recovered Cells (from Tool) - Skip Duplicates
    // Added cells are only expected in layers 2 and 3, so the dedup list only tracks those layers.
    for (const CaloCell* cell : recoveryInfo.addedCells) {
        if (!cell || !cell->caloDDE()) continue;
        
        // Skip if this cell is already in the cluster
        if (std::find(included_cells.begin(), included_cells.end(), cell->ID()) != included_cells.end()) {
            ATH_MSG_DEBUG("Recovered cell " << cell->ID() << " already included in cluster. Skipping to avoid double counting.");
            continue;
        }
        else {
            ATH_MSG_DEBUG("Adding recovered cell " << cell->ID() << " to cluster inputs.");
        }
        
        int sampling = cell->caloDDE()->getSampling();
        double scale_factor = 1.0;
        int layer_idx = -1;

        if (sampling == CaloCell_ID::EMB2 || sampling == CaloCell_ID::EME2) {
            scale_factor = array_layer_scales[2]; layer_idx = 2;
        } else if (sampling == CaloCell_ID::EMB3 || sampling == CaloCell_ID::EME3) {
            scale_factor = array_layer_scales[3]; layer_idx = 3;
        } else {
            // Fallback
            if (sampling == CaloCell_ID::PreSamplerB || sampling == CaloCell_ID::PreSamplerE) { 
                scale_factor = array_layer_scales[0]; layer_idx = 0; 
            } else if (sampling == CaloCell_ID::EMB1 || sampling == CaloCell_ID::EME1) { 
                scale_factor = array_layer_scales[1]; layer_idx = 1; 
            } else {
                continue;
            }
        }

        double final_E = cell->e() * scale_factor;

        cells_E.push_back(final_E);
        cells_eta.push_back(cell->eta());
        cells_phi.push_back(cell->phi());
        cells_x.push_back(cell->x());
        cells_y.push_back(cell->y());
        cells_z.push_back(cell->z());
        cells_layer.push_back(layer_idx);

        switch(layer_idx) {
            case 0: sum_cell_E_L0 += final_E; break;
            case 1: sum_cell_E_L1 += final_E; break;
            case 2: sum_cell_E_L2 += final_E; break;
            case 3: sum_cell_E_L3 += final_E; break;
            case 4: sum_cell_E_Gap += final_E; break;
        }
    }

    // --- 5. Calculate Derived Features (Post-Loop) ---
    const size_t nCells = cells_E.size();
    if (nCells == 0) return 0.0f;

    double sum_cell_E_total = sum_cell_E_L0 + sum_cell_E_L1 + sum_cell_E_L2 + sum_cell_E_L3;
    const double cluster_eta = clus.eta();
    const double cluster_phi = clus.phi();

    std::vector<float> cells_deta, cells_dphi, cells_eFrac;
    cells_deta.reserve(nCells);
    cells_dphi.reserve(nCells);
    cells_eFrac.reserve(nCells);

    for (size_t i = 0; i < nCells; ++i) {
        float deta = cells_eta[i] - cluster_eta;
        float dphi = cells_phi[i] - cluster_phi;
        dphi = std::fmod(dphi + 3.0f * M_PI, 2.0f * M_PI) - M_PI;

        cells_deta.push_back(deta);
        cells_dphi.push_back(dphi);

        float eFrac_layer = 0.0f;
        switch (cells_layer[i]) {
            case 0: eFrac_layer = (sum_cell_E_L0 != 0) ? (cells_E[i] / sum_cell_E_L0) : 0.0f; break;
            case 1: eFrac_layer = (sum_cell_E_L1 != 0) ? (cells_E[i] / sum_cell_E_L1) : 0.0f; break;
            case 2: eFrac_layer = (sum_cell_E_L2 != 0) ? (cells_E[i] / sum_cell_E_L2) : 0.0f; break;
            case 3: eFrac_layer = (sum_cell_E_L3 != 0) ? (cells_E[i] / sum_cell_E_L3) : 0.0f; break;
            case 4: eFrac_layer = (sum_cell_E_Gap != 0) ? (cells_E[i] / sum_cell_E_Gap) : 0.0f; break;
        }
        cells_eFrac.push_back(eFrac_layer);
    }

    // --- 6. Prepare GNN Inputs and Run Inference ---
    double ratio_L1_L2 = (sum_cell_E_L2 != 0) ? (sum_cell_E_L1 / sum_cell_E_L2) : 0.0;
    double main_layers_sum = sum_cell_E_L1 + sum_cell_E_L2 + sum_cell_E_L3;
    double ratio_L0_total = (main_layers_sum != 0) ? (sum_cell_E_L0 / main_layers_sum) : 0.0;
    double ratio_Tile_total = (main_layers_sum != 0) ? (sum_cell_E_Gap / main_layers_sum) : 0.0;

    std::map<std::string, FlavorTagInference::Inputs> gnn_input;

    // Cluster Features
    std::vector<float> cluster_feats = {
        static_cast<float>(sum_cell_E_total),
        static_cast<float>(sum_cell_E_L0),
        static_cast<float>(sum_cell_E_L1),
        static_cast<float>(sum_cell_E_L2),
        static_cast<float>(sum_cell_E_L3),
        static_cast<float>(sum_cell_E_Gap),
        static_cast<float>(cluster_eta),
        static_cast<float>(cluster_phi),
        static_cast<float>(ratio_L1_L2),
        static_cast<float>(ratio_L0_total),
        static_cast<float>(ratio_Tile_total)
    };
    
    // For converted photons, append conversion-specific features in order: convR, convEtOverPt, convPtRatio, conversionType.
    if (m_particleType == xAOD::EgammaParameters::convertedPhoton) {
      const xAOD::Photon* photon = dynamic_cast<const xAOD::Photon*>(eg);
      if (photon) {
        // - convR
        float convR = 799.0f;
        if (egammaMVAFunctions::compute_ptconv(photon) > 3 * GeV) {
          convR = xAOD::EgammaHelpers::conversionRadius(photon);
        }

        // - convEtOverPt
        float convEtOverPt = 0.0f;
        float ptconv = egammaMVAFunctions::compute_ptconv(photon);
        if (xAOD::EgammaHelpers::numberOfSiTracks(photon) == 2 && ptconv > 0.0f) {
           float eacc = (m_useLayerCorrected ? 
                        (raw_Es1 * array_layer_scales[1] + raw_Es2 * array_layer_scales[2] + raw_Es3 * array_layer_scales[3]) :
                        (raw_Es1 + raw_Es2 + raw_Es3));
           float cl_eta = egammaMVAFunctions::compute_cl_eta(*clusterForTransformer);
           convEtOverPt = std::max(0.0f, eacc / (std::cosh(cl_eta) * ptconv));
        }
        convEtOverPt = std::min(convEtOverPt, 2.0f);

        // - convPtRatio
        float convPtRatio = 1.0f;
        if (xAOD::EgammaHelpers::numberOfSiTracks(photon) == 2) {
          float pt1 = egammaMVAFunctions::compute_pt1conv(photon);
          float pt2 = egammaMVAFunctions::compute_pt2conv(photon);
          if ((pt1 + pt2) > 0.0f) {
             convPtRatio = std::max(pt1, pt2) / (pt1 + pt2);
          }
        }

        // - conversionType
         float conversionType = static_cast<float>(photon->conversionType());
          //  must push back in this order as the model expects features in this order
         cluster_feats.push_back(convR);
         cluster_feats.push_back(convEtOverPt);
         cluster_feats.push_back(convPtRatio);
         cluster_feats.push_back(conversionType);
      } else {
         cluster_feats.push_back(0.0f);
         cluster_feats.push_back(0.0f);
         cluster_feats.push_back(0.0f);
         cluster_feats.push_back(0.0f);
      }
    }

            
    gnn_input["cluster_features"] = FlavorTagInference::Inputs(cluster_feats, {1, (int64_t)cluster_feats.size()});

    // Cell Features
    std::vector<float> cell_feats_flat;
    cell_feats_flat.reserve(nCells * m_num_cell_features);
    for (size_t i = 0; i < nCells; ++i) {
        cell_feats_flat.push_back(cells_eFrac[i]);
        cell_feats_flat.push_back(cells_deta[i]);
        cell_feats_flat.push_back(cells_dphi[i]);
        cell_feats_flat.push_back(cells_x[i]);
        cell_feats_flat.push_back(cells_y[i]);
        cell_feats_flat.push_back(cells_z[i]);
        cell_feats_flat.push_back(static_cast<float>(cells_layer[i]));
    }
    gnn_input["cell_features"] = FlavorTagInference::Inputs(cell_feats_flat, {(int64_t)nCells, m_num_cell_features});

    // Run Inference
    auto [out_f, out_vc, out_vf] = m_saltModel->runInference(gnn_input);

    float el_gnn_score = 0.0f;
    if (out_vf.empty() || out_vf.begin()->second.empty()) {
      ATH_MSG_DEBUG("GNN inference output is empty!");
    } else {
      el_gnn_score = out_vf.begin()->second.front();
    }

    // what to do if the Transformer response is 0;
    if (el_gnn_score == 0.0f) {
      return m_clusterEif0 ? clus.e() : 0.0f;
    }

    return el_gnn_score * static_cast<float>(sum_cell_E_total);
}