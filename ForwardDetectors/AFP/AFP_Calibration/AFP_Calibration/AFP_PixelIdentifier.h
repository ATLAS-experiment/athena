/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef AFP_CALIBRATION_AFP_PIXELIDENTIFIER_H
#define AFP_CALIBRATION_AFP_PIXELIDENTIFIER_H

#include "AFP_Geometry/AFP_constants.h"
#include "AFP_Calibration/AFP_DeadPixelTool.h"
#include "AFP_Calibration/AFP_NoisyPixelTool.h"

// STL includes
#include <string>
#include <vector>
#include <memory>
#include <utility>

// ROOT includes
#include "TFile.h"
#include "TH2F.h"


class AFP_PixelIdentifier {
public:
	AFP_PixelIdentifier(const std::string& input_name="AFP_PixelHistoFiller.root",
                            const std::string& output_name="AFP_PixelIdentifier.root",
                            const std::vector<std::string>& pixelTools={"AFP_DeadPixel", "AFP_NoisyPixel"});
	~AFP_PixelIdentifier() = default;

	int execute();
  
private:
	std::unique_ptr<TFile> m_input_file;
	std::unique_ptr<TFile> m_output_file;
	
	static const int m_nStations=4;
	static const int m_nLayers=4;
	int m_nPixelsX;
	int m_nPixelsY;
	
	std::vector<TH2F> m_pixelHits[m_nStations][m_nLayers];
	
	std::vector<std::string> m_pixelTools_names;
	std::vector<std::unique_ptr<IAFP_GenericPixelTool>> m_pixelTools;
};


#endif // AFP_CALIBRATION_AFP_PIXELIDENTIFIER_H
