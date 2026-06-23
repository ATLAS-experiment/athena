/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ANALYSISTRUTHTAGRESULTS_H
#define ANALYSISTRUTHTAGRESULTS_H

#include <map>
#include <vector>
#include <string>
#include <string_view>
#include <stdexcept>

namespace Analysis {

    class TruthTagResults {

    public:

    //map from systematic name to vector of event weights for different number of tagged jets
    // for examples: map_trf_weight_ex["Nominal"].at(3) is the event weight for exactly 3 tagged jets
    // map_trf_weight_in["Nominal"].at(2) is the event weight for 2 or more tagged jets
    std::map<std::string,std::vector<float>, std::less<> > map_trf_weight_ex;
    std::map<std::string,std::vector<float>, std::less<> > map_trf_weight_in;

    //map from systematic name to vector of SF weights for different number of tagged jets
    // for examples: map_SF_ex["Nominal"].at(3) is the SF event weight for exactly 3 tagged jets
    // map_SF_ex["Nominal"].at(2) is the SF event weight for 2 or more tagged jets
    std::map<std::string,std::vector<float>, std::less<> > map_SF_ex;
    std::map<std::string,std::vector<float>, std::less<> > map_SF_in;

    //chosen permutation (does not depend on the systematic variation)
    // trf_chosen_perm_ex.at(3) is the chosen permutation for exactly 3 tagged jets
    std::vector<std::vector<bool> > trf_chosen_perm_ex;
    std::vector<std::vector<bool> > trf_chosen_perm_in;

    //the tag weight bin, similar configuration to the chosen permutation vectors.
    std::vector<std::vector<int> > trf_bin_ex;
    std::vector<std::vector<int> > trf_bin_in;

    //random tag weights generated for the chosen quantiles.
    std::vector<std::vector<float> > trf_bin_score_ex;
    std::vector<std::vector<float> > trf_bin_score_in;

    //random c-tag weights generated for the chosen quantiles.
    std::vector<std::vector<float> > trf_ctag_bin_score_ex;
    std::vector<std::vector<float> > trf_ctag_bin_score_in;

    //direct tagging results
    std::vector<bool> is_tagged;
    std::map<std::string,float, std::less<> > map_SF;

    std::vector<std::string> syst_names;

    void clear(){

        map_trf_weight_ex.clear();
        map_trf_weight_in.clear();
        trf_chosen_perm_ex.clear();
        trf_chosen_perm_in.clear();
        trf_bin_ex.clear();
        trf_bin_in.clear();

    }

    float
    getEvtDirectTagSF(std::string_view syst_name = "Nominal") const {
      const auto itr = map_SF.find(syst_name);
      if (itr == map_SF.end()) {
        throw std::out_of_range{"Unknown systematic name"};
      }
      return itr->second;
    }

    const std::vector<bool>& getDirectTaggedJets()
    {
        return(is_tagged);

    }

    float
    getEventWeight(int nbtag, bool Ex, std::string_view syst_name = "Nominal") const {
      const auto& weights = Ex ? map_trf_weight_ex : map_trf_weight_in;
    
      const auto itr = weights.find(syst_name);
      if (itr == weights.end()) {
        throw std::out_of_range{"Unknown systematic name"};
      }
    
      return itr->second.at(nbtag);
    }

    std::vector<bool> getEventPermutation(int nbtag,bool Ex )
    {
        if(Ex)
        {
            return(trf_chosen_perm_ex.at(nbtag));
        }

        else
        {
            return(trf_chosen_perm_in.at(nbtag));
        }
    }

    std::vector<int> getEventQuantiles(int nbtag,bool Ex )
    {
        if(Ex)
        {
            return(trf_bin_ex.at(nbtag));
        }

        else
        {
            return(trf_bin_in.at(nbtag));
        }
    }


    std::vector<float> getRandomTaggerScores(int nbtag,bool Ex )
    {
        if(Ex)
        {
            return(trf_bin_score_ex.at(nbtag));
        }
        else
        {
            return(trf_bin_score_in.at(nbtag));
        }
    }


    };


}

#endif // ANALYSISTRUTHTAGRESULTS_H

