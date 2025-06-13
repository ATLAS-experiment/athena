/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPTACKER_MAGNET_H
#define FPTACKER_MAGNET_H

#include "IBeamElement.h"
#include "Point.h"
#include "FPTrackerConstants.h"
#include "IBender.h"
#include "Particle.h"
#include <memory>
#include <vector>
#include <iosfwd>

namespace FPTracker{
  class IParticle;
  class TransversePoint;
  class Magnet:public IBeamElement{
  public:
    Magnet(double x,
	   double y,
	   double center,
	   double strength,
	   double length,
	   int apertype,  
	   double aper_A1,
	   double aper_A2,
	   double aper_A3,
	   double aper_A4,
	   double xb,
	   Side side,
	   IBender* bender, 
	   const std::string& label
	   );
    /*
    Magnet(const Magnet&);
    Magnet& operator=(Magnet);
    */

    double frontFace()    const;
    double rearFace()     const;
    double zabspos()      const;
    double zsignedpos()   const;
    double centerZ()      const; //magnet center
    Point  position()     const;
    Side   side()         const;

    bool   isEndElement() const;
    std::string str()     const;

    std::string label()              const;

    void track(IParticle&)           const;
    void calibrate(IParticle&);

    IBeamElement::ConstPtr_t clone() const;


    typedef std::shared_ptr< Magnet >     Ptr_t;
    typedef std::vector< Ptr_t >            Container_t;
    
    enum Type {hbDipole, hfQuadrupole, vfQuadrupole, vbDipole, notAMagnet};




  private:

    Point               m_center;
    double              m_frontFace;
    double              m_rearFace;
    double              m_strength;
    int                 m_apertype;
    double              m_aper_A1;
    double              m_aper_A2;
    double              m_aper_A3;
    double              m_aper_A4;
    double              m_xb;
    Side                m_side;
    IBender * m_bender{};
    std::string m_label;


    bool isOutOfAperture( IParticle& ) const;
    double frontface()                 const;
    void swap(Magnet&); 
  };


  std::ostream& operator<<(std::ostream&, const Magnet&);
  
}
#endif
