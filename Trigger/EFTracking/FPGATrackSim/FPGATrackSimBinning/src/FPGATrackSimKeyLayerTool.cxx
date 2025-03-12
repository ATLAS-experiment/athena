// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimKeyLayerTool.h"

//
//
// Tool for doing Key Layer Math
//
//

std::pair<double, double> FPGATrackSimKeyLayerTool::getXY(double r, double phi) const
{
    return std::pair<double, double>(r * cos(phi), r * sin(phi));
}

std::pair<double, double> FPGATrackSimKeyLayerTool::getRotationAngles(const std::pair<double, double>& xy1, const std::pair<double, double>& xy2) const
{
    double dr12x = xy2.first - xy1.first;
    double dr12y = xy2.second - xy1.second;
    double dr12mag = std::hypot(dr12x, dr12y);
    double cos_rot = dr12y / dr12mag;
    double sin_rot = -dr12x / dr12mag;
    return std::pair<double, double>(cos_rot, sin_rot);
}

std::pair<double, double> FPGATrackSimKeyLayerTool::rotateXY(const std::pair<double, double>& xy, const std::pair<double, double>& ang) const
{
    return std::pair<double, double>(xy.first * ang.first + xy.second * ang.second,
                                     -xy.first * ang.second + xy.second * ang.first);
}

FPGATrackSimKeyLayerTool::rotatedConfig FPGATrackSimKeyLayerTool::getRotatedConfig(const KeyLyrPars &keypars) const
{    
    rotatedConfig result;

    auto xy1 = getXY(m_R1, keypars.phi1);
    auto xy2 = getXY(m_R2, keypars.phi2);

    result.rotang = getRotationAngles(xy1, xy2);
    result.xy1p = rotateXY(xy1, result.rotang);
    result.xy2p = rotateXY(xy2, result.rotang);
    result.y = result.xy2p.second - result.xy1p.second;

    return result;
}

std::pair<double, double> FPGATrackSimKeyLayerTool::getRotatedHit(const std::pair<double, double>& rotang, const FPGATrackSimHit *hit) const
{
    auto xyh = getXY(hit->getR(), hit->getGPhi());
    return rotateXY(xyh, rotang);
}

FPGATrackSimKeyLayerTool::KeyLyrPars FPGATrackSimKeyLayerTool::trackParsToKeyPars(const FPGATrackSimTrackPars &pars) const
{
    KeyLyrPars retv;
    retv.z1 = FPGATrackSimBinUtil::GeomHelpers::zFromPars(m_R1, pars);
    retv.z2 = FPGATrackSimBinUtil::GeomHelpers::zFromPars(m_R2, pars);
    retv.phi1 = FPGATrackSimBinUtil::GeomHelpers::phiFromPars(m_R1, pars);
    retv.phi2 = FPGATrackSimBinUtil::GeomHelpers::phiFromPars(m_R2, pars);

    double Rinv = pars[FPGATrackSimTrackPars::IHIP] * (2.0 * FPGATrackSimBinUtil::GeomHelpers::CurvatureConstant);
    if (Rinv != 0)
    {
        double R = 1 / Rinv;
        auto xy1 = getXY(m_R1, retv.phi1);
        auto xy2 = getXY(m_R2, retv.phi2);
        double ysqr = (xy2.first - xy1.first) * (xy2.first - xy1.first) + (xy2.second - xy1.second) * (xy2.second - xy1.second);
        double sign = (R > 0) - (R < 0);
        retv.xm = -1 * sign * (std::abs(R) - sqrt(R * R - ysqr / 4.0));
    }
    else
    {
        retv.xm = 0;
    }

    return retv;
}

FPGATrackSimTrackPars FPGATrackSimKeyLayerTool::keyParsToTrackPars(const KeyLyrPars &keypars) const
{

    FPGATrackSimTrackPars pars;
    pars[FPGATrackSimTrackPars::IZ0] = (keypars.z1 * m_R2 - keypars.z2 * m_R1) / (m_R2 - m_R1);
    pars[FPGATrackSimTrackPars::IETA] = FPGATrackSimBinUtil::GeomHelpers::EtaFromTheta(std::atan2(m_R2 - m_R1, keypars.z2 - keypars.z1));

    // This is the exact math which is a bit messy
    // if you want the math contact lipeles@sas.upenn.edu

    double xm = keypars.xm;

    auto rotated_coords = getRotatedConfig(keypars);
    auto xy1p = rotated_coords.xy1p;
    auto rotang = rotated_coords.rotang;
    auto y = rotated_coords.y;

    // reverse rotation
    auto revang = rotang;
    revang.second = -rotang.second;

    if (xm != 0)
    {
        double Rinv = 2 * xm / (xm * xm + (y / 2) * (y / 2));
        double d = (y * y / 4.0 - xm * xm) / (2.0 * xm);
        double sign = (xm > 0) - (xm < 0);

        pars[FPGATrackSimTrackPars::IHIP] = -1 * Rinv / (2.0 * FPGATrackSimBinUtil::GeomHelpers::CurvatureConstant);

        std::pair<double, double> xycp(-d + xy1p.first, y / 2.0 + xy1p.second);

        pars[FPGATrackSimTrackPars::ID0] = -1 * sign * (std::abs(1 / Rinv) - std::hypot(xycp.first, xycp.second));

        auto xyc = rotateXY(xycp, revang);

        pars[FPGATrackSimTrackPars::IPHI] = std::atan2(sign * -xyc.first, sign * xyc.second);
    }
    else
    {
        pars[FPGATrackSimTrackPars::IHIP] = 0.0;
        pars[FPGATrackSimTrackPars::ID0] = -1 * xy1p.first;
        pars[FPGATrackSimTrackPars::IPHI] = std::atan2(rotang.first, -rotang.second);
    }

    return pars;
}

double FPGATrackSimKeyLayerTool::zExpected(const KeyLyrPars& keypars, double r) const
{
    return (keypars.z2 - keypars.z1) / (m_R2 - m_R1) * (r - m_R1) + keypars.z1;
}

// takes hit position and calculated what xm should be for that hit
double FPGATrackSimKeyLayerTool::xmForHit(const KeyLyrPars& keypars, const FPGATrackSimHit *hit) const
{
    auto rotated_coords = getRotatedConfig(keypars);
    auto xy1p = rotated_coords.xy1p;
    auto rotang = rotated_coords.rotang;
    auto y = rotated_coords.y;

    auto xyhp = getRotatedHit(rotang, hit);
    double xh = xyhp.first - xy1p.first;
    double yh = xyhp.second - xy1p.second;

    // use taylor expanded xm calculation
    double sign = ((yh > 0) && (yh < y)) ? 1 : -1;
    if ((std::abs(yh) < std::abs(xh)) || (std::abs(y - yh) < std::abs(xh)))
    {
        return ((xh > 0) ? 1 : -1) * 100000;
    }

    double xm_taylor = sign * y * y / (y * y - 4 * xh * xh - 4 * (yh - y / 2.0) * (yh - y / 2.0)) * xh;

    return xm_taylor;
}

// Find shift from nominal track to hit in the "x" direction 
double FPGATrackSimKeyLayerTool::deltaX(const KeyLyrPars& keypars, const FPGATrackSimHit *hit) const
{
    auto rotated_coords = getRotatedConfig(keypars);

    auto rotang = rotated_coords.rotang;

    auto xyhp = getRotatedHit(rotang, hit);

    double xh = xyhp.first-rotated_coords.xy1p.first;

    return xh - xExpected(keypars,hit);
}

// Find shift from nominal track to hit in the "x" direction 
double FPGATrackSimKeyLayerTool::xExpected(const KeyLyrPars& keypars, const FPGATrackSimHit *hit) const
{
    auto rotated_coords = getRotatedConfig(keypars);

    auto rotang = rotated_coords.rotang;
    auto y = rotated_coords.y;

    auto xyhp = getRotatedHit(rotang, hit);

    double yh = xyhp.second-rotated_coords.xy1p.second;

    return keypars.xm * (keypars.xm * keypars.xm + yh * (y - yh)) / (keypars.xm * keypars.xm + (y / 2) * (y / 2));
}


