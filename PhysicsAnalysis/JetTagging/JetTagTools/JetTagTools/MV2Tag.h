/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTAGTOOLS_MV2TAG_H
#define JETTAGTOOLS_MV2TAG_H

/******************************************************
    @class MV2Tag
    Neural-net tagger combining weights of IP3D, SV1 and
    JetFitterCombNN
    @ author L. Vacavant
********************************************************/

#include "AthenaBaseComps/AthAlgTool.h"
#include "JetTagTools/IMultivariateJetTagger.h"
#include <string>
#include <map>
#include <list>
#include "MVAUtils/BDT.h"
#include "JetTagCalibration/JetTagCalibCondData.h"
#include "TLorentzVector.h"

namespace Analysis {

  class MV2Tag : public extends<AthAlgTool, IMultivariateJetTagger>
  {
  public:
    MV2Tag(const std::string& name,
       const std::string& n,
       const IInterface*);

    /**
       Implementations of the methods defined in the abstract base class
    */
    virtual ~MV2Tag() = default;
    virtual StatusCode initialize()override ;

    virtual
    void assignProbability(xAOD::BTagging* BTag,
                           const std::map<std::string,double>& inputs,
                           const std::string& jetauthor) const override;


  private:

    std::string m_taggerNameBase; // unique name for regular and flip versions
    std::string m_varStrName;

    /** Key of calibration data: */
    SG::ReadCondHandleKey<JetTagCalibCondData> m_readKey{this, "HistosKey", "JetTagCalibHistosKey", "Key of input (derived) JetTag calibration data"};
    bool m_forceMV2CalibrationAlias;
    std::string m_MV2CalibAlias;
    std::string m_xAODBaseName;

    std::map<std::string, double > m_defaultvals;
    /// Map from names in tool input to names in calibration file.
    std::map<std::string, std::string > m_MVTM_name_translations;
    /// Map from names in calibration file to names in tool input.
    std::map<std::string, std::string > m_MVTM_name_backtrans;


    const unsigned m_nClasses=3;//b,u,c probabilities. It might be better to read from calib file for future
    //const bool m_writeRootFile=false;//Developer option
    mutable std::atomic<bool> m_disableAlgo;
    mutable std::atomic<int>  m_warnCounter;

    /** This switch is needed to indicate what to do. The algorithm can be run to produce
	reference histograms from the given MC files (m_runModus=0) or to work in analysis mode
	(m_runModus=1) where already made reference histograms are read.*/
    std::string    m_runModus;          //!< 0=Do not read histos, 1=Read referece histos (analysis mode)

    float d0sgn_wrtJet(const TLorentzVector& jet, const TLorentzVector& trk, float d0sig) const;
    float z0sgn_wrtJet(float trackTheta, float trackZ0, float jetEta) const;

    std::vector<float>
    CreateVariables (const std::map<std::string, double> &inputs,
                     const std::vector<std::string>& inputVars) const;

    std::vector<float>
    GetMulticlassResponse(const std::vector<float>& vars,
                          const MVAUtils::BDT* bdt) const
    {
      std::vector<float> v(m_nClasses,-1);
      return (vars.size() ? bdt->GetMultiResponse(vars,m_nClasses) : v);
    }
    double GetClassResponse (const std::vector<float>& vars,
                             const MVAUtils::BDT* bdt) const
    {
      return (vars.size() ? bdt->GetGradBoostMVA(vars) : -9.);
    }


  }; // End class

} // End namespace

#endif
