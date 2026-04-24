/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "EGammaVariableCorrection/ElectronPhotonVariableNFCorrectionTool.h"

#include "EgammaAnalysisHelpers/AsgEGammaConfigHelper.h"
#include "PathResolver/PathResolver.h"

#include "AthOnnxInterfaces/IOnnxRuntimeInferenceTool.h"

#include "TEnv.h"
#include "TString.h"

#include <cmath>
#include <algorithm>



// Ordered list of shower shapes used by the tool
// The order must match the ONNX model inputs and outputs
const std::vector<std::string> ElectronPhotonVariableNFCorrectionTool::s_ssVarNames = {
    "weta2", "weta1", "Rphi", "Reta", "wtots1", "Rhad",  "Rhad1", "f1", "fracs1", "DeltaE", "Eratio"
};

// Mapping of shower shapes to xAOD enums (same order as s_ssVarNames)
const std::vector<xAOD::EgammaParameters::ShowerShapeType> ElectronPhotonVariableNFCorrectionTool::s_ssEnums = {
    xAOD::EgammaParameters::weta2,
    xAOD::EgammaParameters::weta1,
    xAOD::EgammaParameters::Rphi,
    xAOD::EgammaParameters::Reta,
    xAOD::EgammaParameters::wtots1,
    xAOD::EgammaParameters::Rhad,
    xAOD::EgammaParameters::Rhad1,
    xAOD::EgammaParameters::f1,
    xAOD::EgammaParameters::fracs1,
    xAOD::EgammaParameters::DeltaE,
    xAOD::EgammaParameters::Eratio
};


// Constructor, declares properties
ElectronPhotonVariableNFCorrectionTool::ElectronPhotonVariableNFCorrectionTool(const std::string& name) :
    AsgTool(name)
{}

// Select fold index based on event number (and optionally pT)
int ElectronPhotonVariableNFCorrectionTool::selectFold(unsigned long long eventNumber, float phi) const
{
    if(m_forceOneFold) return 0;
    if (m_nFolds <= 1) return 0;

    unsigned long long key = eventNumber;

    if (m_foldStrategy == FoldStrategy::EventNumberPhi) {
        const long long phiBin = static_cast<long long>(std::floor((phi + static_cast<float>(M_PI)) * 100.0f));
        key = eventNumber + static_cast<unsigned long long>(phiBin);
    }

    return static_cast<int>(key % m_nFolds);
}

// Convert string from config to fold strategy
ElectronPhotonVariableNFCorrectionTool::FoldStrategy
ElectronPhotonVariableNFCorrectionTool::parseFoldStrategy(const std::string& s) const
{
    if (s == "eventNumber") return FoldStrategy::EventNumber;
    if (s == "eventNumber_phi") return FoldStrategy::EventNumberPhi;
    ATH_MSG_WARNING("Unknown FoldStrategy '" << s << "'");
    return FoldStrategy::Unknown;
}

bool ElectronPhotonVariableNFCorrectionTool::passSelectionCuts(
    const xAOD::Photon& photon,
    const std::vector<float>& ss) const
{
    // pT cut
    if (photon.pt() < m_pTcutMeV) return false;

    // TruthType cut
    if (m_applyToMode == ApplyToMode::TruthPhotons) {
        static const SG::AuxElement::Accessor<int> acc_truthType("truthType");
        if (!acc_truthType.isAvailable(photon)) {
            ATH_MSG_WARNING("ApplyTo = TruthPhotons but truthType not available — skipping photon");
            return false;
        }
        int truthType = acc_truthType(photon);
        if (truthType < 13 || truthType > 15) return false;
    }

    // Shower shape cuts
    if (m_applyShowerShapeCuts) {
        // weta2
        if (ss[0] <= -10.f || ss[0] >= 10.f) return false;
        // weta1
        if (ss[1] <= -10.f || ss[1] >= 10.f) return false;
        // Rphi
        if (ss[2] <= -10.f || ss[2] >= 10.f) return false;
        // Reta
        if (ss[3] <= -10.f || ss[3] >= 10.f) return false;
        // wtots1
        if (ss[4] < -2.f || ss[4] >= 10.f) return false;
        // Rhad
        if (ss[5] < -2.f || ss[5] > 2.f) return false;
        // Rhad1
        if (ss[6] < -2.f || ss[6] > 2.f) return false;
        // f1
        if (ss[7] <= -2.f || ss[7] >= 2.f) return false;
        // fracs1
        if (ss[8] <= -2.f || ss[8] >= 5.f) return false;
        // DeltaE
        if (ss[9] < 0.f || ss[9] >= 5000.f) return false;
        // Eratio
        if (ss[10] < 0.f || ss[10] > 1.f) return false;
    }

    return true;
}



// Initialize tool: read config, setup ONNX tools and accessors
StatusCode ElectronPhotonVariableNFCorrectionTool::initialize()
{
    if (m_configFile.empty()) {
        ATH_MSG_ERROR("ConfigFile property is empty. Please provide a config file to the tool.");
        return StatusCode::FAILURE;
    }

    std::string resolvedConfig = PathResolverFindCalibFile(m_configFile);
    if (resolvedConfig.empty()) {
        ATH_MSG_ERROR("Failed to resolve config file \"" << m_configFile << "\"");
        return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Use configuration file " << m_configFile);

    TEnv env;
    env.ReadFile(resolvedConfig.c_str(), kEnvLocal);
    env.IgnoreDuplicates(false);

    m_nFolds = (m_forceOneFold)?1:env.GetValue("NFolds", 0);
    if (m_nFolds <= 0) {
        ATH_MSG_ERROR("NFolds not set or invalid in config: " << resolvedConfig);
        return StatusCode::FAILURE;
    }

    TString pattern = env.GetValue("ONNXnamePattern", "");
    if (pattern.IsNull()) {
        ATH_MSG_ERROR("ONNXnamePattern not set in config: " << resolvedConfig);
        return StatusCode::FAILURE;
    }
    m_onnxPattern = pattern.Data();


    TString fs = env.GetValue("FoldStrategy", "eventNumber");
    std::string fsStr = fs.Data();

    m_foldStrategy = parseFoldStrategy(fsStr);

    if (m_foldStrategy == FoldStrategy::Unknown) {
        ATH_MSG_ERROR("FoldStrategy must be 'eventNumber' or 'eventNumber_phi', but got '" << fsStr << "' in config: " << resolvedConfig);
        return StatusCode::FAILURE;
    }


    ATH_MSG_VERBOSE("NFolds = " << m_nFolds << ", pattern = " << m_onnxPattern << ", FoldStrategy = " << fsStr);

    if (static_cast<int>(m_onnxToolsForward.size()) != m_nFolds ||
        static_cast<int>(m_onnxToolsBackward.size()) != m_nFolds) {
        ATH_MSG_ERROR("Expected "<<m_nFolds<<" forward/backward tools, "<< "but got "<<m_onnxToolsForward.size()<<" / "<< m_onnxToolsBackward.size());
        return StatusCode::FAILURE;
    }


    if (m_applyToStr == "TruthPhotons") m_applyToMode = ApplyToMode::TruthPhotons;
    else if (m_applyToStr == "All") m_applyToMode = ApplyToMode::All;
    else {
        ATH_MSG_ERROR("ApplyTo must be TruthPhotons or All, but got '" << m_applyToStr << "'");
        return StatusCode::FAILURE;
    }

    // Cuts on SS vars to remove default values
    m_applyShowerShapeCuts = (env.GetValue("ApplyShowerShapeCuts", 1) == 1);

    ATH_MSG_INFO("ApplyTo = " << m_applyToStr << ", pTcut=" << m_pTcutMeV << " MeV, ApplyShowerShapeCuts=" << m_applyShowerShapeCuts);


    ATH_CHECK(m_onnxToolsForward.retrieve());
    ATH_CHECK(m_onnxToolsBackward.retrieve());

    if (msgLvl(MSG::DEBUG)) {
        for (int i = 0; i < m_nFolds; ++i) {
            ATH_MSG_VERBOSE("Fold " << i << " forward model info:");
            m_onnxToolsForward[i]->printModelInfo();
            ATH_MSG_VERBOSE("Fold " << i << " backward model info:");
            m_onnxToolsBackward[i]->printModelInfo();
        }
    }

    // Prepare decorations for each shower shape
    m_accessors.resize(s_ssVarNames.size());
    for (size_t i = 0; i < s_ssVarNames.size(); ++i) {
        const std::string& var = s_ssVarNames[i];
            m_accessors[i].original = std::make_unique<SG::AuxElement::Accessor<float>>(var + "_original");
    }

    ATH_CHECK(m_eventInfoKey.initialize());

    ATH_MSG_INFO("NF correction tool initialized with " << m_nFolds << " folds. ");

    return StatusCode::SUCCESS;
}


// Apply NF correction to photon shower shapes.
const CP::CorrectionCode ElectronPhotonVariableNFCorrectionTool::applyCorrection(xAOD::Photon& photon) const
{

    const size_t nSS = s_ssEnums.size();
    std::vector<float> ss(nSS);

    // Read shower shapes, then store original values
    for (size_t i = 0; i < nSS; ++i) {
        ss[i] = photon.showerShapeValue(s_ssEnums[i]);
        (*m_accessors[i].original)(photon) = ss[i];
    }


    static const SG::AuxElement::Decorator<char> dec_pass("NFCorrectedShowerShapes");

    // Photon selection
    bool pass = passSelectionCuts(photon, ss);

    dec_pass(photon) = pass ? 1 : 0;

    if (!pass) {
        // If selection is not passed, then SS value will be same to original
        return CP::CorrectionCode::Ok;
    }

    
    // Get event info and select fold
    SG::ReadHandle<xAOD::EventInfo> h(m_eventInfoKey);
    if (!h.isValid()) {
        ATH_MSG_ERROR("Failed to read EventInfo via key " << m_eventInfoKey.key());
        return CP::CorrectionCode::Error;
    }

    const unsigned long long eventNumber = h->eventNumber();
    float ptGeV = photon.pt() / 1000.0f;
    const float phi = static_cast<float>(photon.phi());
    const int fold = selectFold(eventNumber, phi);
 

    if (fold < 0 || fold >= m_nFolds) {
        ATH_MSG_ERROR("Selected fold " << fold << " out of range [0," << (m_nFolds-1) << "]");
        return CP::CorrectionCode::Error;
    }


    // Kinematic inputs
    const bool isConv = photon.conversionType() != xAOD::EgammaParameters::unconverted;
    std::vector<float> kinematic = {
        ptGeV,
        static_cast<float>(photon.eta()),
        static_cast<float>(photon.phi()),
        static_cast<float>(isConv)
    };

    // Forward inference
    std::vector<Ort::Value> inputTensors;

    const auto& onnxToolForward = m_onnxToolsForward[fold];

    // index 0 is for kinematics
    int64_t batchSizeKin = onnxToolForward->getBatchSize(
        static_cast<int64_t>(kinematic.size()), 0);
    if (onnxToolForward->addInput(inputTensors, kinematic, 0, batchSizeKin).isFailure()) {
        ATH_MSG_ERROR("Fold " << fold << ": failed to add kinematic input tensor");
        return CP::CorrectionCode::Error;
    }

    // index 1 is for shower shape varibales
    int64_t batchSizeSS = onnxToolForward->getBatchSize(
        static_cast<int64_t>(ss.size()), 1);
    if (onnxToolForward->addInput(inputTensors, ss, 1, batchSizeSS).isFailure()) {
        ATH_MSG_ERROR("Fold " << fold << ": failed to add shower shape input tensor");
        return CP::CorrectionCode::Error;
    }

    std::vector<Ort::Value> outputTensors;
    std::vector<float> outputData;
    if (onnxToolForward->addOutput(outputTensors, outputData, 0, batchSizeKin).isFailure()) {
        ATH_MSG_ERROR("Fold " << fold << ": failed to add forward output tensor");
        return CP::CorrectionCode::Error;
    }

    if (onnxToolForward->inference(inputTensors, outputTensors).isFailure()) {
        ATH_MSG_ERROR("Fold " << fold << ": forward inference failed");
        return CP::CorrectionCode::Error;
    }

    float* zPtr = outputTensors[0].GetTensorMutableData<float>();
    std::vector<float> zVec(zPtr, zPtr + nSS);


    // Backward inference
    std::vector<Ort::Value> inputTensorsBack;
    std::vector<Ort::Value> outputTensorsBack;
    std::vector<float> outputDataBack;

    const auto& onnxToolBackward = m_onnxToolsBackward[fold];

    // index 0 is for kinematics
    int64_t batchSizeKinBack = onnxToolBackward->getBatchSize(
        static_cast<int64_t>(kinematic.size()), 0);
    if (onnxToolBackward->addInput(inputTensorsBack, kinematic, 0, batchSizeKinBack).isFailure()) {
        ATH_MSG_ERROR("Fold " << fold << ": failed to add kinematic input tensor for backward model");
        return CP::CorrectionCode::Error;
    }

    // index 1 is for shower shapes in latent space
    int64_t batchSizeZBack = onnxToolBackward->getBatchSize(static_cast<int64_t>(zVec.size()), 1);

    if (onnxToolBackward->addInput(inputTensorsBack, zVec, 1, batchSizeZBack).isFailure()) {
        ATH_MSG_ERROR("Fold " << fold << ": failed to add z input tensor for backward model");
        return CP::CorrectionCode::Error;
    }

    // index 2 is for original shower shapes (models use them to cut on std values [-5, 5])
    if (onnxToolBackward->addInput(inputTensorsBack, ss, 2, batchSizeKinBack).isFailure()) {
        ATH_MSG_ERROR("Fold " << fold << ": failed to add original SS input tensor for backward model");
        return CP::CorrectionCode::Error;
    }

    if (onnxToolBackward->addOutput(outputTensorsBack, outputDataBack, 0, batchSizeZBack).isFailure()) {
        ATH_MSG_ERROR("Fold " << fold << ": failed to add backward output tensor");
        return CP::CorrectionCode::Error;
    }

    if (onnxToolBackward->inference(inputTensorsBack, outputTensorsBack).isFailure()) {
        ATH_MSG_ERROR("Fold " << fold << ": backward inference failed");
        return CP::CorrectionCode::Error;
    }

    const auto infoB  = outputTensorsBack[0].GetTensorTypeAndShapeInfo();
    const auto nElB = infoB.GetElementCount();
    if (nElB != nSS) {
        ATH_MSG_ERROR("Fold "<<fold <<": backward output has " <<nElB<< " elements, expected "<<nSS);
        return CP::CorrectionCode::Error;
    }

    // Write corrected shower shapes
    float* corrPtr = outputTensorsBack[0].GetTensorMutableData<float>();
    for (size_t i = 0; i < nSS; ++i) {
        photon.setShowerShapeValue(corrPtr[i], s_ssEnums[i]);
    }

    ATH_MSG_DEBUG("NF correction applied successfully");


    return CP::CorrectionCode::Ok;
}

// Electrons are not supported.
const CP::CorrectionCode ElectronPhotonVariableNFCorrectionTool::applyCorrection(xAOD::Electron&) const
{
    ATH_MSG_ERROR("ElectronPhotonVariableNFCorrectionTool does not support electrons.");
    return CP::CorrectionCode::Error;
}

// Create corrected copy of photon.
const CP::CorrectionCode ElectronPhotonVariableNFCorrectionTool::correctedCopy(const xAOD::Photon& in_photon,
                                                                         xAOD::Photon*& out_photon) const
{

    out_photon = new xAOD::Photon(in_photon);
    return applyCorrection(*out_photon);
}

// Create copy of electron (no correction).
const CP::CorrectionCode ElectronPhotonVariableNFCorrectionTool::correctedCopy(const xAOD::Electron& in_electron,
                                                                         xAOD::Electron*& out_electron) const
{
    ATH_MSG_ERROR("ElectronPhotonVariableNFCorrectionTool cannot correct electrons.");
    out_electron = new xAOD::Electron(in_electron);
    return CP::CorrectionCode::Error;
}
