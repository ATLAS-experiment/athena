/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/* L1Calo_BinsDiffFromStripMedian.cxx is to pick out the problematic bins in 2D histogram assuming that y-axis(the phi direction) be symmetric.
   Originally Based on the BinsDiffFromStripMedian dqm algorithm.
   Author: Will Buttinger
   Email:  will@cern.ch
*/

#include <dqm_core/AlgorithmConfig.h>
#include <dqm_algorithms/L1Calo_BinsDiffFromStripMedian.h>
#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <dqm_core/AlgorithmManager.h>

#include <TH1.h>
#include <TF1.h>
#include <TClass.h>
#include <TRandom3.h>
#include <cmath>

#include <iostream>
#include <string>
#include <set>


bool mySortfunc_ratio(const dqm_algorithms::L1Calo_BinsDiffFromStripMedian::bin& i,
                      const dqm_algorithms::L1Calo_BinsDiffFromStripMedian::bin& j){return (std::abs(i.m_outstandingRatio) > std::abs(j.m_outstandingRatio));}
static dqm_algorithms::L1Calo_BinsDiffFromStripMedian myInstance;

dqm_algorithms::L1Calo_BinsDiffFromStripMedian::L1Calo_BinsDiffFromStripMedian( )
{
  dqm_core::AlgorithmManager::instance().registerAlgorithm("L1Calo_BinsDiffFromStripMedian", this);
}

dqm_algorithms::L1Calo_BinsDiffFromStripMedian::~L1Calo_BinsDiffFromStripMedian()
{
}

dqm_algorithms::L1Calo_BinsDiffFromStripMedian * 
dqm_algorithms::L1Calo_BinsDiffFromStripMedian::clone()
{
  
  return new L1Calo_BinsDiffFromStripMedian();
}


dqm_core::Result *
dqm_algorithms::L1Calo_BinsDiffFromStripMedian::execute(const std::string &  name, 
					   const TObject& object, 
					   const dqm_core::AlgorithmConfig& config ) {
    const TH1* histogram;

    if( object.IsA()->InheritsFrom( "TH1" ) ) {
        histogram = static_cast<const TH1*>(&object);
        if (histogram->GetDimension() > 2 ){
            throw dqm_core::BadConfig( ERS_HERE, name, "dimension > 2 " );
        }
    } else {
        throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH1" );
    }

    const double minstat = dqm_algorithms::tools::GetFirstFromMap( "MinStat", config.getParameters(), -1);
    const double ignoreBelow = dqm_algorithms::tools::GetFirstFromMap( "IgnoreBelow", config.getParameters(), 0);
    const double probThreshold = dqm_algorithms::tools::GetFirstFromMap( "ProbThreshold", config.getParameters(), 0.01);
    const int publishDetail = dqm_algorithms::tools::GetFirstFromMap( "PublishDetail", config.getParameters(), 0x10/*publish status code - since saw some inconsistencies in webdisplay on local testing. Should plan to set to 0 in future*/);

    std::map<std::string,std::set<std::pair<int,int>>> knownBins; // bins which are known to be a particular class

    auto knownBinParser = [&](const std::string& cutName) {
        std::string known = dqm_algorithms::tools::GetFirstFromMap("Known"+cutName, config.getGenericParameters(), "");
        // strip any non-numeric chars from the front
        size_t i = 0;
        while (i < known.length() && !std::isdigit(known[i])) {
            i++;
        }
        known = known.substr(i);
        known += ";"; // add final semicolon
        i = known.find(",");
        while(i != std::string::npos) {
            size_t j = known.find(";");
            knownBins[cutName].insert({TString(known.substr(0,i)).Atoi(),TString(known.substr(i+1,j-i-1)).Atoi()});
            known = known.substr(j+1);
            i = known.find(",");
        }
    };

    std::vector<std::pair<double,std::string>> orderedCuts;
    double mostNegativeCut = 0;
    for(auto& [k,v] : config.getParameters()) {
        TString kk(k);
        if(!kk.EndsWith("Cut")) continue;
        kk = kk(0,kk.Length()-3);
        orderedCuts.push_back({v,kk.Data()});
        mostNegativeCut = std::min(mostNegativeCut,v);
        knownBinParser(kk.Data());
    }
    knownBinParser("Dead"); // also parse for any known dead spots

    // order cuts by magnitude of cut, biggest first
    std::sort(orderedCuts.begin(),orderedCuts.end(),[](const auto& v1, const auto& v2) { return std::abs(v1.first) > std::abs(v2.first); });

    if ( histogram->GetEntries() < minstat ) {
        dqm_core::Result *result = new dqm_core::Result(dqm_core::Result::Undefined);
        result->tags_["InsufficientEntries"] = histogram->GetEntries();
        return result;
    }


    std::vector<int> range=dqm_algorithms::tools::GetBinRange(histogram, config.getParameters());

    // compute medians, means, variances, k-test probabilities
    std::vector<double> stripsMedian;
    std::vector<double> stripsAvg;
    std::vector<double> stripsVariance;
    std::vector<size_t> stripsN;
    std::vector<double> stripsProb;
    TRandom3 r;
    for ( int i = range[0]; i <= range[1]; ++i ) {
        std::vector<double> onestrip;
        double stripSum=0, stripSum2=0;
        for ( int j = range[2]; j <= range[3]; ++j ) {
            if (histogram->GetBinContent(i,j) < ignoreBelow) continue;
            float binvalue = histogram->GetBinContent(i,j);
            onestrip.push_back(binvalue);
            stripSum += binvalue;
            stripSum2 += binvalue*binvalue;
        }
        stripsAvg.push_back(stripSum/onestrip.size());
        // traditional variance calculation, not robust to outliers
        // leaving this commented for reference
        //stripsVariance.push_back( stripSum2/onestrip.size() - std::pow(stripsAvg.back(),2) );

        std::sort(onestrip.begin(),onestrip.end());

        stripsMedian.push_back( onestrip.at(onestrip.size()/2) );
        // estimate variance as square of half of the middle ~68% - more robust against outliers than calculating from sumw2
        stripsVariance.push_back( std::pow((onestrip.at(onestrip.size()*0.84) - onestrip.at(onestrip.size()*0.16))/2.,2) );
        stripsN.push_back(onestrip.size());
        // also compute Kolmogorov test probability vs a same-size dataset generated from a gaussian with the strip mean and variance
        if(stripsVariance.back() > 0) {
            std::vector<double> stripRef;
            for (size_t i = 0; i < onestrip.size(); i++)
                stripRef.push_back(r.Gaus(stripsAvg.back(), std::sqrt(stripsVariance.back())));
            std::sort(stripRef.begin(),stripRef.end());
            stripsProb.push_back(  TMath::KolmogorovTest(onestrip.size(),&onestrip[0],stripRef.size(),&stripRef[0],"")  );
        } else {
            stripsProb.push_back(1);
        }
    }

    dqm_core::Result* result = new dqm_core::Result();
    std::map<std::pair<int,int>,bin> bins;
    for ( int k = range[0]; k <= range[1]; ++k ) {
        double strip_median = stripsMedian[k - range[0]];
        double strip_variance = stripsVariance[k - range[0]];
        for (int l = range[2]; l <= range[3]; ++l) {
            double binvalue = histogram->GetBinContent(k, l);
            if (binvalue < ignoreBelow) continue;
            double residual = (strip_variance) ? ((binvalue - strip_median) / std::sqrt(strip_variance)) : 0;
            bins[{k,l}] = {histogram->GetXaxis()->GetBinCenter(k), histogram->GetYaxis()->GetBinCenter(l), k, l,
                          binvalue, residual};
        }
    }



    std::map<std::string,int> counts;
    // ensure all counts defined, even if will end up being 0
    counts["NDeadStrip"]=0;
    counts["NDead"]=0;
    counts["NWrongKnown"]=0;
    counts["NConsecUnlikelyStrip"]=0;
    for(auto& [cut,k] : orderedCuts) {
        counts["N"+k] = 0;
    }

    // publish deadstrips (whole strip is 0), and unlikely strips
    int nUnlikelyStrips = 0;
    for(size_t i = 0;i<stripsVariance.size();i++) {
        if (stripsN.at(i) > 0 && stripsVariance.at(i) == 0 && stripsAvg.at(i) == 0) {
            result->tags_[TString::Format("_DeadStrip%02ld", i+1).Data()] = histogram->GetXaxis()->GetBinCenter(range[0] + i);
            counts["NDeadStrip"]++;
        }
        if (stripsProb.at(i) < probThreshold) {
            result->tags_[TString::Format("_UnlikelyStrip%02ld", i+1).Data()] = stripsProb.at(i);
            nUnlikelyStrips++;
            if(nUnlikelyStrips > counts["NConsecUnlikelyStrip"]) counts["NConsecUnlikelyStrip"] = nUnlikelyStrips;
        } else {
            nUnlikelyStrips=0; // reset counter
        }
        if(publishDetail & 0x1) {
            result->tags_[TString::Format("_Median%02ld", i+1).Data()] = stripsMedian.at(i);
        }
        if(publishDetail & 0x2) {
            result->tags_[TString::Format("_StdDev%02ld", i+1).Data()] = sqrt(stripsVariance.at(i));
        }
        if(publishDetail & 0x4) {
            result->tags_[TString::Format("_Prob%02ld", i+1).Data()] = stripsProb.at(i);
        }
        if(publishDetail & 0x8) {
            // attempt to estimate residual noise, by subtracting off the statistical variance (which equals the average, i.e. poissonian)
            result->tags_[TString::Format("_Noise%02ld", i+1).Data()] = sqrt(std::abs(stripsVariance.at(i) - stripsMedian.at(i)));
        }
    }

    // publish deadspots (anomalous 0s) and other anomalies defined by the cuts

    for(auto& [pos,bin] : bins) {
        if(bin.m_value==0 && bin.m_outstandingRatio < mostNegativeCut) {
            // publish if spot is not known
            if(knownBins["Dead"].find({bin.m_ix,bin.m_iy})==knownBins["Dead"].end()) {
                result->tags_[TString::Format("_Dead(%d,%d)", bin.m_ix, bin.m_iy).Data()] = bin.m_outstandingRatio;
                counts["NDead"]++;
            }
        } else {
            if( (publishDetail & 0x10) && bin.m_value==0) {
                result->tags_[TString::Format("_Zero(%d,%d)",bin.m_ix,bin.m_iy).Data()] = bin.m_outstandingRatio;
            }
            // loop through cuts, assign bin to one of the ranges, and report if not a known bin
            double classCut = 0;
            for(auto& [cut,k] : orderedCuts) {
                if( (cut < 0 && bin.m_outstandingRatio < cut) || (cut > 0 && bin.m_outstandingRatio > cut) ) {
                    classCut = cut;
                    if(knownBins[k].find({bin.m_ix,bin.m_iy})==knownBins[k].end()) {
                        result->tags_[TString::Format("_%s(%d,%d)", k.c_str(), bin.m_ix,
                                                      bin.m_iy).Data()] = bin.m_outstandingRatio;
                        counts["N"+k]++;
                    }
                    break;
                }
            }
            // if this is a known bin in a given cut range, check if we have any evidence it is wrong
            // start with known dead ... if this bin has an entry, its not dead
            if(bin.m_value>0 && knownBins["Dead"].find({bin.m_ix,bin.m_iy})!=knownBins["Dead"].end()) {
                counts["NWrongKnown"]++;
                result->tags_[TString::Format("_UnDead(%d,%d)", bin.m_ix,
                                              bin.m_iy).Data()] = bin.m_outstandingRatio;
            } else if(classCut != 0) {
                // if class cut is in opposite direction to known bin list, report that too
                for(auto& [cut,k] : orderedCuts) {
                    if(knownBins[k].find({bin.m_ix,bin.m_iy})==knownBins[k].end()) continue;
                    if(cut*classCut < 0) {
                        counts["NWrongKnown"]++;
                        result->tags_[TString::Format("_Un%s(%d,%d)",k.c_str(), bin.m_ix,
                                                      bin.m_iy).Data()] = bin.m_outstandingRatio;
                    }
                }
            }
        }
    }


    // determine algorithm status from provided thresholds

    const auto& redThresholds = config.getRedThresholds();
    const auto& greenThresholds = config.getGreenThresholds();
    result->status_ = dqm_core::Result::Undefined;
    if(publishDetail & 0x20) {
        result->tags_["StatusCode"] = 0;
    }
    for(auto& [k,v] : counts) {
        result->tags_[k] = v;
        if(v>dqm_algorithms::tools::GetFirstFromMap(k, redThresholds, std::numeric_limits<double>::max())||result->status_ == dqm_core::Result::Red) {
            result->status_ = dqm_core::Result::Red;
            if(publishDetail & 0x20) {
                result->tags_["StatusCode"] = 3;
            }
        } else if(v>dqm_algorithms::tools::GetFirstFromMap(k, greenThresholds, std::numeric_limits<double>::max())) {
            result->status_ = dqm_core::Result::Yellow;
            if(publishDetail & 0x20) {
                result->tags_["StatusCode"] = 2;
            }
        } else if(result->status_==dqm_core::Result::Undefined && greenThresholds.find(k)!=greenThresholds.end()) {
            result->status_ = dqm_core::Result::Green;
            if(publishDetail & 0x20) {
                result->tags_["StatusCode"] = 1;
            }
        }
    }


    return result;
  
}



void dqm_algorithms::L1Calo_BinsDiffFromStripMedian::printDescription(std::ostream& out) {
  
  out<<"L1Calo_BinsDiffFromStripMedian: Calculates strip median and then find out bins which are aliens "<<std::endl;
  out<<"Specify cuts with parameters named <cutName>Cut will generate a result of form Nxxx" << std::endl;
  out<<"Specify known anomalies with Known<cutName> string argument - note any leading non-numeric char will be stripped" << std::endl;
  out<<"Special results are: NDead (number of 0 bins below most negative cut), NDeadStrip (strips that are all 0), NConsecUnlikelyStrip (most consecutive strips that are below ProbThreshold)" << std::endl;
  out<<"Thresholds can be set on any of the results\n"<<std::endl;
  
  out<<"Optional Parameter: MinStat: Minimum histogram statistics needed to perform Algorithm"<<std::endl;
  out<<"Optional Parameter: IgnoreBelow: values below which the bins wont be considered (default 0)"<<std::endl;
  out<<"Optional Parameter: ProbThreshold: cutoff for strip k-test probabilities for strip to be considered unlikely (default 0.05)"<<std::endl;
  out<<"Optional Parameter: PublishDetail: Bitmask of what extra info to publish about strips. Starting with MSB: AlgStatusCode,Zeros,Noise,Prob,StdDev,Median (default 000000)"<<std::endl;
  
}

