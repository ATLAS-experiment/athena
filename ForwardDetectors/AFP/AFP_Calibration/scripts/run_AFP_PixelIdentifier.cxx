// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


#include <TROOT.h>
#include <TFile.h>

#include <iostream>
#include <string>
#include <boost/program_options.hpp>

#include "AFP_Calibration/AFP_PixelIdentifier.h"


int main(int argc, char *argv[])
{
	
	std::string input_file_name;
	std::string output_file_name;
	std::vector<std::string> pixel_tools_list;
	
	boost::program_options::options_description main_options("main options");
	
	main_options.add_options()
	("input_file",boost::program_options::value<std::string>(&input_file_name)->default_value("AFP_PixelHistoFiller.root"),"name of output root file")
	("output_file",boost::program_options::value<std::string>(&output_file_name)->default_value("AFP_PixelIdentifier.root"),"name of output root file")
	("pixel_tools_list",boost::program_options::value<std::vector<std::string> >(&pixel_tools_list)->multitoken(), "list of AFP pixel tools");
	;
	
	boost::program_options::variables_map vm;

	try
	{
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, main_options), vm);
		boost::program_options::notify(vm);
	}
	catch(std::exception& e)
	{
		std::cerr << "Bad command line argument" << std::endl;
		std::cerr << e.what() << std::endl;
		return 1;
	}
	
	if(pixel_tools_list.empty())
	{
		pixel_tools_list.push_back("AFP_DeadPixel");
		pixel_tools_list.push_back("AFP_NoisyPixel");
	}
	
	AFP_PixelIdentifier identifier(input_file_name, output_file_name, pixel_tools_list);
	identifier.execute();
	
	return 0;
}






