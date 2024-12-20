/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CPBTAGGINGSELECTIONTOOL_H
#define CPBTAGGINGSELECTIONTOOL_H

#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"
#include "xAODBTagging/BTagging.h"

#include "AsgTools/AsgTool.h"
#include "PATCore/IAsgSelectionTool.h"
#include "CxxUtils/checker_macros.h"
#include "TFile.h"
#include "TSpline.h"
#include "TVector.h"
#include "TMatrixD.h"
#include <string>
#include <set>
#include <vector>
#include <map>
#include <limits>

class BTaggingSelectionTool: public asg::AsgTool,
			     public virtual IBTaggingSelectionTool,
			     public virtual IAsgSelectionTool  {
  typedef double (xAOD::BTagging::* tagWeight_member_t)() const;

  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS2( BTaggingSelectionTool , IAsgSelectionTool, IBTaggingSelectionTool )

  public:
  /// Create a constructor for standalone usage
  BTaggingSelectionTool( const std::string& name );
  StatusCode initialize() override;

  /// Get the decision using a generic IParticle pointer
  virtual asg::AcceptData accept( const xAOD::IParticle* p ) const override;
  virtual asg::AcceptData accept( const xAOD::Jet& jet ) const override;

  /// Get the decision using thet jet's pt and tag weight values
  virtual asg::AcceptData accept(double /* jet pt */, double /* jet eta */, double /* tag_weight */ ) const override;
  virtual asg::AcceptData accept(double /* jet pt */, double /* jet eta*/ , double /* taggerWeight_b */, double /* taggerWeight_c */) const override;
  virtual asg::AcceptData accept(double /* jet pt */, double /* jet eta */, double /* dl1pb */, double /* dl1pc  */, double /* dl1pu */) const override; 
  virtual asg::AcceptData accept(double /* jet pt */, double /* jet eta */, double /* dl1pb */, double /* dl1pc  */, double /* dl1pu  */, double /* dl1ptau */) const override;

  /// Decide in which quantile of the tag weight distribution the jet belongs (continuous tagging)
  /// The return value represents the bin index of the quantile distribution
  virtual int getQuantile( const xAOD::IParticle* ) const override;
  virtual int getQuantile( const xAOD::Jet& ) const override;
  virtual int getQuantile( double /* jet pt */, double /* jet eta */, double /* tag weight */  ) const override;
  virtual int getQuantile( double /*pT*/, double /*eta*/, double /*tag_weight_b*/, double /*tag_weight_c*/ ) const override;

  virtual CP::CorrectionCode getCutValue(double /* jet pt */, double & cutval) const override;
   //1D tagging wrapper
  virtual CP::CorrectionCode getTaggerWeight( const xAOD::Jet& jet, double & tagweight) const override;
  virtual CP::CorrectionCode getTaggerWeight( double pb, double pc, double pu, double & tagweight) const override;
  virtual CP::CorrectionCode getTaggerWeight( double pb, double pc, double pu, double & tagweight, double ptau) const override;

  //flexibility for Continuous2D
  virtual CP::CorrectionCode getTaggerWeight( const xAOD::Jet& jet, double & weight ,bool getCTagW) const override;
  virtual CP::CorrectionCode getTaggerWeight( double /* dl1pb */, double /* dl1pc  */ , double /* dl1pu  */, double & weight, bool getCTagW , double /* dl1ptau  */ = 0.) const override;
  const asg::AcceptInfo& getAcceptInfo( ) const  override {return m_acceptinfo;} 
private:
  /// Helper function that decides whether a jet belongs to the correct jet selection for b-tagging
  virtual bool checkRange( double /* jet pt */, double /* jet eta */ , asg::AcceptData& ) const;
  //fill the spline or vector that store the cut values for a particular working point
  void InitializeTaggerVariables(std::string taggerName,std::string OP, TSpline3 *spline, TVector *constcut, double &fraction);

  bool m_initialised = false;
  bool m_ErrorOnTagWeightFailure = true;
  bool m_StoreNConstituents = false;
  bool m_continuous   = false; //Continuous1D
  bool m_continuous2D = false; //Continuous2D
  bool m_useCTag = false; //use c-tagging or b-tagging in 1D
  bool m_readFromBTaggingObject = true; //use xAOD::BTagging object or not
  /// Object used to store the last decision
  asg::AcceptInfo m_acceptinfo;  

  double m_maxEta{};
  double m_minPt{};
  double m_maxRangePt{};

  std::string m_CutFileName;
  std::string m_taggerName;
  std::string m_OP;
  std::string m_jetAuthor;
  std::string m_ContinuousBenchmarks;
  std::string m_wps_raw;

  TFile *m_inf{};
  std::vector<double> m_continuouscuts;

  SG::AuxElement::ConstAccessor<float> m_accessor_pb;
  SG::AuxElement::ConstAccessor<float> m_accessor_pc;
  SG::AuxElement::ConstAccessor<float> m_accessor_pu;
  SG::AuxElement::ConstAccessor<float> m_accessor_ptau;

  struct taggerproperties{
    std::string  name;
    double fraction_b = std::numeric_limits<double>::max(); 
    double fraction_c = std::numeric_limits<double>::max();
    double fraction_tau = std::numeric_limits<double>::max();
    double fraction_tau_cTag = std::numeric_limits<double>::max();
    TSpline3*  spline = nullptr;
    TVector* constcut = nullptr;
    TMatrixD*  cuts2D = nullptr; //useful only in Continuous2D
    std::vector<int>  benchmarks; //useful only in Continuous WP. list of bins that are considered as tagged. 

    double get2DCutValue(int row, int column) const{
      TMatrixD& cuts2D_safe ATLAS_THREAD_SAFE = *(this->cuts2D); 
      double cut = cuts2D_safe(row,column);
      return cut;
    }

  };

  taggerproperties m_tagger;

  enum Tagger{UNKNOWN, DL1, GN1, GN2, MV2c10};
  Tagger m_taggerEnum{UNKNOWN};

  Tagger SetTaggerEnum(const std::string& taggerName){
    if(taggerName.find("DL1") != std::string::npos) return Tagger::DL1;
    else if(taggerName.find("GN1") != std::string::npos) return Tagger::GN1;
    else if(taggerName.find("GN2") != std::string::npos) return Tagger::GN2;
    else if(taggerName == "MV2c10") return Tagger::MV2c10;
    else 
      ATH_MSG_ERROR("Tagger Name NOT supported.");
    return Tagger::UNKNOWN;
  };
  //get from the CDI file the taggers cut object(that holds the definition of cut values)
  //and flaovur fraction (for DL1 tagger) and store them in the right taggerproperties struct
  StatusCode ExtractTaggerProperties(taggerproperties& tagger, const std::string& taggerName, const std::string& OP);

  std::vector<std::string> split (const std::string &input, const char &delimiter);

};

#endif // CPBTAGGINGSELECTIONTOOL_H
