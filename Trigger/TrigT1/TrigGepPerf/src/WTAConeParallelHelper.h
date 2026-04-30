/*
    Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef WTAConeParallelHelper_h
#define WTAConeParallelHelper_h



#include "./WTAObject.h" // Use the WTAObject
#include "./WTAConeMaker.h" // The base class
#include <vector>
#include <memory>

class WTAConeParallelHelper{
    public:
        WTAConeParallelHelper(unsigned int block_n = 1):
          m_BlockN(block_n), m_DivideByEta(false), m_JetN(10), m_StoreAmbiguousJets(true)
        {}; // Constructor
        ~WTAConeParallelHelper () {}; // Destructor
        
        void SetBlockN(unsigned int block_n); // Call SetBlockN in the constructor
        void SetDivideByEta(bool divide_by_eta){m_DivideByEta = divide_by_eta;}
        void SetJetN(unsigned int jet_n){m_JetN = jet_n;}
        void SetStoreAmbiguousJets(bool store_ambiguous_jets){m_StoreAmbiguousJets = store_ambiguous_jets;}
        /// StoreAmbiguousJets
        // When merging the final four phi-blocks, the FW and cpp implementations differ
        // In FW, the phi-blocks come in according to their latency
        // In cpp, the phi-blocks come in fixed order of 0-1-2-3
        // There can be cases when two or more jets with same pt_t are created,
        // their ordering or some jet may get discarded due to the final truncation
        // If m_StoreAmbiguousJets is set true, it tracks all jets with same pt_t
        // and extend the length of final jet list(m_JetN)
        // Storing all possible jets that can be found in the FW implementation
        // This is a temporary solution(jae.jin.hong@cern.ch)

        template<typename T>
        T PhiWrap(T phi);
        template<typename T>
        bool CheckInsideRegionPhi(T tower_pos, unsigned int iBlock, bool doWithOverlap = false);
        template<typename T>
        bool CheckInsideRegionEta(T tower_pos, unsigned int iBlock, bool doWithOverlap = false);
        void CreateBlocks(const std::vector<WTATrigObj>& all_towers);
        unsigned int GetBlockN(){return m_BlockN;};
        unsigned int GetJetN(){return m_JetN;};
        bool GetStoreAmbiguousJets(){return m_StoreAmbiguousJets;};

        template<typename WTAClassType>
        void RunParallelWTA(std::unique_ptr<WTAClassType>& AnyWTAClass); // Run this N times, without argument
        void CheckJetInCore();
        std::vector<WTAJet> GetAllJets();

        std::vector<WTATrigObj> GetStage1ConstituentListPerBlock(unsigned int iBlock) const {
            return m_stage1_constituentListPerBlock.at(iBlock);
        }
        std::vector<WTATrigObj> GetStage1SeedSortingPerBlock(unsigned int iBlock) const {
            return m_stage1_seedSortingListPerBlock.at(iBlock);
        }
        std::vector<WTAJet> GetStage2ConeSeedsPPerBlock(unsigned int iBlock) const {
            return m_stage2_coneSeedsPerBlock.at(iBlock);
        }
        std::vector<WTAJet> GetStage3ConeJetsPerBlock(unsigned int iBlock) const {
            return m_stage3_coneJetsPerBlock.at(iBlock);
        }

    private:
        unsigned int m_BlockN;
        bool m_DivideByEta;

        // Merge all BlockN jets > OverlapRemoval > final JetN truncation
        unsigned int m_JetN;
        bool m_StoreAmbiguousJets;

        std::vector<std::vector<WTATrigObj>> m_InputTowersPerBlock;
        std::vector<std::vector<WTAJet>> m_OutputJetsPerBlock;

        // For debugging
        std::vector<std::vector<WTATrigObj>> m_stage1_constituentListPerBlock;
        std::vector<std::vector<WTATrigObj>> m_stage1_seedSortingListPerBlock;
        std::vector<std::vector<WTAJet>> m_stage2_coneSeedsPerBlock;
        std::vector<std::vector<WTAJet>> m_stage3_coneJetsPerBlock;
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

    m_stage1_constituentListPerBlock.clear();
    m_stage1_seedSortingListPerBlock.clear();
    m_stage2_coneSeedsPerBlock.clear();
    m_stage3_coneJetsPerBlock.clear();
}

template<typename T>
inline T WTAConeParallelHelper::PhiWrap(T phi) {
#ifdef FLOATING_POINT_SIMULATION
    while(phi >= PHI_MAX)phi -= 2*PI;
    while(phi < PHI_MIN)phi += 2*PI;
    return phi;
#else
    T wrapped = phi % PHI_LEN;
    if (wrapped < 0) wrapped = wrapped + PHI_LEN;
    return wrapped;
#endif
}

template<typename T>
bool WTAConeParallelHelper::CheckInsideRegionEta(T tower_pos, unsigned int iBlock, bool doWithOverlap) {
    T block_half_width = ETA_LEN / T(m_BlockN *2);
    T block_center = ETA_MIN + ETA_LEN / T(m_BlockN) * iBlock + block_half_width;
    if (doWithOverlap) {
        block_half_width = block_half_width + T(CORE_DIST);
    }
    // Compute min and max with potential wrap-around
    T block_min = block_center - block_half_width;
    T block_max = block_center + block_half_width;
    // Check inclusion
    return (tower_pos >= block_min) && (tower_pos < block_max);
}

template<typename T>
bool WTAConeParallelHelper::CheckInsideRegionPhi(T tower_pos, unsigned int iBlock, bool doWithOverlap) {
    T block_center = PHI_MIN + PHI_LEN / T(m_BlockN) * iBlock;
    T block_half_width = PHI_LEN / T(m_BlockN *2);
    if (doWithOverlap) {
        block_half_width = block_half_width + T(CORE_DIST);
    }

    // Compute min and max with potential wrap-around
    T block_min = PhiWrap(block_center - block_half_width);
    T block_max = PhiWrap(block_center + block_half_width);
    tower_pos = PhiWrap(tower_pos);
    // Check inclusion using modular range
    if (block_min < block_max) {
        return (tower_pos >= block_min) && (tower_pos < block_max);
    } else {
        // The Region wraps around (e.g., from +3.2 to -3.2)
        return (tower_pos >= block_min) || (tower_pos < block_max);
    }
}

inline void WTAConeParallelHelper::CreateBlocks(const std::vector<WTATrigObj>& all_towers)
{
    const unsigned tower_n = all_towers.size();
    for(unsigned int iBlock = 0; iBlock < m_BlockN; iBlock++)
    {
        if(m_DivideByEta)
        {
            for(unsigned int iTower = 0; iTower < tower_n; iTower++)
            {
                eta_t tower_pos = all_towers.at(iBlock).eta(); // Get tower's eta position
                if(CheckInsideRegionEta(tower_pos, iBlock, true))
                    m_InputTowersPerBlock.at(iBlock).push_back(all_towers.at(iTower));
            }
        }
        else
        {
            for(unsigned int iTower = 0; iTower < tower_n; iTower++)
            {
                phi_t tower_pos = PhiWrap(all_towers.at(iTower).phi()); // Get tower's phi position
                if(CheckInsideRegionPhi(tower_pos, iBlock, true))
                    m_InputTowersPerBlock.at(iBlock).push_back(all_towers.at(iTower));
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
        m_stage1_constituentListPerBlock.push_back(MyWTAMakerClass->GetConstituentList());
        m_stage1_seedSortingListPerBlock.push_back(MyWTAMakerClass->GetSeedSortingList());
        MyWTAMakerClass->SeedCleaning();
        m_stage2_coneSeedsPerBlock.push_back(MyWTAMakerClass->GetSeedList());
        MyWTAMakerClass->MergeConstsToSeeds();
        MyWTAMakerClass->CreateERingInfo();
        m_stage3_coneJetsPerBlock.push_back(MyWTAMakerClass->GetSeedList());
        m_OutputJetsPerBlock.at(i) = MyWTAMakerClass->GetSeedList(); // Define the ith vector, not push_back
    }
}

inline void WTAConeParallelHelper::CheckJetInCore() // DON'T CALL THIS FOR SINGLE REGION!!
{
    std::vector<WTAJet> all_jets;
    for(unsigned int iBlock = 0; iBlock < m_BlockN; iBlock++)
    {
        unsigned int this_block_jet_n = m_OutputJetsPerBlock.at(iBlock).size();
        if(this_block_jet_n == 0) continue; // Only run if there are jets in the ith ROI

        if (m_DivideByEta)
        {
            for (int j = this_block_jet_n - 1; j >= 0; j--)
            {
                WTAJet jet = m_OutputJetsPerBlock.at(iBlock).at(j);
                eta_t jet_pos = jet.eta();
                if (!CheckInsideRegionEta(jet_pos, iBlock, false))
                    m_OutputJetsPerBlock.at(iBlock).erase(m_OutputJetsPerBlock.at(iBlock).begin() + j);
            }
        }
        else
        {
            for (int j = this_block_jet_n - 1; j >= 0; j--) {
                WTAJet jet = m_OutputJetsPerBlock.at(iBlock).at(j);
                phi_t jet_pos = jet.phi();
                if (!CheckInsideRegionPhi(jet_pos, iBlock, false))
                    m_OutputJetsPerBlock.at(iBlock).erase(m_OutputJetsPerBlock.at(iBlock).begin() + j);
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
    SortByPt(all_jets); // Maximum 40 jets here
    if(m_StoreAmbiguousJets){
        std::vector<pt_t> duplicated_pt_list;
        unsigned int cnt = 0;
        for(unsigned int i = 0; i < all_jets.size() - 1; i++){
            pt_t i_pt = all_jets.at(i).pt();

            if(std::find(duplicated_pt_list.begin(), duplicated_pt_list.end(),i_pt)!= duplicated_pt_list.end()) continue; // Skip counted pt_t
            for(unsigned int j = i + 1; j < all_jets.size(); j++){
                pt_t j_pt = all_jets.at(j).pt();
                if(i_pt == j_pt){
                    cnt++;
                    if(std::find(duplicated_pt_list.begin(), duplicated_pt_list.end(),i_pt)!= duplicated_pt_list.end())duplicated_pt_list.push_back(i_pt);
                }
            }
        }
        m_JetN += cnt;
    }
    if(all_jets.size() > m_JetN)all_jets.resize(m_JetN); // Do finial truncation
    return all_jets;
}

#endif