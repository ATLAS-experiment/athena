/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "SPIdentifierAlg.h"

#include "Identifier/Identifier.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadHandle.h"
#include "MuonTesterTree/EventHashBranch.h"
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "xAODMuonPrepData/MdtDriftCircle.h"
#include <fstream>
#include <TString.h>
#include <AthenaKernel/RNGWrapper.h>
#include "CLHEP/Random/RandFlat.h"

namespace {
    union bucketId{
        int8_t fields[4];
        int hash;
    };

}

namespace MuonR4{

    StatusCode SPIdentifierAlg::initialize() {
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_inSegmentKey.initialize(!m_inSegmentKey.empty()));
        m_tree.addBranch(std::make_shared<MuonVal::EventHashBranch>(m_tree.tree()));
        ATH_CHECK(m_tree.init(this));
        ATH_CHECK(m_idHelperSvc.retrieve());

        ATH_MSG_DEBUG("Successfully initialized");

        return StatusCode::SUCCESS;
    }

    StatusCode SPIdentifierAlg::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }
    
    StatusCode SPIdentifierAlg::execute(){
        const EventContext& ctx{Gaudi::Hive::currentContext()};

        std::unordered_map <const SpacePointBucket*, std::vector<const MuonR4::Segment*>> segmentMap;
        
        SG::ReadHandle readSegment(m_inSegmentKey, ctx);
        ATH_CHECK(readSegment.isPresent());
        for (const MuonR4::Segment* segment : *readSegment) {
            segmentMap[segment->parent()->parentBucket()].push_back(segment);
        }

        SG::ReadHandle<SpacePointContainer> readHandle{m_readKey, ctx};
        ATH_CHECK(readHandle.isPresent());

        bool Sparse = false;
        bool Labels = true;

        std::vector<float> features;
        std::vector<int64_t> edge_src;
        std::vector<int64_t> edge_dst;
        std::vector<int64_t> node_offsets;
        std::vector<int64_t> labels;
        size_t total_nodes = 0;
        size_t bucket_index = 0;

        for(const SpacePointBucket* bucket : *readHandle) {          

            unsigned int segIdx{0};
            std::unordered_map<const SpacePoint*, std::vector<int16_t>> spacePointToSegment;
            
            if (Labels) {
                auto match_itr = segmentMap.find(bucket);
                if (match_itr != segmentMap.end()) {
                    for (const MuonR4::Segment* segment : match_itr->second) {
                        for (const auto& meas : segment->measurements()) {
                            spacePointToSegment[meas->spacePoint()].push_back(segIdx);
                        }
                        ++segIdx;
                    }
                }
            }

            SpacePointPerLayerSorter sorter{*bucket};

            size_t num_points = bucket->size();

            unsigned int layer{0};
            Identifier prevLayer = Identifier();

            std::vector<u_int16_t> Neighbors(num_points, 0);

            size_t bucket_offset = total_nodes;
            node_offsets.push_back(bucket_offset);
            total_nodes += num_points;

            float min_x = 3000;
            float max_x = -1;

            Identifier layId;
            for (u_int16_t i = 0; i < num_points; i++) {
                const auto sp = bucket->at(i);
                const Identifier id = sp->identify();

                if (sp->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                    const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(sp->primaryMeasurement());
                    if (dc->status() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime){
                        continue;
                    }
                    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
                    layId = idHelper.channelID(idHelper.stationName(id), 1, idHelper.stationPhi(id), idHelper.multilayer(id), idHelper.tubeLayer(id), 1);
                    if (layId != prevLayer) {
                        layer++;
                        prevLayer = layId;
                    }
                } else {
                    layId = m_idHelperSvc->gasGapId(id);

                    if (layId != prevLayer) {
                        layer++;
                        prevLayer = layId;
                    }
                }

                if (Labels) {
                    const std::vector<int16_t>& segIdxs = spacePointToSegment[sp.get()];
                    if (segIdxs.size() > 0) {
                        m_spoint_label.push_back(1);
                    } else {
                        m_spoint_label.push_back(0);
                    }
                }
                
                float x = sp->positionInChamber().x();
                float y = sp->positionInChamber().y();
                float z = sp->positionInChamber().z();



                m_spoint_bucket.push_back(bucket_index);
                m_spoint_x.push_back(x);
                m_spoint_y.push_back(y);
                m_spoint_z.push_back(z);
                m_spoint_station.push_back(m_idHelperSvc->stationName(id));

                m_spoint_driftR.push_back(sp->driftRadius());
                m_spoint_layer.push_back(layer);

                if (x < min_x) min_x = x;
                if (x > max_x) max_x = x;


                if (Sparse) {
                    for (u_int16_t j = 0; j < num_points; j++) {
                        if (i == j) continue;
                        const auto sp2 = bucket->at(j);
                        Identifier layId2;
                        const Identifier id2 = sp2->identify();
                        if (sp2->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                            const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
                            layId2 = idHelper.channelID(idHelper.stationName(id2), 1, idHelper.stationPhi(id2), idHelper.multilayer(id2), idHelper.tubeLayer(id2), 1);
                        } else {
                            layId2 = m_idHelperSvc->gasGapId(id2);
                        }
                        float h = sqrt(pow(sp2->positionInChamber().x() - x, 2) + pow(sp2->positionInChamber().y() - y, 2));
                        if (h < 500) {
                            Neighbors[i]++;
                        }
                        if ( h < 2000 and layId != layId2) { 
                            edge_src.push_back(i + bucket_offset);
                            edge_dst.push_back(j + bucket_offset);
                            m_spoint_edges.push_back(j + bucket_offset);
                        }
                    }
                } else {
                    for (u_int16_t j = 0; j < num_points; j++) {
                        if (i == j) continue;    
                        edge_src.push_back(i + bucket_offset);
                        edge_dst.push_back(j + bucket_offset);
                        m_spoint_edges.push_back(j + bucket_offset);
                        if (j < i ) continue;
                        const auto sp2 = bucket->at(j);
                        float h = sqrt(pow(sp2->positionInChamber().x() - x, 2) + pow(sp2->positionInChamber().y() - y, 2));
                        if (h < 500) {
                            Neighbors[i]++;
                            Neighbors[j]++;
                        }
                    }
                }

                m_spoint_neighbors.push_back(Neighbors[i]);
            } // end loop over space points

            m_bucket_layers.push_back(layer);
            float density = 0;
            float bucket_size = bucket->coveredMax() - bucket->coveredMin();
            if (bucket_size == 0 ){
                bucket_size = max_x - min_x;
                if (bucket_size == 0) bucket_size = 1;
            }
            density = num_points / bucket_size;
            m_bucket_density.push_back(density);
            bucket_index++;

        } // end loop over buckets

        features.reserve(total_nodes * 8);
        for (size_t i = 0; i < total_nodes; i++) {
            features.push_back(m_spoint_x[i]);
            features.push_back(m_spoint_y[i]);
            features.push_back(m_spoint_z[i]);
            features.push_back(m_spoint_station[i]);
            features.push_back(m_spoint_driftR[i]);
            features.push_back(float(m_spoint_layer[i]) / (m_bucket_layers[m_spoint_bucket[i]] > 0 ? m_bucket_layers[m_spoint_bucket[i]] : 1));
            features.push_back(m_spoint_neighbors[i]);
            features.push_back(m_bucket_density[m_spoint_bucket[i]]);
            if (Labels) {
                labels.push_back(m_spoint_label[i]);
            }
        }

        if (features.empty()) {
            ATH_MSG_WARNING("No valid feature data available for inference. Skipping event.");
            return StatusCode::SUCCESS;
        }

        std::vector<int64_t> edge_index;
        edge_index.reserve(2 * edge_src.size());
        edge_index.insert(edge_index.end(), edge_src.begin(), edge_src.end());
        edge_index.insert(edge_index.end(), edge_dst.begin(), edge_dst.end());

        ATH_MSG_DEBUG("Total nodes: " << total_nodes);
        ATH_MSG_DEBUG("Features size: " << features.size());
        ATH_MSG_DEBUG("Expected feature shape: (" << total_nodes << ", 8)");

        if (features.size() != total_nodes * 8) {
            ATH_MSG_ERROR("Feature size mismatch! Expected " << total_nodes * 8 << " but got " << features.size());
        }

        ATH_MSG_DEBUG("Edge index size: " << edge_index.size());
        ATH_MSG_DEBUG("Expected edge_index shape: (2, " << edge_index.size() / 2 << ")");

        if (edge_index.size() % 2 != 0) {
            ATH_MSG_ERROR("Edge index format error! Size should be divisible by 2.");
        }

        Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "ONNXInference");
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(1);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        Ort::Session session(env, "torch_GatFourier_fcg_quantized.onnx", session_options);

        Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

        std::vector<int64_t> feature_shape = {static_cast<int64_t>(total_nodes), 8};  // (N, 8)
        std::vector<int64_t> edge_shape    = {2, static_cast<int64_t>(edge_index.size() / 2)};  // (2, E)

        Ort::Value feature_tensor = Ort::Value::CreateTensor<float>(
            memory_info, features.data(), features.size(), feature_shape.data(), feature_shape.size());

        Ort::Value edge_tensor = Ort::Value::CreateTensor<int64_t>(
            memory_info, edge_index.data(), edge_index.size(), edge_shape.data(), edge_shape.size());
        
        std::vector<const char*> input_names = {"features", "edge_index"};
        std::vector<const char*> output_names = {"output"};

        std::vector<Ort::Value> input_tensors;
        input_tensors.push_back(std::move(feature_tensor));
        input_tensors.push_back(std::move(edge_tensor));

        std::vector<Ort::Value> output_tensors = session.Run(
            Ort::RunOptions{nullptr}, input_names.data(), input_tensors.data(),
            input_tensors.size(), output_names.data(), output_names.size());

        float* output_data = output_tensors[0].GetTensorMutableData<float>();
        size_t output_size = output_tensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
        std::vector<float> predictions(output_data, output_data + output_size);

        for (size_t i = 0; i < output_size; i++) {
            if (!std::isfinite(predictions[i])) {
                ATH_MSG_ERROR("Non-finite prediction detected! Setting to zero.");
                predictions[i] = 0.0f;
            }
        }

        ATH_MSG_DEBUG("ONNX output size: " << output_size);
        ATH_MSG_DEBUG("First 5 predictions:");
        for (size_t i = 0; i < std::min(output_size, size_t(5)); i++) {
            ATH_MSG_DEBUG("Prediction[" << i << "]: " << predictions[i]);
        }

        std::vector<int> binary_predictions(output_size);
        for (size_t i = 0; i < output_size; i++) {
            binary_predictions[i] = (predictions[i] > 0.05) ? 1 : 0;
        }
        ATH_MSG_DEBUG("First 5 Binary Predictions:");
        for (size_t i = 0; i < std::min(output_size, size_t(5)); i++) {
            ATH_MSG_DEBUG("Binary_Prediction[" << i << "]: " << binary_predictions[i]);
        }

        if (Labels) {
            float loss = 0;
            float accuracy = 0;
            for (size_t i = 0; i < output_size; i++) {
                if (binary_predictions[i] != labels[i] && labels[i] == 1) {
                    loss++;
                }
                if (binary_predictions[i] == labels[i]) {
                    accuracy++;
                }
            }
            ATH_MSG_ALWAYS("Hits:     " << output_size);
            ATH_MSG_ALWAYS("Loss:     " << loss / output_size);
            ATH_MSG_ALWAYS("Accuracy: " << accuracy / output_size);
        }

        for (size_t i = 0; i < output_size; i++) {
            m_spoint_predictions.push_back(binary_predictions[i]);
        }

        if (!m_tree.fill(ctx)) return StatusCode::FAILURE;
        return StatusCode::SUCCESS;

    }

    CLHEP::HepRandomEngine* SPIdentifierAlg::getRandomEngine(const EventContext&ctx) const {
        ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_streamName);
        std::string rngName = m_streamName;
        rngWrapper->setSeed(rngName, ctx);
        return rngWrapper->getEngine(ctx);
    }

}
