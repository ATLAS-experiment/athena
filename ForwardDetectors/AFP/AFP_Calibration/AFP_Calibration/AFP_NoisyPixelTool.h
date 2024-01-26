/*
	Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AFP_CALIBRATION_AFP_NOISYPIXELTOOL_H
#define AFP_CALIBRATION_AFP_NOISYPIXELTOOL_H

// Package includes
#include "AFP_Calibration/IAFP_GenericPixelTool.h"

// ROOT includes
#include "TF1.h"
#include "TH2F.h"

// STL includes
#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include <utility>
 
class AFP_NoisyPixelTool : public IAFP_GenericPixelTool {
public:
	AFP_NoisyPixelTool() : m_threshNoisy(1.4), m_threshLEff(0.6), m_sensitivity(0.0), m_methods({"2_ROW","2_COL"}) {}
	
	int Identify(std::shared_ptr<const TH2F> input, std::vector<TH2F>& output) const override;
	
	std::vector<std::vector<double>> makeFits(std::shared_ptr<const TH2F> input) const;
	
	void setThreshNoisy(double t) {m_threshNoisy=t;}
	double getThreshNoisy() const {return m_threshNoisy;}
	
	void setThreshLeff(double t) {m_threshLEff=t;}
	double getThreshLeff() const {return m_threshLEff;}
	
	void setSensitivity(double s) {m_sensitivity=s;}
	double getSensitivity() const {return m_sensitivity;}
	
	void setMethods(const std::vector<std::string>& m) {m_methods=m;}
	std::vector<std::string> getMethods() const {return m_methods;}

private:
	
	double m_threshNoisy; // Minimum efficiency of pixels to be classified as noisy
	double m_threshLEff; // Maximum efficiency of pixels to be classified as low efficiency
	double m_sensitivity; // with higher value, the algorithm will be more prone to find noisy pixels near low efficiency ones
	
	std::vector<std::string> m_methods; // methods to be used for identification of noisy pixels; available: \"2_ROW\", \"2_COL\", \"4_PIX\", \"8_PIX\", and \"FIT\""};
	
	std::vector<std::pair<int,int>> getLegitPixels(std::shared_ptr<const TH2F> input, const int col_ID, const int row_ID, const std::string& method) const;
	
	TH2F countInactivePixelsAround(std::shared_ptr<const TH2F> input) const;
	double getNeighbours(std::shared_ptr<const TH2F> input, int row_ID, int col_ID) const;
	std::tuple<TH2F,TH2F,TH2F,TH2F> findLEffAndNoisyPixels(std::shared_ptr<const TH2F> input, const TH2F& inact, const std::string& method) const;
	void filterLEffPixelsAroundNoisy(std::shared_ptr<const TH2F> input, const TH2F& inact, TH2F& fleff, TH2F& fnoisy, TH2F& eleff, TH2F& enoisy, const std::string& method) const;
};

#endif // AFP_CALIBRATION_AFP_NOISYPIXELTOOL_H
