/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////////
// CalibrationDataEigenVariations.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////////

#include "CalibrationDataInterface/CalibrationDataContainer.h"
#include "CalibrationDataInterface/CalibrationDataEigenVariations.h"
#include "CalibrationDataInterface/CalibrationDataInternals.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <set>
#include <algorithm>
#include <string>
#include <cmath>
#include <vector>

#include "TH1.h"
#include "TH2.h"
#include "TVectorT.h"
#include "TDecompSVD.h"
#include "TMatrixDSymEigen.h"
#include "TMatrixT.h"
#include "TROOT.h"
#include "TFile.h"

#include <memory.h>

using Analysis::CalibrationDataEigenVariations;
using Analysis::CalibrationDataGlobalEigenVariations;
using Analysis::CalibrationDataInterface::split;
using std::string;
using std::map;
using std::vector;
using std::set;
using std::pair;
using std::setw;
using std::setprecision;

namespace {

  // Utility functions...
  void validate_reduction(int matrixsize, TMatrixDSym& corr, std::vector<std::pair<TH1*, TH1*>>& eigenvars, TH1* result, const std::string& scheme, const std::string& tagger, const std::string& wp, const std::string& jetauthor){
    // Validating the reduction schemes - you can compare the correlation matrix constructed before and after eigenvector decomposition 
    // using whatever eigenvectors are available at either stage. The less difference, the better.

    // This function simply constructs the relevant matrix relative percentage difference comparison, and saves to file of your choosing.
    // Saving to EOS is recommended - this can then be accessed, downloaded, and plotted with whatever plotting script you prefer.

    // Each systematic strategy scheme has a different way to construct the approximate covariance matrix
    TMatrixDSym approx_cov(matrixsize); // matrix with zeros

    if (scheme == "SFEigen"){
      for (const std::pair<TH1*,TH1*>& var : eigenvars){
        // we want to make the tensor product of (purely) the variation with itself, and store it in a TMatrix
        // ---> get the pure variations by subtracting the result from the up_variation combined eigenvariation histogram
        TH1* pure_var = static_cast<TH1*>(var.first->Clone()); pure_var->SetDirectory(0);
        pure_var->Add(result, -1.0); // do the subtraction, get variation value histogram
        // ---> convert the pure variation into a vector of doubles
        std::vector<double> pure_var_vec;
        for (int binz = 1 ; binz <= pure_var->GetNbinsZ() ; ++binz){
          for (int biny = 1 ; biny <= pure_var->GetNbinsY() ; ++biny){
            for (int binx = 1 ; binx <= pure_var->GetNbinsX() ; ++binx){
              pure_var_vec.push_back(pure_var->GetBinContent(pure_var->GetBin(binx,biny,binz)));
            }
          }
        }
        // then compute the covariance matrix, and sum into the total (approximate) covariance matrix
        for(int i = 0 ; i < corr.GetNrows() ; i++){
          for(int j = 0 ; j < corr.GetNcols() ; j++){
            approx_cov(i,j) += (pure_var_vec[i])*(pure_var_vec[j]); // do the tensor product
          }
        }
      } // end of (reduced set of) eigenvariations loop

    } else if (scheme == "SFGlobalEigen"){
      TH1* comb_result = result; // renaming to match the intended usage...
      
      for (const std::pair<TH1*,TH1*>& var : eigenvars){
        // we want to make the tensor product of (purely) the variation with itself, and store it in a TMatrix
        // get the pure variations by subtracting the comb_result from the up_variation combined eigenvariation histogram
        TH1* pure_var = static_cast<TH1*>(var.first->Clone()); pure_var->SetDirectory(0);
        pure_var->Add(comb_result, -1.0); // do the subtraction, get variation value histogram
        for(int i = 0 ; i < approx_cov.GetNrows() ; i++){
          for(int j = 0 ; j < approx_cov.GetNcols() ; j++){
            approx_cov(i,j) += (pure_var->GetBinContent(i))*(pure_var->GetBinContent(j)); // do the tensor product
          }
        }
      } // end of (reduced set of) eigenvariations loop

    } else {
      std::cout << " Error: The scheme " << scheme << " didn't match SFEigen or SFGlobalEigen." << std::endl;
      return;
    }

    // make the total correlation matrix from the (approximate) total covariance matrix
    TMatrixDSym approx_corr(approx_cov);
    for(int row = 0 ; row < approx_cov.GetNrows() ; row++){
      for(int col = 0 ; col < approx_cov.GetNcols() ; col++){
        double rowvar = sqrt(approx_cov(row,row));
        double colvar = sqrt(approx_cov(col,col));
        approx_corr(row,col) = approx_cov(row,col)/(rowvar * colvar); // divide each element by the variance
      }
    }
    // subtract the approximate correlation matrix from the true correlation matrix, and save to file
    TMatrixDSym numerator = (approx_corr - corr);
    TMatrixDSym denominator = corr;
    TMatrixDSym correlation_comparison(matrixsize);
    for(int row = 0 ; row < approx_corr.GetNrows() ; row++){
      for(int col = 0 ; col < approx_corr.GetNcols() ; col++){
        if (denominator(row,col)==0.0){
          correlation_comparison(row,col) = 0.0;
        } else {
          correlation_comparison(row,col) = 100*std::abs(numerator(row,col)/denominator(row,col)); // store relative percent errors
        }
      }
    }

    string outputname = "Validate_" + scheme + "_" + tagger + "_" + wp + "_" + jetauthor + "_" + std::to_string(eigenvars.size()) + "vars" ; 
    string eospath = "/your/path/here/plotting/"; // <------ Place you want to store plots. Obviously, change this
    string outputfilename = eospath + outputname + ".root"; 
    std::cout << "ATTEMPTING TO WRITE " << outputfilename << " TO FILE..." << std::endl;
    TFile fout(outputfilename.c_str(), "recreate");
    fout.cd();
    std::unique_ptr<TH2D> hout(new TH2D(correlation_comparison));
    
    // Note: you can also look at the other matrices involved - save them to file and plot them if you wish
    hout->SetTitle("Correlation Matrix Comparison");
    hout->Write();
    fout.Close();

  }




  // The covariance matrices constructed by the following two functions adhere to the TH1 binning conventions.
  // But in order to avoid unnecessary overhead, the actual dimensionality of the histograms is accounted for.

  // Construct the (diagonal) covariance matrix for the statistical uncertainties on the "ref" results
  // Note that when statistical uncertainties are stored as variations, they are already accounted for and hence should not be duplicated; hence they are dummy here
  // (this is not very nice, in that the result of this function will technically be wrong,
  // but the statistical uncertainty covariance matrix is anyway not separately visible to the outside world)
  TMatrixDSym getStatCovarianceMatrix(const TH1* hist, bool zero) {
    Int_t nbinx = hist->GetNbinsX()+2, nbiny = hist->GetNbinsY()+2, nbinz = hist->GetNbinsZ()+2;
    Int_t rows = nbinx;
    if (hist->GetDimension() > 1) rows *= nbiny;
    if (hist->GetDimension() > 2) rows *= nbinz;

    // // the "2" below doesn't actually imply that two bins are used...
    // // this is just to make the loops below work
    // if (ref->GetDimension() <= 1) nbiny = 2;
    // if (ref->GetDimension() <= 2) nbinz = 2;

    TMatrixDSym stat(rows);
    for (Int_t binx = 1; binx < nbinx-1; ++binx)
      for (Int_t biny = 1; biny < nbiny-1; ++biny)
	      for (Int_t binz = 1; binz < nbinz-1; ++binz) {
	        Int_t bin = hist->GetBin(binx, biny, binz);
	        double err = zero ? 0 : hist->GetBinError(bin);
	        stat(bin, bin) = err*err;
	  }
    return stat;
  }

  // Construct the covariance matrix assuming that histogram "unc" contains systematic uncertainties
  // pertaining to the "ref" results, and that the uncertainties are fully correlated from bin to bin
  // (unless option "doCorrelated" is false, in which bins are assumed uncorrelated).
  // One exception is made for the uncorrelated case: if we are dealing with a "continuous" calibration
  // object, then a full correlation is still assumed between different tag weight bins.
  TMatrixDSym getSystCovarianceMatrix(const TH1* ref, const TH1* unc, bool doCorrelated, int tagWeightAxis) {
    Int_t nbinx = ref->GetNbinsX()+2, nbiny = ref->GetNbinsY()+2, nbinz = ref->GetNbinsZ()+2;
    Int_t rows = nbinx;
    if (ref->GetDimension() > 1) rows *= nbiny;
    if (ref->GetDimension() > 2) rows *= nbinz;

    TMatrixDSym cov(rows);

    // Carry out a minimal consistency check
    if (unc->GetNbinsX()+2 != nbinx || unc->GetNbinsY()+2 != nbiny || unc->GetNbinsZ()+2 != nbinz || unc->GetDimension() != ref->GetDimension()) {
      std::cout << "getSystCovarianceMatrix: inconsistency found in histograms " << ref->GetName() << " and " << unc->GetName() << std::endl;
      return cov;
    }

    // // the "2" below doesn't actually imply that two bins are used...
    // // this is just to make the loops below work
    // if (ref->GetDimension() <= 1) nbiny = 2;
    // if (ref->GetDimension() <= 2) nbinz = 2;

    // Special case: uncertainties not correlated from bin to bin.
    // The exception here is for tag weight bins, which are always assumed to be fully correlated.
    if (! doCorrelated) {
      if (tagWeightAxis < 0) {
        // truly uncorrelated uncertainties
        for (Int_t binx = 1; binx < nbinx-1; ++binx)
          for (Int_t biny = 1; biny < nbiny-1; ++biny)
            for (Int_t binz = 1; binz < nbinz-1; ++binz) {
              Int_t bin = ref->GetBin(binx, biny, binz);
              double err = unc->GetBinContent(bin);
              cov(bin,bin) = err*err;
            }
        return cov;
      } else if (tagWeightAxis == 0) {
        // continuous histogram with tag weight X axis
        for (Int_t biny = 1; biny < nbiny-1; ++biny)
          for (Int_t binz = 1; binz < nbinz-1; ++binz)
            for (Int_t binx = 1; binx < nbinx-1; ++binx) {
              Int_t bin = ref->GetBin(binx, biny, binz);
              double err = unc->GetBinContent(bin);
              for (Int_t binx2 = 1; binx2 < nbinx-1; ++binx2) {
                Int_t bin2 = ref->GetBin(binx2, biny, binz);
                double err2 = unc->GetBinContent(bin2);
                cov(bin,bin2) = err*err2;
              }
            }
        return cov;
      } else if (tagWeightAxis == 1) {
        // continuous histogram with tag weight Y axis
        for (Int_t binx = 1; binx < nbinx-1; ++binx)
          for (Int_t binz = 1; binz < nbinz-1; ++binz)
            for (Int_t biny = 1; biny < nbiny-1; ++biny) {
              Int_t bin = ref->GetBin(binx, biny, binz);
              double err = unc->GetBinContent(bin);
              for (Int_t biny2 = 1; biny2 < nbiny-1; ++biny2) {
                Int_t bin2 = ref->GetBin(binx, biny2, binz);
                double err2 = unc->GetBinContent(bin2);
                cov(bin,bin2) = err*err2;
              }
            }
        return cov;
      } else if (tagWeightAxis == 2) {
        // continuous histogram with tag weight Z axis
        for (Int_t binx = 1; binx < nbinx-1; ++binx)
          for (Int_t biny = 1; biny < nbiny-1; ++biny)
            for (Int_t binz = 1; binz < nbinz-1; ++binz) {
              Int_t bin = ref->GetBin(binx, biny, binz);
              double err = unc->GetBinContent(bin);
              for (Int_t binz2 = 1; binz2 < nbinz-1; ++binz2) {
                Int_t bin2 = ref->GetBin(binx, biny, binz2);
                double err2 = unc->GetBinContent(bin2);
                cov(bin,bin2) = err*err2;
              }
            }
        return cov;
      }
    }

    for (Int_t binx = 1; binx < nbinx-1; ++binx)
      for (Int_t biny = 1; biny < nbiny-1; ++biny)
        for (Int_t binz = 1; binz < nbinz-1; ++binz) { 
          Int_t bin = ref->GetBin(binx, biny, binz);   
          double err = unc->GetBinContent(bin);   // <------------- For every bin in the "ref" ("result") TH1*, GetBinContents of the corresponding uncertainty bin
          for (Int_t binx2 = 1; binx2 < nbinx-1; ++binx2)
            for (Int_t biny2 = 1; biny2 < nbiny-1; ++biny2)
              for (Int_t binz2 = 1; binz2 < nbinz-1; ++binz2) {
                Int_t bin2 = ref->GetBin(binx2, biny2, binz2); 
                double err2 = unc->GetBinContent(bin2); // <------- Grab the unc contents of every bin, and compute the covariance matrix element
                cov(bin, bin2) = err*err2;             //  <------- err1 and err2 are the uncertainty content of the bins, so "cov" is real, symmetric
              }                                       //   <------- "cov" would imply that the "hunc" histogram stores "x - E[x]" differences from the mean. So in the end, it computes the covariance (as a sum of these)
        }
    return cov;
  }


  // <---------------- Custom covariance matrix method - W.L.
  TMatrixDSym getSystCovarianceMatrix(const vector<double>& unc){
    int rows = unc.size();
    TMatrixDSym cov(rows);

    for(int i = 0 ; i < rows ; i++){
      for(int j = 0 ; j < rows ; j++){
        double err1 = unc[i];
        double err2 = unc[j];
        cov(i,j) = err1*err2;
      }
    }
    return cov;
  }
  
  // <---------------- Custom covariance matrix method - W.L.
  TMatrixDSym getStatCovarianceMatrix(const std::vector<double>& unc, bool zero) {
    // This is only a stop-gap solution, need to better consider how to do this for 1D,2D,3D binning...
    int rows = unc.size();
    TMatrixDSym stat(rows);
    for (int i = 0; i < rows; i++) {
        double err = zero ? 0 : unc[i];
        stat(i,i) = err*err;
  	}
    return stat;
  }


}

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
// CalibrationDataEigenVariations                                                           //
//                                                                                          //
// This class is intended to provide a more proper way to deal with correlations between    //
// calibration bins than would be possible using directly the calibration containers'       //
// methods (the general issue is with the fact that the interface methods are intended to   //
// be use on a jet by jet basis; in this context it is not straightforward to account for   //
// correlations).                                                                           //
//                                                                                          // <-------- Many CDIHistogramContainers in one CDI File
// A CalibrationDataEigenVariations object is associated with a specific                    // <-------- one CDEigenVariations object per CDHistogramContainer
// CalibrationDataHistogramContainer. It starts by constructing the covariance matrix from  //
// the information provided by the container. Subsequently it diagonalises this covariance  // <-------- Each CDHistogramContainer stores hists of central values + systematic variations
// matrix (this is a standard eigenvalue problem, hence the name of the class), and stores  //
// the result as 'variation' histograms (representing the eigenvectors multiplied by the    //
// square root of the corresponding eigenvalues).                                           //
// Since it is possible for systematic uncertainty contributions to be correlated with      //
// corresponding uncertainties in physics analyses, it is possible to exclude such sources  //
// of uncertainty from being used in the construction of the covariance matrix (this is     //
// since effects from the original sources of uncertainty cannot be traced anymore after    //
// application of the eigenvalue decomposition). It is still possible to evaluate correctly //
// these uncertainties in the form of so-called 'named variations' (see class               //
// CalibrationDataInterfaceROOT); however this will always treat uncertainties as being     //
// fully correlated (or anti-correlated) between calibration bins, so it is recommended not //
// to exclude uncertainties that are not correlated between bins from the eigenvector       //
// decomposition.                                                                           //
//////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
ClassImp(CalibrationDataEigenVariations)
#endif
//________________________________________________________________________________
CalibrationDataEigenVariations::CalibrationDataEigenVariations(const std::string& cdipath, const std::string& tagger, const std::string& wp, const std::string& jetcollection, CalibrationDataHistogramContainer* cnt, bool excludeRecommendedUncertaintySet, bool base) :
m_cnt(cnt), m_initialized(false), m_validate(false), m_namedExtrapolation(-1), m_statVariations(false), m_cdipath(cdipath), m_taggername(tagger), m_wp(wp), m_jetauthor(jetcollection), m_totalvariance(0), m_capturedvariance(0), m_verbose(false)
{

  if (m_verbose) std::cout << " CDEV Constructor : info " << cdipath << " " << tagger << " " << wp << " " << jetcollection << std::endl;
  // if specified, add items recommended for exclusion from EV decomposition by the calibration group to the 'named uncertainties' list
  if (excludeRecommendedUncertaintySet && base) {
    std::vector<std::string> to_exclude = split(m_cnt->getExcludedUncertainties());
    for (const auto& name : to_exclude) {
      excludeNamedUncertainty(name, const_cast<CalibrationDataHistogramContainer*>(cnt));
    }
    if (to_exclude.size() == 0) {
      std::cerr << "CalibrationDataEigenVariations warning: exclusion of pre-set uncertainties list requested but no (or empty) list found" <<std::endl;
    }
  }
  // also flag if statistical uncertainties stored as variations (this typically happens as a result of smoothing / pruning of SF results)
  vector<string> uncs = m_cnt->listUncertainties(); // <-------- The "listUncertainties" method retrieves the list of "information" in the CDIHistogramContainer, basically the "list" output from "showCalibration"
  for (const auto& name : uncs) {
    if (name.find("stat_np") != string::npos) m_statVariations = true;
  }

}

//________________________________________________________________________________
CalibrationDataEigenVariations::~CalibrationDataEigenVariations()
{
  // delete all variation histograms owned by us
  for (vector<pair<TH1*, TH1*> >::iterator it = m_eigen.begin(); it != m_eigen.end(); ++it) {
    delete it->first;
    delete it->second;
  }

  for (vector<pair<TH1*, TH1*> >::iterator it = m_named.begin(); it != m_named.end(); ++it) {
      delete it->first;
      delete it->second;
  }
}

//________________________________________________________________________________
void
CalibrationDataEigenVariations::excludeNamedUncertainty(const std::string& name, CalibrationDataContainer* cnt)
{
  // Exclude the source of uncertainty identified by the given name from being used
  // in the construction of the covariance matrix to be diagonalised.
  // Notes:
  // - Some names returned by CalibrationDataContainer::listUncertainties() are not
  //   meaningful in this context, and specifying them is not allowed.
  // - Once the eigenvector diagonalisation has been carried out, this method may
  //   not be used anymore.

  if (m_initialized) {
    std::cerr << "CalibrationDataEigenVariations::excludeNamedUncertainty error:" << " initialization already done" << std::endl;
  } else if (name == "comment"    || name == "result"   || name == "systematics" || 
             name == "statistics" || name == "combined" || name == "extrapolation" || 
             name == "MCreference" || name == "MChadronisation" || name == "ReducedSets" || 
             name == "excluded_set" || name == "" || name == " ") // <--- these last two handle some cases that may turn up
  {
    std::cerr << "CalibrationDataEigenVariations::excludeNamedUncertainty error:" << " name " << name << " not allowed" << std::endl;
  } 
  // in case multiple uncertainties should be discarded
  else if (name.back() == '*'){
    std::string temp_name = name.substr(0, name.size()-1); //remove "*"
    std::vector<std::string> uncs = m_cnt->listUncertainties();
    std::vector<std::string> unc_subgroup;
    std::copy_if(uncs.begin(), uncs.end(), back_inserter(unc_subgroup),
		 [&temp_name](const std::string& el) {
		   return el.compare(0, temp_name.size(), temp_name) == 0;
		 });
    if (m_verbose) std::cout <<"Found a group of uncertainties to exclude: " <<name <<" found " <<unc_subgroup.size() <<" uncertainties corresponding to the query" <<std::endl;
    for (const auto& single_name : unc_subgroup){
      // only really add if the entry is not yet in the list
      if (m_namedIndices.find(single_name) == m_namedIndices.end()) {
        if (m_verbose) std::cout << "Name : " << single_name << std::endl;
        m_named.push_back(std::pair<TH1*, TH1*>(0, 0));
        m_namedIndices[single_name] = m_named.size()-1;
      }
    }
  }
  else if (! cnt->GetValue(name.c_str())){
    std::cerr << "CalibrationDataEigenVariations::excludeNamedUncertainty error:" << " uncertainty named " << name << " not found" << std::endl;
  } 
}

//________________________________________________________________________________
TMatrixDSym
CalibrationDataEigenVariations::getEigenCovarianceMatrix()
{
  // Construct the covariance matrix that is to be diagonalised.
  // Note that extrapolation uncertainties (identified by the keyword "extrapolation,
  // this will pertain mostly to the extrapolation to high jet pt) are always excluded
  // since by definition they will not apply to the normal calibration bins. Instead
  // this uncertainty has to be dealt with as a named variation. In addition there are
  // other items ("combined", "systematics") that will not be dealt with correctly
  // either and hence are excluded as well).
  //
  // Note that if an explicit covariance matrix is supplied (at present this may be
  // the case only for statistical uncertainties: in the case of "continuous tagging",
  // multinomial statistics applies so bin-to-bin correlations exist) this will be
  // used instead of constructing the statistical uncertainties' covariance matrix on
  // the fly.

  // retrieve the central calibration
  TH1* result = dynamic_cast<TH1*>(m_cnt->GetValue("result"));
  if (not result){
    std::cerr<<"CalibrationDataEigenVariations::getEigenCovarianceMatrix(): failed dynamic cast to TH1*\n";
    return  TMatrixDSym();
  }

  // loop over the uncertainties to construct the covariance matrix for all uncertainties
  // to be addressed using the eigenvector method.

  // First, treat the statistics separately.
  // Account for the possibility that this is handled as a (non-trivial) covariance matrix
  TMatrixDSym* sCov = dynamic_cast<TMatrixDSym*>(m_cnt->GetValue("statistics"));
 
  // Alternatively, statistical uncertainties may be accounted for as variations (i.e., much like systematic uncertainties).
  // In that case, we create a null matrix here and add the statistical contributions along with the systematic ones.
  TMatrixDSym cov = (sCov) ? *sCov : getStatCovarianceMatrix(result, m_statVariations);

  // Then loop through the list of (other) uncertainties
  std::vector<string> uncs = m_cnt->listUncertainties();
  for (unsigned int t = 0; t < uncs.size(); ++t) {
    // entries that should never be included
    if (uncs[t] == "comment" || uncs[t] == "result" || uncs[t] == "combined" || 
        uncs[t] == "statistics" || uncs[t] == "systematics" || uncs[t] == "MCreference" || 
        uncs[t] == "MChadronisation" || uncs[t] == "extrapolation" || uncs[t] == "ReducedSets" || 
        uncs[t] == "excluded_set") continue;
    // entries that can be excluded if desired
    if (m_namedIndices.find(uncs[t]) != m_namedIndices.end()) continue;
    TH1* hunc = dynamic_cast<TH1*>(m_cnt->GetValue(uncs[t].c_str()));
    if (not hunc){
      std::cerr<< "CalibrationDataEigenVariations::getEigenCovarianceMatrix(): dynamic cast failed\n";
      continue;
    }
    cov += getSystCovarianceMatrix(result, hunc, m_cnt->isBinCorrelated(uncs[t]), m_cnt->getTagWeightAxis()); // <-------- This is where we add the covariance matrices together to form the "total covariance"
  }

  return cov;
}

//________________________________________________________________________________
TMatrixDSym
CalibrationDataEigenVariations::getEigenCovarianceMatrixFromVariations()
{
  // Construct the (Eigen-variation part of the) covariance matrix from the individual variations.
  // This must be called _after_ initialize()!

  // retrieve the central calibration
  TH1        *result = dynamic_cast<TH1*>(m_cnt->GetValue("result")); 
  TMatrixD    jac = getJacobianReductionMatrix();
  int         nbins = jac.GetNcols();
  TMatrixDSym cov(nbins);
  auto variation = std::make_unique<double[]>(nbins);

  for (const std::pair<TH1*, TH1*>& it : m_eigen){ // m_eigen is vector of pairs of TH1* which point to TH1's representing the upVariation and downVariation respectively
    TH1* resultVariedUp = it.first; // <--------------- This is the "result" but "varied" upwards - i.e. how the result would look like if the systematic "it" was imposed
    for (unsigned int u = 0; u < (unsigned int) nbins; ++u) variation[u] = resultVariedUp->GetBinContent(u) - result->GetBinContent(u);
    for (int u = 0; u < nbins; ++u){ 
      for (int v = 0; v < nbins; ++v){
        cov(u, v) += variation[u]*variation[v];
      }
    }
  }

  return cov;
}

//________________________________________________________________________________
TMatrixD
CalibrationDataEigenVariations::getJacobianReductionMatrix()
{
  // Construct the matrix that removes the rows and columns that fail to influence
  // the eigen-variations. To reduce the covariance matrix, do the following:
  // 
  // TMatrixDSym cov = getEigenCovarianceMatrix();
  // TMatrixDSym jac = getJacobianReductionMatrix(); // <------------ This is an upper triangular matrix of 1's. Similarity transformation will do...
  // TMatrixDSym redSystematicCovMatrix = cov.Similarity(jac);

  // retrieve the central calibration
  TH1* result = dynamic_cast<TH1*>(m_cnt->GetValue("result"));
  if (not result){
    std::cerr<<"CalibrationDataEigenVariations::getJacobianReductionMatrix(): dynamic cast failed\n";
    return TMatrixD();
  }

  // loop over the uncertainties to construct the covariance matrix for all uncertainties
  // to be addressed using the eigenvector method.

  // Retrieve the un-compressed Eigenvector variation covariance matrix
  // (only needed to check for unexpected singularities)
  TMatrixDSym cov = getEigenCovarianceMatrix();

  // Compute the original number of bins
  int nbins = result->GetNbinsX()+2;
  int ndim  = result->GetDimension();
  if (ndim > 1) nbins*= (result->GetNbinsY()+2);
  if (ndim > 2) nbins*= (result->GetNbinsZ()+2);

  // Start by "compressing" the covariance matrix (removing columns/rows containing zeros only)
  int nZeros=0;
  std::vector<int> zeroComponents;
  if (cov.GetNrows() != nbins) {
    std::cerr << " error: covariance matrix size (" << cov.GetNrows() << ") doesn't match histogram size (" << nbins << ")" << std::endl;
    return TMatrixDSym();
  }

  // First flag all the zeros
  for (int i = 0; i < nbins; ++i) {
    // Directly identify the under- and overflow bins
    Int_t binx, biny, binz;
    result->GetBinXYZ(i, binx, biny, binz);
    if ((binx == 0 || binx == result->GetNbinsX()+1) || 
       (ndim > 1 && (biny == 0 || biny == result->GetNbinsY()+1)) || 
       (ndim > 2 && (binz == 0 || binz == result->GetNbinsZ()+1))) 
    {
      ++nZeros;
      zeroComponents.push_back(i);
    }
    // Try a first (quick) identification of rows/columns of zeros by the first element in each row
    // If "successful", check the whole row in more detail
    else if (fabs(cov(i,0)) < 1e-10) {
      bool isThereANonZero(false);
      for (int j = 0; j < nbins; ++j) {
        if (fabs(cov(i,j)) > 1e-10) {
          isThereANonZero=true; break;
        }
      }
      if (! isThereANonZero) {
        ++nZeros;
        zeroComponents.push_back(i);
      }
    }
  }

  // **** COMMENTED OUT FOR NOW. 
  // Leave it here in case the calibration method will change again in the future. 
  // No need to reweight the SF by the efficiency of that bin (MCreference always = 0)

  // Determine whether the container is for "continuous" calibration.
  // This is important since the number of independent scale factors (for each pt or eta bin)
  // is reduced by 1 compared to the number of tag weight bins (related to the fact that the fractions
  // of events in tag weight bins have to sum up to unity).
  // int axis = m_cnt->getTagWeightAxis();
  // bool doContinuous = false;  unsigned int weightAxis = 0;

  // if (axis >= 0) {
  //   doContinuous = true;
  //   // weightAxis = (unsigned int) axis;
  //   // In this case, verify that the special "uncertainty" entry that is in fact the reference MC tag
  //   // weight fractions is present. These tag weight fractions are needed in order to carry out the
  //   // diagonalisation successfully.
  //   if (! dynamic_cast<TH1*>(m_cnt->GetValue("MCreference"))) {
  //     std::cerr << " Problem: continuous calibration object found without MC reference tag weight histogram " << std::endl;
  //     return TMatrixDSym();
  //   }
  // }

  // Only relevant for continuous calibration containers, but in order to void re-computation we
  // retrieve them here
  // Int_t nbinx = result->GetNbinsX()+2, nbiny = result->GetNbinsY()+2, nbinz = result->GetNbinsZ()+2;

  // // If we are indeed dealing with a "continuous" calibration container, ignore one tag weight row
  // const int skipTagWeightBin = 1; // NB this follows the histogram's bin numbering
  // if (doContinuous) {
  //   for (Int_t binx = 1; binx < nbinx-1; ++binx)
  //     for (Int_t biny = 1; biny < nbiny-1; ++biny)
  //       for (Int_t binz = 1; binz < nbinz-1; ++binz) {
  //         if ((weightAxis == 0 && binx == skipTagWeightBin) ||
  //             (weightAxis == 1 && biny == skipTagWeightBin) ||
  //             (weightAxis == 2 && binz == skipTagWeightBin)) {
  //           // At this point we simply add these to the 'null' elements
  //           ++nZeros;
  //           zeroComponents.push_back(result->GetBin(binx, biny, binz));
  //         }
  //       }
  // }

  if (nZeros >= nbins) {
    std::cerr << " Problem. Found n. " << nZeros << " while size of matrix is " << nbins << std::endl;
    return TMatrixDSym();
  }

  int size = nbins - nZeros; // <--- Tis the size of the matrix removing zeros from the covariance matrix

  TMatrixT<double> matrixVariationJacobian(size,nbins);
  int nMissed=0;
  for (int i = 0; i < nbins; ++i) { //full size
    bool missed=false;
    for (unsigned int s = 0; s < zeroComponents.size(); ++s) { // <-------- Basically what this does is it flags "missed" for a given "i" of the full bin size
      if (zeroComponents.at(s) == i) {                        //  <-------- if "i" is in "zeroComponents". Breaks (because it found that it's to be missed)
        missed = true;
        break;
      }
    }
    if (missed) {                                       //        <-------- Finally, if "i" is to be missed, increase "nMissed" by one, and....
      ++nMissed;
      continue; 
    }
    matrixVariationJacobian(i-nMissed,i)=1;         //            <-------- ... this ALWAYS adds a one. If zero "nMissed", add to diagonal. otherwise off-diagonal
  }                                                //             <-------- This matrix only add 1's on/off diagonal, upper triangular matrix

  return matrixVariationJacobian;
}

//________________________________________________________________________________
void
CalibrationDataEigenVariations::initialize(double min_variance)
{
  // This is this class's most important method, in the sense that it does all the
  // math and constructs all "variation" histograms (for both eigenvector and named
  // named variations). This constitutes the full initialisation of the class.
  // This method is meant to be called only after all calls (if any) to the
  // CalibrationDataEigenVariations::excludeNamedUncertainty() method.

  // retrieve the central calibration
  TH1* result = dynamic_cast<TH1*>(m_cnt->GetValue("result"));
  if (not result){
    std::cerr<<"CalibrationDataEigenVariations::initialize: dynamic cast failed\n";
    return;
  }
  // First step: construct the original covariance matrix
  TMatrixDSym cov = getEigenCovarianceMatrix();
  TMatrixDSym corr(result->GetNbinsX()*result->GetNbinsY()*result->GetNbinsZ()); // We want to construct the correlation matrix in order to compare the final eigenvariations correlation matrix to it  

  int rowc = 0;
  for (int row = 0 ; row < cov.GetNrows() ; row++){
    int colc = 0; 
    bool colb = false;
    for (int col = 0 ; col < cov.GetNcols() ; col++){
      if(!cov(row,col) || !cov(row,row) || !cov(col,col) || cov(row,row)<1.0e-6 || cov(col,col)<1.0e-6 ) continue; // really don't want to divide by zero...
      colb=true;
      long double rowvar = sqrt(cov(row,row));
      long double colvar = sqrt(cov(col,col));
      corr(rowc,colc) = cov(row,col)/(rowvar * colvar); // divide each element by the variance
      colc++;
    }
    if (colb){
      rowc++;
    }
  }

  // Second step: create the variations for the named sources of uncertainty (easy...)
  for (map<string, unsigned int>::iterator it = m_namedIndices.begin(); it != m_namedIndices.end(); ++it) {
    pair<TH1*, TH1*>& p = m_named[it->second];
    TH1* hunc = (TH1*) m_cnt->GetValue(it->first.c_str());
    TString namedvar("namedVar");
    namedvar += it->first.c_str();
    TString namedvarUp(namedvar);   namedvarUp   += "_up";
    TString namedvarDown(namedvar); namedvarDown += "_down";
    TH1* resultVariedUp   = (TH1*)result->Clone(namedvarUp);   resultVariedUp->Add(hunc, 1.0);    resultVariedUp->SetDirectory(0);
    TH1* resultVariedDown = (TH1*)result->Clone(namedvarDown); resultVariedDown->Add(hunc, -1.0); resultVariedDown->SetDirectory(0);
    p.first  = resultVariedUp;
    p.second = resultVariedDown;
  }
  // Refinement: add the "extrapolation" uncertainty as a named uncertainty, if the histogram is provided
  // This is a bit special, since the extrapolation uncertainty histogram has a different size than other histograms
  if (TH1* hunc = (TH1*) m_cnt->GetValue("extrapolation")) { // this is just saying "if it exists"...
    TH1* resultVariedUp   = (TH1*) hunc->Clone("extrapolation_up");   resultVariedUp->SetDirectory(0);
    TH1* resultVariedDown = (TH1*) hunc->Clone("extrapolation_down"); resultVariedDown->SetDirectory(0);
    Int_t nbinx = hunc->GetNbinsX()+2, nbiny = hunc->GetNbinsY()+2, nbinz = hunc->GetNbinsZ()+2;
    Int_t firstbinx = hunc->GetXaxis()->FindFixBin(result->GetXaxis()->GetBinCenter(1));
    Int_t firstbiny = result->GetDimension() > 1 ? hunc->GetYaxis()->FindFixBin(result->GetYaxis()->GetBinCenter(1)) : 1;
    Int_t firstbinz = result->GetDimension() > 2 ? hunc->GetZaxis()->FindFixBin(result->GetZaxis()->GetBinCenter(1)) : 1;
    for (Int_t binx = 1; binx < nbinx-1; ++binx) {
      Int_t binxResult = binx - firstbinx + 1;
      if (binxResult < 1) binxResult = 1;
      if (binxResult > result->GetNbinsX()) binxResult = result->GetNbinsX();
      for (Int_t biny = 1; biny < nbiny-1; ++biny) {
        Int_t binyResult = biny - firstbiny + 1;
        if (binyResult > result->GetNbinsY()) binyResult = result->GetNbinsY();
        for (Int_t binz = 1; binz < nbinz-1; ++binz) {
          Int_t binzResult = binz - firstbinz + 1;
          if (binzResult > result->GetNbinsZ()) binzResult = result->GetNbinsZ();
          Int_t bin = hunc->GetBin(binx, biny, binz);
          double contResult = result->GetBinContent(binxResult, binyResult, binzResult);
          resultVariedUp->SetBinContent(bin, contResult + hunc->GetBinContent(bin));
          resultVariedDown->SetBinContent(bin, contResult - hunc->GetBinError(bin));
        }
      }
    }
    m_named.push_back(std::make_pair(resultVariedUp, resultVariedDown));
    m_namedExtrapolation = m_namedIndices["extrapolation"] = m_named.size()-1;
  }

  // Third step: compute the eigenvector variations corresponding to the remaining sources of uncertainty
  int nbins = result->GetNbinsX()+2;
  int ndim  = result->GetDimension();
  if (ndim > 1) nbins*= (result->GetNbinsY()+2);
  if (ndim > 2) nbins*= (result->GetNbinsZ()+2);


  // // Determine whether the container is for "continuous" calibration.
  // // This is important since the number of independent scale factors (for each pt or eta bin)
  // // is reduced by 1 compared to the number of tag weight bins (related to the fact that the fractions
  // // of events in tag weight bins have to sum up to unity).
  //  int axis = m_cnt->getTagWeightAxis();
  // bool doContinuous = false; unsigned int weightAxis = 0;

  //if (axis >= 0) {
  //   doContinuous = true;
  //   weightAxis = (unsigned int) axis;
  //   // In this case, verify that the special "uncertainty" entry that is in fact the reference MC tag
  //   // weight fractions is present. These tag weight fractions are needed in order to carry out the
  //   // diagonalisation successfully.
  //    NOTE: MCreference is not used at the moment (always 0 in the CDI). Comment it out. 
  //    if (! dynamic_cast<TH1*>(m_cnt->GetValue("MCreference"))) {
  //   std::cerr << " Problem: continuous calibration object found without MC reference tag weight histogram " << std::endl;
  //  return;
  // }
  //}

  // Only relevant for continuous calibration containers, but in order to void re-computation we
  // retrieve them here
  // Int_t nbinx = result->GetNbinsX()+2, nbiny = result->GetNbinsY()+2, nbinz = result->GetNbinsZ()+2;

  TMatrixT<double> matrixVariationJacobian = getJacobianReductionMatrix();
  int size = matrixVariationJacobian.GetNrows();

  // Reduce the matrix to one without the zeros, using a "similarity" transformation

  const TMatrixDSym matrixCovariance = cov.Similarity(matrixVariationJacobian); 

  // Carry out the Eigenvector decomposition on this matrix
  TMatrixDSymEigen eigenValueMaker (matrixCovariance);
  TVectorT<double> eigenValues   = eigenValueMaker.GetEigenValues(); 
  TMatrixT<double> eigenVectors  = eigenValueMaker.GetEigenVectors(); 
  TMatrixT<double> matrixVariations (size,size); 

  //compute the total variance by summing the eigenvalues
  m_totalvariance = eigenValues.Sum(); //

  for (int i = 0; i < size; ++i) {
    //now go back
    for (int r = 0; r < size; ++r)
      //first index is the variation number, second corresponds to the pT bin      //<--------- The "eigenvariations" matrix is the "C" matrix which has CKC^T = I with "K" being the o.g. covariance matrix, and "I" is the identity
      matrixVariations(i,r) = -1.0*eigenVectors[r][i]*sqrt(fabs(eigenValues[i])); // <--------- So the result is a matrix (eigenvariation) which is the eigenvector scaled by the sqrt(eigenvalue)
  } // <------- matrixVariations: each row is one variation, each column is the pT bin.

  TMatrixT<double> matrixVariationsWithZeros = matrixVariations * matrixVariationJacobian;

  // Construct the initial set of variations from this
  for (int i = 0; i < matrixVariationsWithZeros.GetNrows(); ++i) {
    // TString superstring(result->GetName());
    // superstring += "_eigenVar";
    TString superstring("eigenVar");
    superstring+=i;

    TString nameUp(superstring);   nameUp   += "_up";
    TString nameDown(superstring); nameDown += "_down";
    // TString nameUnc(superstring);  nameUnc+= "_unc";

    TH1* resultVariedUp   = (TH1*)result->Clone(nameUp);   resultVariedUp->SetDirectory(0); // <------- clone "result" hist, call it "up", and SetDirectory(0) means histogram doesn't belong to any directory.
    TH1* resultVariedDown = (TH1*)result->Clone(nameDown); resultVariedDown->SetDirectory(0);

    for (int u = 0; u < nbins; ++u) {
      resultVariedUp->SetBinContent(u,result->GetBinContent(u) + matrixVariationsWithZeros(i,u)); // <----- This actually constructs the up-variation histogram!
      resultVariedDown->SetBinContent(u,result->GetBinContent(u) - matrixVariationsWithZeros(i,u)); // <--- This likewise constructs the down-var., bin-by-bin.
    }

    m_eigen.push_back(std::make_pair(resultVariedUp, resultVariedDown));

  } //end eigenvector size

  // Remove variations that are below the given tolerance (effectively meaning that they don't have any effect) // <--------- Here we prune systematics
  IndexSet final_set; // <-------- What's IndexSet? It's a typedef of std::set<size_t>, where size_t = int
  size_t current_set = 0;
  for (size_t index = 0; index < m_eigen.size(); ++index) {
    bool keep_variation = false;
    TH1 *up = static_cast<TH1*>(m_eigen[index].first->Clone()); up->SetDirectory(0);

    up->Add(result, -1.0);

    for (int bin = 1; bin <= nbins; ++bin) {
      if (fabs(up->GetBinContent(bin)) > min_variance) { // <----- If you find even ONE bin with big enough variance, we keep the whole systematic.
        keep_variation = true;
        break;
      }
    }
    delete up;
    if (!keep_variation){ // <------ At this stage, if we find no bins in the systematic with large enough variation, we insert it to "final_set" for removal/pruning
      final_set.insert(current_set);
    } else {
      m_capturedvariance += eigenValues[index]; // let's add up the variance that the eigenvariation contributes to the total
    }
    ++current_set;
  }
  if (final_set.size() > 0) 
    if (m_verbose) std::cout << "CalibrationDataEigenVariations: Removing " << final_set.size() << " eigenvector variations leading to sub-tolerance effects, retaining " << m_eigen.size()-final_set.size() << " variations" << std::endl;
  
  removeVariations(final_set); // <----------------- This method actually performs the reduction. The above logic simply flags which variations to GET RID OF, inserting them into "final_set"

  if (m_verbose) std::cout << " The total variance is " << m_totalvariance << " and the reduction captured " << 100*(m_capturedvariance/m_totalvariance) << "% of this." << std::endl;
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///// optionally: perform a validation of the 'SFEigen' method alongside the 'SFGlobalEigen'.                              /////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if (m_validate){ // <---- this flag can be set manually in a local checkout of this package (temporary fix)
    validate_reduction(cov.GetNrows(), corr, m_eigen, result, "SFEigen", m_taggername, m_wp, m_jetauthor); // <------ This method is simply here to report the matrix comparison, for reduction scheme validation
  }

  m_initialized = true;
}

//________________________________________________________________________________
void
CalibrationDataEigenVariations::removeVariations(const IndexSet &set)
{
  if (set.size() == 0) return;

  std::vector<std::pair<TH1*, TH1*> > new_eigen;
  for (size_t index = 0; index < m_eigen.size(); ++index){
      if (set.count(index) == 0) new_eigen.push_back(m_eigen[index]);
      else { delete m_eigen[index].first; delete m_eigen[index].second; }
    }
  m_eigen = std::move(new_eigen);
}

//________________________________________________________________________________
void
CalibrationDataEigenVariations::removeVariations(const IndexSuperSet &set)
{
  IndexSet simple_set;

  for (IndexSuperSet::iterator set_it = set.begin(); set_it != set.end(); ++set_it) {
    for (IndexSet::iterator subset_it = set_it->begin(); subset_it != set_it->end(); ++subset_it)
      simple_set.insert(*subset_it);
  }

  removeVariations(simple_set);
}

//________________________________________________________________________________
void
CalibrationDataEigenVariations::mergeVariationsFrom(const size_t& index)
{
  // Merge all systematic variation starting from the given index.
  // The resulting merged variation replaces the first entry in the list
  // (i.e., the entry specified by the index).
  IndexSet simple_set;

  for (size_t it = index; it < m_eigen.size(); ++it)
    simple_set.insert(it);
  mergeVariations(simple_set);
}

//________________________________________________________________________________
void
CalibrationDataEigenVariations::mergeVariations(const IndexSet &set)
{
  IndexSuperSet sset;
  sset.insert(set);
  mergeVariations(sset);
}

//________________________________________________________________________________
void
CalibrationDataEigenVariations::mergeVariations(const IndexSuperSet &set)
{
  // check for overlap
  IndexSet checker;
  for (IndexSuperSet::iterator set_it = set.begin(); set_it != set.end(); ++set_it) {
    for (IndexSet::iterator subset_it = set_it->begin(); subset_it != set_it->end(); ++subset_it){
      if (checker.count(*subset_it) == 0 && *subset_it <= m_eigen.size())
        checker.insert(*subset_it);
      else {
        std::cerr << "Error in CalibrationDataEigenVariations::mergeVariations: \
          IndexSets must not overlap and must lie between 1 and " << m_eigen.size() << ". Aborting!" << std::endl;
        return;
      }
    }
  }

  // retrieve the central calibration
  TH1 *result = static_cast<TH1*>(m_cnt->GetValue("result"));
  IndexSet toDelete;
  int nbins = result->GetNbinsX()+2;
  int ndim  = result->GetDimension();
  if (ndim > 1) nbins *= (result->GetNbinsY()+2);
  if (ndim > 2) nbins *= (result->GetNbinsZ()+2);

  // TH1 *var_up_final = static_cast<TH1*>(result->Clone()),
  //   *var_down_final = static_cast<TH1*>(result->Clone());

  // var_up_final->Reset();
  // var_down_final->Reset();

  // complex sum
  for (IndexSuperSet::iterator set_it = set.begin(); set_it != set.end(); ++set_it) {
    if (set_it->empty()) continue;

    double sum_H_up = 0.0, sum_H_down = 0.0;
    size_t lowest_index = *set_it->lower_bound(0);
    TH1 *total_var_up = static_cast<TH1*>(m_eigen[lowest_index].first->Clone()),
      *total_var_down = static_cast<TH1*>(m_eigen[lowest_index].second->Clone());
    total_var_up->SetDirectory(0);
    total_var_down->SetDirectory(0);

    total_var_up->Reset();
    total_var_down->Reset();

    // sum all other variations
    for (IndexSet::iterator subset_it = set_it->begin();
	 subset_it != set_it->end(); ++subset_it) {
      size_t actual_index = *subset_it;

      if (actual_index != lowest_index) toDelete.insert(*subset_it);

      TH1 *partial_var_up = static_cast<TH1*>(m_eigen[actual_index].first->Clone()),
	*partial_var_down = static_cast<TH1*>(m_eigen[actual_index].second->Clone());
      partial_var_up->SetDirectory(0);
      partial_var_down->SetDirectory(0);
      
      partial_var_up->Add(result, -1.0); // <----- Is this correct? Should it be +1?
      partial_var_down->Add(result, -1.0);
      for (int i = 0; i < nbins; ++i) {
        partial_var_down->SetBinContent(i, -1.0*partial_var_down->GetBinContent(i));
      }

      for (int u = 0; u < nbins; ++u) {
        double sum_up = total_var_up->GetBinContent(u),
	  sum_down = total_var_down->GetBinContent(u);
        for (int v = 0; v < nbins; ++v) {
          sum_up += partial_var_up->GetBinContent(u)*partial_var_up->GetBinContent(v);
          sum_H_up += partial_var_up->GetBinContent(u)*partial_var_up->GetBinContent(v);
          sum_down += partial_var_down->GetBinContent(u)*partial_var_down->GetBinContent(v);
          sum_H_down += partial_var_down->GetBinContent(u)*partial_var_down->GetBinContent(v);
        }
        total_var_up->SetBinContent(u, sum_up);
        total_var_down->SetBinContent(u, sum_down);
      }
      delete partial_var_up;
      delete partial_var_down;
    }

    // final part of complex summing
    for (int i = 0; i < nbins; ++i) {
      if (sum_H_up != 0.0)
        total_var_up->SetBinContent(i, total_var_up->GetBinContent(i)/sqrt(sum_H_up));
      else
        total_var_up->SetBinContent(i, 0.0);
      if (sum_H_down != 0.0)
        total_var_down->SetBinContent(i, -1.0*total_var_down->GetBinContent(i)/sqrt(sum_H_down));
      else
        total_var_down->SetBinContent(i, 0.0);
    }

    total_var_up->Add(result);
    total_var_down->Add(result);

    m_eigen[lowest_index].first = total_var_up;
    m_eigen[lowest_index].second = total_var_down;
  }

  removeVariations(toDelete);
}

//________________________________________________________________________________
unsigned int
CalibrationDataEigenVariations::getNumberOfNamedVariations() const // <---- I'll have to either report all flavours or something for SFGLobalEigen
{
  // Returns the number of named variations

  return m_namedIndices.size();
}

//________________________________________________________________________________
vector<string>
CalibrationDataEigenVariations::listNamedVariations() const
{
  // Provides the list of named variations

  vector<string> names;
  for (map<string, unsigned int>::const_iterator it = m_namedIndices.begin(); it != m_namedIndices.end(); ++it){
    names.push_back(it->first);
  }
  return names;
}

//________________________________________________________________________________
unsigned int
CalibrationDataEigenVariations::getNumberOfEigenVariations()
{
  if (! m_initialized) initialize();
  return m_eigen.size();
}

//________________________________________________________________________________
bool
CalibrationDataEigenVariations::getEigenvectorVariation(unsigned int variation,
							TH1*& up, TH1*& down)
{
  // Return the pointers to the up- and downward variation histogram for the specified
  // eigenvector variation. In case of an invalid variation number, null pointers will
  // be returned and the return value will be false.
  //
  //     variation:   eigenvector variation number
  //     up:          (reference to) pointer to upward variation histogram
  //     down:        (reference to) pointer to downward variation histogram

  if (! m_initialized) initialize();
  
  if (variation < m_eigen.size()) {
    up   = m_eigen[variation].first;
    down = m_eigen[variation].second;
    return true;
  }

  up = down = 0;
  return false;
}

//________________________________________________________________________________
bool
CalibrationDataEigenVariations::getNamedVariation(const string& name,
						  TH1*& up, TH1*& down)
{
  // Return the pointers to the up- and downward variation histogram for the specified
  // named variation. In case of an invalid named variation, null pointers will
  // be returned and the return value will be false.
  //
  //     name:        named variation
  //     up:          (reference to) pointer to upward variation histogram
  //     down:        (reference to) pointer to downward variation histogram

  map<string, unsigned int>::const_iterator it = m_namedIndices.find(name);
  if (it != m_namedIndices.end()) return getNamedVariation(it->second, up, down);

  up = down = 0;
  return false;
}

//________________________________________________________________________________
bool
CalibrationDataEigenVariations::getNamedVariation(unsigned int nameIndex,
						  TH1*& up, TH1*& down)
{
  // Return the pointers to the up- and downward variation histogram for the specified
  // named variation. In case of an invalid named variation number, null pointers will
  // be returned and the return value will be false.
  //
  //     nameIndex:   named variation number
  //     up:          (reference to) pointer to upward variation histogram
  //     down:        (reference to) pointer to downward variation histogram

  if (! m_initialized) initialize();

  if (nameIndex < m_named.size()) {
    up   = m_named[nameIndex].first;
    down = m_named[nameIndex].second;
    return true;
  }

  up = down = 0;
  return false;
}

//________________________________________________________________________________
unsigned int
CalibrationDataEigenVariations::getNamedVariationIndex(const std::string& name) const
{
  // Return the integer index corresponding to the named variation.
  // Note that no checks are made on the validity of the name provided.

  map<string, unsigned int>::const_iterator it = m_namedIndices.find(name);
  return it->second;
}

//________________________________________________________________________________
bool
CalibrationDataEigenVariations::isExtrapolationVariation(unsigned int nameIndex) const
{
  // Verifies whether the given named variation index corresponds to the extrapolation
  // uncertainty.

  return (m_namedExtrapolation == int(nameIndex));
}

//________________________________________________________________________________
bool
CalibrationDataEigenVariations::EigenVectorRecomposition(const std::string& label, 
							 std::map<std::string, std::map<std::string, float>> &coefficientMap)
{
  // Calculating eigen vector recomposition coefficient map and pass to
  // user by reference. Return true if method success. Return false and
  // will not modify coefficientMap if function failed.
  //
  //     label:          flavour label
  //     coefficientMap: (reference to) coefficentMap which will be used as return value.

  if (! m_initialized) initialize();

  std::vector<TH1*> originSF_hvec;
  std::vector<TH1*> eigenSF_hvec;

  // Retrieving information for calculation
  std::vector<string>fullUncList = m_cnt->listUncertainties();
  std::vector<string> uncList;
  for (unsigned int t = 0; t < fullUncList.size(); ++t) {
    // entries that should never be included
    if (fullUncList[t] == "comment" || fullUncList[t] == "result" || fullUncList[t] == "combined" || 
        fullUncList[t] == "statistics" || fullUncList[t] == "systematics" || fullUncList[t] == "MCreference" || 
        fullUncList[t] == "MChadronisation" || fullUncList[t] == "extrapolation" || fullUncList[t] == "ReducedSets" || 
        fullUncList[t] == "excluded_set") continue;

    // entries that can be excluded if desired
    if (m_namedIndices.find(fullUncList[t]) != m_namedIndices.end()) continue;
    
    TH1* hunc = dynamic_cast<TH1*>(m_cnt->GetValue(fullUncList[t].c_str()));
    if (not hunc){
      std::cerr<<"CalibrationDataEigenVariations::EigenVectorRecomposition: dynamic cast failed\n";
      continue;
    }

    Int_t nx = hunc->GetNbinsX();
    Int_t ny = hunc->GetNbinsY();
    Int_t nz = hunc->GetNbinsZ();
    Int_t bin = 0;
    bool retain = false; // Retain the histogram?

    // discard empty histograms
    // Read all bins without underflow&overflow
    for(Int_t binx = 1; binx <= nx; binx++)
      for(Int_t biny = 1; biny <= ny; biny++)
	for(Int_t binz = 1; binz <= nz; binz++){
	  bin = hunc->GetBin(binx, biny, binz);  
	  if (fabs(hunc->GetBinContent(bin)) > 1E-20){
	    retain = true;
	    break;
	  }
	}// end hist bin for-loop
    if (!retain){
      if (m_verbose) std::cout<<"Eigenvector Recomposition: Empty uncertainty "<<fullUncList.at(t)<<" is discarded."<<std::endl;
      continue; // discard the vector
    }

    uncList.push_back(fullUncList.at(t));
    originSF_hvec.push_back(hunc);
  }

  TH1* nom = dynamic_cast<TH1*>(m_cnt->GetValue("result")); // Nominal SF hist
  if (not nom){
     if (m_verbose) std::cout<<"Eigenvector Recomposition: dynamic cast failed\n";
     return false;
  }
  int dim = nom->GetDimension();
  Int_t nx = nom->GetNbinsX();
  Int_t ny = nom->GetNbinsY();
  Int_t nz = nom->GetNbinsZ();
  Int_t nbins = nx;
  if(dim > 1) nbins *= ny;
  if(dim > 2) nbins *= nz;
  TMatrixD matSF(uncList.size(), nbins);
  Int_t col = 0; // mark the column number
  // Fill the Delta SF Matrix
  for(unsigned int i = 0; i < uncList.size(); i++){
    col = 0;
    // Loop all bins except underflow&overflow bin
    for(int binz = 1; binz <= nz; binz++)
      for(int biny = 1; biny <= ny; biny++)
	for(int binx = 1; binx <= nx; binx++){
 	  Int_t bin = originSF_hvec.at(i)->GetBin(binx, biny, binz);
	  TMatrixDRow(matSF,i)[col] = originSF_hvec[i]->GetBinContent(bin);
	  col++;
	}
  }

  // get eigen vectors of scale factors. Note that this is not the original eigen-vector.
  int nEigen = getNumberOfEigenVariations();
  TH1* up = nullptr;
  TH1* down = nullptr;
  for (int i = 0; i < nEigen; i++){
    if (!getEigenvectorVariation(i, up, down)){
       std::cerr<<"EigenVectorRecomposition: Error on retrieving eigenvector "<<i<<std::endl;
      return false;
    }
    //Need uncertainty value so subtract central calibration here.
    up->Add(nom, -1);
    eigenSF_hvec.push_back(up);
  }
  TMatrixD matEigen(nEigen, nbins);

  // Fill the Eigen Matrix
  for(int i = 0; i < nEigen; i++){
    col = 0;
    // Read 300 bins without underflow&overflow
    for(int binz = 1; binz <= nz; binz++)
      for(int biny = 1; biny <= ny; biny++)
	for(int binx = 1; binx <= nx; binx++){
	  Int_t bin = eigenSF_hvec.at(i)->GetBin(binx, biny, binz);
	  TMatrixDRow(matEigen,i)[col] = eigenSF_hvec[i]->GetBinContent(bin);
	  col++;
	}
  }

  // Sanity check:
  TMatrixD matEigenOriginal = matEigen;
  TMatrixD matEigenTranspose = matEigen;
  matEigenTranspose = matEigenTranspose.T();
  TMatrixD matOriginalTimesTranspose = matEigenOriginal*matEigenTranspose;
  TMatrixD matEigenInvert = matEigenTranspose*matOriginalTimesTranspose.Invert();
  //(matEigenOriginal*matEigenInvert).DrawClone("colz"); // This should give us an identity matrix

  TMatrixD matCoeff = matSF*matEigenInvert;
  int nRows = matCoeff.GetNrows();
  int nCols = matCoeff.GetNcols();
  std::map<std::string, float> temp_map;
  for (int col = 0; col < nCols; col++){
    temp_map.clear();
    for(int row = 0; row < nRows; row++){
      temp_map[uncList[row]] = TMatrixDRow(matCoeff, row)[col];
    }
    coefficientMap["Eigen_"+label+"_"+std::to_string(col)] = temp_map;
  }
  
  return true;
}

void CalibrationDataEigenVariations::setVerbose(bool verbose) {
  m_verbose = verbose;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                  //
// CalibrationDataGlobalEigenVariations                                                             //
//                                                                                                  //
// This class is intended to provide a more proper way to deal with correlations between            //
// calibration bins across all jet jet flavours of a tagger/author/wp combination.                  //
//                                                                                                  //
// A CalibrationDataGlobalEigenVariations object is associated with a specific set of               //
// CalibrationDataHistogramContainer objects. It starts by constructing the covariance matrix from  //
// the information provided by the container. Subsequently it diagonalises this covariance          // 
// matrix (this is a standard eigenvalue problem, hence the name of the class), and stores          //
// the result as 'variation' histograms (representing the eigenvectors multiplied by the            //
// square root of the corresponding eigenvalues).                                                   //
// Since it is possible for systematic uncertainty contributions to be correlated with              //
// corresponding uncertainties in physics analyses, it is possible to exclude such sources          //
// of uncertainty from being used in the construction of the covariance matrix (this is             //
// since effects from the original sources of uncertainty cannot be traced anymore after            //
// application of the eigenvalue decomposition). It is still possible to evaluate correctly         //
// these uncertainties in the form of so-called 'named variations' (see class                       //
// CalibrationDataInterfaceROOT); however this will always treat uncertainties as being             //
// fully correlated (or anti-correlated) between calibration bins, so it is recommended not         //
// to exclude uncertainties that are not correlated between bins from the eigenvector               //
// decomposition.                                                                                   //
//////////////////////////////////////////////////////////////////////////////////////////////////////





//________________________________________________________________________________
CalibrationDataGlobalEigenVariations::CalibrationDataGlobalEigenVariations(const std::string& cdipath, const std::string& tagger, const std::string& wp, const std::string& jetcollection, const std::vector<std::string>& flavours, CalibrationDataHistogramContainer* cnt, bool excludeRecommendedUncertaintySet) : 
CalibrationDataEigenVariations(cdipath, tagger, wp, jetcollection, cnt, excludeRecommendedUncertaintySet, false), m_flav_eigen(), m_flavours(flavours)
//m_blockmatrixsize(1200)
{
  m_blockmatrixsize = 0; // <------- This needs to be computed based off of the dimensions of the result vectors (currently 4x1x5 for continuous WP, 1x300x1 for fixed cut WP, circa 2022)
  m_initialized = false; 
  m_namedExtrapolation = -1; 
  m_statVariations = false;
  // We want to retrieve all the containers that belong in the tagger/jetcollection/wp group
  // These are then stored, with all uncertainties extracted and stored as well, for later use in EV decomposition

  // For now we will open up the CDI file from within the constructor, to get at the data we need
  TString fileName(cdipath);
  TFile* f = TFile::Open(fileName.Data(), "READ");
  f->cd();

  // This loop extracts all the "flavour containers", i.e. all CalibrationDataHistogramContainers pertaining to the same path, but with different flavours
  // It also puts all the uncertainties for all containers into a set, which we can then later loop over, to construct the total covariance matrix
  for (const auto& flavour : m_flavours){
    std::string dir = m_taggername + "/" + m_jetauthor + "/" + m_wp  + "/" + flavour + "/" + "default_SF" ;
    TString contName(dir);
    Analysis::CalibrationDataHistogramContainer* c;
    f->GetObject(contName.Data(), c);
    if (c) {
      if (m_verbose) std::cout << "Found " << contName.Data() << std::endl;
      m_histcontainers.insert({flavour, c}); // Build the mapping between flavour and the corresponding "flavour container", i.e. the CalibrationDataHistogramContainer
      std::vector<std::string> uncs = c->listUncertainties();
      TH1* result = dynamic_cast<TH1*>(c->GetValue("result")); // let's get the size of this for later
      if (not result){
        if (m_verbose) std::cout << "Dynamic cast failed at "<<__LINE__<<"\n";
        continue;
      }
      m_blockmatrixsize+=result->GetNbinsX()*result->GetNbinsY()*result->GetNbinsZ(); // should be ~300 for fixed cut, something else for continuous
      if (m_verbose) std::cout << "m_blockmatrixsize is now " << m_blockmatrixsize << std::endl;
      for (const std::string& unc : uncs){
        if (unc.find("stat_np") != string::npos) m_statVariations = true;
        if ((unc=="result")||(unc=="comment")||(unc=="ReducedSets")||(unc=="systematics")||(unc=="statistics")||(unc=="extrapolation")||(unc=="MChadronisation")||(unc=="combined")||(unc=="extrapolation from charm")) { 
          continue; 
        } else { 
          m_all_shared_systematics.insert(unc); // Construct the set of uncertainties to get a full tally of everything in the container group
        }
      } 
      // If specified, add items recommended for exclusion from EV decomposition by the calibration group to the 'named uncertainties' list
      // This used to work on only one container, but now we do it on all four containers
      if (excludeRecommendedUncertaintySet) {
        std::vector<std::string> to_exclude = split(c->getExcludedUncertainties());
        if (to_exclude.size() == 0) {
          std::cerr << "CalibrationDataEigenVariations warning: exclusion of pre-set uncertainties list requested but no (or empty) list found" << std::endl;
        }
        for (const auto& name : to_exclude) {
          if (name == "") continue;
          excludeNamedUncertainty(name, flavour);
        }
      }
    }
  }
  
  if (m_verbose) std::cout << "\n number of shared uncertainties is " << m_all_shared_systematics.size() << std::endl;
  
  std::set<std::string>::iterator it = m_all_shared_systematics.begin();
  if (it != m_all_shared_systematics.end()){
    if (m_verbose) std::cout << "Printing out all shared uncertainties for " << tagger << "/" << jetcollection << "/" << wp << std::endl;
    if (m_verbose) std::cout << "| " << std::endl;
    while (it != m_all_shared_systematics.end()){
      if (m_verbose) std::cout << "|-- " << (*it) << std::endl;
      ++it;
    }
  } else {
    if (m_verbose) std::cout << "| no shared systematics between ";
    for (const auto& f : m_flavours){
      if (m_verbose) std::cout << f << ", ";
    } if (m_verbose) std::cout << std::endl;
  }
  // End of constructor
}

//________________________________________________________________________________
CalibrationDataGlobalEigenVariations::~CalibrationDataGlobalEigenVariations()
{
  // delete all variation histograms owned by us
  for (const auto& flavour : m_flavours){
    for (vector<pair<TH1*, TH1*> >::iterator it = m_flav_eigen[flavour].begin(); it != m_flav_eigen[flavour].end(); ++it) {
      delete it->first;
      delete it->second;
    }

    for (vector<pair<TH1*, TH1*> >::iterator it = m_flav_named[flavour].begin(); it != m_flav_named[flavour].end(); ++it) {
      delete it->first;
      delete it->second;
    }
  }
}

//________________________________________________________________________________
TMatrixDSym
CalibrationDataGlobalEigenVariations::getEigenCovarianceMatrix()
{
  // Construct the block (global) covariance matrix that is to be diagonalised.
  // Note that extrapolation uncertainties (identified by the keyword "extrapolation,
  // this will pertain mostly to the extrapolation to high jet pt) are always excluded
  // since by definition they will not apply to the normal calibration bins. Instead
  // this uncertainty has to be dealt with as a named variation. In addition there are
  // other items ("combined", "systematics") that will not be dealt with correctly
  // either and hence are excluded as well).
  //
  // Note that if an explicit covariance matrix is supplied (at present this may be
  // the case only for statistical uncertainties: in the case of "continuous tagging",
  // multinomial statistics applies so bin-to-bin correlations exist) this will be
  // used instead of constructing the statistical uncertainties' covariance matrix on
  // the fly.

  std::map<std::string, TMatrixDSym> cov_matrices; // temporary store, just to aid in printing out the individual systematic covariance matrices
  TMatrixDSym global_covariance(m_blockmatrixsize);
  // Then loop through the list of (other) uncertainties
  for (const std::string& unc : m_all_shared_systematics){
    std::vector<int> flavs_in_common;
    TString tunc(unc);
    std::vector<double> comb_syst; // this vector combines the TH1 uncertainty bins for each flavour into one object -> stored in comb_systematics for now, but meant for covariance matrix method
    // For the fixed cut case, we want to bin by groups of flavour > pT (i.e. "blocks" of flavour)
    // For the continuous case, we want to bin by groups of flavour > tagweight > pT (i.e. "blocks" of flavour, containing tagweight blocks... more complicated, but works on same principle)
    for (const auto& flavour : m_flavours){
      Analysis::CalibrationDataHistogramContainer* c = m_histcontainers[flavour]; // pointer to the flavour container
      int flavour_size = 0; // Store the length of the uncertainty in the flavour, initialize to zero
      if (c) {
        // Now we want to get the histogram for this systematic, for this flavour
        TH1* hunc = dynamic_cast<TH1*>(c->GetValue(tunc.Data()));
        TH1* ref = dynamic_cast<TH1*>(c->GetValue("result")); // retrieving this just in case the uncertainty doesn't exist for this flavour, just need it to get dimensions right
        if (not ref){
           if (m_verbose) std::cout << " There was no uncertainty OR SF/EFF results... Are you sure you have the right CDIContainer path?" << std::endl;
           continue;
        }
        int tagweightax = c->getTagWeightAxis(); // for handling the continuous case(s)
        if (hunc){
          flavs_in_common.push_back(1); // report that we have this uncertainty in this flavour
          Int_t nbinx = hunc->GetNbinsX(), nbiny = hunc->GetNbinsY(), nbinz = hunc->GetNbinsZ();
          Int_t rows = nbinx;
          if (hunc->GetDimension() > 1) rows *= nbiny;
          if (hunc->GetDimension() > 2) rows *= nbinz;
          flavour_size = rows; // Record the number of bins in the uncertainty TH1
        } else {
          flavs_in_common.push_back(0); // If the uncertainty doesn't exist for that flavour, just report, we'll set to zero in the combined vector
          // Because the uncertainty doesn't exist for this flavour, we just get the dimensions we need
          Int_t nbinx = ref->GetNbinsX(), nbiny = ref->GetNbinsY(), nbinz = ref->GetNbinsZ();
          Int_t rows = nbinx;
          if (ref->GetDimension() > 1) rows *= nbiny;
          if (ref->GetDimension() > 2) rows *= nbinz;
          flavour_size = rows;
        }
        
        // Now we can loop through the bins of the flavour uncertainty, adding them onto the combined systematic
        if (tagweightax == -1){ //<--- i.e. NOT continuous, but is a fixed cut WP
          for (int i = 1 ; i <= flavour_size ; i++){
            if (hunc){
              Int_t bin = hunc->GetBin(1,i,1);
              double unc_val = hunc->GetBinContent(bin);
              comb_syst.push_back(unc_val); // if uncertainty, push uncertainty bin content to combined systematic vector
            } else {
              comb_syst.push_back(0.0); // if no uncertainty, push 0's
            }
          }
        } else if (tagweightax == 0){
          // X axis is the tagweight axis, meaning Y is pt, Z is abs(eta)
          for (Int_t xbins = 1 ; xbins <= ref->GetNbinsX() ; xbins++){
            for (Int_t zbins = 1 ; zbins <= ref->GetNbinsZ() ; zbins++){
              for (Int_t ybins = 1 ; ybins <= ref->GetNbinsY() ; ybins++){
                if (hunc){
                  Int_t bin = hunc->GetBin(xbins,ybins,zbins); 
                  double unc_val = hunc->GetBinContent(bin); 
                  comb_syst.push_back(unc_val); // if uncertainty, push uncertainty bin content to combined systematic vector 
                } else {
                  comb_syst.push_back(0.0); // if no uncertainty, push 0's
                }
                // And now we should be constructing the initial covariance matrix for block continuous correctly
              }
            }
          }
        } else if (tagweightax == 1){
          // Y axis is the tagweight axis, meaning X is pt, Z is abs(eta)
          for (Int_t ybins = 1 ; ybins <= ref->GetNbinsY() ; ybins++){
            for (Int_t zbins = 1 ; zbins <= ref->GetNbinsZ() ; zbins++){
              for (Int_t xbins = 1 ; xbins <= ref->GetNbinsX() ; xbins++){
                if (hunc){
                  Int_t bin = hunc->GetBin(xbins,ybins,zbins); 
                  double unc_val = hunc->GetBinContent(bin);
                  comb_syst.push_back(unc_val); // if uncertainty, push uncertainty bin content to combined systematic vector 
                } else {
                  comb_syst.push_back(0.0); // if no uncertainty, push 0's
                }
                // And now we should be constructing the initial covariance matrix for block continuous correctly
              }
            }
          }
        } else if (tagweightax == 2){
          // Z axis is the tagweight axis, meaning X is pt, Y is abs(eta)
          for (Int_t zbins = 1 ; zbins <= ref->GetNbinsZ() ; zbins++){
            for (Int_t ybins = 1 ; ybins <= ref->GetNbinsY() ; ybins++){
              for (Int_t xbins = 1 ; xbins <= ref->GetNbinsX() ; xbins++){
                if (hunc){
                  Int_t bin = hunc->GetBin(xbins,ybins,zbins); 
                  double unc_val = hunc->GetBinContent(bin); 
                  comb_syst.push_back(unc_val); // if uncertainty, push uncertainty bin content to combined systematic vector 
                } else {
                  comb_syst.push_back(0.0); // if no uncertainty, push 0's
                }
                // And now we should be constructing the initial covariance matrix for block continuous correctly
              }
            }
          }
        }
      }
    }
    //comb_systematics.insert({unc, comb_syst}); // after having looped through the bins for each flavour, the comb_syst vector should be completed and added (or rather, made into a covariance matrix)
    TMatrixDSym unc_cov(comb_syst.size());
    if (unc == "statistics"){
      unc_cov = getStatCovarianceMatrix(comb_syst, m_statVariations); // we want to handle the "statistics" uncertainty differently
    } else {
      unc_cov = getSystCovarianceMatrix(comb_syst);
    }
    cov_matrices.insert({unc, unc_cov}); // add the covariance matrix for the uncertainty to this map, this is temporary for testing/plotting purposes
    
    // To look only at uncertainties that pertain to more than one flavour (2, 3 or all 4) we can store the covariances separately for later saving and plotting
    if (flavs_in_common[0]+flavs_in_common[1]+flavs_in_common[2]+flavs_in_common[3] > 1){
      m_only_shared_systematics.insert(unc) ; // mark the shared systematics...
    }

    global_covariance += unc_cov;
  }
  return global_covariance;
}


//________________________________________________________________________________
void
CalibrationDataGlobalEigenVariations::excludeNamedUncertainty(const std::string& name, const std::string& flavour)//, CalibrationDataContainer* cnt)
{
  // Exclude the source of uncertainty identified by the given name from being used
  // in the construction of the covariance matrix to be diagonalised.
  // Notes:
  // - Some names returned by CalibrationDataContainer::listUncertainties() are not
  //   meaningful in this context, and specifying them is not allowed.
  // - Once the eigenvector diagonalisation has been carried out, this method may
  //   not be used anymore.

  if (m_initialized) std::cerr << "CalibrationDataGlobalEigenVariations::excludeNamedUncertainty error:" << " initialization already done" << std::endl;

  else if (name == "comment"    || name == "result"   || name == "systematics" || 
           name == "statistics" || name == "combined" || name == "extrapolation" || 
           name == "MCreference" || name == "MChadronisation" || name == "ReducedSets" || 
           name == "excluded_set" || name == "" || name == " ")
    std::cerr << "CalibrationDataGlobalEigenVariations::excludeNamedUncertainty error:" << " name [" << name << "] not allowed" << std::endl;

  // Note: we WANT to exclude named uncertainties FROM ALL FLAVOURS, even if they don't contain the uncertainty (just fill with zeros). So I won't do this check here.
  // only really add if the entry is not yet in the list
  else if (m_flav_namedIndices[flavour].find(name) == m_flav_namedIndices[flavour].end()) {
    m_flav_named[flavour].push_back(std::pair<TH1*, TH1*>(0, 0));
    m_flav_namedIndices[flavour][name] = m_flav_named[flavour].size()-1; // store the index that the name variation pair is stored with in m_named
  }
}


//________________________________________________________________________________
std::vector<std::string>
CalibrationDataGlobalEigenVariations::listNamedVariations(const std::string& flavour) const
{
  // Provides the list of named variations

  std::vector<std::string> names;
  for(const auto& namedar : m_flav_namedIndices.find(flavour)->second){
    names.push_back(namedar.first);
  }
  return names;
}


//________________________________________________________________________________
void
CalibrationDataGlobalEigenVariations::initialize(double min_variance)
{
  // This is this class's most important method, in the sense that it does all the
  // math and constructs all "variation" histograms (for both eigenvector and named
  // named variations). This constitutes the full initialisation of the class.
  // This method is meant to be called only after all calls (if any) to the
  // CalibrationDataGlobalEigenVariations::excludeNamedUncertainty() method.


  // First step: construct the block covariance matrix
  TMatrixDSym cov = getEigenCovarianceMatrix(); 

  TMatrixDSym corr(cov); // We want to construct the correlation matrix in order to compare the final eigenvariations correlation matrix to it
  for (int row = 0 ; row < cov.GetNrows() ; row++){
    for (int col = 0 ; col < cov.GetNcols() ; col++){
      double rowvar = sqrt(cov(row,row));
      double colvar = sqrt(cov(col,col));
      corr(row,col) = corr(row,col)/(rowvar * colvar); // divide each element by the variance
    }
  }

  ///*

  // Second step: create the variations for the named sources of uncertainty (easy...)
  // News flash: This isn't so easy now, as it has to be done for all the flavours to which the uncertainty pertains
  // the following are temporary data structures
  std::vector<double> combined_result; 
  std::map<std::string, int> flav_bins; // store the nbins of each flavour histogram for later use in "reporting"
  // and eventually, at the end, we will want to separate the flavour blocks out...
  for (const auto& flavour : m_flavours) {
    // for each flavour, we want to combine the results and uncertainty values across flavours into one "block" vector
    // retrieve the central calibration
    Analysis::CalibrationDataHistogramContainer* c = m_histcontainers[flavour]; // pointer to the flavour container

    TH1* result = dynamic_cast<TH1*>(c->GetValue("result"));
    if (not result){
      std::cerr<<"CalibrationDataGlobalEigenVariations::initialize: dynamic cast failed\n ";
      continue;
    }
    //construct the combined_result and combined_named_variations (up and down)
    if (c->getTagWeightAxis() == -1){ // For fixed cut WP, the Y axis **should** be the pT axis (but can it can potentially be different in the future)
      flav_bins[flavour] = result->GetNbinsY(); // Add the number of bins of the result histogram with non-zero results...  
      if (m_verbose) std::cout << "flav_bins["<<flavour<<"] = " << flav_bins[flavour] << std::endl;
      for(int i = 0 ; i < flav_bins[flavour] ; i++){
        // push bin content onto the combined_result vector
        Int_t bin = result->GetBin(1,i+1); // This will only pick up the CONTENTS, not of the underflow bin
        double res_value = result->GetBinContent(bin);
        combined_result.push_back(res_value);
      }
    } else if (c->getTagWeightAxis() == 0) { // for continuous WP, the taxweight axis determines which axis is pt and |eta|
      flav_bins[flavour] = result->GetNbinsX()*result->GetNbinsY();
      int tagbins = result->GetNbinsX();
      int ptbins = result->GetNbinsY();
      if (m_verbose) std::cout << "flav_bins["<<flavour<<"] = " << flav_bins[flavour] << std::endl;
      for(int i = 0 ; i < tagbins ; i++){
        for(int j = 0 ; j < ptbins ; j++){
          // push bin content onto the combined_result vector
          Int_t bin = result->GetBin(j+1,1,i+1); // This will only pick up the CONTENTS, not of the underflow bin
          double res_value = result->GetBinContent(bin);
          combined_result.push_back(res_value);
        }
      }
    } else if (c->getTagWeightAxis() == 1) {
      flav_bins[flavour] = result->GetNbinsX()*result->GetNbinsY();
      int tagbins = result->GetNbinsY();
      int ptbins = result->GetNbinsX();
      if (m_verbose) std::cout << "flav_bins["<<flavour<<"] = " << flav_bins[flavour] << std::endl;
      for(int i = 0 ; i < tagbins ; i++){
        for(int j = 0 ; j < ptbins ; j++){
          // push bin content onto the combined_result vector
          Int_t bin = result->GetBin(j+1,1,i+1); // This will only pick up the CONTENTS, not of the underflow bin
          double res_value = result->GetBinContent(bin);
          combined_result.push_back(res_value);
        }
      }
    } else if (c->getTagWeightAxis() == 2) {
      flav_bins[flavour] = result->GetNbinsX()*result->GetNbinsZ();
      int tagbins = result->GetNbinsZ();
      int ptbins = result->GetNbinsX();
      if (m_verbose) std::cout << "flav_bins["<<flavour<<"] = " << flav_bins[flavour] << std::endl;
      for(int i = 0 ; i < tagbins ; i++){
        for(int j = 0 ; j < ptbins ; j++){
          // push bin content onto the combined_result vector
          Int_t bin = result->GetBin(j+1,1,i+1); // This will only pick up the CONTENTS, not of the underflow bin
          double res_value = result->GetBinContent(bin);
          combined_result.push_back(res_value);
        }
      }
    }


    // loop over the excluded uncertainties for this flavour, and construct their actual variations...
    // the "m_flav_namedIndices" are constructed within "excludeNamedUncertainties", which is called in the constructor
    for (map<string, unsigned int>::iterator it = m_flav_namedIndices[flavour].begin(); it != m_flav_namedIndices[flavour].end(); ++it) {
      TH1* hunc = (TH1*) c->GetValue(it->first.c_str()); // this should store the name uncertainty, if it exists for this flavour 
      // I need to test if this uncertainty actually exists, and if it doesn't just use a ZERO variation histogram explicitly
      // but even if it doesn't exist, we want to have it, so that the indices match between flavours
      pair<TH1*, TH1*>& p = m_flav_named[flavour][it->second];
      TString namedvar("namedVar");
      namedvar += it->first.c_str();
      TString namedvarUp(namedvar);   namedvarUp   += "_up";
      TString namedvarDown(namedvar); namedvarDown += "_down";
      TH1* resultVariedUp   = (TH1*)result->Clone(namedvarUp);   
      TH1* resultVariedDown = (TH1*)result->Clone(namedvarDown); 
      if (hunc){
        resultVariedUp->Add(hunc, 1.0);    resultVariedUp->SetDirectory(0);
        resultVariedDown->Add(hunc, -1.0); resultVariedDown->SetDirectory(0);
      } else {
        resultVariedUp->SetDirectory(0);
        resultVariedDown->SetDirectory(0);
      }
      p.first  = resultVariedUp;
      p.second = resultVariedDown;
    } // End of uncertainty in flavour loop 
    
    // Now handle the extrapolation uncertainties per flavour...
    // Refinement: add the "extrapolation" uncertainty as a named uncertainty, if the histogram is provided
    // This is a bit special, since the extrapolation uncertainty histogram has a different size than other histograms
    if (TH1* hunc = (TH1*)c->GetValue("extrapolation")) { // this is just saying "if it exists"...
      TH1* resultVariedUp   = (TH1*) hunc->Clone("extrapolation_up");   resultVariedUp->SetDirectory(0);
      TH1* resultVariedDown = (TH1*) hunc->Clone("extrapolation_down"); resultVariedDown->SetDirectory(0);
      Int_t nbinx = hunc->GetNbinsX()+2, nbiny = hunc->GetNbinsY()+2, nbinz = hunc->GetNbinsZ()+2;
      Int_t firstbinx = hunc->GetXaxis()->FindFixBin(result->GetXaxis()->GetBinCenter(1));
      Int_t firstbiny = result->GetDimension() > 1 ? hunc->GetYaxis()->FindFixBin(result->GetYaxis()->GetBinCenter(1)) : 1;
      Int_t firstbinz = result->GetDimension() > 2 ? hunc->GetZaxis()->FindFixBin(result->GetZaxis()->GetBinCenter(1)) : 1;
      for (Int_t binx = 1; binx < nbinx-1; ++binx) {
        Int_t binxResult = binx - firstbinx + 1;
        if (binxResult < 1) binxResult = 1;
        if (binxResult > result->GetNbinsX()) binxResult = result->GetNbinsX();
        for (Int_t biny = 1; biny < nbiny-1; ++biny) {
          Int_t binyResult = biny - firstbiny + 1;
          if (binyResult > result->GetNbinsY()) binyResult = result->GetNbinsY();
          for (Int_t binz = 1; binz < nbinz-1; ++binz) {
            Int_t binzResult = binz - firstbinz + 1;
            if (binzResult > result->GetNbinsZ()) binzResult = result->GetNbinsZ();
            Int_t bin = hunc->GetBin(binx, biny, binz);
            double contResult = result->GetBinContent(binxResult, binyResult, binzResult);
            resultVariedUp->SetBinContent(bin, contResult + hunc->GetBinContent(bin));
            resultVariedDown->SetBinContent(bin, contResult - hunc->GetBinError(bin));
          }
        }
      }
      m_flav_named[flavour].push_back(std::make_pair(resultVariedUp, resultVariedDown));
      m_flav_namedExtrapolation[flavour] = m_flav_namedIndices[flavour]["extrapolation"] = m_flav_named[flavour].size()-1;
    }


  } // End flavour loop  
  


  // Third step: compute the eigenvector variations corresponding to the remaining sources of uncertainty
  // First, build the combined_result vector into a TH1
  std::unique_ptr<TH1> comb_result(new TH1D("combined_result", "", combined_result.size(), 0., 1.));
  int nbins = comb_result->GetNbinsX()+2;
  int ndim  = comb_result->GetDimension();
  if (ndim > 1) nbins*= (comb_result->GetNbinsY()+2);
  if (ndim > 2) nbins*= (comb_result->GetNbinsZ()+2);

  // I want to take the contents of "combined_result" and fill in "comb_result" without the under/overflow
  for (unsigned int i=0 ; i<combined_result.size() ; i++){
    // assuming dimension == 1, which should be the case...
    comb_result->SetBinContent(i+1, combined_result[i]); // setting i+1 so we don't start with the underflow bin
  } 

  // Get the portions of the covariance matrix that aren't zero, this next step 
  TMatrixT<double> matrixVariationJacobian = getJacobianReductionMatrix(cov); // we pass the covariance matrix in as a starting point

  int size = matrixVariationJacobian.GetNrows();

  // Reduce the matrix to one without the zeros, using a "similarity" transformation
  const TMatrixDSym matrixCovariance = cov.Similarity(matrixVariationJacobian); // <--- This step removes the zeros

  // Carry out the Eigenvector decomposition on this matrix
  TMatrixDSymEigen eigenValueMaker (matrixCovariance);
  TVectorT<double> eigenValues   = eigenValueMaker.GetEigenValues(); 
  TMatrixT<double> eigenVectors  = eigenValueMaker.GetEigenVectors(); 
  TMatrixT<double> matrixVariations (size,size);                     

  //compute the total variance by summing the eigenvalues
  m_totalvariance = eigenValues.Sum();

  for (int i = 0; i < size; ++i) {
    for (int r = 0; r < size; ++r) {
      //first index is the variation number, second corresponds to the pT bin      // The "eigenvariations" matrix is the "C" matrix which has CKC^T = I with "K" being the o.g. covariance matrix, and "I" is the identity.
      matrixVariations(i,r) = -1.0*eigenVectors[r][i]*sqrt(fabs(eigenValues[i])); //  So the result is a matrix (eigenvariation) which is the eigenvector scaled by the sqrt(eigenvalue)
    } 
  } // <------- matrixVariations: each row is one variation, each column is the pT bin.

  TMatrixT<double> matrixVariationsWithZeros = matrixVariations * matrixVariationJacobian; // This step adds in the zero rows again
  
  // Construct the initial set of variations from this
  for (int i = 0; i < matrixVariationsWithZeros.GetNrows(); ++i) {
    TString superstring("eigenVar");
    superstring+=i; 

    TString nameUp(superstring);   nameUp   += "_up"; // In the end you get something like "eigenVar5_up"
    TString nameDown(superstring); nameDown += "_down";
    // TString nameUnc(superstring);  nameUnc+= "_unc";

    TH1* resultVariedUp   = (TH1*)comb_result->Clone(nameUp);   resultVariedUp->SetDirectory(0);
    TH1* resultVariedDown = (TH1*)comb_result->Clone(nameDown); resultVariedDown->SetDirectory(0);

    for (int u = 0; u < comb_result->GetNbinsX(); ++u) {
      resultVariedUp->SetBinContent(u,(comb_result->GetBinContent(u) + matrixVariationsWithZeros(i,u)));
      resultVariedDown->SetBinContent(u,(comb_result->GetBinContent(u) - matrixVariationsWithZeros(i,u)));
    }

    m_eigen.push_back(std::make_pair(resultVariedUp, resultVariedDown)); //<--- This is currently storing the FULL/combined variations, which aren't binned with proper bin widths etc.
    // The "proper binning" isn't necessary for pruning purposes. Later on, after pruning, separate the flavour blocks of each eigenvariation and construct the variations for each flavour, storing results in m_flav_eigen

  } //end eigenvector size

  
  ////////////////////////////////////////////////////////////////////////////////////////////////
  // Without any changes, all eigenvariations are kept due to the variation being non-negligible for SOME flavour...
  ///////////////////////////////////////////////////////////////////////////////////////////
  // Step 4 : Time for PRUNING
  //  > Check the variation bins to see if any exceed a given threshold value of "min_variance"
  //  > Value of E-6 seems to work well for SFGlobalEigen (found through a scan of possible thresholds)
  //    but this is not necessarily going to hold for future CDI's. This needs to be checked through the
  //    "validate_reduction" function, which can be used to compare how well the reduction captures the total covariance.
  //////////////////////////////////////////////////////////////////////////////
  
  // Remove variations that are below the given tolerance (effectively meaning that they don't have any effect)
  IndexSet final_set;
  size_t current_set = 0;

  // We set the custom min_variance here 
  min_variance = 1.0E-6;
  for (size_t index = 0; index < m_eigen.size(); ++index) {
    bool keep_variation = false; // guilty until proven innocent
    TH1* up = static_cast<TH1*>(m_eigen[index].first->Clone()); up->SetDirectory(0); // clone the up variation and check it out
    up->Add(comb_result.get(), -1.0); // now we're left with decimal values centered around 0, i.e. 0.02 or -0.02

    for (int bin = 1; bin <= nbins; ++bin) {
      if (fabs(up->GetBinContent(bin)) > min_variance) { // If you find even ONE bin with big enough variance, we keep the whole systematic.
        keep_variation = true;
        break;
      }
    }
    if (!keep_variation){ // At this stage, if we find no bins in the systematic with large enough variation, we insert it to "final_set" for removal/pruning
      final_set.insert(current_set);
    } else {
      m_capturedvariance += eigenValues[index];
    }
    delete up;
    ++current_set;
  }
  if (final_set.size() > 0){ // at this point, a set of the indices of negligible variations should have been gathered, proceed to remove them...
    if (m_verbose) std::cout << "CalibrationDataEigenVariations: Removing " << final_set.size() << " eigenvector variations leading to sub-tolerance effects, retaining " << m_eigen.size()-final_set.size() << " variations" << std::endl;
  }
  
  CalibrationDataEigenVariations::removeVariations(final_set); // This method actually performs the reduction. The above logic simply flags which variations to get rid of, inserting them into "final_set"
  
  // AT THIS STAGE: Pruning has already occurred, leaving us with a set of combined eigenvariations in "m_eigen", which we can thence recombine and compute the correlation matrix
  // That correlation matrix can then be compared to the original correlation matrix "corr", simply subtracting one from the other. Better approximations will have close to zero deviation.
  // What follows is the construction of this comparison, and the reporting of the comparison (saving it to file)...  
  std::streamsize ss = std::cout.precision();
  if (m_verbose) std::cout << " The total variance is " << m_totalvariance << " and the reduction captured " << std::setprecision(9) << 100.0*(m_capturedvariance/m_totalvariance) << "% of this." << std::endl;
  std::cout.precision(ss); //restore precision
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///// optionally: perform a visual validation of the 'SFGlobalEigen' method alongside the 'SFEigen'.                       /////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if (m_validate){ // <---- this flag can be set manually in a local checkout of this package (temporary fix)
    validate_reduction(m_blockmatrixsize, corr, m_eigen, comb_result.get(), "SFGlobalEigen", m_taggername, m_wp, m_jetauthor); // This method is simply here to report the matrix comparison, for reduction scheme validation
  }


  ///////////////////////////////////////////////////////////////////////////////////////////
  // Let's separate out the flavour histograms now, this should be far simpler to ensure the binning is correct for each flavour.
  // At this stage, we should have constructed a set of combined eigenvariations stored in m_eigen. 
  // Below, we take them one by one, separate, and store the flavour variations in m_flav_eigen, with the proper flavour heading
  // <----- The "mergeVariations" code originally merged the variations in m_eigen. But now that wouldn't work, since m_eigen stores
  //        the combined histograms. We should instead merge the "reported" flavour histograms, as they're properly binned (and physically meaningful)
  //        So if we construct the flavour variations first, keeping the indices all the same, then merge them.
  //////////////////////////////////////////////////////////////////////////////
  

  for(const std::pair<TH1*,TH1*>& var : m_eigen){
    // now we make use of the same old flavour loop and impose the flavour variation to the flavour container
    TString eigenvarup = var.first->GetName();
    TString eigenvardown = var.second->GetName();
    int bin_baseline = 0; // increment this by flav_bins after getting each flavour block
    for (const std::string& flavour : m_flavours){
      Analysis::CalibrationDataHistogramContainer* c = m_histcontainers[flavour];
      TH1* result = dynamic_cast<TH1*>(c->GetValue("result"));
      if (not result){
        std::cerr<<"CalibrationDataGlobalEigenVariations::initialize: dynamic cast failed\n";
        continue;
      }
      TH1* resultVariedUp   = (TH1*)result->Clone(eigenvarup);   resultVariedUp->SetDirectory(0); // copy flavour result, want to set bin contents according to the combined eigenvartion flavour block
      TH1* resultVariedDown = (TH1*)result->Clone(eigenvardown); resultVariedDown->SetDirectory(0);
      int up_to_bin = flav_bins[flavour];
      int current_bin = 1; // starting from 1 for filling histograms
      for(int flav_bin = bin_baseline+1 ; flav_bin < up_to_bin+1 ; flav_bin++){ // the +1's here are to deal with bin numbering yet again...
        Int_t bin = result->GetBin(1,current_bin); // increment along smaller (result) flavour variation TH1
        resultVariedUp->SetBinContent(bin, var.first->GetBinContent(flav_bin));     // Sets the result TH1 bin content to the eigenvariation bin content
        resultVariedDown->SetBinContent(bin, var.second->GetBinContent(flav_bin)); // In this way, we achieve flavour variations with proper binning
        current_bin+=1;
      }
      bin_baseline+=up_to_bin;
      m_flav_eigen[flavour].push_back(std::make_pair(resultVariedUp, resultVariedDown)); // <-------- add the flavour EV to the storage (indexed the same as in m_eigen!)
    }
  }

  for(const auto& f : m_flavours){
    if (m_verbose) std::cout << " " << f << " m_flav_eigen has " << m_flav_eigen[f].size() << std::endl;
  }

  m_initialized = true;
}



//________________________________________________________________________________
TMatrixD
CalibrationDataGlobalEigenVariations::getJacobianReductionMatrix(TMatrixDSym& cov)
{
  // Construct the matrix that removes the rows and columns that fail to influence
  // the eigen-variations. To reduce the covariance matrix, do the following:
  // Note: Previous version called "getEigenCovariance" whereas here we pass it in by reference to save on compute
  // This "cov" is the "uncompressed" eigenvariation covariance matrix for all uncertainties.
  // We will subsequently compress it (removing columns/rows containing zeros only) 
  // and then construct the hopefully rectangular matrix that can remove these ones from the covariance matrix

  int nZeros = 0;
  std::vector<int> zeroComponents;

  // First flag all the zeros
  for (int i = 0 ; i < m_blockmatrixsize ; i++){
    if (fabs(cov(i,0)) < 1e-10){
      bool isThereANonZero = false;
      for (int j = 0 ; j < m_blockmatrixsize ; j++){
        // Try a first (quick) identification of rows/columns of zeros by the first element in each row
        // If "successful", check the whole row in more detail
        if (fabs(cov(i,j)) > 1e-10){
          isThereANonZero = true;
          break;
        }
      }
      if (! isThereANonZero){
        ++nZeros;
        zeroComponents.push_back(i) ;
      }
    }
  }

  if (nZeros >= m_blockmatrixsize) {
    std::cerr << " Problem. Found n. " << nZeros << " while size of matrix is " << m_blockmatrixsize << std::endl;
    return TMatrixDSym();
  }
  
  int size = m_blockmatrixsize - nZeros;
  TMatrixT<double> matrixVariationJacobian(size,m_blockmatrixsize);
  int nMissed = 0;

  for (int i = 0; i < m_blockmatrixsize; ++i) { // full size
    bool missed = false;
    for (unsigned int s = 0 ; s < zeroComponents.size(); ++s) { // <--- Basically what this does is it flags "missed" for a given "i" of the full bin size
      if (zeroComponents.at(s) == i) { // <--- if "i" is in "zeroComponents". Breaks (because it found that it's to be missed)
        missed = true;
        break;
      }
    }
    if (missed) {  // <-------- Finally, if "i" is to be missed, increase "nMissed" by one, and....
      ++nMissed;
      continue;
    }

    matrixVariationJacobian(i-nMissed,i)=1; //  <-------- ... this ALWAYS adds a one. If zero "nMissed", add to diagonal. otherwise off-diagonal
  }

  return matrixVariationJacobian;
}





//________________________________________________________________________________
unsigned int
CalibrationDataGlobalEigenVariations::getNumberOfEigenVariations(const std::string& flavour)
{
  if (! m_initialized) initialize();
  
  if (m_flav_eigen.find(flavour) == m_flav_eigen.end()){
    if (m_verbose) std::cout << "No m_flav_eigen for flavour " << flavour << std::endl;
    return 0;
  }
  return (m_flav_eigen.find(flavour)->second).size();
}

//________________________________________________________________________________
bool
CalibrationDataGlobalEigenVariations::getEigenvectorVariation(const std::string& flavour, unsigned int variation, TH1*& up, TH1*& down)
{
  // Return the pointers to the up- and downward variation histogram for the specified
  // eigenvector variation. In case of an invalid variation number, null pointers will
  // be returned and the return value will be false.
  //
  //     variation:   eigenvector variation number
  //     up:          (reference to) pointer to upward variation histogram
  //     down:        (reference to) pointer to downward variation histogram

  if (! m_initialized) initialize();
  std::vector<std::pair<TH1*,TH1*>> flav_variations = m_flav_eigen.find(flavour)->second;
  if (variation < flav_variations.size()){
    up   = flav_variations[variation].first;
    down = flav_variations[variation].second;
    return true;
  }

  up = down = 0;
  return false;
}

//________________________________________________________________________________
bool
CalibrationDataGlobalEigenVariations::getNamedVariation(const string& flavour, const string& name, TH1*& up, TH1*& down)
{
  // Return the pointers to the up- and downward variation histogram for the specified
  // named variation. In case of an invalid named variation, null pointers will
  // be returned and the return value will be false.
  //
  //     name:        named variation
  //     up:          (reference to) pointer to upward variation histogram
  //     down:        (reference to) pointer to downward variation histogram

  // Find the named variation index (if it exists) and pass to "getNamedVariation"
  std::map<std::string, unsigned int>::const_iterator it = (m_flav_namedIndices.find(flavour)->second).find(name);
  if (it != (m_flav_namedIndices.find(flavour)->second).end()) return getNamedVariation(it->second, flavour, up, down);

  up = down = 0;
  return false;
}

//________________________________________________________________________________
bool
CalibrationDataGlobalEigenVariations::getNamedVariation(unsigned int nameIndex, const std::string& flavour, TH1*& up, TH1*& down)
{
  // Return the pointers to the up- and downward variation histogram for the specified
  // named variation. In case of an invalid named variation number, null pointers will
  // be returned and the return value will be false.
  //
  //     nameIndex:   named variation number
  //     up:          (reference to) pointer to upward variation histogram
  //     down:        (reference to) pointer to downward variation histogram

  if (! m_initialized) initialize();

  if (nameIndex < m_flav_named[flavour].size()) {
    up   = m_flav_named[flavour][nameIndex].first;
    down = m_flav_named[flavour][nameIndex].second;
    return true;
  }

  up = down = 0;
  return false;
}


//________________________________________________________________________________
unsigned int
CalibrationDataGlobalEigenVariations::getNamedVariationIndex(const std::string& name, const std::string& flavour) const
{
  // Return the integer index corresponding to the named variation.
  // Note that no checks are made on the validity of the name provided.
  std::map<std::string, unsigned int>::const_iterator it = (m_flav_namedIndices.find(flavour)->second).find(name);
  return it->second;
}


//________________________________________________________________________________
bool
CalibrationDataGlobalEigenVariations::isExtrapolationVariation(unsigned int nameIndex, const std::string& flavour) const
{
  // Verifies whether the given named variation index corresponds to the extrapolation
  // uncertainty.
  int extrapIndex = m_flav_namedExtrapolation.find(flavour)->second;
  return (extrapIndex == int(nameIndex));
}



//________________________________________________________________________________
void
CalibrationDataGlobalEigenVariations::mergeVariationsFrom(const size_t& index, std::string& flav)
{
  // Merge all systematic variation starting from the given index.
  // The resulting merged variation replaces the first entry in the list
  // (i.e., the entry specified by the index).

  IndexSet simple_set;

  for (size_t it = index; it < m_flav_eigen[flav].size(); ++it)
    simple_set.insert(it);
  mergeVariations(simple_set, flav);
}

//________________________________________________________________________________
void
CalibrationDataGlobalEigenVariations::mergeVariations(const IndexSet &set, std::string& flav)
{
  IndexSuperSet sset;
  sset.insert(set);
  mergeVariations(sset, flav);
}


//________________________________________________________________________________
void
CalibrationDataGlobalEigenVariations::mergeVariations(const IndexSuperSet &set, std::string& flav)
{
  ////////////////////////////////////////////////////////////////////////
  // Merge the (flavour specific) eigenvariations, as stored in m_flav_eigen
  ////////////////////////////////////////////////////////////////////////

  // check for overlap
  IndexSet checker;
  for (IndexSuperSet::iterator set_it = set.begin(); set_it != set.end(); ++set_it) {
    for (IndexSet::iterator subset_it = set_it->begin(); subset_it != set_it->end(); ++subset_it){
      if (checker.count(*subset_it) == 0 && *subset_it <= m_flav_eigen[flav].size())
        checker.insert(*subset_it);
      else {
        std::cerr << "Error in CalibrationDataGlobalEigenVariations::mergeVariations: \
          IndexSets must not overlap and must lie between 1 and " << m_eigen.size() << ". Aborting!" << std::endl;
        return;
      }
    }
  }

  std::string flavour = flav;
  
  Analysis::CalibrationDataHistogramContainer* c = m_histcontainers[flavour];

  TH1* result = dynamic_cast<TH1*>(c->GetValue("result"));
  if (not result){
    std::cerr << "Error in CalibrationDataGlobalEigenVariations::mergeVariations: failed dynamic cast\n";
    return;
  }
  // retrieve the central calibration

  IndexSet toDelete;
  int nbins = result->GetNbinsX()+2;
  int ndim  = result->GetDimension();
  if (ndim > 1) nbins *= (result->GetNbinsY()+2);
  if (ndim > 2) nbins *= (result->GetNbinsZ()+2);

  // complex sum
  for (IndexSuperSet::iterator set_it = set.begin(); set_it != set.end(); ++set_it) {
    if (set_it->empty()) continue;

    double sum_H_up = 0.0, sum_H_down = 0.0;
    size_t lowest_index = *set_it->lower_bound(0);
    TH1 *total_var_up = static_cast<TH1*>(m_flav_eigen[flavour][lowest_index].first->Clone());
    TH1 *total_var_down = static_cast<TH1*>(m_flav_eigen[flavour][lowest_index].second->Clone());
    total_var_up->SetDirectory(0);
    total_var_down->SetDirectory(0);

    total_var_up->Reset();
    total_var_down->Reset();

    // sum all other variations
    for (IndexSet::iterator subset_it = set_it->begin(); subset_it != set_it->end(); ++subset_it) {
      size_t actual_index = *subset_it;

      if (actual_index != lowest_index) toDelete.insert(*subset_it); // 

      TH1 *partial_var_up = static_cast<TH1*>(m_flav_eigen[flavour][actual_index].first->Clone()); 
      TH1 *partial_var_down = static_cast<TH1*>(m_flav_eigen[flavour][actual_index].second->Clone());
      partial_var_up->SetDirectory(0);
      partial_var_down->SetDirectory(0);
      
      partial_var_up->Add(result, -1.0);
      partial_var_down->Add(result, -1.0);
      for (int i = 0; i < nbins; ++i) {
        partial_var_down->SetBinContent(i, -1.0*partial_var_down->GetBinContent(i));
      }

      for (int u = 0; u < nbins; ++u) {
        double sum_up = total_var_up->GetBinContent(u);
        double sum_down = total_var_down->GetBinContent(u);
        for (int v = 0; v < nbins; ++v) {
          sum_up += partial_var_up->GetBinContent(u)*partial_var_up->GetBinContent(v);
          sum_H_up += partial_var_up->GetBinContent(u)*partial_var_up->GetBinContent(v);
          sum_down += partial_var_down->GetBinContent(u)*partial_var_down->GetBinContent(v);
          sum_H_down += partial_var_down->GetBinContent(u)*partial_var_down->GetBinContent(v);
        }
        total_var_up->SetBinContent(u, sum_up);
        total_var_down->SetBinContent(u, sum_down);
      }
      delete partial_var_up;
      delete partial_var_down;
    }

    // final part of complex summing
    for (int i = 0; i < nbins; ++i) {
      if (sum_H_up != 0.0)
        total_var_up->SetBinContent(i, total_var_up->GetBinContent(i)/sqrt(sum_H_up));
      else
        total_var_up->SetBinContent(i, 0.0);
      if (sum_H_down != 0.0)
        total_var_down->SetBinContent(i, -1.0*total_var_down->GetBinContent(i)/sqrt(sum_H_down));
      else
        total_var_down->SetBinContent(i, 0.0);
    }

    total_var_up->Add(result);
    total_var_down->Add(result);

    m_flav_eigen[flavour][lowest_index].first = total_var_up;
    m_flav_eigen[flavour][lowest_index].second = total_var_down;
  }
  
  removeVariations(toDelete, flavour);
}


//________________________________________________________________________________
void
CalibrationDataGlobalEigenVariations::removeVariations(const IndexSet &set, std::string& flav)
{
  if (set.size() == 0) return;
  std::vector<std::pair<TH1*, TH1*> > new_eigen;
  std::vector<std::pair<TH1*, TH1*>> eigen = m_flav_eigen[flav];
  for (size_t index = 0; index < eigen.size(); ++index){
      if (set.count(index) == 0) {
        new_eigen.push_back(eigen[index]);
      } else { 
        delete eigen[index].first; delete eigen[index].second; 
      }
    }

  m_flav_eigen[flav] = std::move(new_eigen);

}

//________________________________________________________________________________
void
CalibrationDataGlobalEigenVariations::removeVariations(const IndexSuperSet &set, std::string& flav)
{
  IndexSet simple_set;

  for (IndexSuperSet::iterator set_it = set.begin(); set_it != set.end(); ++set_it) {
    for (IndexSet::iterator subset_it = set_it->begin(); subset_it != set_it->end(); ++subset_it)
      simple_set.insert(*subset_it);
  }

  removeVariations(simple_set, flav);
}
