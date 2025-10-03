#ifndef WTACone2PassMaker_h
#define WTACone2PassMaker_h

#include <iostream>
#include <algorithm>
#include <map>
#include "./WTAConeMaker.h" // Use the parent class

class WTACone2PassMaker : public WTAConeMaker{       // The 2Pass maker class
    public:
        WTACone2PassMaker(unsigned int RollOffBufferSize = 155)
        : WTAConeMaker(), // Calls WTAConeMaker constructor
          m_RollOffBufferSize(RollOffBufferSize) // Initialize with infinite RollOffBufferSize
        {}; // Constructor
        ~WTACone2PassMaker () {}; // Destructor

        void FillLists(const std::vector<WTATrigObj>& InputTowers) override; // Overwrite the WTAConeMaker::FillLists()
        void SeedCleaning() override; // Overwrite the WTAConeMaker::SeedCleaning()
        void MergeConstsToSeeds() override; // Overwrite the WTAConeMaker::MergeConstsToSeeds()
        void SetRollOffBufferSize(int rolloff_buffersize){m_RollOffBufferSize = rolloff_buffersize;}
        int GetRollOffBufferSize(){return m_RollOffBufferSize;}

        const std::vector<WTATrigObj>& GetRollOffList() const {return m_RollOffList;}; // Access the RollOffList

    private:
        std::vector<WTATrigObj> m_RollOffList; // For 2-Pass
        unsigned int m_RollOffBufferSize; // For 2-Pass
};

inline void WTACone2PassMaker::FillLists(const std::vector<WTATrigObj>& InputTowers) // 2Pass FillLists()
{
    m_ConstituentList.clear(); m_SeedSortingList.clear(); m_RollOffList.clear();
    const unsigned int MaxSeedSortingN = m_WTAConeMakerParameter.GetMaxSeedSortingN();
    const unsigned int MaxConstN = m_WTAConeMakerParameter.GetMaxConstN();
    for(auto tower: InputTowers)
    {
        if(tower.pt() < m_WTAConeMakerParameter.GetConstEtCut())continue; // Skip Et < 2GeV
        if(tower.pt() >= m_WTAConeMakerParameter.GetSeedEtCut())// Harmonize >=
        {
            m_SeedSortingList.insert(m_SeedSortingList.begin(), tower); // Insert incoming tower at the beginning of the sorting list
            SortByPt(m_SeedSortingList); // Do et-sorting as tower comes in, definition of 2-Pass, using std::stable_sort()
            while (m_SeedSortingList.size() > MaxSeedSortingN) // It only runs when m_SeedSortingList.size() = m_MaxSeedSortingN + 1 though
            {
                m_RollOffList.push_back(m_SeedSortingList.back()); // Then, Fill the Roll-Off list
                m_SeedSortingList.pop_back(); // Discard the 51st seed tower
            }
        }
        else m_ConstituentList.push_back(tower); // Always put soft tower at the back of the m_ConstituentList
    }
    if(m_ConstituentList.size() > MaxConstN)m_ConstituentList.resize(MaxConstN); // Truncate the Constituent list
    if(m_RollOffList.size() > m_RollOffBufferSize)m_RollOffList.resize(m_RollOffBufferSize); // Truncate the Roll-Off list
}

inline void WTACone2PassMaker::SeedCleaning() // 2Pass
{
    m_SeedList.clear();
    if(m_DEBUG)std::cout << "HighEtMerge2Pass Seed Cleaning......" << std::endl;
    int seed_N = m_SeedSortingList.size(); // Default: Max 50
    for(int i = 0; i < seed_N; i++){
        WTATrigObj seed = m_SeedSortingList.at(i);
        unsigned int jet_N = m_SeedList.size();
        if(jet_N == 0)m_SeedList.push_back(WTATrigObjToWTAJet(seed)); // Take first seed as jet
        else
        {
            int MaxPtIndex = -1; // MaxPtIndex will be -1 in 2Pass by construction, et-sorted m_SeedSortingList
            std::vector<int> associate_bit = GetAssociateBits(seed, MaxPtIndex); // Associate_bit filling done
            if(std::find(associate_bit.begin(), associate_bit.end(), 1) != associate_bit.end())
            { // When there is at least one association between incoming tower vs existing seeds
                for(unsigned int j = 0; j < jet_N; j++) // Read high-et seed first
                {
                    if(associate_bit.at(j) == 1)
                    {
                        m_SeedList.at(j).MergeConstituent(seed);
                        break; // Done, move to the next tower-object
                    }
                }
            }
            else
            { // No Association
                if(jet_N < m_WTAConeMakerParameter.GetMaxSeedN()) // There is a slot in the seed list
                {
                    m_SeedList.push_back(WTATrigObjToWTAJet(seed)); // Insert seed-object to the end of the seed list
                } // Discard if there is no open slot in the seed list
            }
        } // Main loop
        // No need to pt sort the SeedList. It is sorted by construction
        // No need to resize the SeedList. Only insert the seed-object if(jet_n < m_MaxSeedN)
        if(m_VERBOSE)PrintSeedList(); // Print SeedList, for debug
    } // seed loop
    if(m_DEBUG){
        PrintSeedList();
        std::cout << "HighEtMerge2Pass Seed Cleaning Done......" << std::endl;
    }
}

inline void WTACone2PassMaker::MergeConstsToSeeds()
{
    if(m_RollOffList.size() > 0)
    {
        for (auto off_seed: m_RollOffList)InsertToConstList(off_seed); // m_AddConstFirst is true by default
    }
    for(auto constituent: m_ConstituentList)
    {
        for(unsigned int j = 0; j < m_SeedList.size(); j++) // Assume Jets are pT sorted, WTA means more energetic jet eats constituent first
        {
            IntOrFloat dR2 = constituent.dR2(m_SeedList.at(j));
            if(constituent.IsAssocdR(m_SeedList.at(j), m_WTAConeMakerParameter.GetJet_dR2()) && dR2!=0) // Thistime, the condition is m_JetArea, the usual R2Par, **WARNING: dR2!=0 IS TEMPORARY FOR INT-SIM. NEED TO KNOW TOPOTOWER CREATION
            {
                m_SeedList.at(j).MergeConstituent(constituent);
                ResizeThisJetConstituents(m_SeedList.at(j)); // Check JetConstituent N
                break; // Break the jet loop, move to the next constituent
            }
        }
    }
}

#endif