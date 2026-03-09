/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ElectronPhotonVariableNFCorrectionTool_H
#define ElectronPhotonVariableNFCorrectionTool_H


/**
 * @class ElectronPhotonVariableNFCorrectionTool
 * @brief Normalizing Flows-based correction tool for photon MC shower shape variables.
 * @details The tool applies a two-step NF mapping using ONNX Runtime inference:
 *   - Forward model: maps shower shape tenzor -> latent representation (z)
 *   - Backward model: maps latent representation (z) -> corrected shower shape tenzor
 *
 * The correction is applied to the photon shower shape variables listed in s_ssVarNames
 * (the order should correspond to order in a model). 
 *
 * The ONNX models are provided per-fold via ToolHandleArray, and the fold is selected
 * event-by-event using EventInfo::eventNumber() (optionally including pT).
 *
 * @author Katerina Kazakova <katerina.kazakova@cern.ch>
 * @date   February 2026
 */


//ATLAS includes
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandleArray.h"
#include <AsgTools/PropertyWrapper.h>
#include "EgammaAnalysisInterfaces/IElectronPhotonShowerShapeFudgeTool.h"

#include "AthOnnxInterfaces/IOnnxRuntimeInferenceTool.h"
#include "AthContainers/AuxElement.h"

#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadHandle.h"
#include "xAODEventInfo/EventInfo.h"

//EDM includes
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"

#include <memory>
#include <vector>
#include <string>

// ===========================================================================
// Class ElectronPhotonVariableNFCorrectionTool
// ===========================================================================

class ElectronPhotonVariableNFCorrectionTool : public asg::AsgTool, virtual public IElectronPhotonShowerShapeFudgeTool
{

ASG_TOOL_CLASS(ElectronPhotonVariableNFCorrectionTool, IElectronPhotonShowerShapeFudgeTool)

public:
    /** @brief Standard constructor
     * @param name Internal name of the tool instance
     */
    ElectronPhotonVariableNFCorrectionTool(const std::string& name);

    //! @brief Standard destructor
    ~ElectronPhotonVariableNFCorrectionTool() {};

    /** @brief Initialize the class instance
     * @details Reads the configuration file set via setProperty("ConfigFile", ...) function
     * and sets up the class instance accordingly.

     * The configuration provides:
     *   - SimulationType: FS/AF3
     *   - NFolds: number of folds (must match number of ONNX tools configured)
     *   - ONNXnamePattern: pattern string (informational; tool arrays are configured externally)
     *   - FoldStrategy: eventNumber or eventNumber_phi
     *   - ApplyTo: Signal/All
     */
    virtual StatusCode initialize() override;

    /** @brief Apply the Normalizing Flow correction to the passed photon
     * @param photon The photon to be corrected
     * @details Reads the configured shower-shape variables from the photon, runs forward and backward
     * ONNX inference for the selected fold, and overwrites the photon shower-shape values with the
     * corrected outputs.
     */
    virtual const CP::CorrectionCode applyCorrection(xAOD::Photon& photon) const override;

    /** @brief Not supported, so electrons cannot be corrected by this tool */
    virtual const CP::CorrectionCode applyCorrection(xAOD::Electron& electron) const override;

    /** @brief Make a corrected copy of the passed photon
     * @param in_photon  The original photon to copy
     * @param out_photon The corrected copy
     */
    virtual const CP::CorrectionCode correctedCopy(const xAOD::Photon& in_photon,
                                                    xAOD::Photon*& out_photon) const override;

    /** @brief Not supported, so electrons cannot be corrected by this tool */
    virtual const CP::CorrectionCode correctedCopy(const xAOD::Electron& in_electron,
                                                    xAOD::Electron*& out_electron) const override;


private:
    //! @brief The configuration file for the tool, application mode and minimum photon pT cut in MeV
    Gaudi::Property<std::string> m_configFile {this, "ConfigFile", "", "The configuration file for Normalizing Flows to use"};
    Gaudi::Property<std::string> m_applyToStr {this, "ApplyTo", "TruthPhotons", "TruthPhotons or All"};
    Gaudi::Property<float>       m_pTcutMeV   {this, "pTcut", 10000.f, "Min photon pT in MeV"};
    
    //! @brief Number of model folds configured (must match tool handle array sizes)
    int m_nFolds{0};

    //! @brief Models path pattern string from config
    std::string m_onnxPattern;
    
    //! @brief ReadHandleKey for EventInfo used for fold selection
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfoKey", "EventInfo", "EventInfo key"};
    
    //! @brief NF will be applied only for TruthPhotons, or for All photons
    enum class ApplyToMode { TruthPhotons, All };
    ApplyToMode m_applyToMode{ApplyToMode::TruthPhotons};

    //! @brief Cuts applied to remove default values of shower shapes
    bool m_applyShowerShapeCuts{true};

    //! @brief Returns true if NF correction should be applied to this photon
    bool passSelectionCuts(const xAOD::Photon& photon, const std::vector<float>& ss) const;
    
    /** @brief Fold selection strategy
    * @details
    *   - EventNumber    : fold = eventNumber % NFolds
    *   - EventNumberPhi : fold = (eventNumber + floor((phi + pi)*100)) % NFolds
    */
    enum class FoldStrategy { Unknown, EventNumber, EventNumberPhi };
    
    //! @brief Selected fold strategy (configured via FoldStrategy in the config)
    FoldStrategy m_foldStrategy{FoldStrategy::EventNumber};
    
    /** @brief Select fold index for the current event/photon
    * @param eventNumber EventInfo::eventNumber()
    * @param phi Photon phi in radians
    * @return Fold index in [0, NFolds-1]
    */
    int selectFold(unsigned long long eventNumber, float phi) const;

    //! @brief Parse fold strategy string from config
    FoldStrategy parseFoldStrategy(const std::string& s) const;
    
    //! @brief ToolHandleArray for forward ONNX models (one tool per fold)
    ToolHandleArray<AthOnnx::IOnnxRuntimeInferenceTool> m_onnxToolsForward{this, "OnnxInferenceToolsForward", {}, "Forward ONNX tools per fold"};
    //! @brief ToolHandleArray for backward ONNX models (one tool per fold)
    ToolHandleArray<AthOnnx::IOnnxRuntimeInferenceTool> m_onnxToolsBackward{this, "OnnxInferenceToolsBackward", {}, "Backward ONNX tools per fold"};

    //! @brief List of shower shape variable names (order must match model I/O)
    static const std::vector<std::string> s_ssVarNames;

    //! @brief Egamma shower shape enum mapping for reading/writing values (order matches s_ssVarNames)
    static const std::vector<xAOD::EgammaParameters::ShowerShapeType> s_ssEnums;
    
    /** @brief Accessor used to decorate photons per shower shape variable */
    struct SSAccessors {
        std::unique_ptr<SG::AuxElement::Accessor<float>> original;
    };

    //! @brief Per-variable accessors aligned with s_ssVarNames
    std::vector<SSAccessors> m_accessors;

}; // end class ElectronPhotonVariableNFCorrectionTool

#endif

