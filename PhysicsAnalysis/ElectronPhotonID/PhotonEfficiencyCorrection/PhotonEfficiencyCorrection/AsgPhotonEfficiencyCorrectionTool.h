/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-

#ifndef PHOTONEFFICIENCYCORRECTION_ASGPHOTONEFFICIENCYCORRECTIONTOOL
#define PHOTONEFFICIENCYCORRECTION_ASGPHOTONEFFICIENCYCORRECTIONTOOL

/**
   @class AthPhotonEfficiencyCorrectionTool
   @brief Calculate the egamma scale factors in Athena

   @author Michael Pitt <michael.pitt@cern.ch>, Giovanni Marchiori
   @date   February 2018
*/

// STL includes
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>

//xAOD includes
#include "AsgTools/AsgTool.h"
#include "PATInterfaces/ISystematicsTool.h"
#include "PATInterfaces/SystematicRegistry.h"
#include "PATInterfaces/CorrectionCode.h"
#include "ElectronEfficiencyCorrection/TElectronEfficiencyCorrectionTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonEfficiencyCorrectionTool.h"

#include "xAODEgamma/Egamma.h"

class AsgPhotonEfficiencyCorrectionTool
  : virtual public IAsgPhotonEfficiencyCorrectionTool,
    virtual public CP::ISystematicsTool,
            public asg::AsgTool
{
  ASG_TOOL_CLASS3(AsgPhotonEfficiencyCorrectionTool, IAsgPhotonEfficiencyCorrectionTool, CP::ISystematicsTool, CP::IReentrantSystematicsTool )

public:
  /// Standard constructor
  AsgPhotonEfficiencyCorrectionTool ( const std::string& myname );

  /// Standard destructor
  virtual ~AsgPhotonEfficiencyCorrectionTool();

  /// Gaudi Service Interface method implementations
  virtual StatusCode initialize() override;


public:
  ///Add some method for now as a first step to move the tool to then new interface 
  virtual CP::CorrectionCode getEfficiencyScaleFactor(const xAOD::Egamma& inputObject, double& efficiencyScaleFactor) const override;
  virtual CP::CorrectionCode getEfficiencyScaleFactorError(const xAOD::Egamma& inputObject, double& efficiencyScaleFactorError) const override;
  virtual CP::CorrectionCode applyEfficiencyScaleFactor(xAOD::Egamma& inputObject) const override;

  ///The methods below should notify the user of what is actually in the list , without him having to go in the wiki

  /// returns: whether this tool is affected by the given systematic
  virtual bool isAffectedBySystematic( const CP::SystematicVariation& systematic ) const override;
  
  /// returns: the list of all systematics this tool can be affected by
  virtual CP::SystematicSet affectingSystematics() const override;
  
  /// returns: the list of all systematics this tool recommends to use
  virtual CP::SystematicSet recommendedSystematics() const override;
  
  /// returns: the currently applied systematics
  const CP::SystematicSet& appliedSystematics() const {
    assert (m_appliedSystematics != nullptr);
    return *m_appliedSystematics;
  }
  
 // Correlation Model     
  std::string m_correlation_model_name;
  int m_correlation_model;     

  /// Configure this tool for the given systematics
  virtual StatusCode applySystematicVariation ( const CP::SystematicSet& systConfig ) override;

  StatusCode registerSystematics();

  // Private member variables
private:
  typedef Root::TElectronEfficiencyCorrectionTool::Result Result;
  /// I think these calculate methods are only used internally
  CP::CorrectionCode calculate( const xAOD::Egamma* egam, Result& result ) const;

  /// Pointer to the underlying ROOT based tool
  Root::TElectronEfficiencyCorrectionTool* m_rootTool_unc;
  Root::TElectronEfficiencyCorrectionTool* m_rootTool_con;
  
  /// Systematics filter map
  std::unordered_map<CP::SystematicSet, CP::SystematicSet> m_systFilter;
  
  /// Currently applied systematics
  CP::SystematicSet* m_appliedSystematics = nullptr;
  
  // The prefix for the systematic name
  std::string m_sysSubstring;
  
  // Get the correction filename from the map
  std::string getFileName(const std::string& isoWP, const std::string& trigWP, bool isConv);
  
  // Set prefix of the corresponding calibration filenames:
  std::string m_file_prefix_ID="efficiencySF.offline.";
  std::string m_file_prefix_ISO="efficiencySF.Isolation.";
  std::string m_file_prefix_Trig="efficiencySF.";
  std::string m_file_prefix_TrigEff="efficiency.";
  
  // Properties
  
  /// The list of input file names
  std::string m_corrFileNameConv;
  std::string m_corrFileNameUnconv;
 
  /// The prefix string for the result
  std::string m_resultPrefix;

  /// The string for the result
  std::string m_resultName;

  /// Force the data type to a given value
  int m_dataTypeOverwrite;
  
  /// Isolation working point
  std::string m_isoWP;
  
  /// Trigger name for trigger SF
  std::string m_trigger;
  
  /// map filename
  std::string m_mapFile;  

  // bin boundaries of correction files
  std::map<float, std::vector<float>> m_pteta_bins;
  
  //use RandomRun Number
  bool m_useRandomRunNumber;
  int m_defaultRandomRunNumber;

  int m_nCorrSyst;
  
  // remove TRT converted photon for Run-3
  bool m_removeTRTConversion;

  //For variations list
  std::vector<CP::SystematicVariation> m_corrVarUp;
  std::vector<CP::SystematicVariation> m_corrVarDown;
  std::vector<CP::SystematicVariation> m_uncorrVarUp;
  std::vector<CP::SystematicVariation> m_uncorrVarDown;


}; // End: class definition


#endif

