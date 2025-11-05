/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonVisualizationHelpersR4/VisualizationHelpers.h"

#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include <format>

namespace MuonValR4{
    using namespace MuonR4;
    using namespace SegmentFit;

    std::vector<std::unique_ptr<TObject>> clone(const std::vector<std::unique_ptr<TObject>>& cloneMe) {
        std::vector<std::unique_ptr<TObject>> cloned{};
        for (auto& obj : cloneMe) {
            cloned.emplace_back(obj->Clone());
        }
        return cloned;
    }
    std::unique_ptr<TEllipse> drawDriftCircle(const Amg::Vector3D& center,
                                              const double radius, const int color,
                                              const int fillStyle) {
        auto ellipse = std::make_unique<TEllipse>(center.y(), center.z(), radius);
        ellipse->SetLineColor(color);
        ellipse->SetFillStyle(fillStyle);
        ellipse->SetLineWidth(1);
        ellipse->SetFillColorAlpha(color, 0.2);
        return ellipse;
    }
    std::unique_ptr<TArrow> drawArrow(const Amg::Vector3D& start, const Amg::Vector3D& dir,
                                      const int color, const int lineStyle, const int view){
        constexpr double arrowLength = 2.*Gaudi::Units::cm; 
        const Amg::Vector3D end = start + (arrowLength / std::hypot(dir[view], dir.z()) ) * dir;
        auto arrow = std::make_unique<TArrow>(start[view], start.z(), end[view], end.z(),0.01);
        arrow->SetLineColor(color);
        arrow->SetLineWidth(2);
        arrow->SetLineStyle(lineStyle);
        return arrow;
    }
    std::unique_ptr<TLatex> drawLabel(const std::string& text, 
                                      const double xPos, const double yPos,
                                      const unsigned int fontSize) {
        auto tl = std::make_unique<TLatex>(xPos, yPos, text.c_str());
        tl->SetNDC();
        tl->SetTextFont(43); 
        tl->SetTextSize(fontSize); 
        return tl;
    }
    std::unique_ptr<TBox> drawBox(const Amg::Vector3D& boxCenter,
                                   const double boxWidth, const double boxHeight,
                                   const int color, const int fillStyle,
                                   const int view) {
        return drawBox(boxCenter[view] - 0.5*boxWidth, boxCenter.z() - 0.5*boxHeight,
                       boxCenter[view] + 0.5*boxWidth, boxCenter.z() + 0.5*boxHeight,
                                    color, fillStyle); 
    }
    std::unique_ptr<TBox> drawBox(const double x1, const double y1, 
                                  const double x2, const double y2, 
                                   const int color, const int fillStyle) {
        auto box = std::make_unique<TBox>(x1,y1,x2,y2);
        box->SetFillColor(color);
        box->SetLineColor(color);
        box->SetFillStyle(fillStyle);
        box->SetFillColorAlpha(color, 0.8);
        return box;
    }
    std::unique_ptr<TLine> drawLine(const Parameters& pars,
                                    const double lowEnd, const double highEnd,
                                    const int color, const int lineStyle,
                                    const int view) {
        const auto [pos, dir] = makeLine(pars);
        const double x1 = (pos + Amg::intersect<3>(pos,dir,Amg::Vector3D::UnitZ(), lowEnd).value_or(0.)* dir)[view];
        const double x2 = (pos + Amg::intersect<3>(pos,dir,Amg::Vector3D::UnitZ(), highEnd).value_or(0.)* dir)[view];
        auto seedLine = std::make_unique<TLine>(x1, lowEnd, x2, highEnd);
        seedLine->SetLineColor(color);
        seedLine->SetLineWidth(2);
        seedLine->SetLineStyle(lineStyle);
        return seedLine;
    }
    std::unique_ptr<TLatex> drawAtlasLabel(const double xPos, const double yPos,
                                           const std::string& status) {
        return drawLabel( "#font[72]{ATLAS} "+status, xPos, yPos);
    }
    std::unique_ptr<TLatex> drawLumiSqrtS(const double xPos,
                                          const double yPos,
                                          const std::string_view sqrtS,
                                          const std::string_view lumi) {
        return drawLabel(std::format("#sqrt{{s}}={0} TeV {1}{2}", sqrtS, lumi, lumi.empty() ? "" : "fb^{-1}"), xPos, yPos);
    }
}
