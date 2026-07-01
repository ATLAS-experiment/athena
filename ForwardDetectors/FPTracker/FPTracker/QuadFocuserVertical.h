/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPTRACKER_QUADFOCUSERVERTICAL_H
#define FPTRACKER_QUADFOCUSERVERTICAL_H
#include "IQuadFocuser.h"
namespace FPTracker{
  class Point;
  class TransversePoint;

 class QuadFocuserVertical:public IQuadFocuser{
  public:

    void focus(double, double, double, 
	       const TransversePoint& displacement, 
	       const Point& direction);

    double xe()         const;
    double xae()        const;
    double ye()         const;
    double yae()        const;
    std::string label() const;
  private:

    static const std::string s_label;

    double m_xe = 0;
    double m_xae = 0;
    double m_ye = 0;
    double m_yae = 0;
  };
}
#endif
