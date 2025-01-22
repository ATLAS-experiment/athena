/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtCalibData/SamplePointUtils.h"

#include "MdtCalibData/IRtRelation.h"
#include "MdtCalibData/IRtResolution.h"
#include "MdtCalibData/ITrRelation.h"
#include "MuonCalibMath/UtilFunc.h"

#include "MdtCalibData/RtResolutionLookUp.h"

#include <algorithm>
#include <ranges>

namespace MuonCalib{
    std::vector<SamplePoint> fetchDataPoints(const IRtRelation& rtRel, const double relUnc) {
        assert(relUnc >0);
        std::vector<SamplePoint> points{};
        const double stepWidth = rtRel.tBinWidth();
        double time = rtRel.tLower();
        while (time <= rtRel.tUpper()){
            const double r = rtRel.radius(time);
            points.emplace_back(time, r, relUnc);
            time+=stepWidth;
        }
        return points;
    }
    std::vector<SamplePoint> fetchDataPoints(const IRtRelation& rtRel, const IRtResolution& reso) {
        std::vector<SamplePoint> points{};
        const double stepWidth = rtRel.tBinWidth();
        double time = rtRel.tLower();
        while (time <= rtRel.tUpper()){
            points.emplace_back(time,rtRel.radius(time), reso.resolution(time));
            time+=stepWidth;
        }
        return points;
    }
    std::vector<SamplePoint> resoFromRadius(const std::vector<SamplePoint>& points,
                                            const double uncert) {
        std::vector<SamplePoint> outPoints{};
        outPoints.reserve(points.size());
        for (const SamplePoint& point: points) {
            outPoints.emplace_back(point.x2(),point.error(), uncert);
        }
        return outPoints;
    }
    std::vector<SamplePoint> fetchResolution(const std::vector<SamplePoint>& points,
                                             const double uncert) {
        std::vector<SamplePoint> outPoints{};
        outPoints.reserve(points.size());
        for (const SamplePoint& point: points) {
            outPoints.emplace_back(point.x1(),point.error(), uncert);
        }
        return outPoints;
    }


    std::vector<SamplePoint> swapCoordinates(const std::vector<SamplePoint>& inPoints,
                                             const IRtRelation& rtRel) {
        
        std::vector<SamplePoint> outPoints{};
        outPoints.reserve(inPoints.size());
        /*
         * Let's suppose that dY = f^{\prime} dX
         */
        for (const SamplePoint& in : inPoints) {
            const double fPrime = std::abs(rtRel.driftVelocity(in.x1()));
            outPoints.emplace_back(in.x2(), in.x1(), in.error() / (fPrime > 0 ? fPrime : 1.));
        }
        std::ranges::sort(outPoints, [](const SamplePoint& a, const SamplePoint& b) {
                                        return a.x1() < b.x1();
                                    });
        return outPoints;
    }
    std::vector<SamplePoint> normalizeDomain(const std::vector<SamplePoint>& dataPoints) {
        const auto [minX, maxX] = interval(dataPoints);
        std::vector<SamplePoint> normedPoints{dataPoints};
        for (unsigned k = 0; k < dataPoints.size(); ++k) { 
            normedPoints[k].set_x1(mapToUnitInterval(normedPoints[k].x1(), minX, maxX)); 
        }
        return normedPoints;
    }
    std::pair<double, double> minMax(const std::vector<SamplePoint>& points) {
        double min{std::numeric_limits<double>::max()}, max{-std::numeric_limits<double>::max()};
        for (const SamplePoint& point: points) {
            min = std::min(min, point.x2());
            max = std::max(max, point.x2());
        }        
        return std::make_pair(min,max);
    }
    std::pair<double, double> interval(const std::vector<SamplePoint>& points) {
        double min{std::numeric_limits<double>::max()}, max{-std::numeric_limits<double>::max()};   
        for (const SamplePoint& point: points) {
            min = std::min(min, point.x1());
            max = std::max(max, point.x1());
        }
        return std::make_pair(min, max);
    }
    double calculateChi2(const std::vector<SamplePoint>& dataPoints,
                         const IRtRelation& rtRel){
        double chi2{0.};
        for (const SamplePoint& point: dataPoints){
            chi2+=std::pow((point.x2() - rtRel.radius(point.x1())) / point.error(), 2);
        }
        return chi2;
    }
    double calculateChi2(const std::vector<SamplePoint>& dataPoints,
                         const ITrRelation& trRel) {
        double chi2{0.};
        for (const SamplePoint& point: dataPoints){
            chi2+=std::pow((point.x2() - trRel.driftTime(point.x1()).value_or(2.*trRel.maxRadius()) ) / point.error(), 2);
        }
        return chi2;
    }
    double calculateChi2(const std::vector<SamplePoint>& dataPoints,
                         const IRtResolution& rtReso) {
        double chi2{0.};
        for (const SamplePoint& point: dataPoints){
            chi2+=std::pow((point.x2() - rtReso.resolution(point.x1())) / point.error(), 2);
        }
        return chi2;
    }
}