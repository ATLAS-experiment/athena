/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloClusterCorrection/CaloClusterMLCalibToolLite.h"
#include "CaloClusterCorrection/CaloClusterMLGaussianMixture.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "StoreGate/WriteDecorHandle.h"

#include "GaudiKernel/SystemOfUnits.h"

CaloClusterMLCalibToolLite::CaloClusterMLCalibToolLite(const std::string &type, const std::string &name, const IInterface *parent) : base_class(type, name, parent) {}

CaloClusterMLCalibToolLite::~CaloClusterMLCalibToolLite() {}

StatusCode CaloClusterMLCalibToolLite::initialize()
{
    ATH_MSG_DEBUG("Initializing " << name() << "...");
    m_numFeatures = m_preprocessingTransformNames.size();
    for (int i = 0; i < m_numFeatures; i++)
    {
        PreprocessTransform transform;
        auto funcIt = CaloClusterMLCalib::TRANSFORMATIONS.find(m_preprocessingTransformNames[i]);
        if (funcIt != CaloClusterMLCalib::TRANSFORMATIONS.end())
        {
            transform.processor = funcIt->second;
            std::vector<float> floatParams;
            for (double param : m_preprocessingTransformParams[i])
            {
                floatParams.push_back(static_cast<float>(param));
            }
            transform.parameters = std::move(floatParams);
            m_featurePreprocessingTransforms.push_back(std::move(transform));
        }
        else
        {
            ATH_MSG_WARNING("Undefined transformation " << m_preprocessingTransformNames[i]);
            return StatusCode::FAILURE;
        }
    }

    ATH_CHECK(m_onnxTool.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode CaloClusterMLCalibToolLite::inference(const xAOD::CaloClusterContainer &clusters, const int &nPrimVtx, const double &avgMu, std::vector<double> &clusterE_ML_vec, std::vector<double> &clusterE_ML_Unc_vec) const
{
    ATH_MSG_DEBUG("Executing " << name() << "...");

    double clusterE = 0;
    double clusterEta = 0;
    double cluster_SIGNIFICANCE = 0;
    double cluster_time = 0;
    double cluster_SECOND_TIME = 0;
    double cluster_CENTER_LAMBDA = 0;
    double cluster_CENTER_MAG = 0;
    double cluster_ENG_FRAC_EM_INCL = 0;
    double cluster_FIRST_ENG_DENS = 0;
    double cluster_LONGITUDINAL = 0;
    double cluster_LATERAL = 0;
    double cluster_PTD = 0;
    double cluster_ISOLATION = 0;

    std::vector<float> transformedFeatures;
    bool ok{}; // for checking return value of cluster->retrieveMoment

    for (const xAOD::CaloCluster *cluster : clusters)
    {
        clusterE = cluster->e(xAOD::CaloCluster::UNCALIBRATED) / Gaudi::Units::GeV;
        clusterEta = cluster->eta(xAOD::CaloCluster::UNCALIBRATED);
        ok = cluster->retrieveMoment(xAOD::CaloCluster::MomentType::SIGNIFICANCE, cluster_SIGNIFICANCE);
        ok &= cluster->retrieveMoment(xAOD::CaloCluster::MomentType::SECOND_TIME, cluster_SECOND_TIME);
        cluster_SECOND_TIME /= (Gaudi::Units::nanosecond * Gaudi::Units::nanosecond);
        ok &= cluster->retrieveMoment(xAOD::CaloCluster::MomentType::CENTER_LAMBDA, cluster_CENTER_LAMBDA);
        cluster_CENTER_LAMBDA /= Gaudi::Units::millimeter;
        ok &= cluster->retrieveMoment(xAOD::CaloCluster::MomentType::CENTER_MAG, cluster_CENTER_MAG);
        ok &= cluster->retrieveMoment(xAOD::CaloCluster::MomentType::FIRST_ENG_DENS, cluster_FIRST_ENG_DENS);
        cluster_FIRST_ENG_DENS /= (Gaudi::Units::GeV / Gaudi::Units::millimeter3);
        ok &= cluster->retrieveMoment(xAOD::CaloCluster::MomentType::LONGITUDINAL, cluster_LONGITUDINAL);
        ok &= cluster->retrieveMoment(xAOD::CaloCluster::MomentType::LATERAL, cluster_LATERAL);
        ok &= cluster->retrieveMoment(xAOD::CaloCluster::MomentType::PTD, cluster_PTD);
        ok &= cluster->retrieveMoment(xAOD::CaloCluster::MomentType::ISOLATION, cluster_ISOLATION);
        cluster_time = cluster->time() / Gaudi::Units::nanosecond;

        if (!ok) {
            ATH_MSG_ERROR("retrieveMoment() failed for " << cluster);
            return StatusCode::FAILURE;
        }

        float e_EM = 0.0;
        for (size_t s = CaloSampling::PreSamplerB; s < CaloSampling::Unknown; s++)
        {
            if (s == CaloSampling::EMB1 || s == CaloSampling::EMB2 || s == CaloSampling::EMB3 || s == CaloSampling::EME1 || s == CaloSampling::EME2 || s == CaloSampling::EME3 || s == CaloSampling::FCAL0)
            {
                e_EM += cluster->eSample(static_cast<xAOD::CaloCluster::CaloSample>(s));
            }
        }
        cluster_ENG_FRAC_EM_INCL = e_EM / cluster->rawE();

        std::vector<float> rawValues;

        rawValues.push_back(clusterE);
        rawValues.push_back(clusterEta);
        rawValues.push_back(cluster_SIGNIFICANCE);
        rawValues.push_back(cluster_time);
        rawValues.push_back(cluster_SECOND_TIME);
        rawValues.push_back(cluster_CENTER_LAMBDA);
        rawValues.push_back(cluster_CENTER_MAG);
        rawValues.push_back(cluster_ENG_FRAC_EM_INCL);
        rawValues.push_back(cluster_FIRST_ENG_DENS);
        rawValues.push_back(cluster_LONGITUDINAL);
        rawValues.push_back(cluster_LATERAL);
        rawValues.push_back(cluster_PTD);
        rawValues.push_back(cluster_ISOLATION);
        rawValues.push_back(nPrimVtx);
        rawValues.push_back(avgMu);

        for (int i = 0; i < m_numFeatures; i++)
        {
            const PreprocessTransform &transform = m_featurePreprocessingTransforms.at(i);
            const float &raw = rawValues.at(i);
            float transformed = transform.processor(raw, transform.parameters);
            transformedFeatures.push_back(transformed);
        }
    }

    int numClusters = clusters.size();
    std::vector<int64_t> inputShape = {numClusters, 15};

    AthInfer::InputDataMap inputData;
    inputData["features"] = std::make_pair(
        inputShape, std::move(transformedFeatures));

    AthInfer::OutputDataMap outputData;

    outputData["mus"] = std::make_pair(
        std::vector<int64_t>{numClusters, 3}, std::vector<float>{});
    outputData["sigmas"] = std::make_pair(
        std::vector<int64_t>{numClusters, 3}, std::vector<float>{});
    outputData["alphas"] = std::make_pair(
        std::vector<int64_t>{numClusters, 3}, std::vector<float>{});

    ATH_CHECK(m_onnxTool->inference(inputData, outputData));

    std::vector<float> &onnx_mus = std::get<std::vector<float>>(outputData["mus"].second);
    std::vector<float> &onnx_sigma2s = std::get<std::vector<float>>(outputData["sigmas"].second);
    std::vector<float> &onnx_alphas = std::get<std::vector<float>>(outputData["alphas"].second);

    int nan_in_mus = 0;
    int nan_in_sigma2s = 0;
    int nan_in_alphas = 0;

    for (float val : onnx_mus)
        if (std::isnan(val))
            nan_in_mus++;

    for (float val : onnx_sigma2s)
        if (std::isnan(val))
            nan_in_sigma2s++;

    for (float val : onnx_alphas)
        if (std::isnan(val))
            nan_in_alphas++;

    if (nan_in_mus > 0)
        ATH_MSG_WARNING(nan_in_mus << " NaN value found in `mus` output layer during ONNX inference");

    if (nan_in_sigma2s)
        ATH_MSG_WARNING(nan_in_sigma2s << " NaN value found in `sigmas` output layer during ONNX inference");

    if (nan_in_alphas)
        ATH_MSG_WARNING(nan_in_alphas << " NaN value found in `alphas` output layer during ONNX inference");

    clusterE_ML_vec.clear();
    clusterE_ML_Unc_vec.clear();
    clusterE_ML_vec.reserve(numClusters);
    clusterE_ML_Unc_vec.reserve(numClusters);

    for (int i = 0; i < numClusters; ++i)
    {
        std::vector<float> current_mus = {onnx_mus[i * 3], onnx_mus[i * 3 + 1], onnx_mus[i * 3 + 2]};
        std::vector<float> current_sigma2s = {onnx_sigma2s[i * 3], onnx_sigma2s[i * 3 + 1], onnx_sigma2s[i * 3 + 2]};
        std::vector<float> current_alphas = {onnx_alphas[i * 3], onnx_alphas[i * 3 + 1], onnx_alphas[i * 3 + 2]};

        float mode = CaloClusterMLCalib::modes(current_mus, current_sigma2s, current_alphas);
        float r = std::pow(10, mode);
        float onnx_s = CaloClusterMLCalib::sigma_stoch(current_mus, current_sigma2s, current_alphas);
        float s = std::abs(std::log(10) * r) * onnx_s;

        if (!std::isfinite(r) || std::abs(r) < 1e-6) {
            ATH_MSG_WARNING("ML-correction factor to cluster energy (used as denominator) is " << r << "; The ML-correction factor is reset to 1. Uncertainty is set to 0.");
            r = 1.0;
            s = 0.0;
        }
        
        const double cluster_energy = clusters[i]->e(xAOD::CaloCluster::UNCALIBRATED) / static_cast<double>(r);
        
        clusterE_ML_vec.push_back(cluster_energy);
        clusterE_ML_Unc_vec.push_back(static_cast<double>(s));
    }

    return StatusCode::SUCCESS;
}

StatusCode CaloClusterMLCalibToolLite::finalize()
{
    ATH_MSG_DEBUG("Finalizing " << name() << "...");
    return StatusCode::SUCCESS;
}
