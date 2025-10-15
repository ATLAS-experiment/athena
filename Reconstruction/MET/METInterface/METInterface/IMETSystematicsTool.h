///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// This is a copy from
// $Id: IMuonSmearingTool.h 299883 2014-03-28 17:34:16Z krasznaa $

#ifndef  METINTERFACE_IMETSYSTEMATICSTOOL_H
#define  METINTERFACE_IMETSYSTEMATICSTOOL_H

// Framework include(s):
#include "AsgTools/IAsgTool.h"
#include "PATInterfaces/ISystematicsTool.h"
#include "PATInterfaces/CorrectionCode.h"

// xAOD includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODMissingET/MissingET.h"
#include "xAODMissingET/MissingETAssociationMap.h"
#include "xAODMissingET/MissingETAssociationHelper.h"
#include "xAODJet/JetContainer.h"

namespace met {

  namespace softCaloAffSyst {
    const static CP::SystematicVariation MET_SoftCalo_ScaleUp  ("MET_SoftCalo_Scale__1up"  );
    const static CP::SystematicVariation MET_SoftCalo_ScaleDown("MET_SoftCalo_Scale__1down");
    const static CP::SystematicVariation MET_SoftCalo_ResoPara ("MET_SoftCalo_ResoPara" );
    const static CP::SystematicVariation MET_SoftCalo_ResoPerp ("MET_SoftCalo_ResoPerp" );
    const static CP::SystematicVariation MET_SoftCalo_ResoCorr ("MET_SoftCalo_ResoCorr" );
  }

  namespace softTrkAffSyst {
    const static CP::SystematicVariation MET_SoftTrk_ScaleUp  ("MET_SoftTrk_Scale__1up"  );
    const static CP::SystematicVariation MET_SoftTrk_ScaleDown("MET_SoftTrk_Scale__1down");
    const static CP::SystematicVariation MET_SoftTrk_ResoPara ("MET_SoftTrk_ResoPara" );
    const static CP::SystematicVariation MET_SoftTrk_ResoPerp ("MET_SoftTrk_ResoPerp" );
    const static CP::SystematicVariation MET_SoftTrk_ResoCorr ("MET_SoftTrk_ResoCorr" );
  }

  namespace jetTrkAffSyst {
    const static CP::SystematicVariation MET_JetTrk_ScaleUp   ("MET_JetTrk_Scale__1up"  );
    const static CP::SystematicVariation MET_JetTrk_ScaleDown ("MET_JetTrk_Scale__1down");
  }

  enum SystType {
    INVALID=-1,
    SOFTCALO,
    SOFTTRK,
    JETTRK
  };

  inline SystType getSystType(const CP::SystematicVariation & systematic)
  {
    if(systematic == met::softCaloAffSyst::MET_SoftCalo_ScaleUp ||
       systematic == met::softCaloAffSyst::MET_SoftCalo_ScaleDown ||
       systematic == met::softCaloAffSyst::MET_SoftCalo_ResoPara ||
       systematic == met::softCaloAffSyst::MET_SoftCalo_ResoPerp ||
       systematic == met::softCaloAffSyst::MET_SoftCalo_ResoCorr ) return SOFTCALO;
    if(systematic == met::softTrkAffSyst::MET_SoftTrk_ScaleUp ||
       systematic == met::softTrkAffSyst::MET_SoftTrk_ScaleDown ||
       systematic == met::softTrkAffSyst::MET_SoftTrk_ResoPara ||
       systematic == met::softTrkAffSyst::MET_SoftTrk_ResoPerp ||
       systematic == met::softTrkAffSyst::MET_SoftTrk_ResoCorr ) return SOFTTRK;
    if(systematic == met::jetTrkAffSyst::MET_JetTrk_ScaleUp ||
       systematic == met::jetTrkAffSyst::MET_JetTrk_ScaleDown ) return JETTRK;
    return INVALID;
  }

}

class IMETSystematicsTool : virtual public asg::IAsgTool,
			    virtual public CP::ISystematicsTool
{
  // Declare the interface that the class provides
  ASG_TOOL_INTERFACE( IMETSystematicsTool )

  public:
  virtual ~IMETSystematicsTool() {}

  virtual void setRandomSeed(unsigned long seed) const = 0;

  //we don't inherit from CorrectionTool directly, but we are something close to that
  virtual CP::CorrectionCode applyCorrection(xAOD::MissingET& met,
					     const xAOD::MissingETAssociationHelper& helper
					     ) const = 0;
  virtual CP::CorrectionCode correctedCopy(const xAOD::MissingET& met, xAOD::MissingET*& outputmet,
					   const xAOD::MissingETAssociationHelper& helper
					   ) const = 0;

  // virtual StatusCode applySystematicVariation(const CP::SystematicSet & set) = 0;
  // virtual CP::SystematicSet affectingSystematics() const = 0;
  // virtual CP::SystematicSet recommendedSystematics() const = 0;
  // virtual bool isAffectedBySystematic(const CP::SystematicVariation & systematic) const = 0;

}; // class IMETSystematicsTool

#endif //  METINTERFACE_IMETSYSTEMATICSTOOL_H
