/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "DiTauRecTools/HelperFunctions.h"

#include <TObjString.h>
#include <TObjArray.h>
#include <TFile.h>
#include <TTree.h>

#include <iostream>

namespace DiTauRecTools {
  ANA_MSG_SOURCE(msgHelperFunction, "HelperFunction")
}

//________________________________________________________________________________
std::vector<TString> DiTauRecTools::parseString(const TString& str, const TString& delim/*=","*/){
  std::vector<TString> parsed_strings;
  TObjArray* varList_ar = str.Tokenize(delim);
  for(int i = 0; i != varList_ar->GetEntries(); ++i){
    if (auto *tos = dynamic_cast<TObjString*> (varList_ar->At(i))) {
      TString var = tos->String();
      if(var.Length()==0) continue;
      parsed_strings.push_back(var);
    }
  }
  delete varList_ar;
  return parsed_strings;
}

//________________________________________________________________________________
std::vector<TString> DiTauRecTools::parseStringMVAUtilsBDT(const TString& str, const TString& delim/*=","*/){
  std::vector<TString> parsed_strings = parseString(str, delim);
  for( TString& str : parsed_strings ){
    str.ReplaceAll(" ", "");
    if(str.Contains(":=")){
      str=str(str.Index(":=")+2, str.Length()-str.Index(":=")-2);
    }
  }
  return parsed_strings;
}

//________________________________________________________________________________
std::unique_ptr<MVAUtils::BDT> DiTauRecTools::configureMVABDT( std::map<TString, float*> &availableVars, const TString& weightFile){
  using namespace DiTauRecTools::msgHelperFunction;
  std::unique_ptr<TFile> fBDT(TFile::Open(weightFile));
  if(!fBDT){
    ANA_MSG_ERROR("configureMVABDT: Cannot find tau input BDT file: " << weightFile );
    return nullptr;
  }
  
  TTree* tBDT = dynamic_cast<TTree*> (fBDT->Get("BDT"));
  if(!tBDT){
    ANA_MSG_ERROR("configureMVABDT: Cannot find tau input BDT tree");
    return nullptr;
  }    

  ANA_MSG_INFO("configureMVABDT: opened file: " << weightFile);

  std::vector<float*> vars;

  //parsing of variables done here from TNamed object
  TNamed* n_varList = dynamic_cast<TNamed*> (fBDT->Get("varList"));
  if(!n_varList) {
    ANA_MSG_ERROR("configureMVABDT: no Variable List in file: " << weightFile );
    return nullptr;
  }
  std::vector<TString> varList_ar = DiTauRecTools::parseStringMVAUtilsBDT(n_varList->GetTitle());
  delete n_varList;

  for(const TString& str : varList_ar){
    if(str.Length()==0) continue;
    std::map<TString, float*>::iterator itr = availableVars.find(str);
    if(itr==availableVars.end()){
      ANA_MSG_ERROR("configureMVABDT: Variable : " << str << " is not available" );
      return nullptr;
    }
    vars.push_back( itr->second );
  }
  
  auto reader = std::make_unique<MVAUtils::BDT>(tBDT);
  reader->SetPointers( vars );

  fBDT->Close();
  return reader;
}

