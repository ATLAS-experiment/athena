/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "Utils.h"
#include "PlotAnnotations.h"


using namespace MuonVertexValidationMacroUtils;

namespace MuonVertexValidationMacroPlotAnnotations {

TLegend* makeLegend(double lower_x, double lower_y, double upper_x, double upper_y, double textsize){
    // Takes NDC coordinates and the textsize as a faction of the pad dimensions to construct a TLegend object and return its pointer
    TLegend* legend = new TLegend(lower_x,lower_y,upper_x,upper_y); 
    legend->SetBorderSize(0);
    legend->SetTextFont(42);
    legend->SetTextSize(textsize);
    return legend;
}


void drawATLASlabel(const char* text, double x, double y, double textsize){
    // Draw the ATLAS label with some optional text. The textsize is a fraction of the pad dimensions.  
    TLatex l; 
    l.SetTextFont(42); // 42 for Helvetica and 72 for Helvetica italics
    l.SetTextColor(1);
    l.SetTextSize(textsize); 
    l.DrawLatexNDC(x, y, TString::Format("#it{#bf{ATLAS}} %s",text));

    return;
}


void drawDetectorRegionLabel(const char *name, const char* customlabel, double x, double y, double textsize){
    // Returns an annotation for the detector region based on the name of the histogram:
    // if the histogram name ends with "b" it is assumed to be in the barrel region
    // if the histogram name ends with "e" it is assumed to be in the endcaps region
    // if customlabel is provided, it is used instead of the default label.
    // if non of the above applies, the function returns without drawing anything.
    // The textsize is a fraction of the pad dimensions. 
    
    const char DecReg = name[std::strlen(name)-1];
    TString DecReg_label{}; 
    if (*customlabel != 0){
        DecReg_label = TString(customlabel);
    }
    else if (DecReg == *"b"){
        DecReg_label = TString::Format("Barrel: |#kern[0.2]{#eta}| < %.2f", fidVol_barrel_etaCut);
    }
    else if (DecReg == *"e"){
        DecReg_label = TString::Format("Endcaps: %.2f < |#kern[0.1]{#eta}| < %.2f", fidVol_endcaps_etaCut_low, fidVol_endcaps_etaCut_up); 
    }
    else return;

    TLatex t; 
    t.SetTextFont(42); 
    t.SetTextSize(textsize);
    t.DrawLatexNDC(x, y, DecReg_label);

    return;
}


void drawDetectorBoundaryLine(double x, double y_max, int line_style, const char* text){
    // Draw a vertical line at the detector boundary location x between 0 and y_max in absolute coordinates. 
    TLine* l = new TLine(x, y_max, x, 0); // assures that dashed lines don't start with a gap  
    Int_t ashGrey = TColor::GetColor("#717581");
    l->SetLineColor(ashGrey);
    l->SetLineStyle(line_style);
    l->Draw();

    TText* t = new TText(x, y_max, text);
    t->SetTextFont(42);
    t->SetTextSize(0.03);
    t->SetTextAlign(31); // text is bottom right aligned to the position of the text 
    t->SetTextAngle(90);
    t->Draw();

    return;
}


void drawDetectorBoundaryLines(const char* bin_var, double y_max){
    // Draw annotated vertical lines at the detector boundary locations for plots binned in Lxy or z.
    // Valid inputs for bin_var are "Lxy" or "z".
    // y_max defines the upper end of the line in absolute coordinates. 
    double hcalEnd{0}, mdt1S{0}, mdt1L{0}, mdt2S{0}, mdt2L{0};
    if ((std::string)bin_var == "Lxy") {hcalEnd = 3.9; mdt1S = 4.383; mdt1L = 4.718; mdt2S = 7.888; mdt2L = 6.861;}
    else if ((std::string)bin_var == "z") {hcalEnd = 6.05; mdt1S = 7.023; mdt1L = 7.409; mdt2S = 13.265; mdt2L = 13.660;}
    else printf("Invalid binning variable. Please use 'Lxy' or 'z'.\nReturning without drawing detector boundary lines.\n");

    drawDetectorBoundaryLine(hcalEnd, y_max, 9, "HCal end");
    drawDetectorBoundaryLine(mdt1S, y_max, 7, "MDT1S");
    drawDetectorBoundaryLine(mdt1L, y_max, 2, "MDT1L");
    drawDetectorBoundaryLine(mdt2S, y_max, 7, "MDT2S");
    drawDetectorBoundaryLine(mdt2L, y_max, 2, "MDT2L");

    return;
}


double getMaxy(const TGraphAsymmErrors* graph, double current_max){
    // returns the maximum of the current_max and the maximum y value+error of the passed graph.
    double max = current_max;
    double max_graph;
    for (int i=0; i<graph->GetN(); ++i){
        max_graph = graph->GetPointY(i) + graph->GetErrorYhigh(i);
        max = max_graph > max ? max_graph : max; 
    }

    return max;
}

} // namespace MuonVertexValidationMacroPlotAnnotations
