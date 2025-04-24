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

    const double minstat = dqm_algorithms::tools::GetFirstFromMap( "MinStat", config.getParameters(), 100);
    const double ignoreBelow = dqm_algorithms::tools::GetFirstFromMap( "IgnoreBelow", config.getParameters(), 0);
    const double probThreshold = dqm_algorithms::tools::GetFirstFromMap( "ProbThreshold", config.getParameters(), 0.01);
    const int publishDetail = dqm_algorithms::tools::GetFirstFromMap( "PublishDetail", config.getParameters(), 0x10/*publish status code - since saw some inconsistencies in webdisplay on local testing. Should plan to set to 0 in future*/);
    const int nBinsZ = dqm_algorithms::tools::GetFirstFromMap( "NBinsY", config.getParameters(), 0); // if this is specified, plot is interpreted as being temporal ... this is number of bins in the 'y-axis' direction of each time slice
    const int minDuration = dqm_algorithms::tools::GetFirstFromMap( "MinDuration", config.getParameters(),3); // when in temporal mode, this is the number of consecutive bins in the time axis (x-axis) that an anomaly must exist for to be flagged
    const int liveMode = dqm_algorithms::tools::GetFirstFromMap( "LiveMode", config.getParameters(), 0); // if non-zero, running in live (p1) mode, will influence how results presented

    // use y-axis label to determine convention for temporal plot
    bool reverseConvention = TString(histogram->GetYaxis()->GetTitle()).EndsWith("+y");

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
        result->status_ = dqm_core::Result::Yellow; // will treat almost-empty histograms as a warning .... shifter should think if this is expected
        result->tags_["InsufficientEntries"] = histogram->GetEntries();
        return result;
    }


    std::vector<int> range=dqm_algorithms::tools::GetBinRange(histogram, config.getParameters());

    std::map<int,dqm_core::Result*> resultsByTimeBin;
    std::map<std::string,int> counts;
    dqm_core::Result* lastFilledResult = nullptr;

    for(int t=(nBinsZ>0 ? range[0] : -1) ; t <= (nBinsZ>0 ? range[1] : -1); t++) {
        int xmin = range[0], xmax = range[1];
        int ymin = range[2], ymax = range[3];
        if(t!=-1) {
            // need to adjust ranges using y-axis bins and nBinsZ parameter
            xmin = 1; xmax = histogram->GetNbinsY()/nBinsZ;
            ymin = 1; ymax = nBinsZ;
        }


        std::set<int> filledRows; // will only look for dead strips once all rows are filled

        // compute medians, means, variances, k-test probabilities
        std::vector<double> stripsMedian;
        std::vector<double> stripsAvg;
        std::vector<double> stripsVariance;
        std::vector<size_t> stripsN;
        std::vector<double> stripsProb;
        TRandom3 r;
        for ( int i = xmin; i <= xmax; ++i ) {
            std::vector<double> onestrip;
            double stripSum=0, stripSum2=0;
            for ( int j = ymin; j <= ymax; ++j ) {
                double binvalue = (nBinsZ<=0) ? histogram->GetBinContent(i,j) : histogram->GetBinContent(t,reverseConvention ? ((ymax-ymin+1)*(i-1)+j) :  ((xmax-xmin+1)*(j-1)+i));
                if (binvalue < ignoreBelow) continue;
                if(binvalue>0) filledRows.insert(j); // used to veto running deadstrip tests on sparsely populated plots
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
                for (size_t i = 0; i < onestrip.size(); i++) {
                    // use possion for variances less than 100 (corresponding ~ to averages fewer than 100)
                    // otherwise switch to gaussian
                    double nextVal = 0;
                    do {
                        nextVal = (stripsVariance.back()>=100) ? r.Gaus(stripsAvg.back(), std::sqrt(stripsVariance.back())) : r.Poisson(stripsAvg.back());
                    } while(nextVal<ignoreBelow);
                    stripRef.push_back(nextVal);
                }
                std::sort(stripRef.begin(),stripRef.end());
                stripsProb.push_back(  TMath::KolmogorovTest(onestrip.size(),&onestrip[0],stripRef.size(),&stripRef[0],"")  );
            } else {
                stripsProb.push_back(1);
            }
        }
        if(nBinsZ>=0 && filledRows.empty()) {
            continue; // don't create a result object for empty time slices
        }

        dqm_core::Result* result = new dqm_core::Result();
        std::map<std::pair<int,int>,bin> bins;
        for ( int k = xmin; k <= xmax; ++k ) {
            double strip_median = stripsMedian[k - xmin];
            double strip_variance = stripsVariance[k - xmin];
            for (int l = ymin; l <= ymax; ++l) {
                double binvalue = (nBinsZ<=0) ? histogram->GetBinContent(k,l) : histogram->GetBinContent(t,reverseConvention ? ((ymax-ymin+1)*(k-1)+l) :  ((xmax-xmin+1)*(l-1)+k));
                if (binvalue < ignoreBelow) continue;
                double residual = (strip_variance) ? ((binvalue - strip_median) / std::sqrt(strip_variance)) : 0;
                bins[{k,l}] = {/*histogram->GetXaxis()->GetBinCenter(k), histogram->GetYaxis()->GetBinCenter(l),*/ k, l,
                                                                                                                   binvalue, residual};
            }
        }


        bool testDeadStrips = (filledRows.size() == size_t(ymax-ymin+1));



        // ensure all counts defined, even if will end up being 0
        counts["NDeadStrip"]= (testDeadStrips) ? 0 : -1; // use -1 to flag not running this test
        counts["NDead"]=0;
        counts["NWrongKnown"]=0;
        counts["NConsecUnlikelyStrip"]=0;
        for(auto& [cut,k] : orderedCuts) {
            counts["N"+k] = 0;
        }

        // publish deadstrips (whole strip is 0), and unlikely strips
        int nUnlikelyStrips = 0;
        for(size_t i = 0;i<stripsVariance.size();i++) {
            if (testDeadStrips && stripsVariance.at(i) == 0 && stripsAvg.at(i) == 0) {
                // only dead if at least one of the neighbour strips has enough entries in it
                if( (i>0 && (stripsAvg.at(i-1)*stripsN.at(i-1))>=minstat) || (i<stripsVariance.size()-1 && (stripsAvg.at(i+1)*stripsN.at(i+1))>=minstat)) {
                    result->tags_[TString::Format("_DeadStrip%02ld", i+1).Data()] = histogram->GetXaxis()->GetBinCenter(xmin + i);
                    counts["NDeadStrip"]++;
                }

            }
            if (stripsProb.at(i) < probThreshold) {
                result->tags_[TString::Format("_UnlikelyStrip%02ld", i+1).Data()] = -log(stripsProb.at(i));
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
                    if( (cut < 0 && bin.m_outstandingRatio < cut) || (cut > 0 && bin.m_outstandingRatio > cut && bin.m_value>=minstat) ) {
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

        resultsByTimeBin[t] = result;
        lastFilledResult = result;

    }

    dqm_core::Result* result;
    if(nBinsZ>=0) {
        // ensure all counts defined, even if will end up being 0
        counts["NDeadStrip"]= 0;
        counts["NDead"]=0;
        counts["NWrongKnown"]=0;
        counts["NConsecUnlikelyStrip"]=0;
        for(auto& [cut,k] : orderedCuts) {
            counts["N"+k] = 0;
        }
        dqm_core::Result* lastResult = nullptr;
        result = new dqm_core::Result();
        // must now analyse results by time slice ... require a result in min number of consecutive slices to count an anomaly
        std::map<std::string,int> anomalies;
        for(int t=range[0];t<=range[1]+1;t++) { // go one extra slice to trigger 'empty slice' condition to write active anomalies
            if(resultsByTimeBin.find(t)==resultsByTimeBin.end()) {
                // empty slice .. record all sufficiently large anomalies and reset
                for(auto& [k,v] : anomalies) {
                    if(v>=minDuration) {
                        int lbStart = histogram->GetXaxis()->GetBinLowEdge(t-v);
                        int lbEnd = histogram->GetXaxis()->GetBinLowEdge(t);
                        // in liveMode (for P1 monitoring), don't put the LBs in the result name, so that we get a consistent history plot
                        if(liveMode) {
                            result->tags_[k] = lastResult->tags_[k];
                        } else {
                            result->tags_[k +
                                          TString::Format("_LB%d-%d", lbStart, lbEnd).Data()] = lastResult->tags_[k];
                        }
                        // increment appropriate counter
                        if(k.find("_DeadStrip")==0) {
                            counts["NDeadStrip"]++;
                        } else if(k.find("_Dead")==0) {
                            counts["NDead"]++;
                        } else if(k.find("_UnlikelyStrip")==0) {
                            // not sure how to handle this one
                        } else if(k.find("_Un")==0) {
                            counts["NWrongKnown"]++;
                        }  else {
                            for(auto& [cut,k2] : orderedCuts) {
                                if(k.find("_" + k2)==0) {
                                    counts["N"+k2]++;
                                }
                            }
                        }
                    }
                }
                anomalies.clear();
            } else {
                auto thisResult = resultsByTimeBin[t];
                if(liveMode && thisResult==lastFilledResult) {
                    // don't consider the time slice that is currently being filled, statistics not reliable
                    // can delete this slice because we wont use it
                    delete thisResult;
                    continue;
                }
                // increase counts on all active anomalies ...
                for(auto& [k,v] : thisResult->tags_) {
                    // increment time counter for anomalies
                    if(k.find("_DeadStrip")==0) {
                        anomalies[k]++;
                    } else if(k.find("_Dead")==0) {
                        anomalies[k]++;
                    } else if(k.find("_UnlikelyStrip")==0) {
                        anomalies[k]++;
                    } else if(k.find("_Un")==0) {
                        anomalies[k]++;
                    } else {
                        for(auto& [cut,k2] : orderedCuts) {
                            if(k.find("_" + k2)==0) {
                                anomalies[k]++;
                            }
                        }
                    }
                }

                // reset counts on all disappeared anomalies (recording to results if was long enough)
                // empty slice .. record all sufficiently large anomalies and reset
                for(auto& [k,v] : anomalies) {
                    if(thisResult->tags_.find(k) != thisResult->tags_.end()) continue;
                    if(v>=minDuration) {
                        int lbStart = histogram->GetXaxis()->GetBinLowEdge(t-v);
                        int lbEnd = histogram->GetXaxis()->GetBinLowEdge(t);
                        if(liveMode) {
                            // don't record any result for an anomaly that has disappeared
                        } else {
                            result->tags_[k+TString::Format("_LB%d-%d",lbStart,lbEnd).Data()]=lastResult->tags_[k];
                        }

                        // increment appropriate counter
                        if(k.find("_DeadStrip")==0) {
                            counts["NDeadStrip"]++;
                        } else if(k.find("_Dead")==0) {
                            counts["NDead"]++;
                        } else if(k.find("_UnlikelyStrip")==0) {
                            // not sure how to handle this one
                        } else if(k.find("_Un")==0) {
                            counts["NWrongKnown"]++;
                        } else {
                            for(auto& [cut,k2] : orderedCuts) {
                                if(k.find("_" + k2)==0) {
                                    counts["N"+k2]++;
                                }
                            }
                        }
                    }
                    v=0; // reset counter on the anomaly
                }
                // delete the lastResult if there is one
                if(lastResult) delete lastResult;
                lastResult = thisResult;
            }
        }
        if(lastResult) delete lastResult; // cleans up unneeded results


    } else {
        // not doing a temporal analysis, just use the single result object
        result = resultsByTimeBin[-1];
    }



    // determine algorithm status from provided thresholds

    const auto& redThresholds = config.getRedThresholds();
    const auto& greenThresholds = config.getGreenThresholds();
    result->status_ = dqm_core::Result::Undefined;
    if(publishDetail & 0x20) {
        result->tags_["StatusCode"] = 0;
    }
    for(auto& [k,v] : counts) {
        if(nBinsZ>=0 && k=="NConsecUnlikelyStrip") continue; // not currently counting consecutive unlikely strips in temporal mode
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
  out<<"Special results are: NDead (number of 0 bins below most negative cut), NDeadStrip (strips that are all 0 .. will be -1 if not got an entry in every row of plot), NConsecUnlikelyStrip (most consecutive strips that are below ProbThreshold)" << std::endl;
  out<<"Thresholds can be set on any of the results\n"<<std::endl;
  
  out<<"Optional Parameter: MinStat: Minimum histogram statistics needed to perform Algorithm, also min entries for warm/hot spots (cuts > 0), and min entries in neighbour strip to declare a strip dead"<<std::endl;
  out<<"Optional Parameter: IgnoreBelow: values below which the bins wont be considered (default 0)"<<std::endl;
  out<<"Optional Parameter: ProbThreshold: cutoff for strip k-test probabilities for strip to be considered unlikely (default 0.05)"<<std::endl;
  out<<"Optional Parameter: PublishDetail: Bitmask of what extra info to publish about strips. Starting with MSB: AlgStatusCode,Zeros,Noise,Prob,StdDev,Median (default 000000)"<<std::endl;
  
}

