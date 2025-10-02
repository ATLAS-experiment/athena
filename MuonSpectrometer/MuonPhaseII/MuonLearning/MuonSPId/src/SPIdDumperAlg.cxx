/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SPIdDumperAlg.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "Identifier/Identifier.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadHandle.h"
#include "MuonTesterTree/EventHashBranch.h"
#include "MuonSpacePoint/SpacePointPerLayerSplitter.h"
#include "MuonInferenceInterfaces/GraphData.h"

#include "xAODMuonPrepData/MdtDriftCircle.h"

#include <fstream>

namespace MuonR4 {

    StatusCode SPIdDumperAlg::initialize() {
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_inSegmentKey.initialize());
        ATH_CHECK(m_tree.init(this));
        ATH_CHECK(m_graphFilterTool.retrieve());

        ATH_MSG_DEBUG("Successfully initialized SPIdDumperAlg with ONNX model filtering.");
        return StatusCode::SUCCESS;
    }

    StatusCode SPIdDumperAlg::execute() {
        const EventContext& ctx{Gaudi::Hive::currentContext()};

        SG::ReadHandle<SpacePointContainer> readHandle{m_readKey, ctx};
        if (!readHandle.isValid()) {
            ATH_MSG_ERROR("Failed to retrieve SpacePointContainer from StoreGate.");
            return StatusCode::FAILURE;
        }

        MuonML::GraphRawData graphData;
        ATH_CHECK(m_graphFilterTool->runGraphInference(ctx, graphData));

        if (graphData.graph->dataTensor.size() < 3) {
            ATH_MSG_DEBUG("ONNX inference output tensor is missing.");
            return StatusCode::SUCCESS;
        }

        const float* predictions = graphData.graph->dataTensor[2].GetTensorMutableData<float>();
        size_t predictionIndex   = 0;

        size_t totalNodes           = graphData.graph->dataTensor[2].GetTensorTypeAndShapeInfo().GetElementCount();
        size_t totalFeatureElements = graphData.graph->dataTensor[0].GetTensorTypeAndShapeInfo().GetElementCount();

        if (totalNodes == 0) {
            ATH_MSG_ERROR("Total number of nodes is zero! Cannot divide.");
            return StatusCode::FAILURE;
        }

        size_t numFeaturesPerNode = totalFeatureElements / totalNodes;
        ATH_MSG_DEBUG("Inferred " << numFeaturesPerNode << " features per node from tensor of size " 
                    << totalFeatureElements << " and " << totalNodes << " nodes.");

        std::unordered_map <const SpacePointBucket*, std::vector<const MuonR4::Segment*>> segmentMap;
        SG::ReadHandle readSegment(m_inSegmentKey, ctx);
        ATH_CHECK(readSegment.isPresent());
        for (const MuonR4::Segment* segment : *readSegment) {
            segmentMap[segment->parent()->parentBucket()].push_back(segment);
        }

        for (const MuonR4::SpacePointBucket* bucket : *readHandle) {

            std::unordered_map<const SpacePoint*, std::vector<int16_t>> spacePointToSegment;
            auto match_itr = segmentMap.find(bucket);
            if (match_itr != segmentMap.end()) {
                unsigned int segIdx{0};
                for (const MuonR4::Segment* segment : match_itr->second) {
                    for (const auto& meas : segment->measurements()) {
                        spacePointToSegment[meas->spacePoint()].push_back(segIdx);
                    }
                    ++segIdx;
                }
            }

            SpacePointPerLayerSplitter splitter{*bucket};
            unsigned int layer{0};

            for (const auto& hitsInLay : splitter.mdtHits()) {
                for (const auto sp : hitsInLay){

                    const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(sp->primaryMeasurement());
                    if (dc->status() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime){
                        continue;
                    }
                    m_spoint_x.push_back(sp->localPosition().x());
                    m_spoint_y.push_back(sp->localPosition().y());
                    m_spoint_z.push_back(sp->localPosition().z());
                    m_spoint_driftR.push_back(sp->driftRadius());
                    m_spoint_station.push_back(m_idHelperSvc->stationName(sp->identify()));
                    m_spoint_layer.push_back(layer);

                    if (spacePointToSegment.count(sp) > 0) {
                        m_spoint_label.push_back(1);
                    } else {
                        m_spoint_label.push_back(0);
                    }

                    if (predictionIndex < graphData.graph->dataTensor[2].GetTensorTypeAndShapeInfo().GetElementCount()) {
                        m_spoint_predictions.push_back(predictions[predictionIndex]);
                        predictionIndex++;
                    } else {
                        ATH_MSG_WARNING("Prediction index exceeded ONNX output size.");
                        m_spoint_predictions.push_back(-999);  // Default invalid value
                    }


                }
                ++layer;

            }

            for (const auto& hitsInLay : splitter.stripHits()) {

                for (const auto sp : hitsInLay){

                    m_spoint_x.push_back(sp->localPosition().x());
                    m_spoint_y.push_back(sp->localPosition().y());
                    m_spoint_z.push_back(sp->localPosition().z());
                    m_spoint_driftR.push_back(sp->driftRadius());
                    m_spoint_station.push_back(m_idHelperSvc->stationName(sp->identify()));
                    m_spoint_layer.push_back(layer);

                    if (spacePointToSegment.count(sp) > 0) {
                        m_spoint_label.push_back(1);
                    } else {
                        m_spoint_label.push_back(0);
                    }

                    if (predictionIndex < graphData.graph->dataTensor[2].GetTensorTypeAndShapeInfo().GetElementCount()) {
                        m_spoint_predictions.push_back(predictions[predictionIndex]);
                        predictionIndex++;
                    } else {
                        ATH_MSG_WARNING("Prediction index exceeded ONNX output size.");
                        m_spoint_predictions.push_back(-999);  // Default invalid value
                    }

                }
                ++layer;

            }

        }

        if (!m_tree.fill(ctx)) return StatusCode::FAILURE;
        return StatusCode::SUCCESS;
    }

    StatusCode SPIdDumperAlg::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }

} 
