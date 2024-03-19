/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef dqutilsHanOutputFile_h
#define dqutilsHanOutputFile_h

#include <TObjString.h>
#include <TObject.h>

#include <iosfwd>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "CxxUtils/checker_macros.h"

class TCanvas;
class TDirectory;
class TFile;
class TH1;
class TH2;
class TStyle;
class TGraph;
class TImage;
class TEfficiency;

namespace dqutils
{

  class ATLAS_NOT_THREAD_SAFE HanOutputFile : public TObject
  {
  public:
    HanOutputFile();

    HanOutputFile(const std::string& fileName);

    virtual ~HanOutputFile();

    typedef std::map<std::string, TDirectory*> DirMap_t;
    typedef std::map<std::string, TObject*> DirStrMap_t;  /// DirMap_t but For Version 2 files
    typedef std::map<std::string, std::string> AssMap_t;
    typedef std::map<std::string, AssMap_t*> DirToAssMap_t;

    static void getAllGroupDirs(DirMap_t& dirmap, TDirectory* dir, std::string dirName);
    /**
     * @brief Same as getAllGroupDirs, but works with Version 2.3 files
     *
     * getAllGroupDirs ignores types other than Tdirectories.
     * getAllGroupDirs_V2 works also with TObjstring, as they are JSON
     */
    static void getAllGroupDirs_V2(DirStrMap_t& dirstrmap, TObject* obj,  // For Version 2 files
      std::string objName);
    static void getAllAssessments(AssMap_t& dirmap, TDirectory* dir);

    /**
     * @brief Print path - and name of Assessment, represented as JSON TObjString
     */
    static void printDQGroupJSON(nlohmann::json j, const std::string& location, const char* path_to_file);

    static std::string getStringName(std::string location, int file_version);
    static std::string getInfo(std::string location, int file_version);
    static std::string getInfo(std::string JSON_str);
    static std::string processJSON_ingetInfo(nlohmann::ordered_json j);

    static std::string getIndentation(const std::string& pathName, const std::string& leadingSpace = "");

    /**
     * Clears all previous data and opens the file with the given name for
     * analysis, returning a boolean indicating whether the file was opened
     * successfully or not.
     */

    virtual bool setFile(const std::string& fileName);

    virtual int getFileVersion();  // V1 or V2

    virtual void printAllDQGroups();

    virtual void printAllDQAssessments();
    virtual void printHistoAssessments();

    virtual std::string stringListSystemPaths(std::string location);
    virtual std::string stringAllDQAssessments();
    virtual std::string stringHistoAssessments();
    virtual std::string stringAllHistograms();

    virtual void streamAllDQAssessments(std::ostream& o, bool streamAll);
    virtual void streamHistoAssessments(std::ostream& o, bool streamAll);
    virtual void streamAllHistograms(std::ostream& o, bool streamAll);  //!

    /**
     * cnvsType: 1=pngOnly;2=jsonOnly;3=pngAndJson
     */
    const static int GENERATE_PNG = 1;   // Make PNG with TImage
    const static int GENERATE_JSON = 2;  // Make JSON
    virtual int saveAllHistograms(std::string location, bool drawRefs, std::string run_min_LB, int cnvsType = 1);
    static bool containsDir(std::string dirname, std::string maindir);
    /**
     * @brief Checks JSON file for a key (by its path)
     *
     * The same as containsDir, but works with JSON in V2 files
     * @param pathInJSON - Only inside JSON (without path to JSON string)
     * @param jsonName - Results or Config
     * @return part of JSON string if PathInJSON exists inJSON
     * @return "JSON exists", if @param pathInJSON == ""
     * @return the whole JSON string, if @param pathInJSON == "/"
     * @return "None" if PathInJSON does not exist inJSON, or the  whole JSON string is absent
     */
    static std::optional<std::string> containsKeyInJSON(
      const std::string& pathInJSON, const std::string& jsonName, const std::string& path_to_JSON);

    virtual bool saveHistogramToFile(std::string nameHis, std::string location, TDirectory* groupDir, bool drawRefs,
      std::string run_min_LB, std::string pathName, int cnvsType = 1);
    virtual std::pair<std::string, std::string> getHistogram(std::string nameHis, TDirectory* groupDir, bool drawRefs,
      std::string run_min_LB, std::string pathName, int cnvsType = 1);
    virtual std::string getHistogramPNG(
      std::string nameHis, TDirectory* groupDir, bool drawRefs, std::string run_min_LB, std::string pathName);
    virtual std::pair<std::string, std::string> getHistogramJSON(
      std::string nameHis, TDirectory* groupDir, bool drawRefs, std::string run_min_LB, std::string pathName);
    virtual bool saveHistogramToFileSuperimposed(std::string nameHis, std::string location, TDirectory* groupDir1,
      TDirectory* groupDir2, bool drawRefs, std::string run_min_LB, std::string pathName, int cnvsType = 1);  //!!!

    virtual bool drawH2(TCanvas* canv, TH2* hist, std::string& drawopt, std::string& display);
    virtual bool drawH1(
      TCanvas* canv, TH1* hist, TH1* reference, std::string& drawopt, std::string& display, std::string& AlgoName);
    virtual bool drawReference(
      TCanvas* canv, TH1* hRef, TH1* h, std::string& drawopt, std::string& display, std::string& AlgoName);
    // virtual bool drawGraph(TCanvas* canv,TGraph* g,std::string &drawopt,std::string &display);
    virtual void ratioplot(TCanvas* myC_main, TH1* h, TH1* href, std::string str);
    virtual void ratioplot2D(TCanvas* canvas, TH2* h2, TH2* h2Ref, std::string display);
    virtual void attachFits(TH1* hist, std::string& drawopt, std::string& display);

    virtual void setupCanvas(std::string& drawopt, std::string& display);
    virtual void polynomial(TCanvas* c, std::string str, TH1* h);

    virtual void displayExtra(TCanvas* c, std::string str);
    virtual void axisOption(std::string str, TH1* h);

    virtual void formatTH1(TCanvas* c, TH1* h) const;
    virtual void formatTH2(TCanvas* c, TH2* h) const;
    virtual void formatTGraph(TCanvas* c, TGraph* g) const;
    virtual void formatTEfficiency(TCanvas* c, TEfficiency* e) const;

    virtual double getNEntries(std::string location, std::string histname);
    virtual double getNEntries(const TObject* obj);

  protected:
    virtual void clearData();
    virtual void convertToGraphics(
      int cnvsType, TCanvas* myC, std::string& json, TImage** img = 0, char** x = 0, int* y = 0);
    virtual void convertToGraphics(int cnvsType, TCanvas* myC, std::string namePNG, std::string nameJSON);
    virtual bool saveFile(
      int cnvsType, std::string pngfName, std::string pngContent, std::string jsonfName, std::string jsonfContent);

    virtual bool writeToFile(std::string fName, std::string content);

    TFile* m_file;
    DirMap_t m_indirMap;
    DirStrMap_t m_indirstrMap;  /// Same Dim_indirMaprMap, but for files of version 2
    DirToAssMap_t m_assessMap;
    TStyle* m_style;

    ClassDef(HanOutputFile, 0)  // Provides functions to manipulate and analyze data-quality monitoring files
  };

}  // namespace dqutils

#endif
