/*
    Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef WTAObject_h
#define WTAObject_h

#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>

#include "./WTASimTypes.h" // Use the WTASimTypes

template <typename T>
        static void SortByPt(std::vector<T>& list) // Simple List sorting
        {
            std::stable_sort(list.begin(), list.end(), // Use stable_sort to preserve the order of equal elements
                [](const T &a, const T &b)
                {
                    return a.pt() > b.pt();
                }
            );
        }

class WTATrigObj{
    public:
        WTATrigObj(pt_t pt = 0, eta_t eta = 0, phi_t phi = 0, m_t m = 0, int idx = -99)
        : m_pt(pt), m_eta(eta), m_phi(phi), m_m(m), m_idx(idx){}; // Constructor

        pt_t pt()  const     {return m_pt;};
        void   pt(pt_t pt)   {m_pt = pt;};
        eta_t eta() const     {return m_eta;};
        void   eta(eta_t eta) {m_eta = eta;};
        phi_t phi() const     {return m_phi;};
        void   phi(phi_t phi) {m_phi = phi;};
        m_t m()   const     {return m_m;};
        void   m(m_t m)     {m_m = m;};
        int        idx() const     {return m_idx;};
        void   idx(int idx)        {m_idx = idx;};

#ifdef FLOATING_POINT_SIMULATION
        phi_t d_phi_0_2PI(const WTATrigObj& o2) const;
#endif
        phi_t d_phi_MPI_PI(const WTATrigObj& o2) const;
        eta_t d_eta(const WTATrigObj& o2) const {eta_t tmp = m_eta - o2.eta(); return tmp;}
        bool IsAssocdR(WTATrigObj& tower, tech_t dr);

        // comparison operators for easy sorting
        friend bool operator<  (const WTATrigObj &o1, const WTATrigObj &o2){ return o1.pt() <  o2.pt(); }; // Call .pt(), not directly access private m_pt
        friend bool operator>  (const WTATrigObj &o1, const WTATrigObj &o2){ return o1.pt() >  o2.pt(); };
        friend bool operator<= (const WTATrigObj &o1, const WTATrigObj &o2){ return o1.pt() <= o2.pt(); };
        friend bool operator>= (const WTATrigObj &o1, const WTATrigObj &o2){ return o1.pt() >= o2.pt(); };

        // operation override, for easy merging
        WTATrigObj operator+ (const WTATrigObj& obj) {
            WTATrigObj new_obj;
            new_obj.m_pt = this->pt() + obj.pt(); // Current default is the scalar sum, WTA
            new_obj.m_eta = this->eta();
            new_obj.m_phi = this->phi();
            new_obj.m_m = this->m();
            return new_obj;
        };

    private:
        pt_t m_pt;
        eta_t m_eta;
        phi_t m_phi;
        m_t m_m;
        int m_idx; // Index required to track constituent indices in TrigGepPerf

};

#ifdef FLOATING_POINT_SIMULATION
inline phi_t WTATrigObj::d_phi_0_2PI(const WTATrigObj& o2) const
{
    phi_t tmp = m_phi - o2.phi();
    while(tmp < 0)tmp += 2*PI;
    while(tmp > 2*PI)tmp -= 2*PI;
    return tmp;
}
inline phi_t WTATrigObj::d_phi_MPI_PI(const WTATrigObj& o2) const
{
    phi_t tmp = m_phi - o2.phi();
    while(tmp < -PI)tmp += 2*PI;
    while(tmp > PI)tmp -= 2*PI;
    return tmp;
}
#else
inline phi_t WTATrigObj::d_phi_MPI_PI(const WTATrigObj& o2) const
{
    phi_t tmp;
    if (m_phi >= o2.phi()){
        tmp = m_phi - o2.phi();
    } else {
        tmp = o2.phi() - m_phi;
    }

    // Wrap difference to [-32, +31]
    if (tmp < -HALF_PHI_LEN) tmp = tmp + PHI_LEN;
    else if (tmp >= HALF_PHI_LEN) tmp = tmp - PHI_LEN;

    return tmp;
}
#endif

#ifdef FLOATING_POINT_SIMULATION
inline bool WTATrigObj::IsAssocdR(WTATrigObj& tower, tech_t dr){
    eta_t d_eta = this->d_eta(tower);
    phi_t d_phi = this->d_phi_MPI_PI(tower);
    bool assoc = (dr*dr) >= (d_eta*d_eta + d_phi*d_phi);
    return assoc;
}
#else
inline bool WTATrigObj::IsAssocdR(WTATrigObj& tower, tech_t dr){
    assert(tower.eta() >= 0 && "Input tower.eta() must be non-negative");
    assert(tower.phi() >= 0 && "Input tower.phi() must be non-negative");
    assert(dr >= 0 && "Input dr must be non-negative");
    eta_t d_eta = this->eta() > tower.eta() ? this->d_eta(tower) : tower.d_eta(*this); // ensure positive d_eta d_phi
    phi_t raw_dphi = abs(int32_t(this->phi()) - int32_t(tower.phi()));
    phi_t d_phi = std::min(raw_dphi, PHI_LEN - raw_dphi);
    bool assoc = false;
    for (unsigned int i = 0; i <= dr; i++){
        unsigned int max_j = sqrt(static_cast<double>(dr*dr - i*i)); // Need to be careful with the sqrt
        for(unsigned int j = 0; j <= max_j; j++){
            if(d_eta <= i && d_phi <= j){
                assoc = true;
                break;
            }
        }
    }
    return assoc;
}
#endif

struct WTA4JetERingInfo {
    pt_t ring0_Et = 0;
    pt_t ring1_Et = 0;
    pt_t ring2_Et = 0;
    pt_t ring3_Et = 0;
    pt_t ring4_Et = 0;
    tobn_t total_TobN = 0;
    ring0_tobn_t ring0_TobN = 0;
    ring1_tobn_t ring1_TobN = 0;
    ring2_tobn_t ring2_TobN = 0;
    ring3_tobn_t ring3_TobN = 0;
    ring4_tobn_t ring4_TobN = 0;
};

class WTAJet : public WTATrigObj{       // The class
  public:             // Access specifier
    WTAJet (pt_t pt = 0, eta_t eta = 0, phi_t phi = 0, m_t m = 0, int idx = -99, tech_t jet_dr = R_PAR) :
        WTATrigObj(pt, eta, phi, m, idx) {
            m_Seed = WTATrigObj(pt, eta, phi, m, idx);
            m_jet_dr = jet_dr;
            m_ConstituentList.clear();
            m_ConstituentList.push_back(m_Seed);
            #ifdef FLOATING_POINT_SIMULATION
                m_ring_n = static_cast<unsigned int>(jet_dr / 0.1);
            #else
                m_ring_n = jet_dr;
            #endif
        };

    const WTATrigObj& GetSeed() const {return m_Seed;};
    void MergeConstituent(WTATrigObj &tower);
    void MergeConstituent(WTATrigObj *tower);
    void MergeWTAJet(WTAJet &targetjet);
    void PopOutLastConstituent();
    const std::vector<WTATrigObj>& GetConstituentList() const {return m_ConstituentList;};
    tobn_t GetConstituentCount() const {return static_cast<tobn_t>(m_ConstituentList.size());}; // Match the type
    
    unsigned int GetRingN(const WTATrigObj& tower) const;
    void CreateERingInfo(); // To be called after jet clustering is done
    void PrintERingInfo() const;
    const WTA4JetERingInfo& GetERingInfo() const {return m_ERingInfo;};
    void SetERingInfo(const WTA4JetERingInfo& ering_info) {m_ERingInfo = ering_info;}; // When parsing the ref jet, ERingInfo does not get created automatically

  private:
    WTATrigObj m_Seed;
    std::vector<WTATrigObj> m_ConstituentList;
    tech_t m_jet_dr;
    unsigned int m_ring_n;
    WTA4JetERingInfo m_ERingInfo;
};

inline void WTAJet::MergeConstituent(WTATrigObj& tower)
{
    this->pt(this->pt() + tower.pt());// Scalar sum, WTA
    m_ConstituentList.push_back(tower);
}

inline void WTAJet::MergeConstituent(WTATrigObj* tower)
{
    this->pt(this->pt() + tower->pt());// Scalar sum, WTA
    m_ConstituentList.push_back(*tower);
}

inline void WTAJet::MergeWTAJet(WTAJet &targetjet)
{
    WTATrigObj tmptower(targetjet.pt(), targetjet.eta(), targetjet.phi(), targetjet.m());
    this->pt(this->pt() + tmptower.pt()); // Scalar sum, WTA
    std::vector<WTATrigObj> tmp_const_list = targetjet.GetConstituentList();
    m_ConstituentList.insert(m_ConstituentList.end(), tmp_const_list.begin(), tmp_const_list.end());
}

inline void WTAJet::PopOutLastConstituent()
{
    WTATrigObj lastconst = m_ConstituentList.back(); //std::vector::back() returns the last element
    this->pt(this->pt() - lastconst.pt()); // Scalar subtraction, WTA
    m_ConstituentList.pop_back(); // Popback the last element
}

#ifdef FLOATING_POINT_SIMULATION
inline unsigned int WTAJet::GetRingN(const WTATrigObj& tower) const {
    eta_t d_eta = this->d_eta(tower);
    phi_t d_phi = this->d_phi_MPI_PI(tower);
    tech_t dr = std::sqrt(d_eta*d_eta + d_phi*d_phi);
    unsigned int ring_n = static_cast<unsigned int>(std::ceil(dr / 0.1)); // Each ring has dR of 0.1
    return ring_n;
}
#elif defined(BITWISE_SIMULATION)
inline unsigned int WTAJet::GetRingN(const WTATrigObj& tower) const {
    eta_t d_eta = this->eta() > tower.eta() ? this->d_eta(tower) : tower.d_eta(*this); // ensure positive d_eta d_phi
    phi_t raw_dphi = abs(int32_t(this->phi()) - int32_t(tower.phi()));
    phi_t d_phi = std::min(raw_dphi, PHI_LEN - raw_dphi);
    unsigned int ring_n = std::max(d_eta, static_cast<eta_t>(d_phi.raw()));
    return ring_n;
}
#else
inline unsigned int WTAJet::GetRingN(const WTATrigObj& tower) const {
    eta_t d_eta = this->eta() > tower.eta() ? this->d_eta(tower) : tower.d_eta(*this); // ensure positive d_eta d_phi
    phi_t raw_dphi = abs(int32_t(this->phi()) - int32_t(tower.phi()));
    phi_t d_phi = std::min(raw_dphi, PHI_LEN - raw_dphi);
    unsigned int ring_n = std::max(d_eta, static_cast<eta_t>(d_phi)); // No .raw() in long long int
    return ring_n;
}
#endif

inline void WTAJet::CreateERingInfo(){
    // This function is ideally called after all jet clustering is done

    // Check if the constituent list is empty
    if(m_ConstituentList.empty()){
        std::cerr << "Error: Constituent list is empty. Cannot create ERingInfo." << std::endl;
        return;
    }

    // Reset the ERingInfo
    WTA4JetERingInfo ering_info;
    m_ERingInfo = ering_info;

    for(const auto& tower:m_ConstituentList){
        unsigned int ring_n = this->GetRingN(tower);
        m_ERingInfo.total_TobN = m_ERingInfo.total_TobN + 1;
        switch(ring_n){
            case 0:
                m_ERingInfo.ring0_Et = m_ERingInfo.ring0_Et + tower.pt();
                m_ERingInfo.ring0_TobN = m_ERingInfo.ring0_TobN + 1;
                break;
            case 1:
                m_ERingInfo.ring1_Et = m_ERingInfo.ring1_Et + tower.pt();
                m_ERingInfo.ring1_TobN = m_ERingInfo.ring1_TobN + 1;
                break;
            case 2:
                m_ERingInfo.ring2_Et = m_ERingInfo.ring2_Et + tower.pt();
                m_ERingInfo.ring2_TobN = m_ERingInfo.ring2_TobN + 1;
                break;
            case 3:
                m_ERingInfo.ring3_Et = m_ERingInfo.ring3_Et + tower.pt();
                m_ERingInfo.ring3_TobN = m_ERingInfo.ring3_TobN + 1;
                break;
            case 4:
                m_ERingInfo.ring4_Et = m_ERingInfo.ring4_Et + tower.pt();
                m_ERingInfo.ring4_TobN = m_ERingInfo.ring4_TobN + 1;
                break;
            default:
                break;
        }
    }
}

inline void WTAJet::PrintERingInfo() const {
    std::cout << "WTAJet ERing Info:" << std::endl;
    std::cout << " Raw Jet Et: " << this->pt() << " , Constituent Count: " << this->GetConstituentCount() << std::endl;
    std::cout << "  Total TobN: " << m_ERingInfo.total_TobN << std::endl;
    std::cout << "  Ring 0: Et = " << m_ERingInfo.ring0_Et << ", TobN = " << m_ERingInfo.ring0_TobN << std::endl;
    std::cout << "  Ring 1: Et = " << m_ERingInfo.ring1_Et << ", TobN = " << m_ERingInfo.ring1_TobN << std::endl;
    std::cout << "  Ring 2: Et = " << m_ERingInfo.ring2_Et << ", TobN = " << m_ERingInfo.ring2_TobN << std::endl;
    std::cout << "  Ring 3: Et = " << m_ERingInfo.ring3_Et << ", TobN = " << m_ERingInfo.ring3_TobN << std::endl;
    std::cout << "  Ring 4: Et = " << m_ERingInfo.ring4_Et << ", TobN = " << m_ERingInfo.ring4_TobN << std::endl;
}

#endif
