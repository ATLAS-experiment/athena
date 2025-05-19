/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CPBTAGGINGEFFICIENCYTOOL_H
#define CPBTAGGINGEFFICIENCYTOOL_H

#include "FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h"
#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"
#include "PATInterfaces/ISystematicsTool.h"
#include "xAODBTagging/BTagging.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/AnaToolHandle.h"

#include "CalibrationDataInterface/CalibrationDataVariables.h"
#include "CalibrationDataInterface/CalibrationDataInterfaceROOT.h"

// for the onnxtool
#include "xAODBTaggingEfficiency/SaltModel.h"

#include "xAODBTaggingEfficiency/ToolDefaults.h"
//
#include <fstream>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <memory>


class BTaggingEfficiencyTool: public asg::AsgTool,
            public virtual IBTaggingEfficiencyTool
{
  //  typedef double (xAOD::BTagging::* tagWeight_member_t)() const;

  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS3( BTaggingEfficiencyTool , IBTaggingEfficiencyTool, ISystematicsTool, CP::IReentrantSystematicsTool )

  public:


  /// Create a constructor for standalone usage
  BTaggingEfficiencyTool( const std::string& name );

  /// Create a constructor for standalone usage
  virtual ~BTaggingEfficiencyTool();

  // For now, disable the generation of a default copy constructor (since it would not be constructed correctly)
  // BTaggingEfficiencyTool(const BTaggingEfficiencyTool& other) = delete;

  // /// Silly copy constructor for the benefit of dictionary generation
  // BTaggingEfficiencyTool(const BTaggingEfficiencyTool& other);

  /// @name Methods implementing the main jet-by-jet access in the xAOD context
  /// @{

  /** Computes the data/MC efficiency scale factor for the given jet.
      The tagger and operating point under consideration are part of the configuration and hence aren't function arguments.
   */
  CP::CorrectionCode getScaleFactor( const xAOD::Jet & jet,
             float & sf);

  /** Computes the data efficiency for the given jet.
      The tagger and operating point under consideration are part of the configuration and hence aren't function arguments.
   */
  CP::CorrectionCode getEfficiency( const xAOD::Jet & jet,
				    float & eff);

  /** Computes the data inefficiency for the given jet.
      The tagger and operating point under consideration are part of the configuration and hence aren't function arguments.
   */
  CP::CorrectionCode getInefficiency( const xAOD::Jet & jet,
              float & eff);

  /** Computes the data/MC inefficiency scale factor for the given jet.
      The tagger and operating point under consideration are part of the configuration and hence aren't function arguments.
   */
  CP::CorrectionCode getInefficiencyScaleFactor( const xAOD::Jet & jet,
             float & sf);

  /** Computes the MC efficiency for the given jet.
      The tagger and operating point under consideration are part of the configuration and hence aren't function arguments.
   */
  CP::CorrectionCode getMCEfficiency( const xAOD::Jet & jet,
              float & eff);

  /// @name Methods equivalent to those above but not relying on the xAOD format
  /// @{

  /** Computes the data/MC efficiency scale factor for the jet, given its kinematics, (possibly) tagger weight and truth flavour.
      The tagger and operating point under consideration are part of the configuration and hence aren't function arguments.
  */
  CP::CorrectionCode getScaleFactor( int flavour, const Analysis::CalibrationDataVariables& v,
             float & sf);

  /** Computes the data efficiency for the jet, given its kinematics, (possibly) tagger weight and truth flavour.
      The tagger and operating point under consideration are part of the configuration and hence aren't function arguments.
   */
  CP::CorrectionCode getEfficiency( int flavour, const Analysis::CalibrationDataVariables& v,
            float & eff);

  /** Computes the data inefficiency for the jet, given its kinematics, (possibly) tagger weight and truth flavour.
      The tagger and operating point under consideration are part of the configuration and hence aren't function arguments.
   */
  CP::CorrectionCode getInefficiency( int flavour, const Analysis::CalibrationDataVariables& v,
              float & eff);

  /** Computes the data/MC inefficiency scale factor for the jet, given its kinematics, (possibly) tagger weight and truth flavour.
      The tagger and operating point under consideration are part of the configuration and hence aren't function arguments.
   */
  CP::CorrectionCode getInefficiencyScaleFactor( int flavour, const Analysis::CalibrationDataVariables& v,
						 float & sf);

  /** Computes the MC efficiency for the jet, given its kinematics, (possibly) tagger weight and truth flavour.
      The tagger and operating point under consideration are part of the configuration and hence aren't function arguments.
   */
  CP::CorrectionCode getMCEfficiency( int flavour, const Analysis::CalibrationDataVariables& v,
              float & eff);

  /** Computes the MC efficiency of the jets in a given event. (Uses the onnx tool)
      For fixed cut wp
   */
  CP::CorrectionCode getMCEfficiencyONNX( const std::vector<std::vector<float>>& node_feat, std::vector<float>& effAllJet);
    
  /** Computes the MC efficiency of the jets in a given event. (Uses the onnx tool)
      For continuous wp
   */
  CP::CorrectionCode getMCEfficiencyONNX( const std::vector<std::vector<float>>& node_feat, std::vector<std::vector<float>>& effAllJetAllWp);

  /// @}

  /// @name Other methods implementing the IBTagEfficiencyTool interface
  /// @{

  /** Initialise the tool.
   *
   *  This is the stage at which all of the configuration is done and the underlying @c CalibrationDataInferfaceROOT object is instantiated.
   *  The properties that can be set are documented in the
   *  <a href="https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BTaggingCalibrationDataInterface#xAOD_interface">xAOD interface</a> section
   *  of the CalibrationDataInterface Twiki page.
   */
  StatusCode initialize();

  /** Set the tool to return "shifted" values.
      Note that only single-parameter variations or empty sets (the latter are to revert to nominal results)
      are supported at present; @c StatusCode::FAILURE will be returned for variations of multiple parameters
      or variations that aren't recognised.
   */
  StatusCode applySystematicVariation(const CP::SystematicSet & set);

  /** Return a list of all systematic variations supported by this tool.
      Note that this list depends on the uncertainty model used, and on the (dynamic)
      configuration of the eigenvector variations (if this uncertainty model is used).
   */
  CP::SystematicSet affectingSystematics() const;

  /** Return a list of "recommended" systematic variations supported by this tool.
      At present, multiple views of the same uncertainties (beyond the uncertainty model etc., see above)
      are not implemented, so this method simply calls @c affectingSystematics() .
   */
  CP::SystematicSet recommendedSystematics() const;

  /// Returns whether or not the given systematic variation is supported by this tool
  bool isAffectedBySystematic(const CP::SystematicVariation & systematic ) const;

  /**
   * @brief Specify the "map index" to be used for the given jet flavour (at initialisation time it will be set to 0).
   *
   * @return false if the requested index is invalid (in which case no setting will be changed)
   *
   * See the <a href="https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BTaggingCalibrationDataInterface#MultipleMC">CalibrationDataInterface</a>
   * documentation for more detail on the meaning of the map index
   */
  bool setMapIndex(const std::string& flavour, unsigned int index);
  bool setMapIndex(unsigned int dsid);
  // bool setMapIndex(const std::string& flavour, const std::string & type);
  /// @}

  /// @name query methods
  /// @{
  /// Utility method returning a detailed list of systematics (including the flavours to which they apply)
  const std::map<CP::SystematicVariation, std::vector<std::string> > listSystematics() const;

  /// Retrieve the name of the tagger (as specified in the calibration file)
  const std::string & getTaggerName() const { return m_taggerName;}

  /// Retrieve the operating point (as specified in the calibration file)
  const std::string & getOperatingPoint() const { return m_OP;}

  /// Retrieve the jet collection name (as specified in the calibration file) for which this tool was setup
  const std::string & getJetAuthor() const { return m_jetAuthor;}

  // /// Returns false if the tool isn't initialised yet (it has to be initialised before processing jets)
  // bool isInitialized() const { return m_initialised;}

  /// Specify whether any systematic variation is being used at present
  bool applySystematics() const { return m_applySyst;}

  /**
   * This merely passes on the request to the underlying CDI object (listSystematics() cannot be used here, as corresponding CP::SystematicVariation objects may not exist).
   * Note that the uncertainty naming does not follow the rewriting conventions leading to the names one will see as CP::SystematicVariations, but rather follows the "raw"
   * names used on input (which are appropriate e.g. when excluding uncertainties from the eigenvalue decomposition).
   */
  std::map<std::string, std::vector<std::string> > listScaleFactorSystematics(bool named = false) const;
  /// @}

  /**
   * Run EigenvectorRecomposition method and get the coefficient map.
   * Calling EigenVectorRecomposition method in CDI and retrieve recomposition map.
   * If success, coefficientMap would be filled and return ok.
   * If failed, return error.
   * label  :  flavour label
   * coefficientMap: store returned coefficient map. This map could help expressing eigenvector NPs by linear
   * combination of original uncertainty NPs in workspace level of physics analysis. The coefficient value
   * is stored in the map in the format of:
   * map<"Eigen_B_0", map<"[original uncertainty name]", [corresponding coefficient value]>> 
   */
  CP::CorrectionCode getEigenRecompositionCoefficientMap(const std::string &label, std::map<std::string, std::map<std::string, float>> & coefficientMap);
  /// @}

private:

  struct SystInfo {
    SystInfo() : uncType(Analysis::SFEigen), isUp(true) {;}
    std::map<unsigned int, unsigned int> indexMap;
    Analysis::Uncertainty uncType;
    // bool isNamed;
    bool isUp;
    bool getIndex( unsigned int flavourID, unsigned int & index) const;
  };

  /// add entries to the systematics registry
  bool addSystematics(const std::vector<std::string> & systematicNames,unsigned int flavourID, Analysis::Uncertainty uncType);

  /// generate names for the eigenvector variations for the given jet flavour
  std::vector<std::string> makeEigenSyst(const std::string & flav, int number, const std::string& suffix);

  /// helper function for retrieving object indices
  bool getIndices(unsigned int flavour, unsigned int & sf, unsigned int & ef) const;

  /// convert integer flavour index to its string equivalent
  std::string getLabel(int flavourID) const {
    switch(flavourID) {
    case 5:
      return "B";
      break;
    case 4:
      return "C";
      break;
    case 15:
      return "T";
      break;
    case 0:
      return "Light";
      break;
    default:
      return "Light";
    }
  }

  /// convert string flavour to its integer index equivalent
  unsigned int getFlavourID(const std::string& label, bool conventional = true) const {
    // always default to "light" = 0
    if( label.size() <1)
      return 0;
    if (conventional){
      switch (label[0]) {
      case 'B':
        return 5; break;
      case 'C':
        return 4; break;
      case 'T':
        return 15; break;
      default:
        return 0;
      }
    } else {
        ATH_MSG_WARNING("Non-conventional label, return flavour ID = 0!");
        return 0;
    }
  }

  /** Fill the @c Analysis::CalibrationDataVariables struct with relevant information pertaining to the jet considered

      @return false if the requested information cannot be retrieved (this should never happen
              except if "continuous tagging" is used and the no b-tagging was applied to the jet)
   */
  bool fillVariables(const xAOD::Jet& jet, Analysis::CalibrationDataVariables& x) const;

  /** Fill the @c Analysis::CalibrationDataVariables struct with relevant information pertaining to the jet considered
   */
  bool fillVariables(const double jetPt, const double jetEta, const double jetTagWeight, Analysis::CalibrationDataVariables& x) const;

  /// pointer to the object doing the actual work
  //Analysis::CalibrationDataInterfaceROOT*  m_CDI = nullptr;
   std::shared_ptr<Analysis::CalibrationDataInterfaceROOT> m_CDI;
   /// pointer to the onnx tool
  std::unique_ptr<SaltModel> m_saltModel;

  /// @name core configuration properties (set at initalization time and not modified afterwards)
  /// @{

  /// we need access to a BTaggingSelectionTool, at least for DL1 weight computation
  asg::AnaToolHandle<IBTaggingSelectionTool> m_selectionTool;

  /// name of the data/MC efficiency scale factor calibration file (may be changed by the @c PathResolver)
  Gaudi::Property<std::string> m_SFFile{this, "ScaleFactorFileName", ftag::defaults::cdi_path, "name of the official scale factor calibration CDI file (uses PathResolver)"};
  std::string m_SFFileFull;
  Gaudi::Property<std::string> m_SelectionCDIFile{this, "SelectionCDIFileName", "", "name of the CDI file to be used to configure the selection tool if needed, will use the SF CDI file by default"};
  /// name of the optional MC efficiency file (may be changed by the @c PathResolver)
  Gaudi::Property<std::string> m_EffFile{this, "EfficiencyFileName", "", "name of optional user-provided MC efficiency CDI file"};
  Gaudi::Property<std::string> m_EffConfigFile{this, "EfficiencyConfig", "", "name of config file specifying which efficiency map to use with a given samples DSID"};
  /// names of the data/MC scale factor calibrations
  Gaudi::Property<std::string> m_SFNamesB{this, "ScaleFactorBCalibration", "default", "name of b-jet scale factor calibration object"};
  Gaudi::Property<std::string> m_SFNamesC{this, "ScaleFactorCCalibration", "default", "name of c-jet scale factor calibration object"};
  Gaudi::Property<std::string> m_SFNamesT{this, "ScaleFactorTCalibration", "default", "name of tau-jet scale factor calibration object"};
  Gaudi::Property<std::string> m_SFNamesLight{this, "ScaleFactorLightCalibration", "default", "name of light-flavour jet scale factor calibration object"};
  std::map<std::string, std::string> m_SFNames;

  /// specification of the eigenvector reduction strategy (if eigenvectors are used)
  Gaudi::Property<std::string> m_EVReductionB{this, "EigenvectorReductionB", "Loose", "b-jet scale factor Eigenvector reduction strategy; choose between 'Loose', 'Medium', 'Tight'"};
  Gaudi::Property<std::string> m_EVReductionC{this, "EigenvectorReductionC", "Loose", "c-jet scale factor Eigenvector reduction strategy; choose between 'Loose', 'Medium', 'Tight'"};
  Gaudi::Property<std::string> m_EVReductionLight{this, "EigenvectorReductionLight", "Loose", "light-flavour jet scale factor Eigenvector reduction strategy; choose between 'Loose', 'Medium', 'Tight'"};
  std::map<std::string, std::string> m_EVReduction;

  /// semicolon-separated lists of MC efficiency parametrisation names
  Gaudi::Property<std::string> m_EffNamesB{this, "EfficiencyBCalibrations", "", "(semicolon-separated) name(s) of b-jet efficiency object(s)"};
  Gaudi::Property<std::string> m_EffNamesC{this, "EfficiencyCCalibrations", "", "(semicolon-separated) name(s) of c-jet efficiency object(s)"};
  Gaudi::Property<std::string> m_EffNamesT{this, "EfficiencyTCalibrations", "", "(semicolon-separated) name(s) of tau-jet efficiency object(s)"};
  Gaudi::Property<std::string> m_EffNamesLight{this, "EfficiencyLightCalibrations", "", "(semicolon-separated) name(s) of light-flavour-jet efficiency object(s)"};
  std::map<std::string, std::string> m_EffNames;

  // default value for all flavors, use this if specified
  Gaudi::Property<std::string> m_effName{this, "EfficiencyCalibrations", "default", "default for all flavors"};
  /// semicolon-separated list of uncertainties to be excluded from the eigenvector variation procedure for all flavours
  Gaudi::Property<std::string> m_excludeFromEV{this, "ExcludeFromEigenVectorTreatment", "", "(semicolon-separated) names of uncertainties to be excluded from all eigenvector decompositions (if used)"};
  /// semicolon-separated list of uncertainties to be excluded from the eigenvector variation procedure for b, c, and light-flavour jets
  Gaudi::Property<std::string> m_excludeFlvFromEVB{this, "ExcludeFromEigenVectorBTreatment", "", "(semicolon-separated) names of uncertainties to be excluded from b-jet eigenvector decompositions (if used)"};
  Gaudi::Property<std::string> m_excludeFlvFromEVC{this, "ExcludeFromEigenVectorCTreatment", "", "(semicolon-separated) names of uncertainties to be excluded from c-jet eigenvector decompositions (if used)"};
  Gaudi::Property<std::string> m_excludeFlvFromEVLight{this, "ExcludeFromEigenVectorLightTreatment", "", "(semicolon-separated) names of uncertainties to be excluded from light-flavour-jet eigenvector decompositions (if used)"};
  std::map<std::string, std::string> m_excludeFlvFromEV;

  /// optional (per-flavour) suffix that can be used to decorrelate uncertainties (between flavours, or -in case of a result from different runs- between periods)
  Gaudi::Property<std::string> m_uncertaintySuffixesB{this, "UncertaintyBSuffix", "", "optional suffix for b-jet uncertainty naming"};
  Gaudi::Property<std::string> m_uncertaintySuffixesC{this, "UncertaintyCSuffix", "", "optional suffix for c-jet uncertainty naming"};
  Gaudi::Property<std::string> m_uncertaintySuffixesT{this, "UncertaintyTSuffix", "", "optional suffix for tau-jet uncertainty naming"};
  Gaudi::Property<std::string> m_uncertaintySuffixesLight{this, "UncertaintyLightSuffix", "", "optional suffix for light-flavour-jet uncertainty naming"};
  std::map<std::string, std::string> m_uncertaintySuffixes;
  
  bool m_using_conventional_labels; // flag for if the labels in the CDI configuration are "conventional", e.g. "B", "C", "Light", and "T"
  // remove label dependence - work from a string of semi-colon sep. to populate m_SFNames
  Gaudi::Property<std::string> m_SFName_flex{this, "FlexibleScaleFactorCalibrations", "", "(semicolon-separated) name of scale factor calibration object for (0,1,2..) indexed flavour labels, e.g. '0:default;1:default;2:default;3:default'"};
  // remove label dependence - work from a string of semi-colon sep. to populate m_EVReduction
  Gaudi::Property<std::string> m_EVReduction_flex{this, "FlexibleEigenvectorReduction", "", "(semicolon-separated) list of eigenvector reduction strategy for (0,1,2..) indexed flavour labels; choose between 'Loose', 'Medium', 'Tight' for different labels, e.g. '0:Loose;1:Loose;2:Loose'"};
  // remove label dependence - work from a string of semi-colon sep. to populate m_EffNames
  Gaudi::Property<std::string> m_EffNames_flex{this, "FlexibleEfficiencyCalibrations", "", "(semicolon-separated) name(s) of efficiency object(s) names for (0,1,2..) indexed flavour labels, e.g. '0:default;1:default;2:default;3:default'"};
  // remove label dependence - work from a string of semi-colon sep. to populate m_uncertaintySuffixes
  Gaudi::Property<std::string> m_uncertaintySuffixes_flex{this, "FlexibleUncertaintySuffix", "", "optional (semicolon-separated) list of suffixes for (0,1,2..) indexed flavour label uncertainty naming, e.g. '0:;1:;2:;3:'"};
  // remove label dependence - work from a string of semi-colon sep. to populate m_excludeFlvFromEV
  Gaudi::Property<std::string> m_excludeFlvFromEV_flex{this, "FlexibleExcludeFromEVTreatment", "", "(semicolon-separated) names of uncertainties to be excluded from (0,1,2..) indexed flavour eigenvector decompositions (if used), e.g. '0:;1:;2:;3:'"};

  /// tagger name
  Gaudi::Property<std::string> m_taggerName{this, "TaggerName", ftag::defaults::tagger, "tagging algorithm name as specified in CDI file"};
  Gaudi::Property<std::string> m_selectionTaggerName{this, "SelectionTaggerName", "", "tagging algorithm name as specified in selection CDI file, will use TaggerName by default"};
  /// operating point
  Gaudi::Property<std::string> m_OP{this, "OperatingPoint", ftag::defaults::pcbt_op, "operating point as specified in CDI file"};
  ///  jet collection name
  Gaudi::Property<std::string> m_jetAuthor{this, "JetAuthor", ftag::defaults::jet_collection, "jet collection & JVF/JVT specification in CDI file"};
  ///  minimum jet pT
  Gaudi::Property<float> m_minPt{this, "MinPt", 0., "minimum jet pT cut"};
  /// systematics model to be used (current choices are "SFEigen", "SFEigenRefined", and "Envelope") // <-------- Addoing "SFGlobalEigen" to the list
  Gaudi::Property<std::string> m_systStrategy{this, "SystematicsStrategy", ftag::defaults::strategy, "name of systematics model; presently choose between 'SFEigen' and 'Envelope'"};
  /// if true, attempt to retrieve the data/MC efficiency scale factor calibration files from the @PathResolver development area
  Gaudi::Property<bool> m_useDevFile{this, "UseDevelopmentFile", false,
    "specify whether or not to use the (PathResolver) area for temporary scale factor calibration CDI files"};
  /// if true, use cone-based labelling (as opposed to ghost association)
  Gaudi::Property<bool> m_coneFlavourLabel{this, "ConeFlavourLabel", true, "specify whether or not to use the cone-based flavour labelling instead of the default ghost association based labelling"};
  /// if true, use an 'extended' labelling (allowing for multiple HF hadrons -or perhaps partons- in the jet)
  Gaudi::Property<bool> m_extFlavourLabel{this, "ExtendedFlavourLabel", false, "specify whether or not to use an 'extended' flavour labelling (allowing for multiple HF hadrons or perhaps partons)"};
  /// if true, extract pre-set lists of uncertainties to be recommended from the EV decomposition (in addition to user specified ones)
  Gaudi::Property<bool> m_useRecommendedEVExclusions{this, "ExcludeRecommendedFromEigenVectorTreatment", false, "specify whether or not to add recommended lists to the user specified eigenvector decomposition exclusion lists"};
  /// if true, ignore out-of-extrapolation range errors (i.e., return CorrectionCode::Ok if these are encountered)
  Gaudi::Property<bool> m_ignoreOutOfValidityRange{this, "IgnoreOutOfValidityRange", false, "ignore out-of-extrapolation-range errors as returned by the underlying tool"};
  /// if false, suppress any non-error/warning printout from the underlying tool
  /// 1D tagging only: define wether the cuts refer to b-tagging or c-tagging
  Gaudi::Property<bool> m_useCTag{this, "useCTagging", false, "Enabled only for FixedCut or Continuous WPs: define wether the cuts refer to b-tagging or c-tagging"};
  Gaudi::Property<bool> m_readFromBTaggingObject{this, "readFromBTaggingObject", true, "Enabled to access btagging scores from xAOD::BTagging object; Can be disabled for GN2v01 to access the scores from the jet itself."};
  /// if this string is empty, the onnx tool won't be created
  Gaudi::Property<std::string> m_pathToONNX{this, "pathToONNX", "", "path to the onnx file that will be used for inference"};
  /// @}

  // if true, use the flexible configuration of the CDIReader
  Gaudi::Property<bool> m_useFlex{this, "useFlexibleConfig", false, "Setup the flexible configuration of the xAODBTaggingEfficiencyTool with alternate labeling"};
  std::vector<std::string> m_flex_labels;
  std::vector<unsigned int> m_flex_label_integers;

  /// @name Cached variables
  /// @{

  /// flag to indicate tool is initialized correctly when set
  bool m_initialised;


  bool m_applySyst;
  SystInfo m_applyThisSyst;

  std::map<CP::SystematicVariation,SystInfo> m_systematicsInfo;
  // cached for affectedBySystematics
  CP::SystematicSet m_systematics;
  // specifically for continuous tagging
  bool m_isContinuous;
  // specifically for continuous 2D tagging
  bool m_isContinuous2D;
  // pointer to a member function of a b-tagger
  // tagWeight_member_t m_getTagWeight;

  // cache the mapIndex variables (one for each flavour)
  std::map<std::string, unsigned int> m_mapIndices;

  /// actual information identifying scale factor calibration objects
  std::map<unsigned int, unsigned int> m_SFIndices; // <------------- maps flavourID to an index corresponding to the index at which the container used for SFs is stored in CDIROOT's 
  /// actual information identifying efficiency calibration objects
  std::map<unsigned int, unsigned int> m_EffIndices;

  //cache for efficiency map config file that maps from a sample DSID to the correct efficiency map
  std::map<unsigned int, unsigned int> m_DSID_to_MapIndex;
  /// @}

};

#endif // CPBTAGGINGEFFICIENCYTOOL_H

