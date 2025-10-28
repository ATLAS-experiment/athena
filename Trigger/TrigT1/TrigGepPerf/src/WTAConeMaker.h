#ifndef WTAConeMaker_h
#define WTAConeMaker_h

#include <iostream>
#include <algorithm>
#include <map>
#include "./WTAObject.h" // Use the WTATrigObj

class WTAParameters{ // Stores common WTAParameters, should be a protected variable of WTAConeMaker class
    public:
        WTAParameters(IntOrFloat const_et_cut = 2000, IntOrFloat seed_et_cut = 5000, IntOrFloat jet_dr2 = R2PAR, 
        unsigned int max_const_n = 250, unsigned int max_seed_sorting_n = 50, unsigned int max_seed_n = 10, unsigned int max_const_per_jet_n = 99, 
        bool add_const_first = true) : // Takes 8 arguments, sets 9 parameters!
            m_ConstEtCut(const_et_cut), m_SeedEtCut(seed_et_cut), m_Jet_dR2(jet_dr2), m_Iso_dR2(jet_dr2),
            m_MaxConstN(max_const_n), m_MaxSeedSortingN(max_seed_sorting_n), m_MaxSeedN(max_seed_n), m_MaxConstPerJetN(max_const_per_jet_n), 
            m_AddConstFirst(add_const_first), m_max_input_towers(6400)
            {}; // Constructor

        void SetConstEtCut(IntOrFloat ConstEtCut){m_ConstEtCut = ConstEtCut;};
        void SetSeedEtCut(IntOrFloat SeedEtCut){m_SeedEtCut = SeedEtCut;};
        void SetIso_dR2(IntOrFloat Iso_dR2){m_Iso_dR2 = Iso_dR2;}; // Default is jet_are = Iso_dR2. Use this for different Isolation condition
        void SetJet_dR2(IntOrFloat Jet_dR2){m_Jet_dR2 = Jet_dR2;};
        void SetMaxConstN(int MaxConstN){m_MaxConstN = MaxConstN;};
        void SetMaxSeedSortingN(int MaxSeedSortingN){m_MaxSeedSortingN = MaxSeedSortingN;};
        void SetMaxSeedN(int MaxSeedN){m_MaxSeedN = MaxSeedN;};
        void SetMaxConstPerJetN(int MaxConstPerJetN){m_MaxConstPerJetN = MaxConstPerJetN;};
        void SetAddConstFirst(bool add_const_first){m_AddConstFirst = add_const_first;};
        void SetMaxInputTowers(int max_input_towers){m_max_input_towers = max_input_towers;}

        IntOrFloat GetConstEtCut(){return m_ConstEtCut;};
        IntOrFloat GetSeedEtCut(){return m_SeedEtCut;};
        IntOrFloat GetIso_dR2(){return m_Iso_dR2;};
        IntOrFloat GetJet_dR2(){return m_Jet_dR2;};
        unsigned int GetMaxConstN(){return m_MaxConstN;};
        unsigned int GetMaxSeedSortingN(){return m_MaxSeedSortingN;};
        unsigned int GetMaxSeedN(){return m_MaxSeedN;};
        unsigned int GetMaxConstPerJetN(){return m_MaxConstPerJetN;};
        bool GetAddConstFirst(){return m_AddConstFirst;};
        unsigned int GetMaxInputTowers(){return m_max_input_towers;}

    private:
        IntOrFloat m_ConstEtCut; // What's integer unit of the GeV?
        IntOrFloat m_SeedEtCut;
        IntOrFloat m_Jet_dR2; // Merge Constituents < m_Jet_dR2
        IntOrFloat m_Iso_dR2; // Merge Seeds < m_Iso_dR2
        unsigned int m_MaxConstN; // Can take maximum 240 topotowers, and then separate them into two lists
        unsigned int m_MaxSeedSortingN;
        unsigned int m_MaxSeedN;
        unsigned int m_MaxConstPerJetN;
        bool m_AddConstFirst; // Default: True, failed seeds are insereted to the const-list first. Meaning, it will read the hard towers first
        unsigned int m_max_input_towers;
};

class WTAConeMaker{
    public:
        WTAConeMaker(bool debug = false, bool verbose = false):
            m_DEBUG(debug), m_VERBOSE(verbose)
            {}; // Constructor
        virtual ~WTAConeMaker() = default; // Destructor, make it polymorphic
        
        void ResizeConstituentList();
        void ResizeSeedSortingList();
        void ResizeSeedList();
        void ResizeSeedNConstLists(){ResizeSeedList(); ResizeConstituentList();}
        void ResizeThisJetConstituents(WTAJet &jet);
        void ClearLists(){m_ConstituentList.clear(); m_SeedSortingList.clear(); m_SeedList.clear();};
        void PrintSeedList();
        WTAJet WTATrigObjToWTAJet(const WTATrigObj& obj);
        WTATrigObj WTAJetToWTATrigObj(const WTAJet& jet);
        const std::vector<WTATrigObj>& GetConstituentList() const {return m_ConstituentList;}; // Access these lists whenever we need
        const std::vector<WTATrigObj>& GetSeedSortingList() const {return m_SeedSortingList;};
        const std::vector<WTAJet>& GetSeedList() const {return m_SeedList;};
        std::vector<int> GetAssociateBits(WTATrigObj incoming_seed, int& max_pt_index); // Common seed-SeedList asso.bits

        std::vector<WTATrigObj> LoadInputs(const std::vector<IntOrFloat>& ptVec, const std::vector<IntOrFloat>& etaVec, const std::vector<IntOrFloat>& phiVec, const std::vector<IntOrFloat>& mVec);
        virtual void FillLists(const std::vector<WTATrigObj>& InputTowers); // Overwritten in the 2Pass
        void InitiateInputs(const std::vector<IntOrFloat>& ptVec, const std::vector<IntOrFloat>& etaVec, const std::vector<IntOrFloat>& phiVec, const std::vector<IntOrFloat>& mVec); // LoadInputs() + FillLists()
        void InitiateInputs(const std::vector<WTATrigObj>& InputTowers); // LoadInputs() + FillLists()

        void InsertToConstList(const WTATrigObj& obj);
        virtual void SeedCleaning(); // Do baseline cleaning
        virtual void MergeConstsToSeeds(); // Can be overwritten

        void SetDEBUG(){m_DEBUG = true;}; // Printout for debug
        void SetVERBOSE(){m_DEBUG = true; m_VERBOSE = true;};
        bool GetDEBUG(){return m_DEBUG;}; // Printout for debug
        bool GetVERBOSE(){return m_VERBOSE;};

        WTAParameters m_WTAConeMakerParameter; // This should be accesible, in order to update the parameters

    protected:
        std::vector<WTATrigObj> m_ConstituentList;
        std::vector<WTATrigObj> m_SeedSortingList;
        std::vector<WTAJet> m_SeedList; // Max top-10 seeds

        bool m_DEBUG;
        bool m_VERBOSE;

};

inline std::vector<WTATrigObj> WTAConeMaker::LoadInputs(const std::vector<IntOrFloat>& ptVec, const std::vector<IntOrFloat>& etaVec, const std::vector<IntOrFloat>& phiVec, const std::vector<IntOrFloat>& mVec)
{
    std::vector<WTATrigObj> input_towers;
    unsigned int tower_n = ptVec.size();
    for(unsigned int t = 0; t < tower_n; t++)
    {
        WTATrigObj this_tower(ptVec.at(t), etaVec.at(t), phiVec.at(t), mVec.at(t));
        input_towers.push_back(this_tower);
    }
    return input_towers;

}

inline void WTAConeMaker::FillLists(const std::vector<WTATrigObj>& InputTowers) // Baseline FillLists()
{
    m_ConstituentList.clear(); m_SeedSortingList.clear();
    const unsigned int MaxSeedSortingN = m_WTAConeMakerParameter.GetMaxSeedSortingN();
    const unsigned int MaxConstN = m_WTAConeMakerParameter.GetMaxConstN();
    const int MaxTowersToReadPerEvent = m_WTAConeMakerParameter.GetMaxInputTowers();
    if(m_DEBUG) std::cout << "MaxTowersToReadPerEvent = " << MaxTowersToReadPerEvent << ", Size of event = " << InputTowers.size() << std::endl;
    int nTowers = 0;
    for(const auto& tower: InputTowers)
    {
        if(tower.pt() < m_WTAConeMakerParameter.GetConstEtCut())continue; // Skip Et < 2GeV
        if(tower.pt() >= m_WTAConeMakerParameter.GetSeedEtCut())m_SeedSortingList.push_back(tower); // Harmonize >=
        else m_ConstituentList.push_back(tower); // Initially, always put soft tower at the back of the m_ConstituentList

        // We can read only first N towers out of entier event
        nTowers++;
        if(nTowers>MaxTowersToReadPerEvent) break;
    }
    if(m_SeedSortingList.size() > MaxSeedSortingN)m_SeedSortingList.resize(MaxSeedSortingN);
    if(m_ConstituentList.size() > MaxConstN)m_ConstituentList.resize(MaxConstN); // Resize lists accordingly
}

inline void WTAConeMaker::InitiateInputs(const std::vector<IntOrFloat>& ptVec, const std::vector<IntOrFloat>& etaVec, const std::vector<IntOrFloat>& phiVec, const std::vector<IntOrFloat>& mVec)
{
    std::vector<WTATrigObj> InputTowers = LoadInputs(ptVec, etaVec, phiVec, mVec);
    FillLists(InputTowers);
    if(m_VERBOSE)std::cout << "InputN, ConstN, SeedN = " << InputTowers.size() << " , " << m_ConstituentList.size() << " , " << m_SeedSortingList.size() << std::endl;
}

inline void WTAConeMaker::InitiateInputs(const std::vector<WTATrigObj>& InputTowers)
{
    FillLists(InputTowers);
    if(m_VERBOSE)std::cout << "InputN, ConstN, SeedN = " << InputTowers.size() << " , " << m_ConstituentList.size() << " , " << m_SeedSortingList.size() << std::endl;
}

inline void WTAConeMaker::PrintSeedList()
{
    int AllJetConstN = 0;
    for(const auto& jet: m_SeedList)
    {
        AllJetConstN += jet.GetConstituentCount();
        std::cout << "Jet pT, eta, phi, constN = " << jet.pt() << " , " << jet.eta() << " , " << jet.phi() << " , " << jet.GetConstituentCount() << std::endl;
    }
    std::cout << "PrintSeedList..." << " , SeedN, AllJetConstN, ConstN = " << m_SeedList.size() << " , " << AllJetConstN << " , " << m_ConstituentList.size() <<  std::endl;
    std::cout << "+++++++++++++++++++++++++" << std::endl;
}

inline void WTAConeMaker::InsertToConstList(const WTATrigObj& obj)
{
    if(m_WTAConeMakerParameter.GetAddConstFirst())m_ConstituentList.insert(m_ConstituentList.begin(), obj); // Insert obj at the beginnning
    else m_ConstituentList.push_back(obj); // Insert obj at the end
}

inline void WTAConeMaker::ResizeConstituentList()
{
    const unsigned int MaxConstN = m_WTAConeMakerParameter.GetMaxConstN();
    while(m_ConstituentList.size() > MaxConstN)m_ConstituentList.pop_back();
}

inline void WTAConeMaker::ResizeSeedSortingList()
{
    const unsigned int MaxSeedSortingN = m_WTAConeMakerParameter.GetMaxSeedSortingN();
    while(m_SeedSortingList.size() > MaxSeedSortingN)m_SeedSortingList.pop_back();
}

inline void WTAConeMaker::ResizeThisJetConstituents(WTAJet &jet)
{
    const unsigned int MaxConstPerJetN = m_WTAConeMakerParameter.GetMaxConstPerJetN();
    while(jet.GetConstituentCount() > MaxConstPerJetN)
    {
        jet.PopOutLastConstituent(); // Erase the last constituent, it is NOT appended to the Const list
    }
}

inline void WTAConeMaker::ResizeSeedList()
{
    const unsigned int MaxSeedN = m_WTAConeMakerParameter.GetMaxSeedN();
    while(m_SeedList.size() > MaxSeedN)
    {
        InsertToConstList(WTAJetToWTATrigObj(m_SeedList.back()));
        m_SeedList.pop_back();
    }
}

inline WTAJet WTAConeMaker::WTATrigObjToWTAJet(const WTATrigObj& obj)
{
    WTAJet thisjet(obj.pt(), obj.eta(), obj.phi(), obj.m(), obj.idx());
    return thisjet;
}

inline WTATrigObj WTAConeMaker::WTAJetToWTATrigObj(const WTAJet& jet)
{
    WTATrigObj thisobj(jet.pt(), jet.eta(), jet.phi(), jet.m(), jet.idx());
    return thisobj;
}

inline std::vector<int> WTAConeMaker::GetAssociateBits(WTATrigObj tower, int& MaxPtIndex)
{
    int jet_N = m_SeedList.size();
    std::vector<int> associate_bit(jet_N, 0);
    IntOrFloat MaxPt = tower.pt(); MaxPtIndex = -1;
    for(int j = 0; j < jet_N; j++)
    {
        if(m_VERBOSE)std::cout << "deta, dphi, dR2 = " << tower.d_eta(m_SeedList.at(j)) << " , " << tower.d_phi_MPI_PI(m_SeedList.at(j)) << " , " << tower.dR2(m_SeedList.at(j)) << std::endl;
        if(tower.IsAssocdR(m_SeedList.at(j), m_WTAConeMakerParameter.GetIso_dR2())) // e.g) dR2 < 0.16, association
        {
            associate_bit.at(j) = 1;
            if(m_SeedList.at(j).pt() > MaxPt)
            {
                MaxPtIndex = j;
                MaxPt = m_SeedList.at(j).pt(); // Update the counter
            }
        }
    } // associate_bit filling done
    return associate_bit;
}

inline void WTAConeMaker::SeedCleaning()
{
    m_SeedList.clear();
    if(m_DEBUG)std::cout << "Baseline Seed Cleaning......" << std::endl;
    for(const auto& seed: m_SeedSortingList)
    {
        int jet_N = m_SeedList.size();
        if(jet_N == 0)
        {
            m_SeedList.push_back(WTATrigObjToWTAJet(seed)); // Take first seed as jet
        }
        else
        {   
            int MaxPtIndex = -1;
            std::vector<int> associate_bit = GetAssociateBits(seed, MaxPtIndex); // Associate_bit filling done
            if(std::find(associate_bit.begin(), associate_bit.end(), 1) != associate_bit.end())
            { // When there is at least one association
                if(MaxPtIndex == -1)
                { // Incoming Seed is the highest et, do the seed cleaning
                    WTAJet IncomingSeedAsJet = WTATrigObjToWTAJet(seed);
                    for(int j = jet_N - 1; j >= 0; j--)
                    { // It's important to read bits from the right, lower et jets first
                        if(associate_bit.at(j) == 1)
                        { // If Incoming seed is the highest, and there is an associated old jth jet, pop out jth jet
                            InsertToConstList(WTAJetToWTATrigObj(m_SeedList.at(j))); // Move jth jet to constituent list
                            m_SeedList.erase(m_SeedList.begin() + j); // Then, erase jth jet
                        }
                    }
                    m_SeedList.push_back(IncomingSeedAsJet); // Add this new incoming seed as Jet, well localized protojet
                }
                else
                { // Incoming Seed is not the highest et, add seed to the ConstituentList
                    InsertToConstList(seed);
                }
            } // At least one association loop
            else
            { // When there is no association
                WTAJet IncomingSeedAsJet = WTATrigObjToWTAJet(seed);
                m_SeedList.push_back(IncomingSeedAsJet); // Add seed to the SeedList
            }
        } // Main merging loop
        SortByPt(m_SeedList); // PtSort the SeedList. This is important for Baseline seed cleaning!
        ResizeSeedNConstLists(); // Resize SeedList, and ConstList
        if(m_VERBOSE)PrintSeedList(); // Print SeedList, for debug
    } // seed loop
    if(m_DEBUG){
        PrintSeedList();
        std::cout << "Baseline Seed Cleaning Done......" << std::endl;
    }
}

inline void WTAConeMaker::MergeConstsToSeeds()
{
    for(auto constituent: m_ConstituentList)
    {
        for(unsigned int j = 0; j < m_SeedList.size(); j++) // Assume Seeds are pT sorted, WTA means more energetic jet eats constituent first
        {
            IntOrFloat dR2 = constituent.dR2(m_SeedList.at(j));
            if(dR2 != 0 && constituent.IsAssocdR(m_SeedList.at(j), m_WTAConeMakerParameter.GetJet_dR2())) // Thistime, the condition is m_Jet_dR2, the usual R2Par, **WARNING: dR2!=0 IS TEMPORARY, NEED TO KNOW TOPOTOWER CREATION
            {
                m_SeedList.at(j).MergeConstituent(constituent);
                ResizeThisJetConstituents(m_SeedList.at(j)); // Check JetConstituent N
                break; // Break the jet loop, move to the next constituent
            }
        }
    }
}

#endif