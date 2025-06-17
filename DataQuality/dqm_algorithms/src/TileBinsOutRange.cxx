/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <dqm_core/AlgorithmConfig.h>
#include <dqm_algorithms/TileBinsOutRange.h>
#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <TH1.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TClass.h>
#include <ers/ers.h>

#include <dqm_core/AlgorithmManager.h>

namespace {
  dqm_algorithms::TileBinsOutRange myInstance;
}

namespace  dqm_algorithms {

TileBinsOutRange::TileBinsOutRange(): m_name("TileBinsOutRange") {
  dqm_core::AlgorithmManager::instance().registerAlgorithm(m_name, this);
}


TileBinsOutRange* TileBinsOutRange::clone() {
  return new TileBinsOutRange();
}


dqm_core::Result* TileBinsOutRange::execute(const std::string& name,
                                                  const TObject& object,
                                                  const dqm_core::AlgorithmConfig& config) {

  const TH1* histogram = nullptr;

  if(object.IsA()->InheritsFrom( "TH1" )) {
    histogram = static_cast<const TH1*>(&object);//type already checked in preceding line
    if (histogram->GetDimension() > 2 ){
      throw dqm_core::BadConfig( ERS_HERE, name, "dimension > 2 " );
    }
  } else {
    throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH2");
  }

  const bool publish = static_cast<bool>(dqm_algorithms::tools::GetFirstFromMap( "PublishBins", config.getParameters(), 0));
  const bool publishHistogram = static_cast<bool>(dqm_algorithms::tools::GetFirstFromMap( "PublishHistogram", config.getParameters(), 1));
  const int maxPublish = static_cast<int>(dqm_algorithms::tools::GetFirstFromMap( "MaxPublish", config.getParameters(), 20));
  const double minStat = dqm_algorithms::tools::GetFirstFromMap( "MinStat", config.getParameters(), -1);
  const double ignoreValue = dqm_algorithms::tools::GetFirstFromMap( "IgnoreValue", config.getParameters(), -99999);

  const TProfile* profile(nullptr);
  const TProfile2D* profile2D(nullptr);
  const double minBinEntries = dqm_algorithms::tools::GetFirstFromMap( "MinBinEntries", config.getParameters(), -1);
  if(minBinEntries > 0) {
   if (object.InheritsFrom("TProfile"))        profile   = dynamic_cast<const TProfile*>(&object);
   else if (object.InheritsFrom("TProfile2D")) profile2D = dynamic_cast<const TProfile2D*>(&object);
  }

  if (histogram->GetEntries() < minStat ) {
    dqm_core::Result *result = new dqm_core::Result(dqm_core::Result::Undefined);
    result->tags_["InsufficientEntries"] = histogram->GetEntries();
    return result;
  }

  double minValue;
  double maxValue;
  double redThreshold;
  double greenThreshold;
  try {
    minValue = dqm_algorithms::tools::GetFromMap( "MinValue", config.getGreenThresholds() );
    maxValue = dqm_algorithms::tools::GetFromMap( "MaxValue", config.getGreenThresholds() );
    greenThreshold = dqm_algorithms::tools::GetFromMap("NBins", config.getGreenThresholds());
    redThreshold = dqm_algorithms::tools::GetFromMap("NBins", config.getRedThresholds());
  } catch( dqm_core::Exception & ex ) {
    throw dqm_core::BadConfig( ERS_HERE, name, ex.what(), ex );
  }

  TH1* resultHistogram = nullptr;
  if (publishHistogram) {
    if (histogram->InheritsFrom("TH1")) {
      resultHistogram = static_cast<TH1*>(histogram->Clone());//type already checked
    } else {
      throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH1" );
    }
    resultHistogram->Reset();
  }

  int nBins = 0;
  int nSkippedBins = 0;
  dqm_core::Result* result = new dqm_core::Result();
  std::vector<int> range = dqm_algorithms::tools::GetBinRange(histogram, config.getParameters());
  for (int i = range[0]; i <= range[1]; ++i) {
    for (int j = range[2]; j <= range[3]; ++j) {

      if (minBinEntries > 0) {
        int bin = histogram->GetBin(i, j);
        if (profile) {
          if (profile->GetBinEntries(bin) < minBinEntries) {
            ++nSkippedBins;
            continue;
          }
        } else if (profile2D) {
          if (profile2D->GetBinEntries(bin) < minBinEntries) {
            ++nSkippedBins;
            continue;
          }
        }
      }

      double binValue = histogram->GetBinContent(i, j);
      if((binValue == ignoreValue) || (binValue > minValue && binValue < maxValue )) continue;

      ++nBins;
      if (resultHistogram){
        resultHistogram->SetBinContent(i, binValue);
        if (publish && nBins < maxPublish) {
          dqm_algorithms::tools::PublishBin(histogram, i, 0, binValue, result);
        }
      }
    }
  }

  ERS_DEBUG(1,"Number of bad bins is " << nBins );
  ERS_DEBUG(1,"Green threshold: " << greenThreshold << " bin(s);   Red threshold : " << redThreshold << " bin(s) ");

  result->tags_["NBins"] = nBins;
  result->tags_["NSkippedBins"] = nSkippedBins;
  if (resultHistogram) result->object_ = boost::shared_ptr<TObject>(resultHistogram);

  if (greenThreshold > redThreshold) {
    if (nBins >= greenThreshold) {
      result->status_ = dqm_core::Result::Green;
    } else if (nBins > redThreshold) {
      result->status_ = dqm_core::Result::Yellow;
    } else {
      result->status_ = dqm_core::Result::Red;
    }
  } else {
    if (nBins <= greenThreshold) {
      result->status_ = dqm_core::Result::Green;
    } else if (nBins < redThreshold) {
      result->status_ = dqm_core::Result::Yellow;
    } else {
      result->status_ = dqm_core::Result::Red;
    }
  }

  return result;
}

void TileBinsOutRange::printDescription(std::ostream& out) {

  out << m_name << ": Check number of bins which are out of range (MinValue, MaxValue) " << std::endl;
  out << "Mandatory Green Threshold: MinValue: minimum value of range" << std::endl;
  out << "Mandatory Green Threshold: MaxValue: maximum value of range" << std::endl;
  out << "Mandatory Green/Red Threshold: NBins: Number of non-empty bins to give Green/Red result\n" << std::endl;
  out << "Optional Parameter: MinStat: Minimum histogram statistics needed to perform Algorithm" << std::endl;
  out << "Optional Parameter: IgnoreValue: valued to be ignored for being processed" << std::endl;
  out << "Optional Parameter: MaxPublish: Max number of bins to save (default 20)" << std::endl;
  out << "Optional Parameter: MinBinEntries: Minimum bin entries in profile histogram needed to check this bin (by default: -1)" << std::endl;

}

}
