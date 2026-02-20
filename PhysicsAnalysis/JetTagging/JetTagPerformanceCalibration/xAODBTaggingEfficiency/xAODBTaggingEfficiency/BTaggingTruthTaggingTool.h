// Dear emacs, this is -*- c++ -*-
///////////////////////////////////////////////////////////////////
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
// BTaggingTruthTaggingTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
/**
  @class BTaggingTruthTaggingTool
  Tool to apply flavour-tagging requirements on jets
  @modified by Ilaria Luise, Nilotpal Kakati in March 2022
  @author C. Rizzi, M. Ughetto
  @contact chiara.rizzi@cern.ch, mughetto@cern.ch
  @contact ilaria.luise@cern.ch, nkakati@cern.ch
**/

#ifndef CPBTAGGINGTRUTHTAGGINGTOOL_H
#define CPBTAGGINGTRUTHTAGGINGTOOL_H

#include "FTagAnalysisInterfaces/IBTaggingTruthTaggingTool.h"
#include "xAODBTagging/BTagging.h"

#include "AsgTools/AsgTool.h"
#include "AsgMessaging/MessageCheck.h"

#include "TFile.h"
#include "TRandom3.h"
#include "TVector.h"
#include "TFile.h"
#include "TMatrixD.h"
#include <string>
#include <vector>
#include <map>

// include xAODBtaggingEfficiency classes
#include "FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h"
#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"
#include "xAODBTaggingEfficiency/BTaggingEfficiencyTool.h"
#include "AsgTools/AnaToolHandle.h"
#include <AsgTools/PropertyWrapper.h>


// calibration data variable
#include "CalibrationDataInterface/CalibrationDataVariables.h"
#include "xAODBTaggingEfficiency/TruthTagResults.h"

// xAOD jet
#include "xAODJet/JetContainer.h"

class BTaggingTruthTaggingTool: public asg::AsgTool,
				public virtual IBTaggingTruthTaggingTool {
  //typedef float (xAOD::BTagging::* tagWeight_member_t)() const;
  
  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS3( BTaggingTruthTaggingTool , IBTaggingTruthTaggingTool, ISystematicsTool, CP::IReentrantSystematicsTool )
  
  private:
  
    struct TagBin{
     
      float bcut_low = -99.;   ///
      float bcut_hig = +99.;   ///
      float ccut_low = -99.;   ///
      float ccut_hig = +99.;   ///
      bool is_tagbin = false; ///
      
      //default constructor
      TagBin(){ };

      //constructor
      TagBin(bool is_tb, float b_low, float b_hig, float c_low = -99., float c_hig = +99){    
      	bcut_low = b_low;
	      bcut_hig = b_hig;
	      ccut_low = c_low;
	      ccut_hig = c_hig;
	      is_tagbin = is_tb;
      };

      //destructor
      ~TagBin(){ };

    };

    struct jetVariable{
      Analysis::CalibrationDataVariables vars;
      int flav;
    };

    // all the results about a single event are stored in this object
    struct TRFinfo {
      
      std::vector<jetVariable> jets;
        
      // features that will be used by the onnx tool
      std::vector<std::vector<float> > node_feat;
        
      unsigned int njets;

      TRandom3 rand;

      std::vector<std::vector<bool> > perm_ex; // for each tag mult, vector of bool: is the i-th jet tagged or not?
      std::vector<std::vector<bool> > perm_in;
      std::vector<std::vector<int> > tbins_ex; //for each tag mult, vector of int: quantile of each jet
      std::vector<std::vector<int> > tbins_in;

      std::vector<float>  trfw_ex; // vector with truth-tag weights (pos = # of b-tags)
      std::vector<float>  trfw_in;

      std::map<int,std::vector<float>> effMC_allBins; // map of efficiencies for each tag-bin 
      std::vector<float> permprob_ex; // probablity of chosen perm with nominal SF
      std::vector<float> permprob_in;
      std::vector<float> binsprob_ex; // probability of chosen quantile with nominal SF
      std::vector<float> binsprob_in;

      std::map<int,std::vector<std::vector<std::vector<bool> > > > perms;
      std::vector<std::vector<float> > permsWeight;
      std::vector<std::vector<float> > permsSumWeight;

    };

  public:
  /// Create a constructor for standalone usage
  BTaggingTruthTaggingTool( const std::string& name );

  private:
  StatusCode CalculateResults(TRFinfo &trfinf, Analysis::TruthTagResults& results,int rand_seed = -1);
            
  public:
  StatusCode CalculateResults( std::vector<float>& pt, std::vector<float>& eta, std::vector<int>& flav, std::vector<float>& tagw, Analysis::TruthTagResults& results,int rand_seed = -1);
  StatusCode CalculateResults( const xAOD::JetContainer& jets, Analysis::TruthTagResults& results,int rand_seed = -1);
        
  // will use onnxtool
  StatusCode CalculateResultsONNX( const std::vector<std::vector<float>>& node_feat, std::vector<float>& tagw,  Analysis::TruthTagResults& results, int rand_seed=-1);
  StatusCode CalculateResultsONNX( const xAOD::JetContainer& jets, const std::vector<std::vector<float>>& node_feat, Analysis::TruthTagResults& results, int rand_seed = -1);

  StatusCode setEffMapIndex(const std::string& flavour, unsigned int index);
  void setUseSystematics(bool useSystematics);

  virtual  ~BTaggingTruthTaggingTool();

  StatusCode initialize();

  CP::SystematicSet affectingSystematics() const;
  StatusCode applySystematicVariation( const CP::SystematicSet & systConfig);
  CP::SystematicSet recommendedSystematics() const;
  bool isAffectedBySystematic( const CP::SystematicVariation & systematic ) const;


  private:

  // set the jets in the event (pass same jets that satisfy kinematic criteria for b-tagging in pT and eta)
  StatusCode setJets(TRFinfo &trfinf,std::vector<float>& pt, std::vector<float>& eta, std::vector<int>& flav, std::vector<float>& tagw);
  StatusCode setJets(TRFinfo &trfinf,const xAOD::JetContainer& jets);
  StatusCode setJets(TRFinfo &trfinf,std::vector<int>& flav, std::vector<Analysis::CalibrationDataVariables>* vars);

  // overloaded with node_feat that is used by onnx tool
  StatusCode setJets(TRFinfo &trfinf, const std::vector<std::vector<float>>& node_feat, std::vector<float>& tagw);
  StatusCode setJets(TRFinfo &trfinf, const xAOD::JetContainer& jets, const std::vector<std::vector<float>>& node_feat);
  StatusCode setJets(TRFinfo &trfinf,std::vector<int>& flav, const std::vector<Analysis::CalibrationDataVariables>* vars, const std::vector<std::vector<float>>& node_feat);
            
  // get truth tagging weights
  // for one single systematic (including "Nominal")
  StatusCode GetTruthTagWeights(TRFinfo &trfinf, std::vector<float> &trf_weight_ex, std::vector<float> &trf_weight_in);

  // tag permutation: trf_chosen_perm_ex.at(ntag).at(i) tells if the i-th jet is tagged in a selection requiring == ntag tags
  StatusCode getTagPermutation(TRFinfo &trfinf, std::vector<std::vector<bool> > &trf_chosen_perm_ex, std::vector<std::vector<bool> > &trf_chosen_perm_in);

  // chosen quantile: trf_bin_ex.at(ntag).at(i) tells the quantile in which the i-th jet falls in a selection requiring == ntag tags
  // returns 5 if between 60% and 0%
  // returns 4 if between 70% and 60%
  // returns 3 if between 77% and 70%
  // returns 2 if between 85% and 77%
  // returns 1 if between 100% and 85%
  // returns 0 if smaller than -1e4-> should never happen --> not currently implemented
  // return -1 if bigger than 1e4 or not in b-tagging acceptance --> not currently implemented
  StatusCode getQuantiles(TRFinfo &trfinf,std::vector<std::vector<int> > &trf_bin_ex, std::vector<std::vector<int> > &trf_bin_in);

  // functions to make comparison with direct-tagging easier
  float getEvtSF(TRFinfo &trfinf,std::vector<int> & quantiles);
  StatusCode getDirectTaggedJets(TRFinfo &trfinf,std::vector<bool> &is_tagged);

  //These WP must be listed in ascending order of cut value, meaning 85 to 60
  std::vector<std::string> m_availableOP_fixCut= {"FixedCutBEff_85", "FixedCutBEff_77","FixedCutBEff_70","FixedCutBEff_60"};

  TFile *m_inf{}; //file for reading the cut values from the CDI.

  //vector storing the cuts, one for each tag bin 
  std::vector<TagBin> m_cuts;

  bool m_initialised{};

  StatusCode getTRFweight(TRFinfo &trfinf,unsigned int nbtag, bool isInclusive);

  StatusCode getAllEffMC(TRFinfo &trfinf);
  StatusCode getAllEffMCCDI(TRFinfo &trfinf);
  StatusCode getAllEffMCGNN(TRFinfo &trfinf);
            
  StatusCode getAllEffSF(TRFinfo &trfinf,int =0);
  std::vector<CP::SystematicSet> m_eff_syst;
  std::vector<std::string> m_sys_name;

  // flav labelling
  int jetFlavourLabel (const xAOD::Jet& jet);

  int GAFinalHadronFlavourLabel(const xAOD::Jet& jet);
  int ExclusiveConeHadronFlavourLabel (const xAOD::Jet& jet);
  std::vector<std::string> split(const std::string& str, char token);
  //*********************************//
  // Prop. of BTaggingEfficiencyTool //
  //*********************************//

  /// name of the data/MC efficiency scale factor calibration file (may be changed by the @c PathResolver)
  Gaudi::Property<std::string> m_SFFile {this, "ScaleFactorFileName", "xAODBTaggingEfficiency/13TeV/2016-20_7-13TeV-MC15-CDI-July12_v1.root", "name of the official scale factor calibration CDI file (uses PathResolver)"};
  /// name of the optional MC efficiency file (may be changed by the @c PathResolver)
  Gaudi::Property<std::string> m_EffFile {this, "EfficiencyFileName", "", "name of optional user-provided MC efficiency CDI file"};
  /// name of the data/MC scale factor calibration for b jets
  Gaudi::Property<std::string> m_SFBName {this, "ScaleFactorBCalibration", "default", "name of b-jet scale factor calibration object"};
  /// name of the data/MC scale factor calibration for charm jets
  Gaudi::Property<std::string> m_SFCName {this, "ScaleFactorCCalibration", "default", "name of c-jet scale factor calibration object"};
  /// name of the data/MC scale factor calibration for tau jets
  Gaudi::Property<std::string> m_SFTName {this, "ScaleFactorTCalibration", "default", "name of tau-jet scale factor calibration object"};
  /// name of the data/MC scale factor calibration for light-flavour jets
  Gaudi::Property<std::string> m_SFLightName {this, "ScaleFactorLightCalibration", "default",  "name of light-flavour jet scale factor calibration object"};
  /// specification of the eigenvector reduction strategy for b jets (if eigenvectors are used)
  Gaudi::Property<std::string> m_EVReductionB {this, "EigenvectorReductionB", "Loose", "b-jet scale factor Eigenvector reduction strategy; choose between 'Loose', 'Medium', 'Tight'"};
  /// specification of the eigenvector reduction strategy for c jets (if eigenvectors are used)
  Gaudi::Property<std::string> m_EVReductionC {this, "EigenvectorReductionC", "Loose", "c-jet scale factor Eigenvector reduction strategy; choose between 'Loose', 'Medium', 'Tight'"};
  /// specification of the eigenvector reduction strategy for light-flavour jets (if eigenvectors are used)
  Gaudi::Property<std::string> m_EVReductionLight {this, "EigenvectorReductionLight", "Loose","light-flavour jet scale factor Eigenvector reduction strategy; choose between 'Loose', 'Medium', 'Tight'"};
  /// semicolon-separated list of MC efficiency parametrisation names for b jets
  Gaudi::Property<std::string> m_EffBName {this, "EfficiencyBCalibrations", "default", "(semicolon-separated) name(s) of b-jet efficiency object(s)"};
  /// semicolon-separated list of MC efficiency parametrisation names for charm jets
  Gaudi::Property<std::string> m_EffCName {this, "EfficiencyCCalibrations", "default", "(semicolon-separated) name(s) of c-jet efficiency object(s)"};
  /// semicolon-separated list of MC efficiency parametrisation names for tau jets
  Gaudi::Property<std::string> m_EffTName {this, "EfficiencyTCalibrations", "default", "(semicolon-separated) name(s) of tau-jet efficiency object(s)"};
  /// semicolon-separated list of MC efficiency parametrisation names for light-flavour jets
  Gaudi::Property<std::string> m_EffLightName {this, "EfficiencyLightCalibrations", "default", "(semicolon-separated) name(s) of light-flavour-jet efficiency object(s)"};
  /// semicolon-separated list of uncertainties to be excluded from the eigenvector variation procedure
  Gaudi::Property<std::string> m_excludeFromEV {this, "ExcludeFromEigenVectorTreatment", "", "(semicolon-separated) names of uncertainties to be excluded from eigenvector decomposition (if used)"};
  /// tagger name
  Gaudi::Property<std::string> m_taggerName {this, "TaggerName", "MV2c10", "tagging algorithm name as specified in CDI file"};
  /// operating point
  Gaudi::Property<std::string> m_OP {this, "OperatingPoint", "FixedCutBEff_77", "operating point as specified in CDI file"};
  /// operating point when running in Continuous
  Gaudi::Property<std::string> m_cutBenchmark {this, "CutBenchmark", "1,2", "if you want to run in continuous you need to fix a benchmark - it does something only if running in Continuous OP"};
  ///  jet collection name
  Gaudi::Property<std::string> m_jetAuthor {this, "JetAuthor", "AntiKt4EMTopoJets", "jet collection & JVF/JVT specification in CDI file"};
  /// systematics model to be used (current choices are "SFEigen" and "Envelope")
  Gaudi::Property<std::string> m_systStrategy {this, "SystematicsStrategy", "SFEigen", "name of systematics model; presently choose between 'SFEigen' and 'Envelope'"};
  /// if true, attempt to retrieve the data/MC efficiency scale factor calibration files from the @PathResolver development area
  Gaudi::Property<bool> m_useDevFile {this, "UseDevelopmentFile", false, "specify whether or not to use the (PathResolver) area for temporary scale factor calibration CDI files"};
  /// if true, use cone-based labelling (as opposed to ghost association)
  Gaudi::Property<bool> m_coneFlavourLabel{this, "ConeFlavourLabel", true, "specify whether or not to use the cone-based flavour labelling instead of the default ghost association based labelling"};

  /// in case of continuous WP you can choose to ignore some of the eigenvectors
  Gaudi::Property<std::string> m_excludeEV {this, "ExcludeSpecificEigens", "" , "(semicolon-separated) names of Eigens you want to exclude. in case of continuous some eigenvectors can be ignored to make the computation faster"};
  ///possibility to compute the direct tagging SFs map directly from the TruthTaggingTool
  Gaudi::Property<bool> m_doDirectTag {this, "doDirectTagging", false , "If set to true it also computes and stores the direct tagging choice and the related SFs for each jet"};
  /// if this string is empty, the onnx tool won't be used
  Gaudi::Property<std::string> m_pathToONNX {this, "pathToONNX", "", "path to the onnx file that will be used for inference"};
  /// tagging strategy is required to do TT with GNN, when we don't want to truth tag all the jets (eg. 'leading2SignalJets')          
  Gaudi::Property<std::string> m_taggingStrategy {this, "TaggingStrategy", "AllJets", "tagging strategy in the Analysis (eg. 'leading2SignalJets' in boosted VHbb). Required to do TT with GNN"};
  /// will be set according to m_taggingStrategy
  enum NjetsTagStrategy {AllJets=-1, Leading2SignalJets=2, Leading3SignalJets=3};
  NjetsTagStrategy m_njetsTagStrategy{AllJets};
  
  //*********************************//
  // Prop. of BTaggingSelectionTool  //
  //*********************************//
  Gaudi::Property<float> m_minPt {this, "MinPt", 20000 /*MeV*/, "Minimum jet pT cut (in MeV)"};
  Gaudi::Property<float> m_maxEta {this, "MaxEta", 2.5, "Maximum jet eta cut"};
  Gaudi::Property<float> m_maxRangePt {this, "MaxRangePt", 1000000 /*MeV*/, "Max pT range (in MeV)"};
  
  // properties of truth tagging
  Gaudi::Property<bool> m_doOnlyUpVariations {this, "StoreOnlyUpVariations", false , "If set to true it processes only the __1up b-tagging variations. It speeds up the computation in case of symmetric variations."};
  
  bool m_continuous{};
  bool m_continuous2D{};

  // properties of BTaggingTruthTaggingTool
  Gaudi::Property<bool> m_ignoreSF {this, "IgnoreScaleFactors", true, "ignore scale factors in computation of TRF weight"};
  Gaudi::Property<bool> m_usePerm {this, "UsePermutations", true, "if the chosen permutation is used, a reweighting is applied to the TRF weight for systematics"};
  Gaudi::Property<bool> m_useQuantile {this, "UseQuantile", true, "if the chosen quantile is used, a reweighting is applied to the TRF weight for systematics"};
  Gaudi::Property<bool> m_useSys {this, "UseSystematics", false, "will the results contain all systematic variations, or just the nominal"};


  Gaudi::Property<int> m_nbtag {this, "MaxNtagged", 2, "what is the maximal possible number of tagged jets"};

  int m_nbins{};
  std::vector<int> m_OperatingBins;
  unsigned int m_OP_index_for_GNN{};

  std::map<int, asg::AnaToolHandle<IBTaggingEfficiencyTool> > m_effTool_allBins;


  asg::AnaToolHandle<IBTaggingEfficiencyTool> m_effTool;
  asg::AnaToolHandle<IBTaggingSelectionTool> m_selTool; //!

  StatusCode check_syst_range(unsigned int sys);

  std::vector<std::vector<bool> > generatePermutations(int njets, int tags, int start=0);

  float trfWeight(TRFinfo &trfinf,const std::vector<bool> &tags);

  StatusCode chooseAllTagPermutation(TRFinfo &trfinf,unsigned int nbtag);
  StatusCode chooseTagPermutation(TRFinfo &trfinf,unsigned int nbtag, bool isIncl);

  StatusCode chooseAllTagBins(TRFinfo &trfinf);
  StatusCode chooseTagBins_cum(TRFinfo &trfinf,std::vector<bool> &tagconf, bool isIncl, unsigned int nbtag);
  StatusCode generateRandomTaggerScores(std::vector< std::vector<int> > &quantiles, std::vector< std::vector<float> > &btag_scores, std::vector< std::vector<float> > & ctag_scores);
  float getTagBinsConfProb(TRFinfo &trfinf,std::vector<int> &tagws);

  StatusCode fillVariables(const xAOD::Jet& jet, Analysis::CalibrationDataVariables& x);
  StatusCode fillVariables(const float jetPt, const float jetEta, const float jetTagWeight, Analysis::CalibrationDataVariables& x);

};

#endif // CPBTAGGINGTRUTHTAGGINGTOOL_H

