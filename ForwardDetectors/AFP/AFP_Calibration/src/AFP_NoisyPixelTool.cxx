/*
	Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AFP_Calibration/AFP_NoisyPixelTool.h"

int AFP_NoisyPixelTool::Identify(std::shared_ptr<const TH2F> input, std::vector<TH2F>& output) const
{
	if(output.size()!=1)
	{
		return 0;
	}
	
	output[0].Reset();
	output.resize(4, output[0]);//container now has 4 copies of the 0'th element
	auto name = [&output](const char * n)->TString{
	  return Form(n,output[0].GetName());
	};
	auto title = [&output](const char * t)->TString{
	  return Form(t,output[0].GetTitle());
	};
	output[1].SetNameTitle(name("leffpixels_found_%s"), title("low efficiency pixels, found, %s"));
	output[2].SetNameTitle(name("noisypixels_eff_%s"), title("noisy pixels, efficiency, %s"));
	output[3].SetNameTitle(name("leffpixels_eff_%s"), title("low efficiency pixels, efficiency, %s"));
	output[0].SetNameTitle(name("noisypixels_found_%s"), title("noisy pixels, found, %s"));
	
  TH2F& noisypixels_found_output = output[0];
	TH2F& leffpixels_found_output = output[1];
	TH2F& noisypixels_eff_output = output[2];
	TH2F& leffpixels_eff_output = output[3];
		
	
	if(input->GetMaximum()<0.5) return 0;

	std::vector<TH2F> vec_found_leff{},vec_found_noisy{},vec_eff_leff{},vec_eff_noisy{};

	for(const auto& method : m_methods)
	{
		TH2F tmp_eff_noisy, tmp_eff_leff;
		TH2F tmp_found_leff, tmp_found_noisy;
		
		if(method!="FIT")
		{
			// count inactive pixels around each pixel
			TH2F tmp_inact_pix_around=countInactivePixelsAround(input);
			
			// find low eff. and noisy pixels
			std::tie(tmp_found_leff,tmp_found_noisy,tmp_eff_leff, tmp_eff_noisy) = findLEffAndNoisyPixels(input, tmp_inact_pix_around, method);
			
			// reconsider some pixels
			filterLEffPixelsAroundNoisy(input, tmp_inact_pix_around, tmp_found_leff,tmp_found_noisy,tmp_eff_leff,tmp_eff_noisy, method);
		}
		else
		{
			// method is "FIT"
			
			tmp_eff_noisy=TH2F(*input);
			tmp_eff_noisy.SetName("tmp_eff_noisy");
			tmp_eff_noisy.Reset();
			tmp_eff_leff=TH2F(*input);
			tmp_eff_leff.SetName("tmp_eff_leff");
			tmp_eff_leff.Reset();
			tmp_found_noisy=TH2F(*input);
			tmp_found_noisy.SetName("tmp_found_noisy");
			tmp_found_noisy.Reset();
			tmp_found_leff=TH2F(*input);
			tmp_found_leff.SetName("tmp_found_leff");
			tmp_found_leff.Reset();
			
			auto p=makeFits(input);
			
			const double Threshold_LEFF=5.;
			const double Threshold_NOISY=(m_threshNoisy-1)*100;
			
			for(int col_ID=1; col_ID<=input->GetNbinsY(); col_ID++)
			{
  				for(int row_ID=1; row_ID<=input->GetNbinsX(); row_ID++)
  				{
  					double ratio_leff=0, ratio_noisy=0;
  					double gauss_helper_12 = (col_ID-p[1][row_ID])/p[2][row_ID];
  					double gauss_helper_45 = (col_ID-p[4][row_ID])/p[5][row_ID];
	 				double fit = p[0][row_ID]*exp(-0.5*gauss_helper_12*gauss_helper_12)
	 				           + p[3][row_ID]*exp(-0.5*gauss_helper_45*gauss_helper_45) + p[6][row_ID];
					double sigma=std::sqrt(fit);
					if(sigma<1) sigma=1;
					
					double bin_content=input->GetBinContent(row_ID,col_ID);
					
					if(fit>=bin_content) ratio_leff = std::abs(fit-bin_content)/sigma;
					else                 ratio_noisy = std::abs(fit-bin_content)/sigma;
					
					if(bin_content!=0)
					{
						if( ratio_leff	> Threshold_LEFF) tmp_found_leff.SetBinContent(row_ID,col_ID,1);
						if( ratio_noisy	> Threshold_NOISY) tmp_found_noisy.SetBinContent(row_ID,col_ID,1);
					}
					tmp_eff_leff.SetBinContent(row_ID,col_ID,100.*ratio_leff);
					tmp_eff_noisy.SetBinContent(row_ID,col_ID,100.*ratio_noisy);
				}
			}
		}
		
		// save output for this method
		vec_found_leff.push_back(tmp_found_leff);
		vec_found_noisy.push_back(tmp_found_noisy);
		vec_eff_leff.push_back(tmp_eff_leff);
		vec_eff_noisy.push_back(tmp_eff_noisy);	
	}
	
	// sum found pixels for all methods, set efficiency to the maximum of all methods
	for(unsigned int m=0; m<m_methods.size(); m++)
	{	
		for(int col_ID=1; col_ID<=input->GetNbinsY(); col_ID++)
		{
  			for(int row_ID=1; row_ID<=input->GetNbinsX(); row_ID++)
  			{	
				noisypixels_found_output.Fill(row_ID,col_ID, vec_found_noisy[m].GetBinContent(row_ID,col_ID));
				leffpixels_found_output.Fill(row_ID,col_ID, vec_found_leff[m].GetBinContent(row_ID,col_ID));
				
				if(m==0)
				{
					noisypixels_eff_output.SetBinContent(row_ID,col_ID, vec_eff_noisy[m].GetBinContent(row_ID,col_ID));
					leffpixels_eff_output.SetBinContent(row_ID,col_ID, vec_eff_leff[m].GetBinContent(row_ID,col_ID));
				}
				else
				{ 
					if(noisypixels_eff_output.GetBinContent(row_ID,col_ID)<vec_eff_noisy[m].GetBinContent(row_ID,col_ID))
					{
						noisypixels_eff_output.SetBinContent(row_ID,col_ID, vec_eff_noisy[m].GetBinContent(row_ID,col_ID));
					}
					if(leffpixels_eff_output.GetBinContent(row_ID,col_ID)>vec_eff_leff[m].GetBinContent(row_ID,col_ID))
					{
						leffpixels_eff_output.SetBinContent(row_ID,col_ID, vec_eff_leff[m].GetBinContent(row_ID,col_ID));
					}
				}
			}
		}
	}

	// report only pixels that are identified by all methods
	for(int col_ID=1; col_ID<=input->GetNbinsY(); col_ID++)
	{
  		for(int row_ID=1; row_ID<=input->GetNbinsX(); row_ID++)
  		{	
			if(noisypixels_found_output.GetBinContent(row_ID,col_ID)==m_methods.size())	noisypixels_found_output.SetBinContent(row_ID,col_ID,1);
			else noisypixels_found_output.SetBinContent(row_ID,col_ID,0);
			
			if(leffpixels_found_output.GetBinContent(row_ID,col_ID)==m_methods.size())	leffpixels_found_output.SetBinContent(row_ID,col_ID,1);
			else leffpixels_found_output.SetBinContent(row_ID,col_ID,0);
		}
	}

	return 0;
}


std::vector<std::pair<int,int>> AFP_NoisyPixelTool::getLegitPixels(std::shared_ptr<const TH2F> input, const int col_ID, const int row_ID, const std::string& method="8_PIX") const
{
	// method="8_PIX" means to investigate all pixels around
	
	std::vector<std::pair<int,int>> legit_pixels={};
	
	for(int r=-1; r<2; r++)
	{
		for(int c=-1; c<2; c++)
		{
			if( (row_ID+r)>=1 && (col_ID+c)>=1 && (col_ID+c)<=input->GetNbinsY() && (row_ID+r)<=input->GetNbinsX() && (r!=0 || c!=0))
			{
				if( (method=="2_ROW" && c==0) || (method=="2_COL" && r==0)
					|| (method=="4_PIX" && (c==0 || r==0)) || (method=="8_PIX") )
				{
					legit_pixels.push_back(std::pair<int,int>(row_ID+r,col_ID+c));
				}
			}
		}
	}
	
	return legit_pixels;
}


TH2F AFP_NoisyPixelTool::countInactivePixelsAround(std::shared_ptr<const TH2F> input) const
{
	TH2F tmp_inact_pix_around(*input);
	tmp_inact_pix_around.SetName("tmp_inact_pix_around");
	tmp_inact_pix_around.Reset();
			
	for(int col_ID=1; col_ID<=input->GetNbinsY(); col_ID++)
	{
		for(int row_ID=1; row_ID<=input->GetNbinsX(); row_ID++)
		{					
			int inactive_pixels_around=0;
			auto legit_pixels=getLegitPixels(input, col_ID, row_ID);
			
			for(auto legpix : legit_pixels)
			{
				if(input->GetBinContent(legpix.first, legpix.second)<1) ++inactive_pixels_around;
			}

			tmp_inact_pix_around.SetBinContent(row_ID,col_ID, inactive_pixels_around+0.01);
		}
	}
	
	return tmp_inact_pix_around;
}


double AFP_NoisyPixelTool::getNeighbours(std::shared_ptr<const TH2F> input, int row_ID, int col_ID) const
{
	if( row_ID!=input->GetNbinsX() && row_ID!=1 && col_ID!=1 && col_ID!=input->GetNbinsY()) return 8.;
	else if( (row_ID==input->GetNbinsX() || row_ID==1) && (col_ID==1 || col_ID==input->GetNbinsY())) return 3.;
	else return 5;
}


std::tuple<TH2F,TH2F,TH2F,TH2F> AFP_NoisyPixelTool::findLEffAndNoisyPixels(std::shared_ptr<const TH2F> input, const TH2F& tmp_inact_pix_around, const std::string& method) const
{
	TH2F tmp_found_leff(*input);
	tmp_found_leff.SetName("tmp_found_leff");
	tmp_found_leff.Reset();
	
	TH2F tmp_found_noisy(*input);
	tmp_found_noisy.SetName("tmp_found_noisy");
	tmp_found_noisy.Reset();
	
	TH2F tmp_eff_leff(*input);
	tmp_eff_leff.SetName("tmp_eff_leff");
	tmp_eff_leff.Reset();
	
	TH2F tmp_eff_noisy(*input);
	tmp_eff_noisy.SetName("tmp_eff_noisy");
	tmp_eff_noisy.Reset();
	
	for(int col_ID=1; col_ID<=input->GetNbinsY(); col_ID++)
	{
		for(int row_ID=1; row_ID<=input->GetNbinsX(); row_ID++)
		{
			if(input->GetBinContent(row_ID, col_ID)!=0 && tmp_inact_pix_around.GetBinContent(row_ID, col_ID)<0.1)
			{	
				double npixels=getNeighbours(input, row_ID, col_ID);
				
				double sum=0.;
				auto legit_pixels=getLegitPixels(input, col_ID, row_ID, method);
				for(auto legpix : legit_pixels)
				{
					sum+=input->GetBinContent(legpix.first,legpix.second);
				}
				
				double av_noisy=0;
				double av_leff=0;
				
				// TODO: something about edges?
				if(method=="2_ROW" || method=="2_COL")
				{
					av_noisy=sum/(std::abs(npixels-4)*0.5);
					av_leff=sum/2.;
				}
				else if(method=="4_PIX") 
				{
					av_noisy=sum/(std::ceil(npixels*0.5));
					av_leff=sum/4.;
				}
				else if(method=="8_PIX")
				{
					av_noisy=sum/(npixels);
					av_leff=sum/8.;
				}
				
				
				double ratio_leff  = input->GetBinContent(row_ID,col_ID)/av_leff;
				double ratio_noisy = input->GetBinContent(row_ID,col_ID)/av_noisy;
				tmp_eff_leff.SetBinContent(row_ID,col_ID,100.*ratio_leff);
				tmp_eff_noisy.SetBinContent(row_ID,col_ID,100.*ratio_noisy);
				
				if(ratio_leff  < m_threshLEff) tmp_found_leff.SetBinContent(row_ID,col_ID,1.); 
				if(ratio_noisy > m_threshNoisy) tmp_found_noisy.SetBinContent(row_ID,col_ID,1.);	
			}
		}
	}
	
	return {tmp_found_leff,tmp_found_noisy,tmp_eff_leff,tmp_eff_noisy};
}


void AFP_NoisyPixelTool::filterLEffPixelsAroundNoisy(std::shared_ptr<const TH2F> input, const TH2F& tmp_inact_pix_around, TH2F& tmp_found_leff, TH2F& tmp_found_noisy, TH2F& tmp_eff_leff, TH2F& tmp_eff_noisy, const std::string& method) const
{	
	for(int col_ID=1; col_ID<=input->GetNbinsY(); col_ID++)
	{
		for(int row_ID=1; row_ID<=input->GetNbinsX(); row_ID++)
		{	
  			if(input->GetBinContent(row_ID,col_ID)==0) continue;
  					
  			int inactive_pixels_around=tmp_inact_pix_around.GetBinContent(row_ID, col_ID);
  			int leff_pixels_around=0;
			auto legit_pixels=getLegitPixels(input, col_ID,row_ID);
			for(auto legpix : legit_pixels)
			{
				if(tmp_found_leff.GetBinContent(legpix.first,legpix.second)>0) leff_pixels_around++;
			}
  			
			if(leff_pixels_around!=0  && inactive_pixels_around==0)
			{
				//if low eff. pixels around > 0, recalculate avarage
				double re_av=0.;
				int npix=0;
				
				auto legit_pixels=getLegitPixels(input, col_ID,row_ID, method);
				for(auto legpix : legit_pixels)
				{
					if(tmp_found_leff.GetBinContent(legpix.first,legpix.second)==0) 
					{
						re_av+=input->GetBinContent(legpix.first,legpix.second);
						npix++;
					}
				}
			
				if(npix==0)
				{
					re_av=0;
					for(auto legpix : legit_pixels)
					{
						re_av+=input->GetBinContent(legpix.first,legpix.second);
					}
					
					if(re_av/input->GetBinContent(row_ID,col_ID)<1.5 && re_av/input->GetBinContent(row_ID,col_ID)>0.5) re_av=1;
				}
				else
				{
					re_av/=npix;
				}
						
				double ratio_noisy = m_sensitivity+input->GetBinContent(row_ID,col_ID)/re_av;
				double ratio_leff  = input->GetBinContent(row_ID,col_ID)/re_av; 
				if(re_av==1)
				{
					tmp_eff_leff.SetBinContent(row_ID,col_ID,1);
					tmp_eff_noisy.SetBinContent(row_ID,col_ID,1);
				}
						
				if(row_ID!=input->GetNbinsX() && row_ID!=1 && col_ID!=1 && col_ID!=input->GetNbinsY())
				{
					if( ((ratio_noisy>m_threshNoisy && re_av!=1) || re_av==1) )
					{
						tmp_found_noisy.SetBinContent(row_ID,col_ID,1);
						if(re_av!=1) tmp_eff_noisy.SetBinContent(row_ID,col_ID,100.*ratio_noisy);		
					}
					if(	((ratio_leff<m_threshLEff && re_av!=1) || re_av==1) )
					{
						tmp_found_leff.SetBinContent(row_ID,col_ID,1);  	
						if(re_av!=1) tmp_eff_leff.SetBinContent(row_ID,col_ID,100.*ratio_leff);
					}
				}
			}
		}
	}
	
	return;
}
			

std::vector<std::vector<double>> AFP_NoisyPixelTool::makeFits(std::shared_ptr<const TH2F> input) const
{
	const int nParams=7;
	std::vector<std::vector<double>> params(nParams, std::vector<double>(input->GetNbinsX()+1, 0.));
	
	for(int row_ID=0; row_ID<=input->GetNbinsX(); row_ID++)
	{
		std::string srow_ID = std::to_string(row_ID);
		std::unique_ptr<TH1F> hist(new TH1F(("ROW"+srow_ID).c_str(), ("ROW"+srow_ID).c_str(), input->GetNbinsY(),  0.5,  input->GetNbinsX()+0.5));
		
		for(int col_ID=1; col_ID<=input->GetNbinsY(); col_ID++)
		{  
			hist->Fill(col_ID, 1.0*input->GetBinContent(row_ID,col_ID)); 
		}
		
		//Set double gaussian fit with starting parameters
		std::shared_ptr<TF1> FIT_2(new TF1("gaus","gaus(0)+gaus(3)+[6]",0,input->GetNbinsY(),nParams));
		if(row_ID==1)
		{
			FIT_2->SetParameters(25000,hist->GetMean(),hist->GetStdDev()/std::sqrt(2),
			                      5000,hist->GetMean(),hist->GetStdDev()/std::sqrt(2),
			                     std::max(hist->GetBinContent(2),std::max(hist->GetBinContent(3),hist->GetBinContent(4))) );
		}
		else
		{
			for(int par=0;par<nParams;++par)
			{
				FIT_2->SetParameter(par,params.at(par).at(row_ID-1));
			}
		}
		
		//fit
		hist->Fit(FIT_2.get(),"Q");
		
		for(int par=0;par<nParams;++par)
		{
			params.at(par).at(row_ID-1)=FIT_2->GetParameter(par);
		}
	}
	
	return params;
}


