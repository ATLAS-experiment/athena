/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_TAUSMEARINGTOOL_H
#define TAUANALYSISTOOLS_TAUSMEARINGTOOL_H

/*
  author: Dirk Duschinger
  mail: dirk.duschinger@cern.ch
*/

// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/AsgMetadataTool.h"
#include "AsgTools/PropertyWrapper.h"

// Local include(s):
#include "TauAnalysisTools/Enums.h"
#include "TauAnalysisTools/ITauSmearingTool.h"
#include "TauAnalysisTools/CommonSmearingTool.h"

namespace TauAnalysisTools
{

class TauSmearingTool
  : public asg::AsgMetadataTool
  , public virtual ITauSmearingTool
{
  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS( TauSmearingTool, TauAnalysisTools::ITauSmearingTool )

public:
  /// Create a constructor for standalone usage
  TauSmearingTool( const std::string& sName );

  ~TauSmearingTool();

  /// Function initialising the tool
  virtual StatusCode initialize();
  
  virtual StatusCode beginInputFile();
  
  /// Apply the correction on a modifyable object
  virtual CP::CorrectionCode applyCorrection( xAOD::TauJet& xTau ) const;

  /// Create a corrected copy from a constant tau
  virtual CP::CorrectionCode correctedCopy( const xAOD::TauJet& input,
      xAOD::TauJet*& output ) const;

  /// returns: whether this tool is affected by the given systematis
  virtual bool isAffectedBySystematic( const CP::SystematicVariation& systematic ) const;

  /// returns: the list of all systematics this tool can be affected by
  virtual CP::SystematicSet affectingSystematics() const;

  /// returns: the list of all systematics this tool recommends to use
  virtual CP::SystematicSet recommendedSystematics() const;

  virtual StatusCode applySystematicVariation( const CP::SystematicSet& systConfig );

private:
  ToolHandle<ITauSmearingTool> m_tCommonSmearingTool{this, "Tool", {}};

  Gaudi::Property<std::string> m_sInputFilePath{this, "InputFilePath", ""};
  Gaudi::Property<std::string> m_sRecommendationTag{this, "RecommendationTag", "2025-prerec"};
  Gaudi::Property<std::string> m_sCampaign{this, "Campaign", "mc23"};
  Gaudi::Property<bool> m_bSkipTruthMatchCheck{this, "SkipTruthMatchCheck", false};
  Gaudi::Property<bool> m_bApplyFading{this, "ApplyFading", true};
  Gaudi::Property<bool> m_bMVATESQualityCheck{this, "MVATESQualityCheck", true};
  Gaudi::Property<bool> m_bApplyInsituCorrection{this, "ApplyInsituCorrection", true};
  Gaudi::Property<bool> m_useFastSim{this, "useFastSim", false}; 
  Gaudi::Property<bool> m_useGNTau{this, "useGNTau",  false};

}; // class TauSmearingTool

} // namespace TauAnalysisTools

#endif // TAUANALYSISTOOLS_TAUSMEARINGTOOL_H
