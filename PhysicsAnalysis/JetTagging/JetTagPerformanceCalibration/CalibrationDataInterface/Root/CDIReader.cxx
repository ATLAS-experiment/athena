/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////
// CDIReader.h, (c) ATLAS Detector software
//////////////////////////////////////////////////////////////////////

#include "CalibrationDataInterface/CDIReader.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

Analysis::CDIReader::CDIReader(const std::string& cdipath, bool verbose) :  m_use_json(false), m_cdipath(cdipath), m_CDIFile(TFile::Open(m_cdipath.c_str(), "READ"))
{
    TObjString* s;
    m_CDIFile->GetObject("VersionInfo/BuildNumber", s);
    if (s && verbose){
        std::cout << " CDI file build number: " << s->GetName() << std::endl;
    }
    TList* taggerkeys = m_CDIFile->GetListOfKeys();
    for (const auto tagger : *taggerkeys){
      const char* taggername = tagger->GetName();
      if(strcmp(taggername, "VersionInfo") != 0){
        // now we have the top-level tagger name, we want to add this to our overall meta data
        TDirectoryFile* taggerDir = (TDirectoryFile*)m_CDIFile->Get(taggername);
        record_metadata(taggername, 0);
        crawlCDI(taggerDir, 0, taggername);
      }
    }
}


void Analysis::CDIReader::crawlCDI(TDirectoryFile* parentDir, int depth, const std::string& metamap){

  TList* keys = parentDir->GetListOfKeys(); // get directories
  if(isWPdirectory(keys)){
    // now that we are at the level of the individual label directories
    // interact with them, and extract the data they store

    TList* labelkeys = parentDir->GetListOfKeys();
    Data theseData; // labels, systematics, DSIDS, etc.
    Labels theseLabels;
    std::set<std::string> DSID_set; // record all DSID names in the flavour configuration
    std::set<std::string> systematics_set; // record all uncertainties met in flavour configuration

    Labels path = split(metamap);
    std::string taggername = path.at(0);
    std::string jetcollname = path.at(1);
    std::string workingpointname = path.at(2);

    // For each "label" stored in the working point directory,
    // we need to access the contents of the label directory
    // and construct the metadata map
    for(const auto label : *labelkeys){
      std::string labelname = label->GetName();
      if(labelname == "cutvalue" || labelname.find("fraction") != std::string::npos) continue;
      m_labels.insert(labelname);
      theseLabels.push_back(labelname);
      
      // now enter the directory to access uncertainty info for this flavour
      TDirectoryFile* flavourDir = (TDirectoryFile*)parentDir->Get(labelname.c_str());
      if(flavourDir){
        Labels uncertainties; // flavour specific uncertainties
        TList* DSIDkeys = flavourDir->GetListOfKeys(); // this is the list of all the items in the flavour (DSID etc)
        for(const auto CDHistCont : *DSIDkeys){
          std::string DSIDname = CDHistCont->GetName();
          DSID_set.insert(DSIDname);
          m_DSIDs.insert(DSIDname);
          if(DSIDname == "default_SF"){ // let's access the systematic uncertainties
            // construct the total path
            std::string dir = taggername + "/" + jetcollname + "/" + workingpointname + "/" + labelname + "/default_SF";
            Analysis::CalibrationDataHistogramContainer* cont;
            m_CDIFile->GetObject(dir.c_str(), cont);
            if(!cont){
              std::cout << "No default_SF CalibrationDataHistogramContainer?" << std::endl;
            } else { 
              uncertainties = cont->listUncertainties();
              for(const std::string &s : uncertainties){
                systematics_set.insert(s);
              }
            }
            // add the flavour specific uncertainties here
            std::string flav_spec_unc_name = labelname + "_syst";
            theseData[flav_spec_unc_name] = uncertainties;
          }
        }
      } else {
        std::cout << "No flavour directory?" << std::endl;
      }
    }
    // sort and add the labels to the Data object
    std::sort(theseLabels.begin(), theseLabels.end());
    theseData["labels"] = std::move(theseLabels);
    // convert DSID set to vector of strings
    Labels theseDSIDs(DSID_set.size());
    std::copy(DSID_set.begin(), DSID_set.end(), theseDSIDs.begin());
    theseData["DSIDs"] = std::move(theseDSIDs);
    // convert systematic set to vector of strings
    Labels theseSystematics(systematics_set.size());
    std::copy(systematics_set.begin(), systematics_set.end(), theseSystematics.begin());
    theseData["systematics"] = std::move(theseSystematics);
    // Construct this branch of the metadata map
    // and record the Data object
    record_metadata_map(theseData, metamap);
  } else {
    for(const auto coll: *keys){
      std::string collname = coll->GetName();
      if ( collname.find("cutvalue") != std::string::npos || collname.find("fraction") != std::string::npos) continue;
      // track the metadata as you traverse
      record_metadata(collname, depth+1);
      TDirectoryFile* collDir = (TDirectoryFile*)parentDir->Get(collname.c_str());
      if(collDir && collname != "VersionInfo"){
        std::string nextmap = metamap + ";" + collname;
        crawlCDI(collDir, depth+1, nextmap); // traverse further
      } else {
        std::cout << "No collection directory?" << std::endl;
      }
    }
  }
}




void Analysis::CDIReader::printMetadata(int tagger, int jetcoll, int wpoint, int label){
  /*
    This method prints subsets of the available metadata collected from the CDI file.
    
    The CDI data is organized in a hierarchical directory structure, where taggers contain jet-collections, and jet-collections
    contain working points, etc. 
    
    This method prints wildcard data such as:
    - tagger / *
    - tagger / jetcoll/ *
    - tagger / jetcoll/ wpoint / *
    - tagger / jetcoll / wpoint / label
    
    The method operates on simple integer inputs, to indicate what to print out:
    - Positive integer values for tagger, jetcoll, wpoint, and label (up to the number of available items of each)
      will print out information specific to that particular entry (e.g. tagger == 1 will print tagger #1 specific info)
    - Negative integer values indicate a wildcard, and will print out all available items of this type.
    - Zero indicates to print nothing for that category.
  */
  int current_tagger = (tagger < 0) ? -1 : 0;
  int current_jetcoll = (jetcoll < 0) ? -1 : 0;
  int current_wpoint = (wpoint < 0) ? -1 : 0;
  int current_label = (label < 0) ? -1 : 0;

  for (const auto& [tag, jets] : m_metadata){
    if(current_tagger != -1) current_tagger += 1;
    if(tagger != current_tagger || tagger == 0) continue;

    std::cout << "| " << tag << std::endl; // print only the tagger you're interested in

    for (const auto& [jet, wps] : jets){
      if(current_jetcoll != -1) current_jetcoll += 1;
      if(jetcoll != current_jetcoll || jetcoll == 0) continue;

      std::cout << "|\\__ " << jet << std::endl;

      int num_wps = wps.size();
      int num_wp_seen = 0;
      for(const auto& [wp, labels] : wps){
        if(current_wpoint != -1) current_wpoint += 1;
        if(wpoint != current_wpoint || wpoint == 0) continue;
        num_wp_seen += 1;
        if(num_wp_seen != num_wps){
          std::cout << "|   |\\__" << wp << std::endl;
        } else {
          std::cout << "|    \\__" << wp << std::endl;
        }

        int label_index = 0;
        Data d = labels;
        for(const std::string& l : d["labels"]){
          if(current_label != -1) current_label += 1;
          if(label != current_label || label == 0) continue;
          if(num_wp_seen != num_wps && label_index == 0){
            std::cout << "|   |   \\___" << " (" << label_index << ") " << l << std::endl;
          } else if(label_index != 0 && num_wp_seen != num_wps) {
            std::cout << "|   |   \\___" << " (" << label_index << ") " << l  << std::endl;
          } else {
            std::cout << "|       \\___" << " (" << label_index << ") " << l  << std::endl;
          }
          label_index += 1;
        }
      }
    }
  }
}

bool Analysis::CDIReader::checkConfig(const std::string& tagger, const std::string& jetcoll, const std::string& wp, bool verbose){
  // this method checks if your config is correct or not
  // returns true if correct, false if not
  // if not correct, it will also print a helpful message
  bool configured = false;
  // get the number that would correspond to the index of the tagger/jetcoll/wp
  // if these were each stored in a vector of strings, sorted alphanumerically
  // which happens already by default in the (ordered) map
  int tagger_ind = 0;
  int jetcoll_ind = 0;
  int wp_ind = 0;

  if(m_metadata.count(tagger) > 0){
    // get the tagger index
    for(const auto& tag : m_metadata){
      tagger_ind += 1;
      if(tag.first == tagger) break;
    }
    if(m_metadata[tagger].count(jetcoll) > 0){
      // get the jet collection index
      for(const auto& jet : m_metadata[tagger]){
        jetcoll_ind += 1;
        if(jet.first == jetcoll) break;
      }
      if(m_metadata[tagger][jetcoll].count(wp) > 0){
        // get the working point index
        for(const auto& wpoint : m_metadata[tagger][jetcoll]){
          wp_ind += 1;
          if(wpoint.first == wp) break;
        }
        if (verbose) std::cout << " Your configuration looks good! Available labels are : " << std::endl;
        if (verbose) printMetadata(tagger_ind, jetcoll_ind, wp_ind, -1);
        // construct vector of labels
        for(const std::string& flavour_label : m_metadata[tagger][jetcoll][wp]["labels"]){
          m_label_vec.push_back(flavour_label);
        }
        // sort the vector of labels
        std::sort(m_label_vec.begin(), m_label_vec.end());
        configured = true;
      } else {
        if (verbose) std::cout << "Couldn't find \"" << wp << "\" for " << tagger << " / " << jetcoll << " in this CDI file!" <<  std::endl;
        if (verbose) std::cout << "Here are your options :" << std::endl;
        if (verbose) printMetadata(tagger_ind, jetcoll_ind, -1, 0);
      }
    } else {
      if (verbose) std::cout << "Couldn't find \"" << jetcoll << "\" under " << tagger << " in this CDI file!" <<  std::endl;
      if (verbose) std::cout << "Here are your options :" << std::endl;
      if (verbose) printMetadata(tagger_ind, -1, 0, 0);
    }
  } else {
    if (verbose) std::cout << "Couldn't find \"" << tagger << "\" in this CDI file" <<  std::endl;
    if (verbose) std::cout << "Here are your options :" << std::endl;
    if (verbose) printMetadata(-1,0,0,0);
  }

  if(m_use_json){
    // let's make a json object from the nlohmann package and save it to file
    json json_metadata(m_metadata);
    // get cwd to save in
    std::filesystem::path cwd = std::filesystem::current_path();
    std::filesystem::path filepath = cwd / "CDI.json";
    std::ofstream output(filepath);
    output << std::setw(4) << json_metadata << std::endl;
  }

  m_initialized = configured;
  return configured;
}


std::vector<std::string> Analysis::CDIReader::getDSIDs(const std::string& tagger, const std::string& jetcollection, const std::string& workingpoint){
  if(!m_initialized){
    std::cout << " CDIReader :: You need to validate your configuration before working with (flavour) labels!" << std::endl;
  }
  Labels DSIDs;
  if (tagger.empty() || jetcollection.empty() || workingpoint.empty()){
    // unless specified, return the vector of all known DSIDs from this CDI file
    Labels DSID_vec(m_DSIDs.size());
    std::copy(m_DSIDs.begin(), m_DSIDs.end(), DSID_vec.begin());
    DSIDs = std::move(DSID_vec);
  //} else if(std::find(m_taggers.begin(), m_taggers.end(), tagger) == m_taggers.end()){
  } else if(m_taggers.find(tagger) == m_taggers.end()){
    std::cout << " The tagger [" << tagger << "] doesn't exist in this CDI file!" << std::endl;
  //} else if(std::find(m_jetcollections.begin(), m_jetcollections.end(), jetcollection) == m_jetcollections.end()){
  } else if(m_jetcollections.find(jetcollection) == m_jetcollections.end()){
    std::cout << " The jet collection [" << jetcollection << "] doesn't exist in " << tagger << " this CDI file!" << std::endl;
  //} else if(std::find(m_workingpoints.begin(), m_workingpoints.end(), workingpoint) == m_workingpoints.end()){
  } else if(m_workingpoints.find(workingpoint) == m_workingpoints.end()){
    std::cout << " The working point [" << workingpoint << "] doesn't exist in " << tagger << "/" << jetcollection <<  " this CDI file!" << std::endl;
  } else {
    for (const std::string& DSID : m_metadata[tagger][jetcollection][workingpoint]["DSIDs"]){
      DSIDs.push_back(DSID);
    }
  }
  return DSIDs;
}

std::vector<std::string> Analysis::CDIReader::getLabels(const std::string& tagger, const std::string& jetcollection, const std::string& workingpoint){
  if(!m_initialized){
    std::cout << " CDIReader :: You need to validate your configuration before working with (flavour) labels!" << std::endl;
  }
  Labels labels;
  if (tagger.empty() || jetcollection.empty() || workingpoint.empty()){
    // unless specified, return the vector of all flavour labels
    labels = m_label_vec;
  } else if(m_taggers.find(tagger) == m_taggers.end()){
    std::cout << " The tagger [" << tagger << "] doesn't exist in this CDI file!" << std::endl;
  } else if(m_jetcollections.find(jetcollection) == m_jetcollections.end()){
    std::cout << " The jet collection [" << jetcollection << "] doesn't exist in " << tagger << " this CDI file!" << std::endl;
  } else if(m_workingpoints.find(workingpoint) == m_workingpoints.end()){
    std::cout << " The working point [" << workingpoint << "] doesn't exist in " << tagger << "/" << jetcollection <<  " this CDI file!" << std::endl;
  } else {
    for (const std::string& label : m_metadata[tagger][jetcollection][workingpoint]["labels"]){
      labels.push_back(label);
    }
  }
  return labels;
}

std::vector<std::string> Analysis::CDIReader::getJetCollections(const std::string& tagger){
  Labels jetcolls;
  if (tagger.empty()){
    // unless specified, return the vector of all jet collections
    jetcolls.assign(m_jetcollections.begin(), m_jetcollections.end());
  } else if (m_taggers.find(tagger) == m_taggers.end()) {
    std::cout << " The tagger [" << tagger << "] doesn't exist in this CDI file!" << std::endl;
  } else {
    // return the jetcollections of this tagger
    for(const auto& jet : m_metadata[tagger]){
      jetcolls.push_back(jet.first);
    }
  }
  return jetcolls;
}


std::vector<std::string> Analysis::CDIReader::getWorkingPoints(const std::string& tagger, const std::string& jetcollection){
  Labels wps;
  if (tagger.empty() || jetcollection.empty()){
    // unless specified, return the vector of all working points
    wps.assign(m_workingpoints.begin(), m_workingpoints.end());
  } else if(m_taggers.find(tagger) == m_taggers.end()){
    std::cout << " The tagger [" << tagger << "] doesn't exist in this CDI file!" << std::endl;
  } else if(m_jetcollections.find(jetcollection) == m_jetcollections.end()){
    std::cout << " The jet collection [" << jetcollection << "] doesn't exist in " << tagger << " this CDI file!" << std::endl;
  } else {
    for(const auto& wp : m_metadata[tagger][jetcollection]){
      wps.push_back(wp.first);
    }
  }
  return wps;
}

std::vector<std::string> Analysis::CDIReader::getTaggers(){
  Labels taggers;
  taggers.assign(m_taggers.begin(), m_taggers.end());
  return taggers;
}
