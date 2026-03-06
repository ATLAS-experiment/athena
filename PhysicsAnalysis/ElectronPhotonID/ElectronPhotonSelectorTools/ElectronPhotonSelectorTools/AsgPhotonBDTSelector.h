
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
  @class AsgPhotonBDTSelector
  @brief ASG selection tool implementing a photon-ID working point based on a BDT score.

  This tool provides an interface for photon
  identification, using a BDT-based discriminant.

  It applies:
    - a set of preselection requirements (f1 and e277),
    - a BDT score cut that depends on (|eta|, Et) bin and conversion status.

  The working point (binning and thresholds) is read from a TEnv
  configuration file

  The tool can either:
    - compute the score on-the-fly via PhotonBDTCalculator (if allowed and the
      score decoration is missing), or
    - reuse an existing score decoration already present on the photon.

  @note The meaning of the isEM bits is specific to this BDT selector and is
        documented in this implementation.

  @author Ruggero Turra, Elena Mazzeo
*/

#ifndef ELECTRONPHOTONSELECTORTOOLS_ASGPHOTONBDTSELECTOR_H
#define ELECTRONPHOTONSELECTORTOOLS_ASGPHOTONBDTSELECTOR_H

#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/CurrentContext.h"
#include "xAODEgamma/PhotonFwd.h"
#include "xAODEgamma/EgammaFwd.h"
#include "xAODEgamma/ElectronFwd.h"
#include "xAODBase/IParticle.h"

#include "EgammaAnalysisInterfaces/IAsgPhotonIsEMSelector.h"
#include "PhotonBDTCalculator.h"


#include <string>
#include <vector>

class EventContext;

namespace PhotonIDBDT {

class AsgPhotonBDTSelector : public asg::AsgTool, virtual public IAsgPhotonIsEMSelector
{
  ASG_TOOL_CLASS2(AsgPhotonBDTSelector, IAsgPhotonIsEMSelector, IAsgSelectionTool)

public:
  AsgPhotonBDTSelector(const std::string& name);
  virtual ~AsgPhotonBDTSelector() override = default;

  virtual StatusCode initialize() override;

  // Accept methods 
  virtual asg::AcceptData accept(const xAOD::IParticle* part) const override;
  virtual asg::AcceptData accept(const EventContext& ctx, const xAOD::IParticle* part) const override;
  virtual asg::AcceptData accept(const EventContext& ctx, const xAOD::Egamma* part) const override;
  virtual asg::AcceptData accept(const EventContext& ctx, const xAOD::Photon* part) const override;
  virtual asg::AcceptData accept(const EventContext& ctx, const xAOD::Electron* part) const override;

  virtual StatusCode execute(const EventContext& ctx, const xAOD::Egamma* eg, unsigned int& isEM) const override;
  
  virtual std::string getOperatingPointName() const override;

private:
  StatusCode loadConfig();
  // Core accept method (BDT-based selection)
  asg::AcceptData acceptBDT(const EventContext& ctx, const xAOD::Photon& ph, unsigned int* isEM = nullptr) const;
  const asg::AcceptInfo& getAcceptInfo() const;

  // Tool to compute and decorate score
  ToolHandle<PhotonBDTCalculator> m_bdtTool {this, "BDTTool", "", "Photon BDT calculator tool"};

  // Properties
  Gaudi::Property<std::string> m_workingPoint {this, "WorkingPoint", "", "Name of the Photon ID BDT Working point"};
  Gaudi::Property<std::string> m_scoreDecoration {this, "ScoreDecoration", "BDTScore", "Name of the float decoration containing the score"};
  Gaudi::Property<bool> m_computeIfMissing {this, "ComputeIfMissing", true, "Compute score with BDTTool if decoration missing"};
  Gaudi::Property<bool> m_excludeTRT {this, "ExcludeTRT", true, "Conversion definition for Run 3"};
  Gaudi::Property<bool> m_reapplyWPIfNoShowerShapes {this, "ReapplyWPIfNoShowerShapes", true, "Reapply the WP calculation, based on the BDT score and isEM word (works only if these are available!) "};
  Gaudi::Property<std::string> m_isEMDecoration {this, "IsEMDecoration", "BDTIsEM", "Name of the int decoration containing the isEM word (used if ReapplyWPIfNoShowerShapes is true)"};

  // Config file with binning and cuts
  std::string m_configFile;

  // Binning and cuts
  std::vector<float> m_etaBins;
  std::vector<float> m_etBinsGeV;
  // Preselection values on f1 and e277 variables 
  std::vector<float> m_cutF1Conv;
  std::vector<float> m_cutF1Unconv;
  std::vector<float> m_cutE277Conv;
  std::vector<float> m_cutE277Unconv;

  // BDT score cuts for converted and unconverted photons
  std::vector<float> m_cutConv;   
  std::vector<float> m_cutUnconv; 

  // Accept machinery
  asg::AcceptInfo m_acceptInfo;
  int m_cutPosScore{-1};
  int m_cutPosInRange{-1};
  int m_cutPosHasScore{-1};
  int m_cutPosPreF1{-1};
  int m_cutPosPreE277{-1};
  int m_cutPosPassPreselection{-1};

  // helpers
  bool isConverted(const xAOD::Photon& ph) const;
  bool findBin(const float absEta, const float etGeV, size_t& iEta, size_t& iEt) const;
  float getCut(const bool converted, const size_t iEta, const size_t iEt) const;
  asg::AcceptData makeReject(const asg::AcceptInfo& info) const;
  float getShowerShape(const xAOD::Photon& ph, xAOD::EgammaParameters::ShowerShapeType t, const char *name = "") const;

};

} // namespace PhotonIDBDT

#endif