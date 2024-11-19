// Dear Emacs, this is -*- C++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAMVACALIB_EGAMMAMVASVC_H
#define EGAMMAMVACALIB_EGAMMAMVASVC_H

#include "xAODEgamma/EgammaEnums.h"
#include "EgammaAnalysisInterfaces/IegammaMVASvc.h"
#include "EgammaAnalysisInterfaces/IegammaMVACalibTool.h"
// Framework includes
#include "AsgServices/AsgService.h"
#include "AsgTools/PropertyWrapper.h"

#include "xAODEgamma/PhotonFwd.h"


#include <string>


class egammaMVASvc : public extends<asg::AsgService, IegammaMVASvc>
{
public:
  using extends::extends;  // base class constructor

  virtual ~egammaMVASvc() override {};
  virtual StatusCode initialize() override;


  /** Compute the calibrated energy **/
  StatusCode getEnergy(const xAOD::CaloCluster& cluster,
                       const xAOD::Egamma& eg,
                       double& mvaE) const override final;

  /** Compute the calibrated energy when the full egamma object is not available **/
  StatusCode getEnergy(const xAOD::CaloCluster& cluster,
                       const xAOD::EgammaParameters::EgammaType egType,
                       double& mvaE) const override final;

  /** Main execute. We need to calibrate the cluster.
      Use full egamma object instead of Type
      As we employ further variables than the ones present in the cluster
      This method needs to be valid also for reconstruction
  */
  StatusCode execute(xAOD::CaloCluster& cluster,
                     const xAOD::Egamma& eg) const override final;

  /** Calibrate the cluster, when the full egamma object is not available.
   *  Only variables related to the cluster are used (e.g. no conversion are used here)
   *  If the full egamma object use the other version.
   */
  StatusCode execute(xAOD::CaloCluster& cluster,
                     const xAOD::EgammaParameters::EgammaType egType) const override final;


private:

  /// MVA tool for electron
  ToolHandle<IegammaMVACalibTool> m_mvaElectron {this,
      "ElectronTool", "", "Tool to handle MVA trees for electrons"};

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
