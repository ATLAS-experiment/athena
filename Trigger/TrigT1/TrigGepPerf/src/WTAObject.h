#ifndef WTAObject_h
#define WTAObject_h

#include <iostream>
#include <algorithm>

#ifdef BITWISE_SIMULATION
// 1: Integer type for bitwise simulation
typedef long long int IntOrFloat;
const IntOrFloat PI = 32;
const IntOrFloat R2PAR = 16;
#elif defined(FLOATING_POINT_SIMULATION)
// 2: Floating point for athena simulation
// typedef double IntOrFloat;
typedef float IntOrFloat;
const IntOrFloat PI = 3.14159265;
const IntOrFloat R2PAR = 0.16;
#else
#error "Simulation type not defined. Define either BITWISE_SIMULATION or FLOATING_POINT_SIMULATION."
#endif

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
        WTATrigObj(IntOrFloat pt = 0, IntOrFloat eta = 0, IntOrFloat phi = 0, IntOrFloat m = 0, int idx = -99)
        : m_pt(pt), m_eta(eta), m_phi(phi), m_m(m), m_idx(idx){}; // Constructor

        IntOrFloat pt()  const     {return m_pt;};
        void   pt(IntOrFloat pt)   {m_pt = pt;};
        IntOrFloat eta() const     {return m_eta;};
        void   eta(IntOrFloat eta) {m_eta = eta;};
        IntOrFloat phi() const     {return m_phi;};
        void   phi(IntOrFloat phi) {m_phi = phi;};
        IntOrFloat m()   const     {return m_m;};
        void   m(IntOrFloat m)     {m_m = m;};
        int        idx() const     {return m_idx;};
        void   idx(int idx)        {m_idx = idx;};

        IntOrFloat d_phi_0_2PI(const WTATrigObj& o2);
        IntOrFloat d_phi_MPI_PI(const WTATrigObj& o2);
        IntOrFloat d_eta(const WTATrigObj& o2){IntOrFloat tmp = m_eta - o2.eta(); return tmp;}
        IntOrFloat dR2(const WTATrigObj& o2);
        bool IsAssocdR(WTATrigObj& tower, IntOrFloat dr2);

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
        IntOrFloat m_pt;
        IntOrFloat m_eta;
        IntOrFloat m_phi;
        IntOrFloat m_m;
        int m_idx;

};

inline IntOrFloat WTATrigObj::d_phi_0_2PI(const WTATrigObj& o2)
{
    IntOrFloat tmp = m_phi - o2.phi();
    while(tmp < 0)tmp += 2*PI;
    while(tmp > 2*PI)tmp -= 2*PI;
    return tmp;
}
inline IntOrFloat WTATrigObj::d_phi_MPI_PI(const WTATrigObj& o2)
{
    IntOrFloat tmp = m_phi - o2.phi();
    while(tmp < -PI)tmp += 2*PI;
    while(tmp > PI)tmp -= 2*PI;
    return tmp;
}
inline IntOrFloat WTATrigObj::dR2(const WTATrigObj& o2)
{
    IntOrFloat d_phi = d_phi_MPI_PI(o2);
    IntOrFloat d_eta_tmp = d_eta(o2);
    IntOrFloat tmp = (d_phi * d_phi) + (d_eta_tmp * d_eta_tmp);
    return tmp;
}

inline bool WTATrigObj::IsAssocdR(WTATrigObj& tower, IntOrFloat dr2){
    auto dR2 = this->dR2(tower);
    return dR2 <= dr2;
}

class WTAJet : public WTATrigObj{       // The class
  public:             // Access specifier
    WTAJet (IntOrFloat pt = 0, IntOrFloat eta = 0, IntOrFloat phi = 0, IntOrFloat m = 0, int idx = -99, IntOrFloat jet_dr2 = R2PAR) :
        WTATrigObj(pt, eta, phi, m, idx){
            m_Seed = WTATrigObj(pt, eta, phi, m, idx);
            m_Jet_dR2 = jet_dr2; 
            m_ConstituentList.clear();
            m_ConstituentList.push_back(m_Seed);
        };

    const WTATrigObj& GetSeed() const {return m_Seed;};
    void MergeConstituent(WTATrigObj &tower);
    void MergeConstituent(WTATrigObj *tower);
    void MergeWTAJet(WTAJet &targetjet);
    void PopOutLastConstituent();
    const std::vector<WTATrigObj>& GetConstituentList() const {return m_ConstituentList;};
    unsigned int GetConstituentCount() const {return m_ConstituentList.size();}; // Need to quickly access the jet's constituent number

  private:
    WTATrigObj m_Seed;
    std::vector<WTATrigObj> m_ConstituentList;
    IntOrFloat m_Jet_dR2;
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

#endif
