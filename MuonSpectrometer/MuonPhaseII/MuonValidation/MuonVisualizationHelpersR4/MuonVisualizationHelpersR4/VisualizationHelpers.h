/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONVISUALIZATIONHELPERS_H
#define MUONR4_MUONVISUALIZATIONHELPERS_H

#include <memory>
#include <GeoPrimitives/GeoPrimitives.h>
#include <MuonPatternEvent/SegmentFitterEventData.h>


#include "TEllipse.h"
#include "TBox.h"
#include "TLatex.h"
#include "TLine.h"
#include "TArrow.h"

namespace MuonValR4 {
    /** @brief Filling codes for hollow / fullFilling / hatched filling */
    constexpr int hollowFilling = 0;
    constexpr int fullFilling = 1001;
    constexpr int hatchedFilling = 3344;
    
    /** @brief ObjectView */
    constexpr int objViewEta = Acts::toUnderlying(MuonR4::SegmentFit::AxisDefs::etaCov);
    constexpr int objViewPhi = Acts::toUnderlying(MuonR4::SegmentFit::AxisDefs::phiCov);
    
    /** @brief Create a TEllipse for drawing a drift circle
     *  @param center: Position of the drift cirle expressed in the chambers frame
     *                  y-coordinate corresponds to the tube layer
     *                  z-coordinate corresponds to the tube in the layer
     *  @param radius: Radius of the drift circle to draw
     *  @param color: Color of the circle to draw
     *  @param fillStyle: Style to pick the filling of the circle */
    std::unique_ptr<TEllipse> drawDriftCircle(const Amg::Vector3D& center,
                                              const double radius,
                                              const int color = kViolet,
                                              const int fillStyle = hollowFilling);

    
    /** @brief Creates a box for drawing, e.g strip measurements
     *  @param boxCenter: Center of the box to be placed expressed in chamber frame coordinates
     *  @param boxWidth: Width of the drawn box
     *  @param boxHeight: Height of the drawn box
     *  @param color: Color of the surrounding line & filling
     *  @param fillStyle: Box fill style
     *  @param view: Is the box placed in the y-z or in the x-z plane */
    std::unique_ptr<TBox> drawBox(const Amg::Vector3D& boxCenter,
                                  const double boxWidth,
                                  const double boxHeight,
                                  const int color = kGreen +2,
                                  const int fillStyle = hollowFilling,
                                  const int view = objViewEta);
    /** @brief Creates a box for drawing, e.g strip measurements
     *  @param x1: Left edge location
     *  @param y1: Bottom edge location
     *  @param x2: Right edge location
     *  @param y2: TopEdgeLocation
     *  @param color: Color of the surrounding line & filling
     *  @param fillStyle: Box fill style
     *  @param view: Is the box placed in the y-z or in the x-z plane */
    std::unique_ptr<TBox> drawBox(const double x1,
                                  const double y1,
                                  const double x2,
                                  const double y2,
                                  const int color = kGreen +2,
                                  const int fillStyle = hollowFilling);
    /** @brief Draws a line from the segment fit parameters
     *  @param pars: Segment fit parameters to extract position & direction from
     *  @param lowEnd: Lower boundary in Canvas-y of the line
     *  @param highEnd: Upper boundaty in Canbas-y of the line
     *  @param color: Color of the line (Cf. TColor documentation)
     *  @param lineStyle: Style of the drawn line (cf. TAttLine documentation)
     *  @param view: Is the line placed in the y-z or in the x-z plane */
    std::unique_ptr<TLine> drawLine(const MuonR4::SegmentFit::Parameters& pars,
                                    const double lowEnd, const double highEnd,
                                    const int color = kRed +1,
                                    const int lineStyle = kDashed,
                                    const int view = objViewEta);
    
    std::unique_ptr<TArrow> drawArrow(const Amg::Vector3D& start,
                                      const Amg::Vector3D& dir,
                                      const int color = kRed +1,
                                      const int lineStyle = kDashed,
                                      const int view = objViewEta);
    /** @brief Create a TLatex label,
     *  @param text: Label text
     *  @param xPos: x-position of the label on the Canvas
     *  @param yPos: y-position of the label on the Canvas
     *  @param fontSize: Size of the label font */
    std::unique_ptr<TLatex> drawLabel(const std::string& text, 
                                      const double xPos, 
                                      const double yPos,
                                      const unsigned int fontSize = 18);
    /** @brief Create a ATLAS label
     *  @param xPos: x-position of the label on the Canvas
     *  @param yPos: y-position of the label on the Canvas
     *  @param status: ATLAS label status to be drawn */
    std::unique_ptr<TLatex> drawAtlasLabel(const double xPos,
                                           const double yPos,
                                           const std::string& status = "Internal");
    /** @brief Create a luminosity sqrtS label
     *  @param xPos: x-position of the label on the Canvas
     *  @param yPos: y-position of the label on the Canvas
     *  @param sqrtS: Centre of mass energy [TeV]
     *  @param lumi: Luminosity [fb^{-1}]. If less <0 not drawn */
    std::unique_ptr<TLatex> drawLumiSqrtS(const double xPos,
                                          const double yPos,
                                          const std::string_view sqrtS="14",
                                          const std::string_view lumi = "");
}
#endif
