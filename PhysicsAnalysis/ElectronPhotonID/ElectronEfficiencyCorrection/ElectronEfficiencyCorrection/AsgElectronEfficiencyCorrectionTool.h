/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef __ASGELECTRONEFFICIENCYCORRECTIONTOOL__
#define __ASGELECTRONEFFICIENCYCORRECTIONTOOL__

/**
  @class AthElectronEfficiencyCorrectionTool
  @brief Calculate the egamma scale factors in Athena

  @author Kristin Lohwasser , Rob Roy Fletcher, Karsten Koeneke
  @date   May 2014
  */

// Fwd Declarations
class TH2F;
#include "ElectronEfficiencyCorrection/TElectronEfficiencyCorrectionTool.h"
#include "xAODEgamma/ElectronFwd.h"
// Interface
#include "EgammaAnalysisInterfaces/IAsgElectronEfficiencyCorrectionTool.h"
// STL includes
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
// PAT includes
#include "AsgTools/AsgMetadataTool.h"
#include "PATCore/PATCoreEnums.h"
#include "PATInterfaces/CorrectionCode.h"
#include "PATInterfaces/SystematicRegistry.h"

class AsgElectronEfficiencyCorrectionTool final
  : virtual public IAsgElectronEfficiencyCorrectionTool
  , public asg::AsgMetadataTool
{
  ASG_TOOL_CLASS(AsgElectronEfficiencyCorrectionTool,
                 IAsgElectronEfficiencyCorrectionTool)

public:
  /// Standard constructor
  AsgElectronEfficiencyCorrectionTool(const std::string& myname);
  /// Standard destructor
  virtual ~AsgElectronEfficiencyCorrectionTool() override final;
  /// Gaudi Service Interface method implementations
  virtual StatusCode initialize() override final;

  /*
   * Metadata tool methods
   */
  virtual StatusCode beginInputFile() override final;
  virtual StatusCode beginEvent() override final;

  /*
   * ISystematic tool methods
   */
  /// returns: the list of all systematics this tool can be affected by
  virtual CP::SystematicSet affectingSystematics() const override final;
  /// returns: the list of all systematics this tool recommends to use
  virtual CP::SystematicSet recommendedSystematics() const override final;
  /// returns: whether this tool is affected by the given systematis
  virtual bool isAffectedBySystematic(
    const CP::SystematicVariation& systematic) const override final;
  /// Configure this tool for the given systematics
  virtual StatusCode applySystematicVariation(
    const CP::SystematicSet& systConfig) override final;

  /*
   * IAsgElectronEfficiencyCorrectionTool methods
   */
  virtual CP::CorrectionCode getEfficiencyScaleFactor(
    const xAOD::Electron& inputObject,
    double& efficiencyScaleFactor) const override final;
  //
  virtual CP::CorrectionCode getEfficiencyScaleFactor(
    const double et,  /*in MeV*/
    const double cluster_eta, /*cluster*/
    const unsigned int runNumber,
    double& efficiencyScaleFactor) const override final;
  //
  virtual CP::CorrectionCode applyEfficiencyScaleFactor(
    const xAOD::Electron& inputObject) const override final;
  //
  virtual int getNumberOfToys() const override final
  {
    return m_number_of_toys;
  };
  /// print available/implemented correlation models
  virtual void printCorrelationModels() const override final
  {
    ATH_MSG_INFO(
      " Available Correlation Models for the ElectronEfficiencyCorrectionTool");
    ATH_MSG_INFO("FULL");
    ATH_MSG_INFO("SIMPLIFIED");
    ATH_MSG_INFO("MCTOYS");
    ATH_MSG_INFO("SYST");
    ATH_MSG_INFO("COMBMCTOYS");
    ATH_MSG_INFO("TOTAL");
  };

  /// returns: the currently applied systematics
  virtual const CP::SystematicSet& appliedSystematics() const override final
  {
    return *m_appliedSystematics;
  }

  virtual int systUncorrVariationIndex(
    const xAOD::Electron& inputObject) const override final;

private:
  StatusCode registerSystematics();

  int currentSimplifiedUncorrSystRegion(const double cluster_eta,
                                        const double et) const;
  int currentUncorrSystRegion(const double cluster_eta, const double et) const;

  /// initialize the systematics
  StatusCode InitSystematics();

  // Gets the correction filename from map
  virtual StatusCode getFile(const std::string& recokey,
                             const std::string& idkey,
                             const std::string& isokey,
                             const std::string& trigkey);
  // Get the simulation type from metadata
  StatusCode get_simType_from_metadata(
    PATCore::ParticleDataType::DataType& result) const;

  // struct for toys
  struct SystConf
  {
    float m_total;
    unsigned m_toy_index;
    float m_toy_scale;
  };

  /// To check if the metadata can be retrieved
  bool m_metadata_retrieved = false;

  /// Pointer to the underlying ROOT based tool
  std::unique_ptr<Root::TElectronEfficiencyCorrectionTool> m_rootTool = nullptr;

  /// Systematics filter map
  std::unordered_map<CP::SystematicSet, CP::SystematicSet> m_systFilter;

  /// Affected systematic, should be done only once
  CP::SystematicSet m_affectedSys;

  /// Currently applied systematics
  CP::SystematicSet* m_appliedSystematics;

  // Correlation Model
  std::string m_correlation_model_name;
  int m_correlation_model;

  // Map-key properties
  std::string m_mapFile;

  // Four SF key Properties
  std::string m_recoKey;
  std::string m_idKey;
  std::string m_isoKey;
  std::string m_trigKey;

  /// The list of file names
  std::vector<std::string> m_corrFileNameList;

  /// The prefix string for the result
  std::string m_resultPrefix;

  /// The string for the result
  std::string m_resultName;

  // The prefix for the systematic name
  std::string m_sysSubstring;

  /// The Event info collection name
  std::string m_eventInfoCollectionName;

  // use RandomRun Number
  bool m_useRandomRunNumber;
  int m_defaultRandomRunNumber;

  // The data type
  PATCore::ParticleDataType::DataType m_dataType;

  /// Force the data type to a given value
  int m_dataTypeOverwrite;

  // Number of toys:
  int m_number_of_toys;

  // seed for toys:
  int m_seed_toys;

  // scale for toy uncertainty:
  double m_scale_toys;

  // number of systematic uncertainties
  int m_nCorrSyst;
  int m_nUncorrSyst;
  std::vector<std::pair<float, std::vector<float>>> m_pteta_bins;

  // custom binning for SIMPLIFIED model
  std::vector<float> m_uncorrSimplfEtaBinsUser;
  std::vector<float> m_uncorrSimplfEtBinsUser;

  // simplified uncorrelation regions
  TH2F* m_UncorrRegions;
  int m_nSimpleUncorrSyst;

  // Caching of variations we know after initialize
  std::string m_toysBasename;
  std::vector<CP::SystematicVariation> m_corrVarUp;
  std::vector<CP::SystematicVariation> m_corrVarDown;
  std::vector<CP::SystematicVariation> m_uncorrVarUp;
  std::vector<CP::SystematicVariation> m_uncorrVarDown;

}; // End: class definition

#endif
