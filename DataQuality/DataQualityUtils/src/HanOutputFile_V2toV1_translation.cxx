/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
#include <iostream>
#include <nlohmann/json.hpp>  //to write JSON strings
#include <typeinfo>           //to print the type of variable
#include <TFile.h>
#include <TH1.h>
#include <TKey.h>
#include <TObjString.h>
#include <TROOT.h>  //To use gRoot
#include <gperftools/profiler.h>

#include <chrono>   //To measure time
#include <cstring>  //to convert string to char array (to save json file as TObjString)

// compilation
// g++ -std=c++11 HanOutputFile_V2toV1_translation.cxx -O2 `root-config --cflags` `root-config --libs --glibs` -o
// HanOutputFile_V2toV1_translation

// Function, that converts JSON string back to TDirectory structure
void from_JSON_to_TDirectory(nlohmann::json str_content, TDirectory* place_to_save);
// Function, that converts back ATLAS file from new vesrion to old version
void conversion_back(TObject* obj_in, TObject* obj_to);
// Function, that checks, if "dirname" TDirectory exists inside obj_in or not
int dir_exists(const std::string& dirname, TObject* obj_in);

int main() {
  TFile* f = new TFile("run_364030_lowStat_LB121-140_han_converted.root");
  TFile* f_output = new TFile("run_364030_lowStat_LB121-140_han_converted_back.root", "recreate");
  // Find a TObjString with JSON inside
  auto start = std::chrono::system_clock::now();

  conversion_back(f, f_output);
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";

  f->Close();  // Close the file
  f_output->Close();
  return 0;
}

void conversion_back(TObject* obj_in, TObject* obj_to) {
  TObjString* TOS_w_JSON;  // JSON string

  std::string content;
  std::string obj_input_type = obj_in->ClassName();
  bool is_file;
  if (obj_input_type == "TFile") {
    is_file = true;
  } else {
    is_file = false;
  }
  TDirectory* obj_in_dir = static_cast<TDirectory*> (obj_in);
  TDirectory* obj_to_dir = static_cast<TDirectory*> (obj_to);
  std::string name = obj_in->GetName();
  TDirectory* copy_dir;
  if (is_file == false) {
    copy_dir = obj_to_dir->mkdir(name);
  } else {
    copy_dir = obj_to_dir;
  }
  copy_dir->cd();

  TIter next(obj_in_dir->GetListOfKeys());
  TKey* key;
  while ((key = (TKey*) next())) {
    TObject* obj_inside;
    std::string key_name = key->GetName();
    std::string key_type = key->GetClassName();
    if (key_type == "TObjString") {
      if (key_name == "File_version") { // File version is a flag of new version of files
        continue;
      } else {
        obj_in_dir->GetObject(key_name, TOS_w_JSON);
        // Get JSON from TObjString
        content = TOS_w_JSON->GetString();
        TDirectoryFile* idir;
        if (dir_exists(key_name, copy_dir) == 0) { // When Reference hist should be saved to Results dir
                                                   // there is no Results dir at that moment and we created it. So, when
                                                   // it's time to process
                                                   // JOSN string "Results", we shouldn't create Results directory again
          copy_dir->cd();
          idir = new TDirectoryFile(key_name, "");
        } else {
          copy_dir->GetObject(key_name, idir);
        }
        using json = nlohmann::json;
        // Get JSON from TObjString
        auto j = json::parse(content);
        from_JSON_to_TDirectory(j, idir);
        copy_dir->cd();
        idir->Write();
        delete idir;
        delete TOS_w_JSON;
      }
    }
    // We process hist another way, than TObjstrings and TDirectories
    else if (key_type == "TH1I" || key_type == "TH2I" || key_type == "TH1F" || key_type == "TH2F" ||
             key_type == "TProfile2D" || key_type == "TProfile" || key_type == "TGraphAsymmErrors" ||
             key_type == "TGraphErrors" || key_type == "TH1D" || key_type == "TH2S") {
      obj_inside = obj_in_dir->GetKey(key_name)->ReadObj();
      copy_dir->cd();
      if (key_name == "Reference") { // We should place "Reference" hists to result in old-version files
        if (!dir_exists("Results", copy_dir)) {
          copy_dir->mkdir("Results");
        }
        copy_dir->cd("Results");
      }
      obj_inside->Write(key_name);
      copy_dir->cd();
      delete obj_inside;
    } else if (key_type == "TDirectoryFile") {
      obj_inside = obj_in_dir->GetKey(key_name)->ReadObj();
      conversion_back(obj_inside, copy_dir);
      delete obj_inside;
    }
  }
}

void from_JSON_to_TDirectory(nlohmann::json str_content, TDirectory* place_to_save) {
  using json = nlohmann::json;
  // Get the size of JSON
  int size = str_content.size();
  int num_of_key = 0;
  for (json::iterator it = str_content.begin(); it != str_content.end(); ++it) {
    // Create array of subdirectories
    TDirectory* nextLevelDirs[size];
    // Keyname of json will become subdirectory name
    const char* keyname = it.key().c_str();
    auto valuestring = it.value();
    // Now lets make subdir in our dir
    if (dir_exists(keyname, place_to_save) == 0) {
      place_to_save->mkdir(keyname);
    }
    place_to_save->cd(keyname);
    nextLevelDirs[num_of_key] = gDirectory;
    if (strncmp(valuestring.type_name(), "string", 6) == 0) {
      TObjString leaf;
      nextLevelDirs[num_of_key]->cd();
      std::string string_name = valuestring.dump();
      leaf.SetString(string_name);
      nextLevelDirs[num_of_key]->WriteTObject(&leaf, string_name);
      // leaf.Write();
    } else {
      from_JSON_to_TDirectory(valuestring, nextLevelDirs[num_of_key]);
    }
    num_of_key++;
  }
  return;
}

int dir_exists(const std::string& dirname, TObject* obj_in) {
  TDirectory* obj_in_dir = static_cast<TDirectory*> (obj_in);
  TList* keys = obj_in_dir->GetListOfKeys();
  TKey* k = (TKey*) keys->FindObject(dirname);

  if (k && !strcmp(k->GetClassName(), "TDirectoryFile")) {
    return 1;
  } else {
    return 0;
  }
  return 0;
}
