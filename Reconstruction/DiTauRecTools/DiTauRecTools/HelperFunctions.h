/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAURECTOOLS_HELPERFUNCTIONS_H
#define DITAURECTOOLS_HELPERFUNCTIONS_H

#include "xAODTau/DiTauJet.h"

#include "AsgMessaging/MessageCheck.h"

#include "MVAUtils/BDT.h"
#include "TLorentzVector.h"
#include "TString.h"

#include <vector>
#include <map>


namespace DiTauRecTools
{
  ANA_MSG_HEADER(msgHelperFunction)

  std::unique_ptr<MVAUtils::BDT> configureMVABDT( std::map<TString, float*> &availableVars, const TString& weightFile);

  std::vector<TString> parseString(const TString& str, const TString& delim=",");
  std::vector<TString> parseStringMVAUtilsBDT(const TString& str, const TString& delim=",");
}

#endif // DITAURECTOOLS_HELPERFUNCTIONS_H
