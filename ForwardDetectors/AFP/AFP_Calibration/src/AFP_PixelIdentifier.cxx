/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AFP_Calibration/AFP_PixelIdentifier.h"


AFP_PixelIdentifier::AFP_PixelIdentifier(const std::string& input_name, const std::string& output_name, const std::vector<std::string>& pixelTools) :
  m_input_file (std::make_unique<TFile>(input_name.c_str(),"read")),
  m_output_file (std::make_unique<TFile>(output_name.c_str(),"recreate")),
  m_pixelTools_names(pixelTools)
{
	for(const std::string& pixel_name: m_pixelTools_names)
	{
		if(pixel_name=="AFP_DeadPixel")
		{
			m_pixelTools.push_back(std::make_unique<AFP_DeadPixelTool>());
		}
		else if(pixel_name=="AFP_NoisyPixel")
		{
			m_pixelTools.push_back(std::make_unique<AFP_NoisyPixelTool>());
		}
	}
	
	AFP_CONSTANTS afp_consts;
	m_nPixelsX=afp_consts.SiT_Pixel_amount_x;
	m_nPixelsY=afp_consts.SiT_Pixel_amount_y;
}


int AFP_PixelIdentifier::execute()
{
	if(m_pixelTools.empty())
	{
		return 0;
	}
	
	std::unique_ptr<TH1I> hist_lb(static_cast<TH1I*>(m_input_file->Get("LBRangeLength")));
	int LBRangeLength=hist_lb->GetBinContent(2)/hist_lb->GetBinContent(1);
	
	for(int st=0;st<m_nStations;++st)
	{
		for(int la=0;la<m_nLayers;++la)
		{
			int lbIdx=0;
			while(true) // repeat until there are valid histograms
			{
				std::shared_ptr<const TH2F> pixelHits(dynamic_cast<TH2F*>(m_input_file->Get(
				    Form("pixel_hits_lb_%d_%d_station_%d_layer_%d", lbIdx*LBRangeLength, (lbIdx+1)*LBRangeLength-1, st,la)
				)));
				
				if(!pixelHits.get()) break;
				
				for(auto &pixelTool : m_pixelTools)
				{
					std::vector<TH2F> output{TH2F(Form("lb_%d_%d_station_%d_layer_%d",lbIdx*LBRangeLength,(lbIdx+1)*LBRangeLength-1,st,la),
					                              Form("lb %d-%d, station %d, layer %d", lbIdx*LBRangeLength,(lbIdx+1)*LBRangeLength-1,st,la),
					                              m_nPixelsX,0.5,m_nPixelsX+0.5, m_nPixelsY,0.5,m_nPixelsY+1)};
					
					pixelTool->Identify(pixelHits,output);
					
					for(TH2F& out : output)
					{
						out.Write();
					}
				}
				
				++lbIdx;
			}
		}
	}
	
	m_output_file->Close();
	return 0;
}


