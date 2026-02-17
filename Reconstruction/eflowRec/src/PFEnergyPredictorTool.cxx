/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PFEnergyPredictorTool.h"
#include "eflowCaloObject.h"
#include "eflowTrackClusterLink.h"
#include "eflowRecCluster.h"

#include "CaloGeoHelpers/CaloSampling.h"

PFEnergyPredictorTool::PFEnergyPredictorTool(const std::string& type, const std::string& name, const IInterface* parent) : AthAlgTool(type, name, parent)
{

}


StatusCode PFEnergyPredictorTool::initialize()
{
   ATH_MSG_DEBUG("Initializing " << name());
   if(m_model_filepath == "////"){
      ATH_MSG_WARNING("model not provided tool will not work");
      return StatusCode::SUCCESS;
   }
   ATH_CHECK(m_svc.retrieve());
   std::string path  = m_model_filepath;//Add path resolving code

   Ort::SessionOptions session_options;
   Ort::AllocatorWithDefaultOptions allocator;
   session_options.SetIntraOpNumThreads(1);
   session_options.SetGraphOptimizationLevel(ORT_ENABLE_BASIC);
   m_session = std::make_unique<Ort::Session>(m_svc->env(), path.c_str(), session_options);

    ATH_MSG_INFO("Created ONNX runtime session with model " << path);

    size_t num_input_nodes = m_session->GetInputCount();
    m_input_node_names.resize(num_input_nodes);

    for (std::size_t i = 0; i < num_input_nodes; i++) {
        // print input node names
        char* input_name = m_session->GetInputNameAllocated(i, allocator).release();
        ATH_MSG_INFO("Input " << i << " : "
                              << " name= " << input_name);
        m_input_node_names[i] = input_name;
        // print input node types
        Ort::TypeInfo type_info = m_session->GetInputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
        ONNXTensorElementDataType type = tensor_info.GetElementType();
        ATH_MSG_INFO("Input " << i << " : "
                              << " type= " << type);

        // print input shapes/dims
        m_input_node_dims = tensor_info.GetShape();
        m_input_node_dims[1] = 5430/5;
        ATH_MSG_INFO("Input " << i << " : num_dims= " << m_input_node_dims.size());
        for (std::size_t j = 0; j < m_input_node_dims.size(); j++) {
            if (m_input_node_dims[j] < 0) m_input_node_dims[j] = 1;
            ATH_MSG_INFO("Input " << i << " : dim " << j << "= " << m_input_node_dims[j]);
        }
    }
    
    // output nodes
    std::vector<int64_t> output_node_dims;
    size_t num_output_nodes = m_session->GetOutputCount();
    ATH_MSG_INFO("Have output nodes " << num_output_nodes);
    m_output_node_names.resize(num_output_nodes);

    for (std::size_t i = 0; i < num_output_nodes; i++) {
        // print output node names
        char* output_name = m_session->GetOutputNameAllocated(i, allocator).release();
        ATH_MSG_INFO("Output " << i << " : "
                               << " name= " << output_name);
        m_output_node_names[i] = output_name;

        Ort::TypeInfo type_info = m_session->GetOutputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
        ONNXTensorElementDataType type = tensor_info.GetElementType();
        ATH_MSG_INFO("Output " << i << " : "
                               << " type= " << type);

        // print output shapes/dims
        output_node_dims = tensor_info.GetShape();
        ATH_MSG_INFO("Output " << i << " : num_dims= " << output_node_dims.size());
        for (std::size_t j = 0; j < output_node_dims.size(); j++) {
            if (output_node_dims[j] < 0) output_node_dims[j] = 1;
            ATH_MSG_INFO("Output" << i << " : dim " << j << "= " << output_node_dims[j]);
        }
    }

    return StatusCode::SUCCESS;
}

float PFEnergyPredictorTool::runOnnxInference(std::vector<float> &tensor) const {
    using std::endl;
    using std::cout;
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    auto input_tensor_size = tensor.size();

    Ort::Value input_tensor =
        Ort::Value::CreateTensor<float>(memory_info, tensor.data(), input_tensor_size,
         m_input_node_dims.data(), m_input_node_dims.size());

    auto output_tensors = m_session->Run(Ort::RunOptions{nullptr}, m_input_node_names.data(), &input_tensor, m_input_node_names.size(),
                                         m_output_node_names.data(), m_output_node_names.size());

    const float *output_score_array = output_tensors.front().GetTensorData<float>();

    // Binary classification - the score is just the first element of the output tensor
    float output_score = output_score_array[0];

    return output_score;
}

std::array<double,19> getEtaTrackCalo(const eflowTrackCaloPoints& trackCaloPoints) {
    return std::array<double,19> { trackCaloPoints.getEta(eflowCalo::EMB1), trackCaloPoints.getEta(eflowCalo::EMB2), trackCaloPoints.getEta(eflowCalo::EMB3),
    trackCaloPoints.getEta(eflowCalo::EME1), trackCaloPoints.getEta(eflowCalo::EME2), trackCaloPoints.getEta(eflowCalo::EME3),
    trackCaloPoints.getEta(eflowCalo::HEC1), trackCaloPoints.getEta(eflowCalo::HEC2), trackCaloPoints.getEta(eflowCalo::HEC3),trackCaloPoints.getEta(eflowCalo::HEC4),    
    trackCaloPoints.getTileEta(CaloSampling::TileBar0),trackCaloPoints.getTileEta(CaloSampling::TileBar1),trackCaloPoints.getTileEta(CaloSampling::TileBar2),
    trackCaloPoints.getTileEta(CaloSampling::TileGap1),trackCaloPoints.getTileEta(CaloSampling::TileGap2),trackCaloPoints.getTileEta(CaloSampling::TileGap3),
    trackCaloPoints.getTileEta(CaloSampling::TileExt0),trackCaloPoints.getTileEta(CaloSampling::TileExt1),trackCaloPoints.getTileEta(CaloSampling::TileExt2)};
}


std::array<double,19> getPhiTrackCalo(const eflowTrackCaloPoints& trackCaloPoints) {
    return std::array<double,19> { trackCaloPoints.getPhi(eflowCalo::EMB1), trackCaloPoints.getPhi(eflowCalo::EMB2), trackCaloPoints.getPhi(eflowCalo::EMB3),
    trackCaloPoints.getPhi(eflowCalo::EME1), trackCaloPoints.getPhi(eflowCalo::EME2), trackCaloPoints.getPhi(eflowCalo::EME3),
    trackCaloPoints.getPhi(eflowCalo::HEC1), trackCaloPoints.getPhi(eflowCalo::HEC2), trackCaloPoints.getPhi(eflowCalo::HEC3),trackCaloPoints.getPhi(eflowCalo::HEC4),
    trackCaloPoints.getTilePhi(CaloSampling::TileBar0),trackCaloPoints.getTilePhi(CaloSampling::TileBar1),trackCaloPoints.getTilePhi(CaloSampling::TileBar2),
    trackCaloPoints.getTilePhi(CaloSampling::TileGap1),trackCaloPoints.getTilePhi(CaloSampling::TileGap2),trackCaloPoints.getTilePhi(CaloSampling::TileGap3),
    trackCaloPoints.getTilePhi(CaloSampling::TileExt0),trackCaloPoints.getTilePhi(CaloSampling::TileExt1),trackCaloPoints.getTilePhi(CaloSampling::TileExt2)};
}


float PFEnergyPredictorTool::nnEnergyPrediction(const eflowRecTrack *ptr) const{

     constexpr std::array<int,19> calo_numbers{1,2,3,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
     constexpr std::array<int,12> fixed_r_numbers = {1,2,3,12,13,14,15,16,17,18,19,20};
     constexpr std::array<double,12> fixed_r_vals = {1532.18, 1723.89, 1923.02, 2450.00, 2995.00, 3630.00, 3215.00,
                                        3630.00, 2246.50, 2450.00, 2870.00, 3480.00
                                       };
     constexpr std::array<int, 7> fixed_z_numbers = {5,6,7,8,9,10,11};
     constexpr std::array<double, 7> fixed_z_vals = {3790.03, 3983.68, 4195.84, 4461.25, 4869.50, 5424.50, 5905.00};
     std::unordered_map<int, double> r_calo_dict;//change to flatmap in c++23
     std::unordered_map<int, double> z_calo_dict;
     for(size_t i=0; i<fixed_r_vals.size(); i++) r_calo_dict[fixed_r_numbers[i]] = fixed_r_vals[i];
     for(size_t i=0; i<fixed_z_numbers.size(); i++) z_calo_dict[fixed_z_numbers[i]] = fixed_z_vals[i];

     std::vector<float> inputnn;
     inputnn.assign(5430, 0.0);
     std::vector<eflowRecCluster*> matchedClusters;
     std::vector<eflowTrackClusterLink*> links = ptr->getClusterMatches();

    std::array<double, 19> etatotal = getEtaTrackCalo(ptr->getTrackCaloPoints());
    std::array<double, 19> phitotal = getPhiTrackCalo(ptr->getTrackCaloPoints());

     const std::array<double, 2> track{ptr->getTrack()->eta(), ptr->getTrack()->phi()};

     for(auto *clink : links){
        auto *cell = clink->getCluster()->getCluster();
        float clusterE = cell->e()*1e-3;
        float clusterEta = cell->eta();

        if (clusterE < 0.0 || clusterE > 1e4f || std::abs(clusterEta) > 2.5) continue;

        constexpr bool cutOnR = false;
        if(cutOnR){
            std::array<double, 2> p{clink->getCluster()->getCluster()->eta(), clink->getCluster()->getCluster()->phi()};
            double part1 = p[0] - track[0];
            double part2 = p[1] - track[1];
            while(part1 > M_PI) part1 -= 2*M_PI;
            while(part1 < -M_PI) part1 += 2*M_PI;
            while(part2 > M_PI) part2 -= 2*M_PI;
            while(part2 < -M_PI) part2 += 2*M_PI;
            double R = std::sqrt(part1 * part1 + part2*part2);
            if(R >= 1.2) continue;
        }

        matchedClusters.push_back(clink->getCluster());
     }

     std::vector<std::array<double, 5>> cells;

    const eflowTrackCaloPoints& trackCaloPoints = ptr->getTrackCaloPoints();
    bool trk_bool_em[2] = {false,false};
    std::array<double,2> trk_em_eta = {trackCaloPoints.getEta(eflowCalo::EMB2), trackCaloPoints.getEta(eflowCalo::EME2)};
    std::array<double,2> trk_em_phi = {trackCaloPoints.getPhi(eflowCalo::EMB2), trackCaloPoints.getPhi(eflowCalo::EME2)};
    double eta_ctr;
    double phi_ctr;
    for(int i =0; i<2; i++) {
        trk_bool_em[i] = std::abs(trk_em_eta[i]) < 2.5 && std::abs(trk_em_phi[i]) <= M_PI;
    }
    int nProj_em = (int)trk_bool_em[0] + (int)trk_bool_em[1];

    if(nProj_em ==1) {
        eta_ctr = trk_bool_em[0] ? trk_em_eta[0] : trk_em_eta[1];
        phi_ctr = trk_bool_em[0] ? trk_em_phi[0] : trk_em_phi[1];
    } else if(nProj_em==2) {
        eta_ctr = (trk_em_eta[0] + trk_em_eta[1]) / 2.0;
        phi_ctr = (trk_em_phi[0] + trk_em_phi[1]) / 2.0;
    } else {
        eta_ctr = ptr->getTrack()->eta();
        phi_ctr = ptr->getTrack()->phi();
    }



     for(auto *cptr : matchedClusters){
        auto *clustlink = cptr->getCluster();

        for(auto it_cell = clustlink->cell_begin(); it_cell != clustlink->cell_end(); it_cell++){
           const CaloCell* cell = (*it_cell);
           float cellE = cell->e()*(it_cell.weight())*1e-3f;
           if(cellE < 0.005) continue;//Cut from ntuple maker
           const auto *theDDE=it_cell->caloDDE();
           double cx=theDDE->x();
           double cy=theDDE->y();

           cells.emplace_back( std::array<double, 5> { cellE,
                    theDDE->eta() -  eta_ctr,
                    theDDE->phi() -  phi_ctr,
                    std::hypot(cx,cy), //rperp
                    0.0    } );
        }
     }


    std::vector<bool> trk_bool(calo_numbers.size(), false);
    std::vector<std::array<double,4>> trk_full(calo_numbers.size());
    for(size_t j=0; j<phitotal.size(); j++) {
        int cnum = calo_numbers[j];
        double eta = etatotal[j];
        double phi = phitotal[j];
        if(std::abs(eta) < 2.5 && std::abs(phi) <= M_PI) {
            trk_bool[j] = true;
            trk_full[j][0] = eta;
            trk_full[j][1] = phi;
            trk_full[j][3] = cnum;
            double rPerp =-99999;
            if(auto itr = r_calo_dict.find(cnum); itr != r_calo_dict.end()) rPerp = itr->second;
            else if(auto itr = z_calo_dict.find(cnum); itr != z_calo_dict.end())
            {
                double z = itr->second;
                if(eta != 0.0){
                   double aeta = std::abs(eta);
                   rPerp = z*2.*std::exp(aeta)/(std::exp(2.0*aeta)-1.0);
                }else rPerp =0.0; //Check if this makes sense
            } else {
                throw std::runtime_error("Calo sample num not found in dicts..");
            }
            trk_full[j][2] = rPerp;
        } else {
            trk_full[j].fill(0.0);
        }
    }
    double trackP = std::abs(1. / ptr->getTrack()->qOverP()) * 1e-3;
    int trk_proj_num = std::accumulate(trk_bool.begin(), trk_bool.end(), 0);
    if(trk_proj_num ==0) {
        trk_proj_num =1;
        std::array<double,5> trk_arr{};

        trk_arr[0] = trackP;
        trk_arr[1] = ptr->getTrack()->eta() - eta_ctr;
        trk_arr[2] = ptr->getTrack()->phi() - phi_ctr;
        trk_arr[3] = 1532.18; // just place it in EMB1
        trk_arr[4] = 1.;

        cells.emplace_back(trk_arr);
    } else {
        for(size_t i =0; i<calo_numbers.size(); i++) {
            if(!trk_bool[i]) continue;
            std::array<double,5> trk_arr{};
            trk_arr[0]= trackP/double(trk_proj_num);
            trk_arr[1]= trk_full[i][0] - eta_ctr;
            trk_arr[2]= trk_full[i][1] - phi_ctr;
            trk_arr[3]= trk_full[i][2];
            trk_arr[4]= 1.;

            cells.emplace_back(trk_arr);
        }
    }

    int index = 0;
    for(auto &in : cells){
      std::copy(in.begin(), in.end(), inputnn.begin() + index);
      index+=5;
      if(index >= static_cast<int>(inputnn.size()-4)) {
        ATH_MSG_WARNING("Data exceeded tensor size");
        break;
      }
    }
        
    //Normalization prior to training
    NormalizeTensor(inputnn, cells.size() * 5 );
        
    float predictedEnergy = exp(runOnnxInference(inputnn)) * 1000.0;//Correct to MeV units 
    ATH_MSG_DEBUG("NN Predicted energy " << predictedEnergy);      
    return predictedEnergy;

}

void PFEnergyPredictorTool::NormalizeTensor(std::vector<float> &inputnn, size_t limit) const{
    size_t i=0;
    for(i =0;i<limit;i+=5){
      auto &f = inputnn[i+3];
      if(f!= 0.0f) f/= 3630.f;
      auto &e = inputnn[i+0];
      if(e!= 0.0f){
       e = std::log(e);
       e = (e - m_cellE_mean)/m_cellE_std;
      }
      auto &eta = inputnn[i+1];
      if(eta!= 0.0) eta /= 0.7f;
      auto &phi = inputnn[i+2];
      if(phi!= 0.0) phi /= m_cellPhi_std;
    }
    if(i> inputnn.size()){
        ATH_MSG_ERROR("Index exceeded tensor MEMORY CORRUPTION");
    }
}



StatusCode PFEnergyPredictorTool::finalize()
{
  return StatusCode::SUCCESS;
}

