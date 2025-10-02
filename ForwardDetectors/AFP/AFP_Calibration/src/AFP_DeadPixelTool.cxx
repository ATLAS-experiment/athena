/*
	Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AFP_Calibration/AFP_DeadPixelTool.h"
#include <cmath>

int AFP_DeadPixelTool::Identify(std::shared_ptr<const TH2F> input, std::vector<TH2F>& output) const
{
	if(output.size()!=1)
	{
		return 0;
	}
	
	TH2F tmp_dead_pixels(*static_cast<TH2F*>(input->Clone("tmp_dead_pixels")));
	tmp_dead_pixels.Reset();
	
	TH2F& deadpixels_output = output.at(0);
	deadpixels_output.SetName(Form("deadpixels_%s",deadpixels_output.GetName()));
	deadpixels_output.SetTitle(Form("dead pixels, %s",deadpixels_output.GetTitle()));
	deadpixels_output.Reset();
	
	auto maxHitValue=input->GetMaximum();
	
	// first iteration (find singular dead pixels)
	for(int col_ID=1; col_ID<=input->GetNbinsY(); col_ID++)
	{
		for(int row_ID=1; row_ID<=input->GetNbinsX(); row_ID++)
		{
			if(input->GetBinContent(row_ID, col_ID)==0 )
			{
				double nNeighbours=getNeighbours(input, row_ID,col_ID);

				int sum_neighbours=0;
				int inactive_pixels_around=0;
				
				auto legit_pixels=getLegitPixels(input, col_ID, row_ID);
				for(auto legpix : legit_pixels)
				{
					sum_neighbours+=input->GetBinContent(legpix.first, legpix.second);
					if(input->GetBinContent(legpix.first, legpix.second)<1) ++inactive_pixels_around;
				}
	
				if(inactive_pixels_around<4 && std::round(sum_neighbours/nNeighbours)>=m_range*maxHitValue)
				{
					deadpixels_output.Fill(row_ID, col_ID);
					tmp_dead_pixels.SetBinContent(row_ID,col_ID, 1);
				}
			}
		}
	}
	
	// second iteration (find groups of dead pixels)
	bool newDEAD=true;
	while(newDEAD)
	{	
		newDEAD=false;
		for(int col_ID=1; col_ID<=input->GetNbinsY(); col_ID++)
		{
			for(int row_ID=1; row_ID<=input->GetNbinsX(); row_ID++)
			{	
				if(tmp_dead_pixels.GetBinContent(row_ID,col_ID)==0 && input->GetBinContent(row_ID, col_ID)==0)
				{
					double nNeighbours=getNeighbours(input, row_ID,col_ID);

					int sum_neighbours=0;
					int inactive_pixels_around=0;
					int dead_pixels_around=0;
					
					auto legit_pixels=getLegitPixels(input, col_ID, row_ID);
					for(auto legpix : legit_pixels)
					{
						sum_neighbours+=input->GetBinContent(legpix.first, legpix.second);
						if(input->GetBinContent(legpix.first, legpix.second)<1) ++inactive_pixels_around;
					
						if(tmp_dead_pixels.GetBinContent(legpix.first, legpix.second)>0) dead_pixels_around++;
					}

				
					if(dead_pixels_around>0 && inactive_pixels_around>3 && std::round(sum_neighbours/nNeighbours)>=m_range*maxHitValue)
					{
						deadpixels_output.Fill(row_ID, col_ID);
						tmp_dead_pixels.SetBinContent(row_ID,col_ID, 1);
						newDEAD=true;
					}
				}
			}
		}
	}

	return 0;
}


std::vector<std::pair<int,int>> AFP_DeadPixelTool::getLegitPixels(std::shared_ptr<const TH2F> input, const int col_ID, const int row_ID) const
{	
	std::vector<std::pair<int,int>> legit_pixels={};
	
	for(int r=-1; r<2; r++)
	{
		for(int c=-1; c<2; c++)
		{
			if( (row_ID+r)>=1 && (col_ID+c)>=1 && (col_ID+c)<=input->GetNbinsY() && (row_ID+r)<=input->GetNbinsX() && (r!=0 || c!=0))
			{
				legit_pixels.push_back(std::pair<int,int>(row_ID+r,col_ID+c));
			}
		}
	}
	
	return legit_pixels;
}


double AFP_DeadPixelTool::getNeighbours(std::shared_ptr<const TH2F> input, int row_ID, int col_ID) const
{
	if( row_ID!=input->GetNbinsX() && row_ID!=1 && col_ID!=1 && col_ID!=input->GetNbinsY()) return 8.; // inner pixels
	else if( (row_ID==input->GetNbinsX() || row_ID==1) && (col_ID==1 || col_ID==input->GetNbinsY())) return 3.; // corner pixels
	else return 5; // pixels along edges
}
