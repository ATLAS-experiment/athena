/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#ifndef dqutilsHistogramDataCOOL_h
#define dqutilsHistogramDataCOOL_h

// Protect CINT from some system definitions that cause problems
#ifndef __CINT__
//COOL API include files (CoolKernel)
  #include "CoolKernel/pointers.h"
  #include "CoolKernel/ValidityKey.h"
#else
namespace cool {
  class IDatabasePtr {};
  class IFolderPtr {};
}
#endif





#include <TObject.h>

//CORAL API include files
#include "CoralBase/AttributeList.h"

//COOL API include files (CoolApplication)
#include "CoolApplication/Application.h"
// --> limits.h is needed for CoolKernel/types.h
#include <limits.h>
#include "CoolKernel/types.h"
#include "CoolKernel/ChannelId.h"
#include "CoolKernel/RecordSpecification.h"
#include "CoolKernel/ChannelSelection.h"

#include "CxxUtils/checker_macros.h"

#include <string>

class TGraphErrors;
class TCanvas;

namespace coral {
  class AttributeList;
}

namespace cool {
  class RecordSpecification;
  class ChannelSelection;
}


namespace dqutils {
  class ATLAS_NOT_THREAD_SAFE HistogramDataCOOL: public cool::Application, public TObject {
  private:
// Protect CINT from some system definitions that cause problems
// CINT does not need to know about these private variables
#ifndef __CINT__
    cool::ValidityKey m_since {};
    cool::ValidityKey m_until {};
    cool::IDatabasePtr m_coolDb {};
    cool::IFolderPtr m_coolFolder {};
    cool::IFolderPtr m_coolFolderH {};
#endif
  public:
    // Connects to the database. Throws a "DatabaseDoesNotExis" exception if database does not exist.
    cool::IDatabasePtr coolDbInstance(const std::string& dbStr, bool readOnly);

    // Browses the COOL folder. Throws a "FolderNotFound" exception if folder does not exist.
    cool::IFolderPtr coolFolderInstance(const std::string& folderStr);

    // Various methods to set and print the intervall of validity.
    void setSince(cool::Int64 run, cool::Int64 lumi);
    void setUntil(cool::Int64 run, cool::Int64 lumi);
    void printIOV();
    void setIOV(cool::Int64 runS, cool::Int64 lumiS, cool::Int64 runU, cool::Int64 lumiU);
    void setIOV(cool::Int64 run);

    // Methods needed to come up to COOL framework.
    cool::RecordSpecification createSpec();
    coral::AttributeList createPayload(int colourCode, float dfrac, float thrust, const
                                       cool::RecordSpecification& spec);
    cool::RecordSpecification createSpecH();
    coral::AttributeList createPayloadH(int colourCode,
                                        const std::string& algo,
                                        int entries, float par1, float par2, float par3, float par4, float par5, const
                                        cool::RecordSpecification& specH);

    // Constructors and Destructors.
    HistogramDataCOOL(const std::string& dbStr, const std::string& folderStr, int runS, int lumiS, int runU, int lumiU);
    HistogramDataCOOL(int runS, int lumiS, int runU, int lumiU);
    HistogramDataCOOL();
    virtual ~HistogramDataCOOL ();

    // Methods of "interest". To insert a status flag you can use either ChannelId or ChannelName.
    void dump(cool::ChannelSelection selection, const std::string & tag_name);
    std::string dumpHisto(cool::ChannelId channelId, const std::string &field, const std::string &tag_name);
    int dumpCode(const std::string& channelName, const std::string& tag_name);
    void dumpall(const std::string& tag_name);

    void insertH(cool::ChannelId channelId, int code,
                 const std::string& algo,
                 int entries, float par1, float par2, float par3, float par4, float par5,
                 const std::string& tag_name);
    void insertH(const std::string & channelName, int code, const std::string & algo, int entries, float par1, float par2, float par3,
                 float par4, float par5, const std::string & tag_name);

    virtual void ntupleDB(int HistoId, const std::string& nameHisto, const std::string& configuration, int Irun,
                          int Frun);
    virtual void historyDB(int HistoId, const std::string& nameHisto, const std::string& tag_name);
    std::string defParName(const std::string& Algorithm, const std::string& nameHisto, int i);
    virtual void formatGraph(TCanvas* c, TGraphErrors* gr) const;

    // Some further getters.
    cool::IFolderPtr getCoolFolderH();
    cool::IDatabasePtr getCoolDb();


    // Needed for the ROOT interface.
    ClassDef(HistogramDataCOOL, 0) // A class for modifying DQ info in the COOL database
  };
}

#endif
