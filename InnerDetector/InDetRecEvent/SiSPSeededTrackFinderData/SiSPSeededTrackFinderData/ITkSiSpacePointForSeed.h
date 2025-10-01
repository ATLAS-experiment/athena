// -*- C++ -*-

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
//  Header file for class ITk::SiSpacePointForSeed
/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class space points for seed maker
/////////////////////////////////////////////////////////////////////////////////
// Version 1.0 18/11/2004 I.Gavrilenko
/////////////////////////////////////////////////////////////////////////////////

#ifndef ITkSiSpacePointForSeed_h
#define ITkSiSpacePointForSeed_h

#include <cmath>
#include <span>

namespace Trk {
  class SpacePoint;
  class Surface;
}

namespace InDet {
  class SiCluster;
}

namespace ITk
{
  class SiSpacePointForSeed final{

    /////////////////////////////////////////////////////////////////////////////////
    // Public methods:
    /////////////////////////////////////////////////////////////////////////////////

  public:

    SiSpacePointForSeed() = default;
    SiSpacePointForSeed(const SiSpacePointForSeed&) = default;
    SiSpacePointForSeed& operator  = (const SiSpacePointForSeed&) =default;
    SiSpacePointForSeed(SiSpacePointForSeed&&) noexcept = default;
    SiSpacePointForSeed& operator  = (SiSpacePointForSeed&&) noexcept =default;
    ~SiSpacePointForSeed() = default;
    
    SiSpacePointForSeed(const Trk::SpacePoint*, std::span<float const, 15>);
    SiSpacePointForSeed(const Trk::SpacePoint*, std::span<float const, 15>, std::span<float const, 15>);

    void set(const Trk::SpacePoint*,std::span<float const, 15>)  ;
    void set(const Trk::SpacePoint*,std::span<float const, 15>,std::span<float const, 15>);
    void setQuality(float);
    void setParam(float p) {  m_param = p; }
    void setDR(float dr) { m_dR = dr;}
    void setDZDR(float dzdr) { m_dzdr = dzdr; }
    void setEta(float eta) { m_eta = eta; }
    void setScorePenalty(float score) {m_scorePenalty = score;}
    void setPt(const float pt) { m_pt = pt; }

    const Trk::SpacePoint* spacepoint = nullptr ;
    float          x() const {return m_x;}
    float          y() const {return m_y;}
    float          z() const {return m_z;}
    float     radius() const {return m_r;}
          float         phi() const {return atan2(m_y,m_x);}
    float       covr() const {return m_covr;}
    float       covz() const {return m_covz;}
    float      param() const {return m_param;}
    float    quality() const {return m_q ;}
    float       dzdr() const {return m_dzdr;}
    float        eta() const {return m_eta;}
    float         pt() const {return m_pt;}
    float      scorePenalty() const {return m_scorePenalty;} /// penalty term in the seed score
    float         dR() const {return m_dR;} /// distance between top and central SP
    const Trk::Surface* sur() const {return m_su;}
    const Trk::Surface* sun() const {return m_sn;}
    const float*  b0() const {return m_b0;}
    const float*  b1() const {return m_b1;}
    const float*  dr() const {return m_dr;}
    const float*  r0() const {return m_r0;}

    bool coordinates(const float*,float*);

  private:

    float m_x{}   ; // x-coordinate in beam system coordinates
    float m_y{}   ; // y-coordinate in beam system coordinates
    float m_z{}   ; // z-coordinate in beam system coordinetes
    float m_r{}   ; // radius       in beam system coordinates
    float m_covr{}; //
    float m_covz{}; //
    float m_param{};
    float m_q{}   ;
    float m_scorePenalty{}; /// penalty term in the seed score
    float m_dR{};
    float m_eta{} ;
    float m_pt{}  ;
    float m_dzdr{};


    float m_b0[3]{};
    float m_b1[3]{};
    float m_dr[3]{};
    float m_r0[3]{};

    const Trk::Surface* m_su = nullptr;
    const Trk::Surface* m_sn = nullptr;
  };

} // end of name space ITk

#endif  // ITkSiSpacePointForSeed_h
