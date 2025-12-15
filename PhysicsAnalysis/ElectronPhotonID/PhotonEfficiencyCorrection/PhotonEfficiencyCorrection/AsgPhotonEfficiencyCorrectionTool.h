/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
#include "AsgTools/PropertyWrapper.h"
#include "PATInterfaces/ISystematicsTool.h"
#include "PATInterfaces/SystematicRegistry.h"
#include "PATInterfaces/CorrectionCode.h"
#include "ElectronEfficiencyCorrection/TElectronEfficiencyCorrectionTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonEfficiencyCorrectionTool.h"

#include <ColumnarCore/ColumnAccessor.h>
#include "ColumnarCluster/ClusterHelpers.h"
#include <ColumnarEventInfo/EventInfoDef.h>
#include <ColumnarCore/LinkColumn.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/VectorColumn.h>
#include <ColumnarEgamma/EgammaHelpers.h>

class AsgPhotonEfficiencyCorrectionTool
  : virtual public IAsgPhotonEfficiencyCorrectionTool,
    virtual public CP::ISystematicsTool,
            public asg::AsgTool,
            public columnar::ColumnarTool<>
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
  CP::CorrectionCode getEfficiencyScaleFactor(columnar::EgammaId inputObject, columnar::EventInfoId eventInfo, double& efficiencyScaleFactor) const;
  virtual CP::CorrectionCode getEfficiencyScaleFactorError(const xAOD::Egamma& inputObject, double& efficiencyScaleFactorError) const override;
  CP::CorrectionCode getEfficiencyScaleFactorError(columnar::EgammaId inputObject, columnar::EventInfoId eventInfo, double& efficiencyScaleFactorError) const;
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
  
  /// Configure this tool for the given systematics
  virtual StatusCode applySystematicVariation ( const CP::SystematicSet& systConfig ) override;

  StatusCode registerSystematics();

  // Private member variables
private:
  typedef Root::TElectronEfficiencyCorrectionTool::Result Result;
  /// I think these calculate methods are only used internally
  CP::CorrectionCode calculate( columnar::EgammaId egam, columnar::EventInfoId eventInfo, Result& result ) const;

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

  // remove TRT converted photon for Run-3
  bool m_removeTRTConversion;

  Gaudi::Property<bool> m_allowMissingLinks{ this, "AllowMissingLinks", false, "Allow missing links in the input objects. This should only be used by experts running on expert formats." };

  // an accessor structure that hides the columnar accessors from the
  // root dictionaries that can't handle them.  these dictionaries are
  // used by some users to instantiate the tools (instead of using the
  // factory mechanism).
  struct Accessors : public columnar::ColumnarTool<>
  {
    Accessors(AsgPhotonEfficiencyCorrectionTool& tool) : columnar::ColumnarTool<>(&tool) {}

    columnar::EventInfoAccessor<columnar::ObjectColumn> eventInfoAcc {*this, "EventInfo", {.addMTDependency=true}};
    columnar::EgammaAccessor<columnar::ObjectColumn> photonsAcc {*this, "Photons"};
    columnar::ClusterAccessor<columnar::ObjectColumn> clusterAcc {*this, "egammaClusters"};
    columnar::VertexAccessor<columnar::ObjectColumn> verticesAcc {*this, "GSFConversionVertices"};
    columnar::TrackAccessor<columnar::ObjectColumn> tracksAcc {*this, "GSFTrackParticles"};

    columnar::EventInfoAccessor<uint32_t> randomRunNumberAcc {*this, "RandomRunNumber"};
  
    columnar::EgammaAccessor<float> etaAcc{*this,"eta"};
    columnar::EgammaHelpers::IsConvertedPhotonAccessor<> isConvertedPhotonAcc{*this};
    columnar::EgammaDecorator<float> sfDec{*this,"sfOut"};
    columnar::EgammaDecorator<char> validDec{*this,"validOut"};
  
    columnar::EgammaAccessor<std::vector<columnar::OptClusterId>> caloClusterAcc {*this, "caloClusterLinks"};
    columnar::ClusterAccessor<float> clusterEAcc {*this, "calE"};
    columnar::ClusterHelpers::EtaBEAccessor<> clusterEtaBEAcc {*this};
  };
  std::unique_ptr<Accessors> m_accessors {std::make_unique<Accessors>(*this)};


public:

  void callSingleEvent (columnar::EgammaRange photons, columnar::EventInfoId event) const;
  void callEvents (columnar::EventContextRange events) const override;

}; // End: class definition


#endif

