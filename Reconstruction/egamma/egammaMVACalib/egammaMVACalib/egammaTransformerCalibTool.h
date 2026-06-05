/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAOD_ANALYSIS

#ifndef EGAMMATRANSFORMERCALIB_EGAMMATRANSFORMERCALIBTOOL_H
#define EGAMMATRANSFORMERCALIB_EGAMMATRANSFORMERCALIBTOOL_H

// Package includes
#include "EgammaAnalysisInterfaces/IegammaMVACalibTool.h"
#include "FlavorTagInference/SaltModel.h"
#include "xAODEgamma/EgammaEnums.h"
#include "egammaLayerRecalibTool/egammaLayerRecalibTool.h"
#include "egammaInterfaces/IegammaCellRecoveryTool.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

// Framework includes
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"

// Root includes
#include "TFormula.h"
#include "TH2Poly.h"
#include "TObject.h"
#include "TString.h"

// STL includes
#include <functional>
#include <memory>
#include <string>

/**
 * @brief A tool used by the egammaMVASvc to help calibrate energy for one
 * particle type.
 *
 * The particle type to be calibrated must be specified by the property
 * ParticleType. The property folder must be set with the path to a folder
 * containig three transformer model in onnx format Since Transformer use cells
 * directly as input, layer calibration will be applied to one of the decorator
 * and another decorator will not have layer calibration applied. On data the
 * property use_layer_corrected should be set to true. In reconstruction this
 * flag is always false. In PhysicsAnalysis it should be set appropriately. When
 * set to true when using the layer energies as input the data-driver-corrected
 * version are used.
 *
 * Public members of this function is kept same as BDT for interface
 * consistency.
 **/

class egammaTransformerCalibTool : public asg::AsgTool,
                                   virtual public IegammaMVACalibTool {
  ASG_TOOL_CLASS(egammaTransformerCalibTool, IegammaMVACalibTool)
 public:
  egammaTransformerCalibTool(const std::string& type);
  virtual ~egammaTransformerCalibTool() override;

  virtual StatusCode initialize() override;

  /** returns the calibrated energy **/
  float getEnergy(const xAOD::CaloCluster& clus, const xAOD::Egamma* eg,
                  const egammaMVACalib::GlobalEventInfo& gei =
                      egammaMVACalib::GlobalEventInfo()) const override final;

 private:
  Gaudi::Property<int> m_particleType{
      this, "ParticleType", xAOD::EgammaParameters::NumberOfEgammaTypes,
      "What type of particle do we use"};

  Gaudi::Property<bool> m_clusterEif0{
      this, "useClusterIf0", true, "Use cluster energy if MVA response is 0"};

  /// string with folder for weight files
  Gaudi::Property<std::string> m_folder{this, "folder", "",
                                        "string with folder for weight files"};

  /// Layer calibration related properties 
  Gaudi::Property<bool> m_isMC{this, "isMC", false, "Whether the input file is MC or data"};

  Gaudi::Property<bool> m_useLayerCorrected{this, "useLayerCorrection", true,
                                            "whether to use layer corrections"};

  Gaudi::Property<std::string> m_layerCalibTune{this, "layerRecalibrationTune", "es2025_run3_extrapolate_gnn_v0",
                                            "layer to use for layer corrections"};

  Gaudi::Property<bool> m_useSaccCorrection{this, "useSaccCorrection", true,
                                            "whether to use SACC correction for layer recalibration"};

  Gaudi::Property<bool> m_useFixForMissingCells{this, "useFixForMissingCells", true,
                                            "whether to apply fix for missing cells in layer recalibration, this is applied by default, this must be set to true whenever the timing cut fix is applied. Otherwise the layer calibration will not have correct scale factors for cell energies after timing cut fix"};

  Gaudi::Property<bool> m_useExtraLayerScales{this, "useExtraLayerScales", false,
                                            "whether to apply extra layer scales, this is for systematics studies, by default it is false and the extra layer scales are set to 1.0"};

  Gaudi::Property<std::string> m_electronModelFile{
      this, "ElectronModelFile", "electron_model_calibration.onnx",
      "ONNX file for electron transformer model"};

  Gaudi::Property<std::string> m_unconvertedPhotonModelFile{
      this, "UnconvertedPhotonModelFile",
        "unconverted_photon_model_calibration.onnx",
      "ONNX file for unconverted photon transformer model"};

  Gaudi::Property<std::string> m_convertedPhotonModelFile{
      this, "ConvertedPhotonModelFile",
      "converted_photon_model_calibration.onnx",
      "ONNX file for converted photon transformer model"};

  Gaudi::Property<std::string> m_forwardElectronModelFile{
      this, "ForwardElectronModelFile",
      "",
      "ONNX file for forward electron transformer model, this is NOT "
      "implemented yet"};

  // Transformer model utils
  std::unique_ptr<const FlavorTagInference::SaltModel> m_saltModel;
  int m_num_cluster_features = 0;
  int m_num_cell_features = 0;
  static constexpr double m_timeCut = 12.;

  std::unique_ptr<egammaLayerRecalibTool> m_layerRecalibTool = nullptr;

  // LG: scale for MG and LG need to be added later since the amount of work is not small.

    /** @brief Pointer to the egammaCellRecoveryTool*/
  ToolHandle<IegammaCellRecoveryTool> m_egammaCellRecoveryTool{
    this,
    "egammaCellRecoveryTool",
    "egammaCellRecoveryTool/egammaCellRecoveryTool",
    "Optional tool that adds cells in L2 or L3 "
    "that could have been rejected by timing cut"
  };

  /// a utility to set up the transformer model, separated from initialize for
  /// better readability
  StatusCode setupTransformerModel(const std::string& fileName);

};

#endif

#endif
