/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPTRACKER_QUADFOCUSERNULL_H
#define FPTRACKER_QUADFOCUSERNULL_H
#include "IQuadFocuser.h"
namespace FPTracker{
  class TransversePoint;
 class QuadFocuserNull:public IQuadFocuser{
  public:
    void focus(double, double, double, const TransversePoint& direction, const Point& displacment) ;
    double xe()  const;
    double xae() const;
    double ye()  const;
    double yae() const;
    std::string label() const;

  private:

    const static std::string s_label;
    double m_xe = 0;
    double m_xae = 0;
    double m_ye = 0;
    double m_yae = 0;
  };
}
#endif
