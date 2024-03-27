/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*! \file RootFitTEff.cxx fits TEfficiency and returns dqm_core::Result
 * \author Akimasa Ishikawa (akimasa.ishikawa), 15th Apr 2010
 * \author Valerio Ippolito (valerio.ippolito), 14th Mar 2024
 *  based on Fermi fit in RootFit.cxx by Matt King
 */

#include <dqm_algorithms/RootFitTEff.h>
#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <TH1.h>
#include <TF1.h>
#include <TEfficiency.h>
#include <TGraphAsymmErrors.h>
#include <TClass.h>
#include <ers/ers.h>
#include <TROOT.h>
#include <dqm_core/AlgorithmManager.h>
#include <cmath>

namespace
{
  dqm_algorithms::RootFitTEff fermi_fit( "fermi" );
  dqm_algorithms::RootFitTEff erf_fit( "erf" );
  dqm_algorithms::RootFitTEff flat_fit( "flat" ); 
}

dqm_algorithms::RootFitTEff::RootFitTEff( const std::string & name )
  : m_name( name )
{ 

  if (m_name == "fermi"){
    m_func = std::make_unique<TF1> ( "fermi","[0]/(1+exp(([1]-x)/[2]))" );
  }
  if (m_name == "erf"){
    m_func = std::make_unique<TF1> ( "erf","[0]*TMath::Erf((x-[1])/(sqrt(2.)*[2]))" );
  }
  if (m_name == "flat"){
    m_func = std::make_unique<TF1> ( "flat","[0]" );
  }
  dqm_core::AlgorithmManager::instance().registerAlgorithm( "Simple_"+name +"_Fit_TEff", this );
}

dqm_algorithms::RootFitTEff::~RootFitTEff()
{
  // totally defeats the purpose of unique_ptr, but fixes a segfault in 5.34 ...
  (void)m_func.release();
}

dqm_algorithms::RootFitTEff * 
dqm_algorithms::RootFitTEff::clone()
{
  return new RootFitTEff( m_name );
}


dqm_core::Result *
dqm_algorithms::RootFitTEff::execute(	const std::string & name, 
					const TObject & object, 
					const dqm_core::AlgorithmConfig & config )
{  

  //std::cout<<"ROOTFITTEFF = calling rootfit with name "<<name<<std::endl;
  const TEfficiency * eff;
  if(object.IsA()->InheritsFrom( "TEfficiency" ))
    {  
      eff = static_cast<const TEfficiency*>(&object);
    }
  else {
    throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TEfficiency" );
  }
  
  //std::cout<<"ROOTFITTEFF = trying to get parameters"<<std::endl;  
  Axis_t xmin = dqm_algorithms::tools::GetFirstFromMap( "xmin", config.getParameters(), eff->GetTotalHistogram()->GetXaxis()->GetXmin());
  Axis_t xmax = dqm_algorithms::tools::GetFirstFromMap( "xmax", config.getParameters(), eff->GetTotalHistogram()->GetXaxis()->GetXmax());

  Axis_t ymin = dqm_algorithms::tools::GetFirstFromMap( "ymin", config.getParameters(), eff->GetTotalHistogram()->GetYaxis()->GetXmin());
  Axis_t ymax = dqm_algorithms::tools::GetFirstFromMap( "ymax", config.getParameters(), eff->GetTotalHistogram()->GetYaxis()->GetXmax());

  double xaxismean = (xmax + xmin)/2.;
  double xdiff = xmax - xmin;
  double ydiff = ymax - ymin;


  const double minpoint = dqm_algorithms::tools::GetFirstFromMap( "MinPoint", config.getParameters(), -1);
  const bool verbose = static_cast<bool>(dqm_algorithms::tools::GetFirstFromMap( "Verbose", config.getParameters(), 0));
  const double minSig = dqm_algorithms::tools::GetFirstFromMap( "MinSignificance", config.getParameters(), 0);
  const bool improve = static_cast<bool>(dqm_algorithms::tools::GetFirstFromMap( "ImproveFit", config.getParameters(), 0));

  //std::cout << "verbose " << verbose 
  //<< " draw " << draw  
  //<< " lf " << lf << std::endl;
  //std::cout<<"ROOTFITTEFF = trying to get num points"<<std::endl;  
  const TH1* eff_hist = eff->GetTotalHistogram();
  const Int_t eff_points = eff_hist->GetNbinsX();
  if (eff_points < minpoint || eff_points==0) {
    if(verbose)std::cout << name << " number of points are too small " << eff_points << std::endl;
    dqm_core::Result *result = new dqm_core::Result(dqm_core::Result::Undefined);
    result->tags_["InsufficientN"] = eff_points;
    return result;
  }
  if(verbose)std::cout << name << " enough number of points  " << eff_points << std::endl;
  //std::cout<<"ROOTFITTEFF = trying to get axes"<<std::endl;    
  const TAxis *x = eff_hist->GetXaxis();
  int nbins = x->GetNbins();
  double high = x->GetBinUpEdge(nbins);
  double low = x->GetBinUpEdge(0);


  if ( xmin>high || xmin<low || xmax>high || xmax<low) {
    throw dqm_core::BadConfig( ERS_HERE, name, "xmin and/or xmax value not in eff bin range" );
  }
	 
  std::string option;
  //Always set option end to avoid making graphics object and drawing it.
  //draw fit curve if DrawFitCurve == 1.0 
  if (verbose){
    //if( draw == 1.0 ) option = "";
    //else option="N";
    option="N";
  } else {
    //if( draw == 1.0 ) option = "Q";
    //else option = "QN";
    option = "QN";
  }

  //Use Minos technique as recommended for better error calculation, if errors are important:
  // if both minos and improve are specified ("EM"), minos is used by ROOT automatically
  if ( minSig != 0    ) option += "E";
  if ( improve ) option += "M";
  //show histo name and fit option for verbose mode
  if( verbose ){
    std::cout <<" eff name " << eff->GetName() << std::endl;
    std::cout <<" fit option " << option << std::endl;
  }
  //std::cout<<"ROOTFITTEFF = trying to do fits"<<std::endl;  
  if (m_name == "fermi") {
    if(verbose)std::cout << "set "<<name<< " parameters" << std::endl;
    m_func->SetParameter(0,ymax*0.9);
    m_func->SetParameter(1,xaxismean);
    m_func->SetParameter(2,xdiff/50.);
    m_func->SetParNames("Plateau","Threshold","Resolution");

    m_func->SetParLimits(0, ymin - 0.1 * ydiff, ymax + 0.1 * ydiff );
    m_func->SetParLimits(1, xmin, xmax );
    m_func->SetParLimits(2, 0., xdiff/4. );
  }
  else if(m_name == "erf") {
    if(verbose)std::cout << "set "<<name<< " parameters" << std::endl;
    m_func->SetParameter(0,ymax*0.9);
    m_func->SetParameter(1,xaxismean);
    m_func->SetParameter(2,xdiff/50.);
    m_func->SetParNames("Plateau","Threshold","Resolution");

    m_func->SetParLimits(0, ymin - 0.1 * ydiff, ymax + 0.1 * ydiff );
    m_func->SetParLimits(1, xmin, xmax );
    m_func->SetParLimits(2, 0., xdiff/4. );
  }
  else if(m_name == "flat") {
    if(verbose)std::cout << "set "<<name<< " parameters" << std::endl;
    m_func->SetParNames("Height");
  }
/*
  const int numsig = m_func->GetParNumber("Sigma");

  if (numsig != -1 ){
  	  double sigmaup = dqm_algorithms::tools::GetFirstFromMap( "Sigma_upperLimit", config.getParameters(), 1000000);
          m_func->SetParLimits(numsig, 0., sigmaup);
  }
  */ 

  option += "R"; // we need to specify the fit range to fit the TEfficiency (alternative would have been to use TEfficiency::CreateGraph())
  m_func->SetRange(xmin, xmax);

  if(verbose)std::cout << "fit "<<name<< " with interval cut " << xmin << " - " << xmax  << std::endl;
  std::shared_ptr<TGraphAsymmErrors> graph(eff->CreateGraph()); // we own this!
  graph->Fit( m_func.get(), option.c_str());

  const int numsig = m_func->GetParNumber("Sigma");
  if (numsig != -1 ){
	  double sigma=m_func->GetParameter(numsig);
	  m_func->SetParameter(numsig,std::abs(sigma));
  }

  try {
    dqm_core::Result *result= dqm_algorithms::tools::GetFitResult (m_func.get() , config, minSig );
    return result;
  }
  catch ( dqm_core::Exception & ex ) {
    throw dqm_core::BadConfig( ERS_HERE, name, ex.what(), ex );
  }
}

void
dqm_algorithms::RootFitTEff::printDescription(std::ostream& out)
{
  out<<"Simple_"+m_name+"_Fit_TEff: Does simple "+m_name+" fit to eff and checks fit parameters against thresholds\n"<<std::endl;
  if (m_name == "fermi" ) {
    out<<"The following fit Parameters can be checked with Red and Green Thresholds; only one parameter is needed to get back real result"<<std::endl;
    out<<"Green/Red Threshold: Plateau : Plateau fit value to give Green/Red Result"<<std::endl;
    out<<"Green/Red Threshold: Threshold : Fermi energy fit value to give Green/Red Result"<<std::endl;
    out<<"Green/Red Threshold: Resolution : Templature fit value to give Green/Red Result\n"<<std::endl;
  }else if (m_name == "erf" ) {
    out<<"The following fit Parameters can be checked with Red and Green Thresholds; only one parameter is needed to get back real result"<<std::endl;
    out<<"Green/Red Threshold: Plateau : Plateau fit value to give Green/Red Result"<<std::endl;
    out<<"Green/Red Threshold: Threshold : mean of gaussian fit value to give Green/Red Result"<<std::endl;
    out<<"Green/Red Threshold: Resolution : sigma of gaussian fit value to give Green/Red Result\n"<<std::endl;
  }
  out<<"Optional Parameter: Verbose: Write out fit results to log file (set to 1)"<<std::endl;
  out<<"Optional Parameter: MinPoint: Minimum eff point needed to perform Algorithm"<<std::endl;
  out<<"Optional Parameter: MinSignificance : Minimum multiple of the error in fit paramenter by which the parameter must exceed the thresholds"<<std::endl;
  out<<"Optional Parameter: ImproveFit : IMPROVE is used to avoid local minima"<<std::endl;
  out<<"Optional Parameter: xmin: minimum x range"<<std::endl;
  out<<"Optional Parameter: xmax: maximum x range"<<std::endl;
  out<<"Optional Parameter: SubtractFromMean: value subtracted from XMean before test is applied: allows using AbsXMean for non-zero expected mean"<<std::endl;
//  out<<"Optional Parameter: Sigma_upperLimit: Upper limit on Sigma- lower limit set to 0. and default upper value is 1e^6\n"<<std::endl;
                                                                                                                                            
}

