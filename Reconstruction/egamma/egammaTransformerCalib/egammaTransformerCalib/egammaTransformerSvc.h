/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMATRANSFORMERCALIB_EGAMMATRANSFORMERSVC_H
#define EGAMMATRANSFORMERCALIB_EGAMMATRANSFORMERSVC_H

#include "xAODEgamma/EgammaEnums.h"
#include "EgammaAnalysisInterfaces/IegammaMVASvc.h"
#include "EgammaAnalysisInterfaces/IegammaMVACalibTool.h"
// Framework includes
#include "AsgServices/AsgService.h"
#include "AsgTools/PropertyWrapper.h"

#include "xAODEgamma/PhotonFwd.h"


#include <string>


class egammaTransformerSvc : public extends<asg::AsgService, IegammaMVASvc>
{
public:
  using extends::extends;  // base class constructor

  virtual ~egammaTransformerSvc() override {};
  virtual StatusCode initialize() override;

  /** Compute the calibrated energy **/
  // Note for both egammaTransformerSvc::getEnergy overloads below:
  // zero-response handling is delegated to the underlying calibration tools
  // (m_mvaElectron/m_mvaFwdElectron/m_mvaConvertedPhoton/m_mvaUnconvertedPhoton).
  // By default their property useClusterIf0 (m_clusterEif0) is true, so a zero
  // model response returns the uncalibrated cluster energy instead of 0.
  // Therefore mvaE is not expected to be 0 by default unless the user
  // explicitly changes that tool property in the calibration configuration.
  StatusCode getEnergy(const xAOD::CaloCluster& cluster,
                       const xAOD::Egamma& eg,
                       double& mvaE,
		       const egammaMVACalib::GlobalEventInfo& gei = egammaMVACalib::GlobalEventInfo()) const override final;

  /** Compute the calibrated energy when the full egamma object is not available **/
  StatusCode getEnergy(const xAOD::CaloCluster& cluster,
                       const xAOD::EgammaParameters::EgammaType egType,
                       double& mvaE,
		       const egammaMVACalib::GlobalEventInfo& gei = egammaMVACalib::GlobalEventInfo()) const override final;

  /** Main execute. We need to calibrate the cluster.
      Use full egamma object instead of Type
      As we employ further variables than the ones present in the cluster
      This method needs to be valid also for reconstruction
  */
  StatusCode execute(xAOD::CaloCluster& cluster,
                     const xAOD::Egamma& eg,
		     const egammaMVACalib::GlobalEventInfo& gei = egammaMVACalib::GlobalEventInfo()) const override final;

  /** Calibrate the cluster, when the full egamma object is not available.
   *  Only variables related to the cluster are used (e.g. no conversion are used here)
   *  If the full egamma object use the other version.
   */
  StatusCode execute(xAOD::CaloCluster& cluster,
                     const xAOD::EgammaParameters::EgammaType egType,
		     const egammaMVACalib::GlobalEventInfo& gei = egammaMVACalib::GlobalEventInfo()) const override final;


private:

  /// MVA tool for electron
  ToolHandle<IegammaMVACalibTool> m_mvaElectron {this,
      "ElectronTool", "", "Tool to handle MVA trees for electrons"};

  /// MVA tool for forward electron
  ToolHandle<IegammaMVACalibTool> m_mvaFwdElectron {this,
      "FwdElectronTool", "", "Tool to handle MVA trees for forward electrons"};

  /// MVA tool for unconverted photon
  ToolHandle<IegammaMVACalibTool> m_mvaUnconvertedPhoton {this,
      "UnconvertedPhotonTool", "", "Tool to handle MVA trees for unconverted photons"};

  /// MVA tool for converted photon
  ToolHandle<IegammaMVACalibTool> m_mvaConvertedPhoton {this,
      "ConvertedPhotonTool", "", "Tool to handle MVA trees for converted photons"};

  Gaudi::Property<float> m_maxConvR {this,
      "MaxConvRadius", 800.0,
      "The maximum conversion radius for a photon to be considered converted"};

  Gaudi::Property<int> m_removeTRTConvBarrel {this,
      "RemoveTRTConvBarrel", -1,
      "Remove TRT converted photons in the barrel: no=0, yes=1, automatic=-1"};

  Gaudi::Property<std::string> m_folder {this,
      "folder", "", "folder for weight files"};


  /**
   * @brief Decide if the photon is converted or not
   */
  bool isConvCalib(const xAOD::Photon& ph) const;

  StatusCode resolve_flags();


};

#endif
