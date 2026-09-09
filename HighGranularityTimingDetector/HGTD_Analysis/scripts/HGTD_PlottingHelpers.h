/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/scripts/HGTD_PlottingHelpers.h
 *
 * @author M. Sutton (original AtlasStyle/AtlasUtils);
 *         Alexander Leopold <alexander.leopold@cern.ch> (merge and trim)
 *
 * @brief ATLAS style and label helpers for the HGTD validation plotting macros.
 *
 * ATLAS Style, based on a style file from BaBar. Merged and trimmed from the
 * AtlasStyle.{cxx,h} / AtlasUtils.{cxx,h} pair as carried in
 * atlas-hgtd/SimulationAndPerformance/hgtdanalysisathena (HGTD_Plots/).
 *
 * Only the three entry points the HGTD macros actually use are kept --
 * SetAtlasStyle(), atlas::ATLAS_LABEL() and atlas::myText(). The remaining
 * AtlasUtils graph helpers (myTGraphErrorsDivide, myMakeBand, myAddtoBand,
 * TH1TOTGraph, myBoxText, myMarkerText) are used by no macro in that repository
 * and were dropped.
 *
 * Definitions are inline in this header on purpose: these are ROOT-interpreted
 * macros, so the previous arrangement had the macro headers #include the .cxx
 * bodies by relative path. A single guarded header removes that, along with the
 * guard collision between AtlasStyle.cxx and AtlasStyle.h and the duplicated
 * default argument on ATLAS_LABEL.
 *
 * NOTE: interpreted by cling only -- never compiled into a library. It is
 * installed as package data (see the package CMakeLists.txt).
 */

/// @cond -- ROOT macro helper, not part of the package API.

#ifndef HGTD_ANALYSIS_HGTD_PLOTTINGHELPERS_H
#define HGTD_ANALYSIS_HGTD_PLOTTINGHELPERS_H

#include <cmath>
#include <iostream>

#include "TGraphAsymmErrors.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TLatex.h"
#include "TLine.h"
#include "TMarker.h"
#include "TPave.h"
#include "TROOT.h"
#include "TStyle.h"

/// Build the ATLAS TStyle.
inline TStyle *AtlasStyle() {
  TStyle *atlasStyle = new TStyle("ATLAS", "Atlas style");

  // use plain black on white colors
  Int_t icol = 0; // WHITE
  atlasStyle->SetFrameBorderMode(icol);
  atlasStyle->SetFrameFillColor(icol);
  atlasStyle->SetCanvasBorderMode(icol);
  atlasStyle->SetCanvasColor(icol);
  atlasStyle->SetPadBorderMode(icol);
  atlasStyle->SetPadColor(icol);
  atlasStyle->SetStatColor(icol);
  // atlasStyle->SetFillColor(icol); // don't use: white fill color for *all*
  // objects

  // set the paper & margin sizes
  atlasStyle->SetPaperSize(20, 26);

  // set margin sizes
  atlasStyle->SetPadTopMargin(0.05);
  atlasStyle->SetPadRightMargin(0.05);
  atlasStyle->SetPadBottomMargin(0.16);
  atlasStyle->SetPadLeftMargin(0.16);

  // set title offsets (for axis label)
  atlasStyle->SetTitleXOffset(1.4);
  atlasStyle->SetTitleYOffset(1.4);

  // use large fonts
  // Int_t font=72; // Helvetica italics
  Int_t font = 42; // Helvetica
  Double_t tsize = 0.05;
  atlasStyle->SetTextFont(font);

  // atlasStyle->SetLabelOffset(0.015, "X");

  atlasStyle->SetTextSize(tsize);
  atlasStyle->SetLabelFont(font, "x");
  atlasStyle->SetTitleFont(font, "x");
  atlasStyle->SetLabelFont(font, "y");
  atlasStyle->SetTitleFont(font, "y");
  atlasStyle->SetLabelFont(font, "z");
  atlasStyle->SetTitleFont(font, "z");

  atlasStyle->SetLabelSize(tsize, "x");
  atlasStyle->SetTitleSize(tsize, "x");
  atlasStyle->SetLabelSize(tsize, "y");
  atlasStyle->SetTitleSize(tsize, "y");
  atlasStyle->SetLabelSize(tsize, "z");
  atlasStyle->SetTitleSize(tsize, "z");

  // use bold lines and markers
  atlasStyle->SetMarkerStyle(20);
  atlasStyle->SetMarkerSize(1.2);
  atlasStyle->SetHistLineWidth(2.);
  atlasStyle->SetLineStyleString(2, "[12 12]"); // postscript dashes

  // get rid of X error bars
  // atlasStyle->SetErrorX(0.001);
  // get rid of error bar caps
  atlasStyle->SetEndErrorSize(0.);

  // do not display any of the standard histogram decorations
  atlasStyle->SetOptTitle(0);
  // atlasStyle->SetOptStat(1111);
  atlasStyle->SetOptStat(0);
  // atlasStyle->SetOptFit(1111);
  atlasStyle->SetOptFit(0);

  // put tick marks on top and RHS of plots
  atlasStyle->SetPadTickX(1);
  atlasStyle->SetPadTickY(1);

  return atlasStyle;
}

/// Apply the ATLAS TStyle globally.
inline void SetAtlasStyle() {
  static TStyle *atlasStyle = 0;
  std::cout << "[SetAtlasStyle] Applying ATLAS style settings...\n";
  if (atlasStyle == 0)
    atlasStyle = AtlasStyle();
  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(0);
  gROOT->ForceStyle();
}

namespace atlas {

/// Draw the italic "ATLAS" label at NDC position (x, y).
inline void ATLAS_LABEL(Double_t x, Double_t y, Color_t color = 1,
                        float fac = 1.) {
  TLatex l; // l.SetTextAlign(12); l.SetTextSize(tsize);
  l.SetNDC();
  l.SetTextFont(72);
  l.SetTextSize(fac * l.GetTextSize());
  l.SetTextColor(color);
  l.DrawLatex(x, y, "ATLAS");
}

/// Draw free text at NDC position (x, y), using the current text size.
inline void myText(Double_t x, Double_t y, Color_t color, const char *text) {

  // Double_t tsize=0.05;
  TLatex l; // l.SetTextAlign(12); l.SetTextSize(tsize);
  l.SetNDC();
  l.SetTextColor(color);
  l.DrawLatex(x, y, text);
}

/// Draw free text at NDC position (x, y), with an explicit text size.
inline void myText(Double_t x, Double_t y, Color_t color, const char *text,
                   Double_t tsize) {

  TLatex l;
  // l.SetTextAlign(12);
  l.SetTextSize(tsize);
  l.SetNDC();
  l.SetTextColor(color);
  l.DrawLatex(x, y, text);
}

} // namespace atlas

#endif // HGTD_ANALYSIS_HGTD_PLOTTINGHELPERS_H

/// @endcond
