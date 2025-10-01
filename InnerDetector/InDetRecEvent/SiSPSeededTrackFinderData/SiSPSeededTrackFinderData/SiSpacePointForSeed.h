// -*- C++ -*-

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
//  Header file for class SiSpacePointForSeed
/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class space points for seed maker 
/////////////////////////////////////////////////////////////////////////////////
// Version 1.0 18/11/2004 I.Gavrilenko
/////////////////////////////////////////////////////////////////////////////////

#ifndef SiSpacePointForSeed_h
#define SiSpacePointForSeed_h

#include <cmath>
#include <span>

namespace Trk {
  class SpacePoint;
  class Surface;
}

namespace InDet {
  class SiCluster;

  class SiSpacePointForSeed final{
    
    /////////////////////////////////////////////////////////////////////////////////
    // Public methods:
    /////////////////////////////////////////////////////////////////////////////////
    
  public:
    
    SiSpacePointForSeed() = default;
    SiSpacePointForSeed(const SiSpacePointForSeed&) = default;
    SiSpacePointForSeed& operator  = (const SiSpacePointForSeed&) = default;
    SiSpacePointForSeed(SiSpacePointForSeed&&) noexcept = default;
    SiSpacePointForSeed& operator  = (SiSpacePointForSeed&) noexcept = default;
    ~SiSpacePointForSeed() = default;

    SiSpacePointForSeed(const Trk::SpacePoint*,std::span<float const, 3>);
    SiSpacePointForSeed(const Trk::SpacePoint*,std::span<float const, 3>, std::span<float const, 4>);

    void set(const Trk::SpacePoint*,std::span<float const, 3>);
    void set(const Trk::SpacePoint*,std::span<float const, 3>, std::span<float const, 4>);
    void setQuality(float q) { if(q <= m_q) m_q = q; }
    void setParam(float p) {m_param = p;}
    void setD0(float d0) { m_d0 = d0; }
    void setEta(float eta) { m_eta = eta; }
    void setDZDR(float dzdr) { m_dzdr = dzdr; }
    void setPt(float pt) { m_pt = pt; }
    void setScorePenalty(float par) {m_scorePenalty=par;}

    const Trk::SpacePoint* spacepoint = nullptr; 
    float          x() const {return m_x;}
    float          y() const {return m_y;}
    float          z() const {return m_z;}
    float     radius() const {return m_r;}
    float         phi() const {return std::atan2(m_y,m_x);}
    float       covr() const {return m_covr;}
    float       covz() const {return m_covz;}
    float         d0() const {return m_d0;}
    float        eta() const {return m_eta;}
    float      param() const {return m_param;} /// impact parameter
    float      scorePenalty() const {return m_scorePenalty;} /// penalty term in the seed score
    float    quality() const {return m_q ;}      /// quality of the best seed this candidate was seen on 
    float       dzdr() const {return m_dzdr;}
    float         Pt() const {return m_pt;}
    const Trk::Surface* sur() const {return m_su;}
    const Trk::Surface* sun() const {return m_sn;}

  private:
    
    float m_x{}   ; // x-coordinate in beam system coordinates  
    float m_y{}   ; // y-coordinate in beam system coordinates
    float m_z{}   ; // z-coordinate in beam system coordinetes
    float m_r{}   ; // radius       in beam system coordinates
    float m_covr{}; //
    float m_covz{}; //
    float m_d0 = 0.f;
    float m_eta = 0.f;
    float m_dzdr = 0.f;
    float m_pt = 0.f;
    float m_param{};  /// impact parameter
    float m_scorePenalty=0.f; /// penalty term in the seed score
    float m_q{};   /// quality of the best seed this candidate was seen on
    const Trk::Surface* m_su = nullptr;
    const Trk::Surface* m_sn = nullptr;
  };
 
} // end of name space

#endif  // SiSpacePointForSeed_h
