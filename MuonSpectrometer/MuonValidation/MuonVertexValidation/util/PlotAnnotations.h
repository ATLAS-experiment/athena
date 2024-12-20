/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PLOTANNOTATIONS_H
#define PLOTANNOTATIONS_H

#include <vector>
#include <string>

#include "TGraphAsymmErrors.h"

#include "TLatex.h"
#include "TString.h"
#include "TLegend.h"
#include "TLine.h"
#include "TColor.h"


namespace MuonVertexValidationMacroPlotAnnotations {
    TLegend* makeLegend(double lower_x=0.6, double lower_y=0.75, double upper_x=0.92, double upper_y=0.92, double textsize=0.04);
    void drawATLASlabel(const char* text="", double x=0.2, double y=0.88, double textsize=0.05);
    void drawDetectorRegionLabel(const char* name, const char* customlabel="", double x=0.2, double y=0.83, double textsize=0.04);
    void drawDetectorBoundaryLine(double x, double y_max, int line_style, const char* text);
    void drawDetectorBoundaryLines(const char* bin_var, double y_max=1.1);
    double getMaxy(const TGraphAsymmErrors* graph, double current_max=0.0);
}

#endif // PLOTANNOTATIONS_H
