/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "JetToolHelpers/MCJESInputBase.h"
#include "TAxis.h"
#include "TFile.h"
#include "TObjString.h"



#include <vector>

namespace JetHelper {

MCJESInputBase::MCJESInputBase(const std::string& name)
     : asg::AsgTool(name)
     , m_config{nullptr}
 { }

bool MCJESInputBase::readMCJESFromText()
{
    // Open the input file
    std::string local_path=static_cast<std::string> (m_fileName);
    m_config = std::unique_ptr<TEnv>(new TEnv(local_path.c_str()));

    std::string jetAlgo=static_cast<std::string> (m_jetAlgo);
    std::string corrName=static_cast<std::string> (m_corrName);

    std::vector<double> etaBins = VectorizeD(m_config->GetValue("JES.EtaBins","")," ");
    if (etaBins.size()==0){ // default binning
	for (int i=0;i<=90; i++) 
            etaBins.push_back(0.1*i-4.5);
    }

    ATH_MSG_DEBUG("Number eta bins: " << etaBins.size());

    m_etaBinAxis = new TAxis(etaBins.size()-1,&etaBins[0]);// this is to search for eta bin

    for (uint ieta=0; ieta<etaBins.size()-1; ++ieta)
    {

    	TString key=Form("%s.%s_Bin%d",corrName.c_str(),jetAlgo.c_str(),ieta);

 	ATH_MSG_DEBUG("reading: " << key);
	ATH_MSG_DEBUG("values in text: " << m_config->GetValue(key,""));

	std::vector<double> params = VectorizeD(m_config->GetValue(key,"")," ");
    	m_nPar = params.size();

    	ATH_MSG_DEBUG("Number of parameters: " << m_nPar);

    	if(m_corrName == "JES" ) for (uint ipar=0;ipar<m_nPar;++ipar) m_JESFactors[ieta][ipar] = params[ipar];
	if(m_corrName == "EtaCorr" ) for (uint ipar=0;ipar<m_nPar;++ipar) m_etaCorrFactors[ieta][ipar] = params[ipar];
	if(m_corrName == "EmaxJES") for (uint ipar=0;ipar<m_nPar;++ipar) m_energyFreezeJES[ieta] = params[ipar];
    }

    return true;
}

double MCJESInputBase::getJES(const double X, const double Y, const double Emax) const
{
    double JES_R;
    int binEta = getEtaBin(Y);
    const double *factors = m_JESFactors[binEta];

    double E;
    if(Emax < 0){ // if Emax is negative, we're done, just use raw E
        E = X;
    } else { // Emax is positive, so compare to raw E
        if (X > Emax){ // case where we do apply the frozen value
            E = Emax;
        } else { // no freezing necessary, use raw E
            E = X;
        }
    }

    double R = getLogPolN(factors,E);

    JES_R = 1/R;

    return JES_R;
}

double MCJESInputBase::getEtaCorr(const double X, const double Y) const
{
    int binEta = getEtaBin(Y);
    const double *factors = m_etaCorrFactors[binEta];

    double eta_corr = getLogPolN(factors,X);

    return -eta_corr;
}

double MCJESInputBase::getEmaxJES(const double Y) const
{
	int binEta = getEtaBin(Y);
	double emaxJES = m_energyFreezeJES[binEta];

	return emaxJES;
}

double MCJESInputBase::getLogPolN(const double *factors, double x) const
{
  double y=0;
  for ( uint i=0; i<m_nPar; ++i )
    y += factors[i]*TMath::Power(log(x),Int_t(i));
  return y;
}

std::vector<double> MCJESInputBase::VectorizeD(const TString& str, TString sep) const
{
  std::vector<double> result;
  std::unique_ptr<TObjArray> tokens(str.Tokenize(sep));
  std::unique_ptr<TIter> istr(new TIter(tokens.get()));
  while (TObjString* os = dynamic_cast<TObjString*>(istr->Next())) {
      result.push_back(atof(os->GetString()));
  }  

  return result;
}

int MCJESInputBase::getEtaBin(double eta_det) const 
{
  int bin = std::as_const(m_etaBinAxis)->FindBin(eta_det);
  if (bin<=0) return 0;
  if (bin>m_etaBinAxis->GetNbins()) return bin-2; // overflow
  return bin-1;
}
} // namespace JetHelper
