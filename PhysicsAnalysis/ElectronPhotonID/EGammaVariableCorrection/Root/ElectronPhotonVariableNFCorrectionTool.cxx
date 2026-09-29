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

bool ElectronPhotonVariableNFCorrectionTool::passPhotonSelection(const xAOD::Photon& photon) const
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

    return true;
}

bool ElectronPhotonVariableNFCorrectionTool::passShowerShapeCuts(const std::vector<float>& ss) const
{
    if (!m_applyShowerShapeCuts) return true;

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

    return true;
}

CP::CorrectionCode ElectronPhotonVariableNFCorrectionTool::applyFallbackFudge(
    xAOD::Photon& photon,
    const std::vector<float>& ss) const
{
    if (m_fallbackFudgeTool->applyCorrection(photon) != CP::CorrectionCode::Ok) {
        ATH_MSG_ERROR("Fallback fudge tool failed to correct photon");
        return CP::CorrectionCode::Error;
    }

    // Keep default values of weta1 (index 1) and wtots1 (index 4) untouched
    for (size_t i : {size_t(1), size_t(4)}) {
        if (ss[i] < s_defaultValueThreshold) {
            photon.setShowerShapeValue(ss[i], s_ssEnums[i]);
        }
    }
    // Keep fracs1 (index 8) = 0 untouched (fudging smears it)
    if (ss[8] == 0.f) {
        photon.setShowerShapeValue(0.f, s_ssEnums[8]);
    }

    ATH_MSG_DEBUG("Photon failed shower shape cuts: fallback fudge correction applied");
    return CP::CorrectionCode::Ok;
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

    const int nFoldsConfig = env.GetValue("NFolds", 0);
    if (nFoldsConfig <= 0) {
        ATH_MSG_ERROR("NFolds not set or invalid in config: " << resolvedConfig);
        return StatusCode::FAILURE;
    }

    if (m_nFoldsOverride > 0) {
        if (m_nFoldsOverride > nFoldsConfig) {
            ATH_MSG_ERROR("NFoldsOverride (" << m_nFoldsOverride.value() << ") exceeds NFolds in config (" << nFoldsConfig << ")");
            return StatusCode::FAILURE;
        }
        m_nFolds = m_nFoldsOverride;
    } else {
        m_nFolds = nFoldsConfig;
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

    if (!m_fallbackFudgeTool.empty()) {
        ATH_CHECK(m_fallbackFudgeTool.retrieve());
        ATH_MSG_INFO("Photons failing shower shape cuts will be corrected with fallback fudge tool " << m_fallbackFudgeTool.name());
    }

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
    static const SG::AuxElement::Decorator<char> dec_fudged("FallbackFudgedShowerShapes");

    // Photon selection
    const bool passPhoton = passPhotonSelection(photon);
    const bool passSS = passPhoton && passShowerShapeCuts(ss);
    const bool fallback = passPhoton && !passSS && !m_fallbackFudgeTool.empty();

    dec_pass(photon) = passSS ? 1 : 0;
    dec_fudged(photon) = fallback ? 1 : 0;

    if (fallback) {
        // NF is not applied because of the shower shape cuts, so use fudging instead
        return applyFallbackFudge(photon, ss);
    }

    if (!passSS) {
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

    // Keep fracs1 (index 8) = 0 untouched (NF smears it)
    if (ss[8] == 0.f) {
        photon.setShowerShapeValue(0.f, s_ssEnums[8]);
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
