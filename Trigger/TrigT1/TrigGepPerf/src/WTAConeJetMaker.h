/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_WTACONEJETMAKER_H
#define TRIGGEPPERF_WTACONEJETMAKER_H

#include "IJetMaker.h"


#include "WTAConeMaker.h" // WTAConeMaker is the core header
#include "WTACone2PassMaker.h" // WTACone2PassMaker is the 2-Pass header

#include <string>
#include <vector>
#include <memory>

namespace Gep{
  class Jet;
  class Cluster;
}

 enum WTAConeMakerEnum{ // use WTAConeMakerEnum for algorithm variants
  Baseline = 0,
  TwoPass = 1
};

 
 namespace Gep
 {
   class WTAConeJetMaker : public IJetMaker
   {
   public:
 
   WTAConeJetMaker(unsigned int block_n = 4, unsigned int rolloff_buffersize = 155) :
           m_GEPWTAParameters(),
           m_BlockN(block_n), m_SeedCleaningAlgo(0), m_RollOffBufferSize(rolloff_buffersize)
           {};
 
     
     virtual std::string toString() const override{ return "WTAConeJet"; }
     virtual std::vector<Gep::Jet> makeJets(const std::vector<Gep::Cluster>& TopoTowers) const override; // To makeJet, m_seeds and m_consts must not be empty vectors

     std::unique_ptr<WTAConeMaker> CreateWTAConeMaker(enum WTAConeMakerEnum seed_cleaning_algo) const
     { // Allow user to choose the seed cleaning algorithm
        switch(seed_cleaning_algo)
        {
            case Baseline:
                return std::make_unique<WTAConeMaker>();
            case TwoPass:
                return std::make_unique<WTACone2PassMaker>();
            default:
                return nullptr;
        }
    }

     void SetBlockN(unsigned int block_n){m_BlockN = block_n; /* m_WTAParallelHelper.SetBlockN(m_BlockN);*/ };
     void SetSeedCleaningAlgo(unsigned int algo){m_SeedCleaningAlgo = algo;};
     void SetRollOffBufferSize(int rolloff_buffersize){m_RollOffBufferSize = rolloff_buffersize;}
     int GetRollOffBufferSize(){return m_RollOffBufferSize;}
 
     WTAParameters m_GEPWTAParameters;
 
   private:
    //  WTAConeParallelHelper m_WTAParallelHelper;
     unsigned int m_BlockN{};
     unsigned int m_SeedCleaningAlgo{};
     unsigned int m_RollOffBufferSize{}; // Only for TwoPass
 
   };
 
 }
 
 #endif //TRIGL0GEPPERF_EXCONEJETMAKER_H
 