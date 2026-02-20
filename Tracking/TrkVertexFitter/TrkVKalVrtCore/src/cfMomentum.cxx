/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkVKalVrtCore/cfMomentum.h"
#include "TrkVKalVrtCore/CommonPars.h"
#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"
#include "TrkVKalVrtCore/VKalVrtBMag.h"
#include <cmath>
#include <array>
#include <iostream>

namespace Trk {

void vkPerigeeToP( const double *perig3, double *pp, double effectiveBMAG)
{
    double constB = effectiveBMAG  * vkalMagCnvCst;
    double phiv = perig3[1];
    double pt   = std::abs( constB/perig3[2] );
    pp[0] = pt * cos(phiv);
    pp[1] = pt * sin(phiv);
    pp[2] = pt / tan(perig3[0]);
}

std::array<double, 4> getFitParticleMom( const VKTrack * trk, const VKVertex *vk)
{
    std::array<double, 4> p{};
    double fieldPos[3];
    fieldPos[0]=vk->refIterV[0]+vk->fitV[0];
    fieldPos[1]=vk->refIterV[1]+vk->fitV[1];
    fieldPos[2]=vk->refIterV[2]+vk->fitV[2];
    double vBx,vBy,vBz,magConst;
    Trk::vkalMagFld::getMagFld(fieldPos[0], fieldPos[1], fieldPos[2],vBx,vBy,vBz,(vk->vk_fitterControl).get());
    magConst = Trk::vkalMagFld::getEffField(vBx, vBy, vBz, trk->fitP[1], trk->fitP[0]) * Trk::vkalMagFld::getCnvCst();

    double cth = 1. / tan( trk->fitP[0]);
    double phi      = trk->fitP[1];
    double pt       = std::abs( magConst/trk->fitP[2] );
    double m   = trk->getMass();
    p[0] = pt * cos(phi);
    p[1] = pt * sin(phi);
    p[2] = pt * cth;
    p[3] = sqrt(p[0]*p[0]+p[1]*p[1]+p[2]*p[2] + m*m );
    return p;
}
std::array<double, 4> getFitParticleMom(const VKTrack * trk, double effectiveBMAG)
{
    std::array<double, 4> p{};
    double magConst = effectiveBMAG  * vkalMagCnvCst;

    double cth = 1. / tan( trk->fitP[0]);
    double phi      = trk->fitP[1];
    double pt       = std::abs( magConst/trk->fitP[2] );
    double m   = trk->getMass();
    p[0] = pt * cos(phi);
    p[1] = pt * sin(phi);
    p[2] = pt * cth;
    p[3] = sqrt(p[0]*p[0]+p[1]*p[1]+p[2]*p[2] + m*m );
    return p;
}

std::array<double, 4> getIniParticleMom( const VKTrack * trk, const VKVertex *vk)
{
    std::array<double, 4> p{};
    double vBx,vBy,vBz,magConst;
    Trk::vkalMagFld::getMagFld(vk->refIterV[0],vk->refIterV[1],vk->refIterV[2],vBx,vBy,vBz,(vk->vk_fitterControl).get());
    magConst = Trk::vkalMagFld::getEffField(vBx, vBy, vBz, trk->iniP[1], trk->iniP[0]) * Trk::vkalMagFld::getCnvCst();

    double cth = 1. / tan( trk->iniP[0]);
    double phi      =      trk->iniP[1];
    double pt       = std::abs( magConst/trk->iniP[2] );
    double m   = trk->getMass();
    p[0] = pt * cos(phi);
    p[1] = pt * sin(phi);
    p[2] = pt * cth;
    p[3] = sqrt(p[0]*p[0]+p[1]*p[1]+p[2]*p[2] + m*m );
    return p;
}
std::array<double, 4> getIniParticleMom(const VKTrack * trk, double effectiveBMAG)
{
    std::array<double, 4> p{};
    double magConst = effectiveBMAG  * vkalMagCnvCst;

    double cth = 1. / tan( trk->iniP[0]);
    double phi      =      trk->iniP[1];
    double pt       = std::abs( magConst/trk->iniP[2] );
    double m   = trk->getMass();
    p[0] = pt * cos(phi);
    p[1] = pt * sin(phi);
    p[2] = pt * cth;
    p[3] = sqrt(p[0]*p[0]+p[1]*p[1]+p[2]*p[2] + m*m );
    return p;
}


std::array<double, 4> getCnstParticleMom( const VKTrack * trk, const VKVertex *vk )
{
    std::array<double, 4> p{};
    double cnstPos[3];
    cnstPos[0]=vk->refIterV[0]+vk->cnstV[0];
    cnstPos[1]=vk->refIterV[1]+vk->cnstV[1];
    cnstPos[2]=vk->refIterV[2]+vk->cnstV[2];
    double vBx,vBy,vBz,magConst;
    Trk::vkalMagFld::getMagFld(cnstPos[0],cnstPos[1],cnstPos[2],vBx,vBy,vBz,(vk->vk_fitterControl).get());
    magConst = Trk::vkalMagFld::getEffField(vBx, vBy, vBz, trk->cnstP[1], trk->cnstP[0]) * Trk::vkalMagFld::getCnvCst();

    double cth = 1. / tan( trk->cnstP[0]);
    double phi      =      trk->cnstP[1];
    double pt       = std::abs( magConst/trk->cnstP[2] );
    double m   = trk->getMass();
    p[0] = pt * cos(phi);
    p[1] = pt * sin(phi);
    p[2] = pt * cth;
    p[3] = sqrt(p[0]*p[0]+p[1]*p[1]+p[2]*p[2] + m*m );
    return p;
}
std::array<double, 4> getCnstParticleMom(const VKTrack * trk, double effectiveBMAG )
{
    std::array<double, 4> p{};
    double magConst = effectiveBMAG  * vkalMagCnvCst;

    double cth = 1. / tan( trk->cnstP[0]);
    double phi      =      trk->cnstP[1];
    double pt       = std::abs( magConst/trk->cnstP[2] );
    double m   = trk->getMass();
    p[0] = pt * cos(phi);
    p[1] = pt * sin(phi);
    p[2] = pt * cth;
    p[3] = sqrt(p[0]*p[0]+p[1]*p[1]+p[2]*p[2] + m*m );
    return p;
}

}
