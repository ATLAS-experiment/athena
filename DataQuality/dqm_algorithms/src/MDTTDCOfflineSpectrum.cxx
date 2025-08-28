/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#include <dqm_core/AlgorithmConfig.h>
#include <dqm_algorithms/MDTTDCOfflineSpectrum.h>
#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <TF1.h>
#include <TClass.h>
#include <ers/ers.h>

#include <iostream>

#include <dqm_core/AlgorithmManager.h>

namespace
{
  dqm_algorithms::MDTTDCOfflineSpectrum MDTTDCOfflineSpectrum( "AlgMDTTDCOfflineSpectrum" );
}


dqm_algorithms::MDTTDCOfflineSpectrum::MDTTDCOfflineSpectrum( const std::string & name )
  : m_name( name )
{
  dqm_core::AlgorithmManager::instance().registerAlgorithm(name, this);
}

dqm_algorithms::MDTTDCOfflineSpectrum * 
dqm_algorithms::MDTTDCOfflineSpectrum::clone()
{
  
  return new MDTTDCOfflineSpectrum( m_name );
}


dqm_core::Result *
dqm_algorithms::MDTTDCOfflineSpectrum::execute(	const std::string &  name, 
						const TObject & object, 
                                                const dqm_core::AlgorithmConfig & config )
{


  if (!object.IsA()->InheritsFrom("TH1")) {
        throw dqm_core::BadConfig(ERS_HERE, name, "does not inherit from TH1");
  }
  std::unique_ptr<TH1> histogram(static_cast<TH1 *>(object.Clone())); // we just checked that this is really a TH1, so we can safely type-cast the pointer                                                                                
  if (histogram->GetDimension() > 2) {
    throw dqm_core::BadConfig(ERS_HERE, name, "dimension > 2");
  }

  const double minstat = dqm_algorithms::tools::GetFirstFromMap( "MinStat", config.getParameters(), -1);
  
  if (histogram->GetEntries() < minstat ) {
    dqm_core::Result *result = new dqm_core::Result(dqm_core::Result::Undefined);
    result->tags_["InsufficientEntries"] = histogram->GetEntries();
    return result;
  }
 
  double t0_low_warning;
  double t0_low_error;
  double t0_high_warning;
  double t0_high_error;
  double tmax_low_warning;
  double tmax_low_error;
  double tmax_high_warning;
  double tmax_high_error;
  try {
    t0_low_warning = dqm_algorithms::tools::GetFirstFromMap( "t0_low_Warning", config.getParameters() );
    t0_low_error = dqm_algorithms::tools::GetFirstFromMap( "t0_low_Error", config.getParameters() );
    t0_high_warning = dqm_algorithms::tools::GetFirstFromMap( "t0_high_Warning", config.getParameters() );
    t0_high_error = dqm_algorithms::tools::GetFirstFromMap( "t0_high_Error", config.getParameters() );
    tmax_low_warning = dqm_algorithms::tools::GetFirstFromMap( "tMax_low_Warning", config.getParameters() );
    tmax_low_error = dqm_algorithms::tools::GetFirstFromMap( "tMax_low_Error", config.getParameters() );
    tmax_high_warning = dqm_algorithms::tools::GetFirstFromMap( "tMax_high_Warning", config.getParameters() );
    tmax_high_error = dqm_algorithms::tools::GetFirstFromMap( "tMax_high_Error", config.getParameters() );
  }
  catch ( dqm_core::Exception & ex ) {
    throw dqm_core::BadConfig( ERS_HERE, name, ex.what(), ex );
  }


  dqm_core::Result* result = new dqm_core::Result();

  double t0;
  double tmax;
  double tdrift;
  double t0Err;
  double tmaxErr;
    
  TF1* t0Fit = histogram->GetFunction("func1");
  TF1* tmaxFit = histogram->GetFunction("func2");
  
  if(!t0Fit || !tmaxFit){
    MDTFitTDC(histogram.get(), t0, t0Err, tmax, tmaxErr);
    tdrift = tmax - t0;
  }else{
    t0 = t0Fit->GetParameter(1);
    tmax = tmaxFit->GetParameter(1);
    tdrift = tmax - t0;
    t0Err = t0Fit->GetParameter(2);
    tmaxErr = tmaxFit->GetParameter(2);
  }

  
  ERS_DEBUG(1, m_name << " TDrift " << " is " << tdrift );
  ERS_DEBUG(1,"Green threshold: "<< t0_low_warning << " < t0 < "<< t0_high_warning << " &&  " << tmax_low_warning <<" < tmax < " << tmax_high_warning <<   
	    " ;  Red threshold : t0 < " << t0_low_error      << "\n" << 
	    "t0 > " << t0_high_error     << "\n" << 
	    "tmax > " << tmax_high_error << "\n" <<
	    "tmax < " << tmax_low_error
            );    
  
  std::map<std::string,double> tags;

  if (t0 > t0_low_warning && t0 < t0_high_warning && tmax < tmax_high_warning && tmax > tmax_low_warning && std::abs(tdrift) < 1000) {
    result->status_ = dqm_core::Result::Green;
  }
  else if (t0 > t0_low_error && t0 < t0_high_error && tmax < tmax_high_error && tmax > tmax_low_error && std::abs(tdrift) < 1200) {
    if(t0 < t0_low_warning) tags["t0_low_Warning"] = t0;
    else if(t0 > t0_high_warning) tags["t0_high_Warning"] = t0;
    else tags["t0"] = t0;
    if(tmax > tmax_high_warning) tags["tMax_high_Warning"] = tmax;
    else if(tmax < tmax_low_warning) tags["tMax_low_Warning"] = tmax;
    else tags["tMax"] = tmax;
    if( std::abs(tdrift) > 1200 ) tags["tDrift_Warning"] = tdrift;
    else tags["tdrift"] = tdrift;
    result->status_ = dqm_core::Result::Yellow;
  }
  else {
    result->status_ = dqm_core::Result::Red;
    if(t0 < t0_low_error) tags["t0_low_Error"] = t0;
    else if(t0 > t0_high_error) tags["t0_high_Error"] = t0;
    else tags["t0"] = t0;
    if(tmax > tmax_high_error) tags["tMax_high_Error"] = tmax;
    else if(tmax < tmax_low_error) tags["tMax_low_Error"] = tmax;
    else tags["tMax"] = tmax;
    if( std::abs(tdrift) > 1400 ) tags["tDrift_Error"] = tdrift;
    else tags["tdrift"] = tdrift;
  }

  if(tags.size()==0) {
    tags["t0"] = t0;
    tags["tmax"] = tmax;
    tags["tdrift"] = tdrift;
  }
  if(t0Err > 2.*t0){
    tags["t0_FitError"] = t0Err;
    result->status_ = dqm_core::Result::Red;
  }
  if(tmaxErr > 2.*tmax) {
    tags["tmax_FitError"] = tmaxErr;
    result->status_ = dqm_core::Result::Red;
  }

  result->tags_ = tags;
 
  return result;
  
}

void
dqm_algorithms::MDTTDCOfflineSpectrum::printDescription(std::ostream& out)
{
  
  out << "Analyze pre-fitted MDTTDC spectra histograms" << std::endl;
  out << "Required Parameters: name " << m_name << ", t0Warning threshold" <<  ", tMaxWarning threshold" << std::endl;
  out << "Required parameters: t0Error" << ", tMaxError " << std::endl;
  out << "Returns yellow if t0 < t0Warning or t0 > 800 or if tMax > tMaxWarning or tMax < 800" << std::endl;
  out << "Returns red if t0 < t0Error or t0 > 1000 or if tMax > tMaxError or tMax < 700" << std::endl;
  out << "Returns red if t0Err > 2.*t0 or tMaxErr > 2.*tMax" << std::endl;
  out << "Requires that a histogram has a function named \"func1\" for the t0 fit and \"func2\" for the tMax fit" << std::endl;

}

void dqm_algorithms::MDTTDCOfflineSpectrum::MDTFitTDC(TH1* h, double &t0, double &t0err, double &tmax, double &tmaxerr)
{
  t0 = tmax = 0;
  t0err = tmaxerr = 0;
  double up = h->GetBinCenter(h->GetMaximumBin()+1);
  if( up > 200 ) up = 200;
  double down = up + 650;
  if( up < 50 ) up = 50;
  double parESD0 = h->GetBinContent(h->GetMinimumBin());
  double parESD1 = up;
  double parESD2 = 20;
  double parESD3 = h->GetBinContent(h->GetMaximumBin()) - h->GetBinContent(h->GetMinimumBin());
  std::unique_ptr<TF1> func1 = std::make_unique<TF1>("func1", "[0]+([3]/(1+(TMath::Exp((-x+[1])/[2]))))", 0, up); // tzero             
  func1->SetParameters(parESD0, parESD1, parESD2, parESD3);
  if(h->GetEntries()>100){
    h->Fit("func1","NRQ");
    t0 = func1->GetParameter(1) ;
    t0err = func1->GetParError(1);
    double binAtT0 = (double)h->GetBinContent(h->FindBin(t0));
    if(binAtT0<1) binAtT0 = 1;
    t0err += 10.0 * func1->GetChisquare() / (0.01*binAtT0*binAtT0*(double)func1->GetNumberFitPoints()); // to additionally account for bad fits                                                                                   
  }
  
  parESD0 = h->GetBinContent(h->GetMinimumBin());
  parESD1 = down;
  parESD2 = 50;
  parESD3 = (h->GetBinContent(h->GetMaximumBin())-h->GetBinContent(h->GetMinimumBin()))/10.;
  std::unique_ptr<TF1> func2 = std::make_unique<TF1>("func2", "[0]+([3]/(1+(TMath::Exp((x-[1])/[2]))))", down-135, down+135); // tmax
  func2->SetParameters(parESD0,parESD1,parESD2,parESD3);
  if(h->GetEntries()>100){
    func2->SetParLimits(0, parESD0, 2.0*parESD0+1);
    func2->SetParLimits(2, 5, 90);
    func2->SetParLimits(3, 0.2*parESD3, 7*parESD3);
    h->Fit("func2","WWNRQ+");
    tmax = func2->GetParameter(1);
    tmaxerr = func2->GetParError(1);
    double binAtTmax = (double)h->GetBinContent(h->FindBin(tmax));
    if(binAtTmax<1) binAtTmax = 1;
    tmaxerr += 10.0 * func2->GetChisquare() / (0.01*binAtTmax*binAtTmax*(double)func2->GetNumberFitPoints()); // to additionally account for bad fits                                                                             
  }

}
