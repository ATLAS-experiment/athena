/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCLUSTERCORRECTION_CALOCLUSTERMLCALIBTOOLLITE_H
#define CALOCLUSTERCORRECTION_CALOCLUSTERMLCALIBTOOLLITE_H

#include "CaloUtils/CaloClusterProcessor.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "AthOnnxInterfaces/IAthInferenceTool.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "CaloClusterCorrection/ICaloClusterMLCalibToolLite.h"
#include "CaloClusterCorrection/CaloClusterMLCalibFeatureTransform.h"

struct PreprocessTransform
{
    CaloClusterMLCalib::TransformFunc processor;
    std::vector<float> parameters;
};

class CaloClusterMLCalibToolLite : public extends<AthAlgTool, ICaloClusterMLCalibToolLite>
{

public:
    CaloClusterMLCalibToolLite(const std::string &type, const std::string &name, const IInterface *parent);
    ~CaloClusterMLCalibToolLite();

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    // Perform batch-inference for a CaloClusterContainer
    virtual StatusCode inference(const xAOD::CaloClusterContainer &clusters, const int &nPrimVtx, const double &avgMu, std::vector<double> &clusterE_ML_vec, std::vector<double> &clusterE_ML_Unc_vec) const override;

private:
    Gaudi::Property<std::vector<std::string>> m_preprocessingTransformNames{this, "PreprocessingTransformNames", {}, "Names of preprocessing transforms"};
    Gaudi::Property<std::vector<std::vector<double>>> m_preprocessingTransformParams{this, "PreprocessingTransformParams", {}, "Parameters for preprocessing transforms"};

    int m_numFeatures = 0;
    std::vector<PreprocessTransform> m_featurePreprocessingTransforms;

    ToolHandle<AthInfer::IAthInferenceTool> m_onnxTool{
        this, "ORTInferenceTool", "AthOnnx::OnnxRuntimeInferenceTool"};
};

#endif // CALOCLUSTERCORRECTION_CALOCLUSTERMLCALIBTOOLLITE_H
