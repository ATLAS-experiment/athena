/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PixelChargeInterpolationPlot_h
#define PixelChargeInterpolationPlot_h

#include "CxxUtils/checker_macros.h"
#include <string>
#include <vector>
class TH1F;
class TCanvas;
class TGaxis;

namespace PixelCalib{

class PixelChargeInterpolationParameters;

class PixelChargeInterpolationPlot{

public :
  PixelChargeInterpolationPlot(const PixelChargeInterpolationParameters &parameters, bool oneconst = false);
	virtual ~PixelChargeInterpolationPlot();
	void AddReference(const PixelChargeInterpolationParameters &parameters,
                          const std::string& title = "Reference", int color = 0,
                          const std::string& drawopt = "HIST"); 
	void Plot(const std::string& output);
	void Write();
	
private:

  PixelChargeInterpolationPlot(const PixelChargeInterpolationPlot &);
  PixelChargeInterpolationPlot &operator=(const PixelChargeInterpolationPlot&);

	// Histograms to be used
	std::vector < TH1F* > *m_histogramsX;
	std::vector < TH1F* > *m_histogramsY;
	std::vector < std::vector < TH1F* > > *m_RefHistosX;
	std::vector < std::vector < TH1F* > > *m_RefHistosY;


	// bins
	double* m_etabins;
	double* m_phibins;
	int m_netabins;
	int m_nphibins;
	static const int m_nlayers; // = 3;
	std::vector <std::string> *m_referenceDrawOpt;
	bool m_oneconst;
  inline static const std::string s_options{"P0same"};
  inline static const std::string s_direction{"phi"};
  inline static const std::string s_title{""};
	// utility methods!
	void PlotDirection(const std::string& filename, const std::string& direction = s_direction);

	void DrawOneHisto(TH1F *histo, const std::string& direction = s_direction,float maximum = 0);
	void DrawHistoMarkers(TH1F* histo, const std::string& options  = s_options,  int goodj = 0);
	void DrawLayerLegend(float xlegend, float ylegend);
	void DrawAxis(float y1, float y2, float x1, float x2, const std::string& direction = s_direction);

	std::vector < TH1F*> *HistogramsFromConstants(
			const PixelChargeInterpolationParameters &parameters,
			const std::string& direction = s_direction, int color = 1, const std::string& title = s_title);
};

}
#endif
