/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// $Id: PMGCrossSectionTool.cxx 764400 2016-07-26 17:47:39Z tripiana $

#include <fstream>
#include <sstream>
#include <string>
#include <TSystem.h>
#include <TString.h>
#include <stdlib.h>
#include <limits>

// Local include(s):
#include "PMGTools/PMGCrossSectionTool.h"

namespace PMGTools {

PMGCrossSectionTool::
PMGCrossSectionTool(const std::string& name) : asg::AsgTool(name) { }


StatusCode PMGCrossSectionTool::initialize() {

  // Tell the user what's happening:
  ATH_MSG_INFO( "Initializing " << name() << "..." );
  //ATH_MSG_INFO( "Read in all Xsec Info ..." );

  //readInfosFromDir(inputDir.c_str());

  // Return gracefully:
  return StatusCode::SUCCESS;
}


bool PMGCrossSectionTool::readInfosFromFiles(const std::vector<std::string> & InputFiles)
{

  for (const auto& currentFileName : InputFiles) {

    std::ifstream currentFile(currentFileName);
    if (not currentFile.is_open()) {
      ATH_MSG_WARNING("cannot open file " << currentFileName);
      continue;
    }

    // skip first line
    currentFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    int nfound = 0;  // for this file
    for (std::string line; std::getline(currentFile, line); ) {
      std::stringstream input_line(line);
      AllSampleInfo help;

      input_line >> help.dsid;
      input_line >> help.containerName;
      input_line >> help.amiXSec;
      input_line >> help.filterEff;
      input_line >> help.kFactor;
      input_line >> help.XSecUncUP;
      input_line >> help.XSecUncDOWN;
      // we don't care about the generator name here
      // let's skip it
      std::string dummy;
      input_line >> dummy;

      std::string etag;
      input_line >> etag;
      etag = etag.substr(1);
      help.etag = std::stoi(etag);
      // :: below is for future use?
      //input_line >> help.br;
      //input_line >> help.higherOrderXsecTotal;
      //input_line >> help.higherOrderXsecSample; // this is hoXsec * filter eff

      if (input_line.fail()) { ATH_MSG_ERROR("cannot parse line '" << line << "' from file " << currentFileName); continue; }

      m_fStoreSampleInfo[{help.dsid, -1}] = help;
      m_fStoreSampleInfo[{help.dsid, help.etag}] = help;
      ++nfound;
    }

    if (nfound == 0) { ATH_MSG_WARNING("no sample read from file " << currentFileName); }
  }

  if (m_fStoreSampleInfo.empty()) {
    ATH_MSG_ERROR("list of sample is empty");
    return false;
  }

  return true;
}


bool PMGCrossSectionTool::readInfosFromDir(const std::string& inputDir)
{

  TString mydir = inputDir;
  gSystem->ExpandPathName (mydir);
  void *dirp = 0;

  std::vector<std::string> inFiles = {};

  try
  {
    dirp = gSystem->OpenDirectory (mydir.Data());
    const char *file = 0;
    while ((file = gSystem->GetDirEntry (dirp)))
    {
      std::string myfile = inputDir + "/" + file;
      if (myfile.size() > 4 && myfile.substr (myfile.size() - 4) == ".txt")
        inFiles.push_back(std::move(myfile));
    }
    gSystem->FreeDirectory (dirp);
  }
  catch (...)
  {
    gSystem->FreeDirectory (dirp);
    //throw;
    return false;
  }

  readInfosFromFiles(inFiles);
  return true;
}


double PMGCrossSectionTool::getFilterEff(const int dsid, const int etag) const
{
  const auto it = m_fStoreSampleInfo.find({dsid, etag});
  if (it != m_fStoreSampleInfo.end()) { return it->second.filterEff; }

  ATH_MSG_ERROR("Sample with DSID " << dsid << " and e-tag " << etag << " has no info stored!!!");
  return -1;
}


std::string PMGCrossSectionTool::getSampleName(const int dsid, const int etag) const
{
  const auto it = m_fStoreSampleInfo.find({dsid, etag});
  if (it != m_fStoreSampleInfo.end()) { return it->second.containerName; }

  ATH_MSG_ERROR("Sample with DSID " << dsid << " and e-tag " << etag << " has no info stored!!!");
  return std::string("");
}


double PMGCrossSectionTool::getAMIXsection(const int dsid, const int etag) const
{
  const auto it = m_fStoreSampleInfo.find({dsid, etag});
  if (it != m_fStoreSampleInfo.end()) { return it->second.amiXSec; }

  ATH_MSG_ERROR("Sample with DSID " << dsid << " and e-tag " << etag << " has no info stored!!!");
  return -1;
}


// :: below is for future use?
/*double PMGCrossSectionTool::getBR(const int dsid, const int etag) const
{
  const auto it = m_fStoreSampleInfo.find({dsid, etag});
  if (it != m_fStoreSampleInfo.end()) { return it->second.br; }

  ATH_MSG_ERROR("Sample with DSID " << dsid << " and e-tag " << etag << " has no info stored!!!");
  return -1;

}*/


double PMGCrossSectionTool::getXsectionUncertaintyUP(const int dsid, const int etag) const
{
  const auto it = m_fStoreSampleInfo.find({dsid, etag});
  if (it != m_fStoreSampleInfo.end()) { return it->second.XSecUncUP; }

  ATH_MSG_ERROR("Sample with DSID " << dsid << " and e-tag " << etag << " has no info stored!!!");
  return -1;
}

double PMGCrossSectionTool::getXsectionUncertaintyDOWN(const int dsid, const int etag) const
{
  const auto it = m_fStoreSampleInfo.find({dsid, etag});
  if (it != m_fStoreSampleInfo.end()) { return it->second.XSecUncDOWN; }
  
  ATH_MSG_ERROR("Sample with DSID " << dsid << " and e-tag " << etag << " has no info stored!!!");
  return -1;
}

double PMGCrossSectionTool::getXsectionUncertainty(const int dsid, const int etag) const
{
  //symmetrize the up and down variations
  return 0.5*( fabs(getXsectionUncertaintyDOWN(dsid, etag)) + fabs(getXsectionUncertaintyUP(dsid, etag)) );
}

double PMGCrossSectionTool::getKfactor(const int dsid, const int etag) const
{
  const auto it = m_fStoreSampleInfo.find({dsid, etag});
  if (it != m_fStoreSampleInfo.end()) { return it->second.kFactor; }

  ATH_MSG_ERROR("Sample with DSID " << dsid << " and e-tag " << etag << "  has no info stored!!!");
  return -1;
}


// :: below is for future use?
double PMGCrossSectionTool::getSampleXsection(const int dsid, const int etag) const
{
  const auto it = m_fStoreSampleInfo.find({dsid, etag});
  if (it != m_fStoreSampleInfo.end()) { return it->second.amiXSec * it->second.kFactor * it->second.filterEff; }
    // :: below is for future use?
    //return info.higherOrderXsecSample;

  ATH_MSG_ERROR("Sample with DSID " << dsid << " and e-tag " << etag << " has no info stored!!!");
  return -1;
}


std::vector<int> PMGCrossSectionTool::getLoadedDSIDs() const 
{
  std::vector<int> dsids;
  dsids.reserve(m_fStoreSampleInfo.size());
  for (const auto& key_info : m_fStoreSampleInfo) {
    dsids.push_back(key_info.second.dsid);
  }
  // DSIDs can appear multiple times with different e-tags
  // remove duplicates here
  std::sort( dsids.begin(), dsids.end() );
  dsids.erase( std::unique( dsids.begin(), dsids.end() ), dsids.end() );

  return dsids;
}

} // end namespace
