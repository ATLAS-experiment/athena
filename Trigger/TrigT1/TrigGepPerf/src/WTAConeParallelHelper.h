/*
*   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef WTAConeParallelHelper_h
#define WTAConeParallelHelper_h



#include "./WTAObject.h" // Use the WTAObject
#include "./WTAConeMaker.h" // The base class
#include <vector>
#include <memory>

#ifdef BITWISE_SIMULATION
// 1: Integer type for bitwise simulation
const IntOrFloat ETA_MIN = 0;
const IntOrFloat ETA_MAX = 99;
const IntOrFloat ETA_LEN = 100; // 100 x 64 grid
const IntOrFloat PHI_MIN = 0;
const IntOrFloat PHI_MAX = 63;
const IntOrFloat PHI_LEN = 64;
const IntOrFloat CORE_DIST = 8;
#elif defined(FLOATING_POINT_SIMULATION)
// 2: Floating point for athena simulation
const IntOrFloat ETA_MIN = -2.5;
const IntOrFloat ETA_MAX = 2.5;
const IntOrFloat ETA_LEN = 5.0;
const IntOrFloat PHI_MIN = -PI;
const IntOrFloat PHI_MAX = PI;
const IntOrFloat PHI_LEN = 2 * PI;
const IntOrFloat CORE_DIST = 0.8;
#else
#error "Simulation type not defined. Define either BITWISE_SIMULATION or FLOATING_POINT_SIMULATION."
#endif

class WTAConeParallelHelper{
    public:
        WTAConeParallelHelper(unsigned int block_n = 1):
          m_BlockN(block_n), m_DivideByEta(false)
        {SetBlockN(m_BlockN);}; // Constructor
        ~WTAConeParallelHelper () {}; // Destructor
        
        void SetBlockN(unsigned int block_n); // Call SetBlockN in the constructor
        void SetDivideByEta(bool divide_by_eta){m_DivideByEta = divide_by_eta;}
        IntOrFloat PhiWrap(IntOrFloat phi);
        bool CheckInsideRegion(const WTATrigObj& tower, IntOrFloat min, IntOrFloat max);
        void CreateBlocks(const std::vector<WTATrigObj>& all_towers);
        unsigned int GetBlockN(){return m_BlockN;};
        template<typename WTAClassType>
        void RunParallelWTA(std::unique_ptr<WTAClassType>& AnyWTAClass); // Run this N times, without argument
        void CheckJetInCore();
        std::vector<WTAJet> GetAllJets();

    private:
        unsigned int m_BlockN;
        bool m_DivideByEta;
        std::vector<std::vector<WTATrigObj>> m_InputTowersPerBlock;
        std::vector<std::vector<WTAJet>> m_OutputJetsPerBlock;

};

inline void WTAConeParallelHelper::SetBlockN(unsigned int block_n)
{
    m_BlockN = block_n;
    m_InputTowersPerBlock.clear(); m_OutputJetsPerBlock.clear();
    for(unsigned int i = 0; i < m_BlockN; i++)
    {
        m_InputTowersPerBlock.emplace_back();
        m_OutputJetsPerBlock.emplace_back();
    }
}

inline IntOrFloat WTAConeParallelHelper::PhiWrap(IntOrFloat phi)
{
    while(phi >= PHI_MAX)phi -= 2*PI;
    while(phi < PHI_MIN)phi += 2*PI;
    return phi;
}


inline bool WTAConeParallelHelper::CheckInsideRegion(const WTATrigObj& tower, IntOrFloat min, IntOrFloat max)
{
    bool inside = false;
    if(m_DivideByEta)
    {
        IntOrFloat this_eta = tower.eta();
        inside = (this_eta >= min) && (this_eta < max);
    }
    else
    { // Watchout for the PhiWrapping
        IntOrFloat this_phi = PhiWrap(tower.phi());
        IntOrFloat min_wrap = PhiWrap(min);
        IntOrFloat max_wrap = PhiWrap(max);
        if(min_wrap <= max_wrap){
          inside = (this_phi >= min_wrap) && (this_phi < max_wrap);
        }
        else{ // E.g) Block0 has min = -PI - 0.8, max = PI + 0.8
          inside = !((this_phi >= max_wrap) && (this_phi < min_wrap));
        }
    }
    return inside;
}

inline void WTAConeParallelHelper::CreateBlocks(const std::vector<WTATrigObj>& all_towers)
{
    IntOrFloat BlockLow = -99;
    IntOrFloat BlockHigh = -99;
    const unsigned tower_n = all_towers.size();
    for(unsigned int i = 0; i < m_BlockN; i++)
    {
        if(m_DivideByEta)
        {
            BlockLow = ETA_MIN - CORE_DIST + i * (float)(ETA_LEN / m_BlockN);
            BlockHigh = ETA_MAX + CORE_DIST - (m_BlockN - 1 - i) * (float)(ETA_LEN / m_BlockN);
        }
        else
        {
            BlockLow = PHI_MIN - CORE_DIST + i * (float)(PHI_LEN / m_BlockN);
            BlockHigh = PHI_MAX + CORE_DIST - (m_BlockN - 1 - i) * (float)(PHI_LEN / m_BlockN);
        }
        for(unsigned int t = 0; t < tower_n; t++)
        {
            if(CheckInsideRegion(all_towers.at(t), BlockLow, BlockHigh))
            {
                m_InputTowersPerBlock.at(i).push_back(all_towers.at(t));
            }
        }
    }
}



template<typename WTAClassType>
inline void WTAConeParallelHelper::RunParallelWTA(std::unique_ptr<WTAClassType>& MyWTAMakerClass)
{
    for(unsigned int i = 0; i < m_BlockN; i++)
    {
        MyWTAMakerClass->InitiateInputs(m_InputTowersPerBlock.at(i));
        MyWTAMakerClass->SeedCleaning();
        MyWTAMakerClass->MergeConstsToSeeds();
        m_OutputJetsPerBlock.at(i) = MyWTAMakerClass->GetSeedList(); // Define the ith vector, not push_back
    }
}

inline void WTAConeParallelHelper::CheckJetInCore() // DON'T CALL THIS FOR SINGLE REGION!!
{
    IntOrFloat BlockLow = -99;
    IntOrFloat BlockHigh = -99;
    std::vector<WTAJet> all_jets;
    for(unsigned int i = 0; i < m_BlockN; i++)
    {
        if(m_DivideByEta)
        {
            BlockLow = ETA_MIN + i * (float)(ETA_LEN / m_BlockN);
            BlockHigh = ETA_MAX - (m_BlockN - 1 - i) * (float)(ETA_LEN / m_BlockN);
        }
        else
        { // Remember, Cores should be equivalent with linspace(4, PHI_MIN, PHI_MAX)
            BlockLow = PHI_MIN + i * (float)(PHI_LEN / m_BlockN);
            BlockHigh = PHI_MAX - (m_BlockN - 1 - i) * (float)(PHI_LEN / m_BlockN);
        }
        unsigned int this_block_jet_n = m_OutputJetsPerBlock.at(i).size();
        if(this_block_jet_n) // Only run if there are jets in the ith ROI
        {
            for(int j = this_block_jet_n - 1; j >= 0; j--) // Because we are erasing, read from right to left
            {
                WTAJet jet = m_OutputJetsPerBlock.at(i).at(j);
                if(!CheckInsideRegion(jet, BlockLow, BlockHigh))m_OutputJetsPerBlock.at(i).erase(m_OutputJetsPerBlock.at(i).begin() + j);
            }
        }
    }
}

inline std::vector<WTAJet> WTAConeParallelHelper::GetAllJets()
{
    std::vector<WTAJet> all_jets;
    for(unsigned int i = 0; i < m_BlockN; i++)
    {
        all_jets.insert(all_jets.end(), m_OutputJetsPerBlock.at(i).begin(), m_OutputJetsPerBlock.at(i).end());
    }
    SortByPt(all_jets);
    return all_jets;
}

#endif