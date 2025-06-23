/*
  Copyright (C) 2021-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
*  ____________________________________________________________________________
*  @file postProcessIDTPMHistos.cxx
*  @author: Max Goblirsch
*  Based on code by Liza Mijovic, Soeren Prell and the analogous script in IDPVM
*
*  Goal: Update resolutions extracted from 2D histograms 
*        after merging several output files (typically after grid running) 
*   ____________________________________________________________________________
*/

#include "InDetPhysValMonitoring/ResolutionHelper.h"

#include "TFile.h"
#include "TSystem.h"
#include "TH1.h"
#include "TH2.h"
#include "TObject.h"

#include <iostream>
#include <memory>
#include <string>

using namespace std;


bool file_exists(const string & p_name) {
  return !gSystem->AccessPathName(p_name.c_str(), kFileExists);    
}

// check if the name of an object matches what we expect from a resolution helper 
bool isResolutionHelper(TObject* entry){
  const std::string objName{entry->GetName()}; 
  return ((objName.find("resHelper") == 0 || objName.find("pullHelper") == 0) && dynamic_cast<TH1*>(entry));
}

// get the type ("res" or "pull") string and the substring
// after "Helper" in the 2D histo name, which specifies the x axis observableand resolution (y axis) 
// Relies on the conventions within IDTPM
std::pair< std::string, std::string > getTypeAndVars( const TObject* resHelper ) {
    const std::string name{ resHelper->GetName() };
    const std::string keyWord{ "Helper" }; 
    const size_t pos = name.find( keyWord );
    const size_t pos2 = pos + keyWord.size();
    return { name.substr( 0, pos ), name.substr( pos2 ) };
}

// clone an existing histogram of a known name
TH1* cloneExisting(const std::string & name){
    auto *h = gDirectory->Get(name.c_str()); 
    if (!h){ 
        std::cerr << "Could not find existing histogram "<<name<<" - will not postprocess "<<std::endl; 
        return nullptr;
    }
    auto *ret = dynamic_cast<TH1*>(h->Clone(name.c_str()));
    if (!ret){ 
        std::cerr << "Found an existing object "<<name<<", but it is not a histogram ("<<h->IsA()->GetName()<<") - will not postprocess "<<std::endl; 
    }
    return ret; // will also catch ret == nullptr
}

// get the names of the 1D histograms following IDTPM conventions. 
std::pair<std::string, std::string> getPullAndResoNames(const std::string & type){
  if (type == "res"){
    return {"resolution","resmean"}; 
  }
  else if (type == "pull"){
    return {"pullwidth","pullmean"}; 
  }
  else {
    std::cerr << " Not able to identify the histogram names for a resolution type "<<type<<" - supported are 'res' and 'pull'. "<<std::endl;
  }
  return {"",""}; 
}

int postProcessHistos(
    TObject* resHelper,
    IDPVM::ResolutionHelper & theHelper,
    IDPVM::ResolutionHelper::methods & theMethod )
{
    // here we have to rely on the naming conventions of IDTPM to identify what we are looking at 
    std::string type = getTypeAndVars( resHelper ).first;
    std::string vars = getTypeAndVars( resHelper ).second;
     
    // cast to TH2
    TH2* resHelper2D = dynamic_cast<TH2*>(resHelper); 
    if (!resHelper2D){
        std::cerr <<"Unable to reduce the histogram "<<resHelper->GetName()<<" to a TH2 - this histo can not yet be postprocessed! " <<std::endl; 
        return 1; 
    }
    const auto & oneDimNames = getPullAndResoNames(type); 
    // get the corresponding 1D histos by cloning the existing ones in the same folder 
    TH1* h_width = cloneExisting( oneDimNames.first + vars ); 
    TH1* h_mean = cloneExisting( oneDimNames.second + vars ); 
    // then call the resolution helper as done in "online" IDTPM
    theHelper.makeResolutions( resHelper2D, h_width, h_mean, theMethod );
    // update our 1D histos 
    h_width->Write();
    h_mean->Write();
    // and we are done
    return 0;
}

// recursively parse a directory tree, post-processing any histos seen along the way 
int postProcessDir(
    TDirectory* dir,
    IDPVM::ResolutionHelper & theHelper,
    IDPVM::ResolutionHelper::methods & theMethod )
{
  int outcome = 0; 
  auto theCWD = gDirectory; 
  // walk through all keys in this directory 
  dir->cd();
  auto *keys = dir->GetListOfKeys();
  for (auto *const key : *keys){
    // Check if it's a directory and handle separately
    TDirectory* theDir = dynamic_cast<TDirectory*>(dir->Get(key->GetName()));
    if (theDir){
      outcome |= postProcessDir( theDir, theHelper, theMethod );
      continue;  // Do NOT delete directories, ROOT manages them
    }

    // If it's a histogram, wrap it in a unique_ptr
    std::unique_ptr<TObject> gotIt(dir->Get(key->GetName()));
    if (isResolutionHelper(gotIt.get())){
      outcome |= postProcessHistos( gotIt.get(), theHelper, theMethod );
    }
  }
  theCWD->Purge(); // deleting old cycles
  theCWD->cd();
  return outcome;
}

// function driving the postprocessing for this file
int pproc_file( const std::string& p_infile, const std::string& methodStr ) {

    IDPVM::ResolutionHelper theHelper;
    IDPVM::ResolutionHelper::methods theMethod; 

    std::unique_ptr<TFile> infile(TFile::Open(p_infile.c_str(),"UPDATE"));
    if (!infile || infile->IsZombie()) {
      std::cerr << "could not open input file "<<p_infile<<" for updating "<< std::endl;
      return 1;
    }

    /// Defining map for resolution method
    using methodMap_t = std::unordered_map<
        std::string, IDPVM::ResolutionHelper::methods >;
    methodMap_t methodMap = {
      { "iterRMS"         , IDPVM::ResolutionHelper::iterRMS_convergence },
      { "gaussFit"        , IDPVM::ResolutionHelper::Gauss_fit },
      { "iterRMSgaussFit" , IDPVM::ResolutionHelper::fusion_iterRMS_Gaussfit },
      { "iterGaussFit"    , IDPVM::ResolutionHelper::iterGaussFit_convergence }
    };

    methodMap_t::const_iterator mitr = methodMap.find( methodStr );
    if( mitr == methodMap.end() ) {
      std::cerr << "Error: Method " << methodStr <<
                     " not found. Using iterRMS by default." << std::endl;
      theMethod = IDPVM::ResolutionHelper::iterRMS_convergence;
    } else {
      theMethod = mitr->second;
    }

    int res = postProcessDir( infile.get(), theHelper, theMethod ); // recursively post-process the directory tree, starting from the root.  
    return res;
}


int main(int argc, char* argv[]) {
  
  std::string infile{""}; 
  std::string methodStr{"iterRMS"}; 
  /// Standard usage. The user passes a file they wish to update. 
  if( argc >= 2 ) {
    infile  = argv[1];
    if( argc == 3 ) methodStr = argv[2];
  }
  else {
    std::cerr<<" Usage: postProcessIDTPMHistos <File to post-process> <resolution method>"<<std::endl;
    std::cerr<< "    where the file is typically obtained by hadding" << std::endl;
    std::cerr<< "    outputs of several independent IDTPM runs." << std::endl;
    std::cerr<< "    The resolution method (optional) is set by default to iterRMS_convergence." << std::endl;
    return 1; 
  }
  /// check if the input exists
  if (!file_exists(infile)) {
    std::cerr << "Error: invalid input file: " << infile << std::endl;
    return 1;
  }
  /// and post-process if it does
  std::cout << " Post-processing file " << infile << "\n" << std::endl;
  return pproc_file( infile, methodStr );
}
