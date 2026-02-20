# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# @file PyUtils.scripts.cmt_newanalysisalg
# @purpose streamline and ease the creation of new athena algs
# @author Will Buttinger
# @date February 2017

#Note - this code could use a serious rewrite, I just hacked it together to get something working

__author__ = "Will Buttinger"
__doc__ = "streamline and ease the creation of new AthAnalysisAlgorithm"

### imports -------------------------------------------------------------------
import os
import textwrap
import subprocess
import PyUtils.acmdlib as acmdlib

class Templates:

    script_template = """\
#!/usr/bin/env python

# Run this application/script like this:
# run%(klass)s.py --filesInput file.root --evtMax 100
# See --help for more arguments and flag options

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()
flags._parser = flags.getArgumentParser(description=\"\"\"My Demo Application\"\"\") # an argparse.ArgumentParser
flags.parser().add_argument('--accessMode',default="POOLAccess",               # can add arguments to the parser as usual
                            choices={"POOLAccess","ClassAccess"},help="Input file reading mode (ClassAccess can be faster but is less supported)")
# changes to default flag values (done before fillFromArgs so appears in the --help flag system
flags.Exec.PrintAlgsSequence = True # displays algsequence at start of job


args = flags.fillFromArgs() # parse command line arguments
flags.lock() # lock the flags


# configure main services and input file reading
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.Enums import Format
cfg = MainServicesCfg(flags)
if flags.Input.Format is Format.BS:
    # read RAW (bytestream)
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    cfg.merge(ByteStreamReadCfg(flags))
else:
    # reading POOL, use argument to decide which read mode
    if flags.args().accessMode == "POOLAccess":
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(flags))
    else:
        from AthenaRootComps.xAODEventSelectorConfig import xAODReadCfg,xAODAccessMode
        cfg.merge(xAODReadCfg(flags, AccessMode = xAODAccessMode.CLASS_ACCESS))

# configure output ROOT files from Output.HISTOutputs flag (should be of form: "STREAMNAME:file.root")
from AthenaConfiguration.ComponentFactory import CompFactory
if flags.Output.HISTFileName != "":
    outputs = []
    for file in (flags.Output.HISTFileName if type(flags.Output.HISTFileName)==list else flags.Output.HISTFileName.split(",")):
        streamName = file.split(":")[0] if ":" in file else "ANALYSIS"
        fileName = file.split(":")[1] if ":" in file else file
        outputs += ["{} DATAFILE='{}' OPT='RECREATE'".format(streamName,fileName)]
    cfg.addService(CompFactory.THistSvc(Output = outputs))

# add our algorithm
cfg.addEventAlgo(CompFactory.%(klass)s(),sequenceName="AthAlgSeq")


# final cfg tweaks before launching:
if cfg.getAppProps()["EventLoop"]=="AthenaEventLoopMgr":
    cfg.getService("AthenaEventLoopMgr").IntervalInSeconds = 5 # enable processing rate reporting every 5s
# suppress logging from some core services that we usually don't care about hearing from
cfg.getService("MessageSvc").setWarning += ["ClassIDSvc","PoolSvc","AthDictLoaderSvc","AthenaPoolAddressProviderSvc",
                                            "ProxyProviderSvc","DBReplicaSvc","MetaDataSvc","MetaDataStore","AthenaPoolCnvSvc",
                                            "TagMetaDataStore","EventSelector","CoreDumpSvc","AthMasterSeq","EventPersistencySvc",
                                            "ActiveStoreSvc","AthOutSeq","AthRegSeq","FPEAuditor"]

# run the job
if cfg.run().isFailure():
    exit(1)
"""

    alg_hdr_template = """\
#ifndef %(guard)s
#define %(guard)s 1

#include "AthAnalysisBaseComps/AthAnalysisAlgorithm.h"

//Example ROOT Includes
//#include "TTree.h"
//#include "TH1D.h"

%(namespace_begin)s

class %(klass)s: public ::AthAnalysisAlgorithm { 
 public: 
  %(klass)s( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~%(klass)s(); 

  ///uncomment and implement methods as required

                                        //IS EXECUTED:
  virtual StatusCode  initialize();     //once, before any input is loaded
  virtual StatusCode  beginInputFile(); //start of each input file, only metadata loaded
  //virtual StatusCode  firstExecute();   //once, after first eventdata is loaded (not per file)
  virtual StatusCode  execute();        //per event
  //virtual StatusCode  endInputFile();   //end of each input file
  //virtual StatusCode  metaDataStop();   //when outputMetaStore is populated by MetaDataTools
  virtual StatusCode  finalize();       //once, after all events processed
  

  ///Other useful methods provided by base class are:
  ///evtStore()        : ServiceHandle to main event data storegate
  ///inputMetaStore()  : ServiceHandle to input metadata storegate
  ///outputMetaStore() : ServiceHandle to output metadata storegate
  ///histSvc()         : ServiceHandle to output ROOT service (writing TObjects)
  ///currentFile()     : TFile* to the currently open input file
  ///retrieveMetadata(...): See twiki.cern.ch/twiki/bin/view/AtlasProtected/AthAnalysisBase#ReadingMetaDataInCpp


 private: 

   //Example algorithm property, see constructor for declaration:
   //int m_nProperty = 0;

   //Example histogram, see initialize method for registration to output histSvc
   //TH1D* m_myHist = 0;
   //TTree* m_myTree = 0;

}; 
%(namespace_end)s
#endif //> !%(guard)s
"""

    alg_cxx_template = """\
// %(pkg)s includes
#include "%(namespace_klass)s.h"

//#include "xAODEventInfo/EventInfo.h"


%(namespace_begin)s

%(klass)s::%(klass)s( const std::string& name, ISvcLocator* pSvcLocator ) : AthAnalysisAlgorithm( name, pSvcLocator ){

  //declareProperty( "Property", m_nProperty = 0, "My Example Integer Property" ); //example property declaration

}


%(klass)s::~%(klass)s() {}


StatusCode %(klass)s::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");
  //
  //This is called once, before the start of the event loop
  //Retrieves of tools you have configured in the joboptions go here
  //

  //HERE IS AN EXAMPLE
  //We will create a histogram and a ttree and register them to the histsvc
  //Remember to configure the histsvc stream in the joboptions
  //
  //m_myHist = new TH1D("myHist","myHist",100,0,100);
  //CHECK( histSvc()->regHist("/MYSTREAM/myHist", m_myHist) ); //registers histogram to output stream
  //m_myTree = new TTree("myTree","myTree");
  //CHECK( histSvc()->regTree("/MYSTREAM/SubDirectory/myTree", m_myTree) ); //registers tree to output stream inside a sub-directory


  return StatusCode::SUCCESS;
}

StatusCode %(klass)s::finalize() {
  ATH_MSG_INFO ("Finalizing " << name() << "...");
  //
  //Things that happen once at the end of the event loop go here
  //


  return StatusCode::SUCCESS;
}

StatusCode %(klass)s::execute() {  
  ATH_MSG_DEBUG ("Executing " << name() << "...");
  setFilterPassed(false); //optional: start with algorithm not passed



  //
  //Your main analysis code goes here
  //If you will use this algorithm to perform event skimming, you
  //should ensure the setFilterPassed method is called
  //If never called, the algorithm is assumed to have 'passed' by default
  //


  //HERE IS AN EXAMPLE
  //const xAOD::EventInfo* ei = 0;
  //CHECK( evtStore()->retrieve( ei , "EventInfo" ) );
  //ATH_MSG_INFO("eventNumber=" << ei->eventNumber() );
  //m_myHist->Fill( ei->averageInteractionsPerCrossing() ); //fill mu into histogram


  setFilterPassed(true); //if got here, assume that means algorithm passed
  return StatusCode::SUCCESS;
}

StatusCode %(klass)s::beginInputFile() { 
  //
  //This method is called at the start of each input file, even if
  //the input file contains no events. Accumulate metadata information here
  //

  //example of retrieval of CutBookkeepers: (remember you will need to include the necessary header files and use statements in requirements file)
  // const xAOD::CutBookkeeperContainer* bks = 0;
  // CHECK( inputMetaStore()->retrieve(bks, "CutBookkeepers") );

  //example of IOVMetaData retrieval (see https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/AthAnalysisBase#How_to_access_file_metadata_in_C)
  //float beamEnergy(0); CHECK( retrieveMetadata("/TagInfo","beam_energy",beamEnergy) );
  //std::vector<float> bunchPattern; CHECK( retrieveMetadata("/Digitiation/Parameters","BeamIntensityPattern",bunchPattern) );



  return StatusCode::SUCCESS;
}

%(namespace_end)s
"""


### functions -----------------------------------------------------------------
@acmdlib.command(
    name='cmake.new-analysisalg'
    )
@acmdlib.argument(
    'algname',
    help="name of the new alg"
    )

def main(args):
    """create a new AthAnalysisAlgorithm inside the current package. Call from within the package directory

    ex:
     $ acmd cmake new-analysisalg MyAlg
    """
    sc = 0
    
    full_alg_name = args.algname

    #determine the package from the cwd 
    cwd = os.getcwd()
    #check that src dir exists and CMakeLists.txt exists (i.e. this is a package)
    if not os.path.isdir(cwd+"/src") or not os.path.isfile(cwd+"/CMakeLists.txt"):
        print("ERROR you must call new-analysisalg from within the package you want to add the algorithm to")
        return -1
   
   
    full_pkg_name = os.path.basename(cwd)
    print(textwrap.dedent("""\
    ::: create alg [%(full_alg_name)s] in pkg [%(full_pkg_name)s]""" %locals()))

    
    #first we must check that CMakeLists.txt file has the AthAnalysisBaseComps dependency in it
    foundBaseComps=False
    hasxAODEventInfo=False
    hasAtlasROOT=False
    hasAsgTools=False
    lastUse=0 
    lineCount=0
    hasLibraryLine=False
    hasComponentLine=False
    for line in open('CMakeLists.txt'):
        lineCount +=1 
        if "atlas_add_library" in line: hasLibraryLine=True
        if "atlas_add_component" in line: hasComponentLine=True

#GOT THIS FAR WITH EDITING

        
        

    
    #following code borrowed from gen_klass
    hdr = Templates.alg_hdr_template
    cxx = Templates.alg_cxx_template
    
    namespace_klass = full_alg_name.replace('::','__')
    namespace_begin,namespace_end = "",""
    namespace = ""
    if full_alg_name.count("::")>0:
        namespace    = full_alg_name.split("::")[0]
        full_alg_name = full_alg_name.split("::")[1]
        namespace_begin = "namespace %s {" % namespace
        namespace_end   = "} //> end namespace %s" % namespace
        pass

    guard = "%s_%s_H" % (full_pkg_name.upper(), namespace_klass.upper())

    d = dict( pkg=full_pkg_name,
              klass=full_alg_name,
              guard=guard,
              namespace_begin=namespace_begin,
              namespace_end=namespace_end,namespace_klass=namespace_klass,namespace=namespace
              )
    fname = os.path.splitext("src/%s"%namespace_klass)[0]
    #first check doesn't exist 
    if os.path.isfile(fname+'.h'):
       print(":::  ERROR %s.h already exists" % fname)
       return -1
    print(":::  INFO Creating %s.h" % fname)
    o_hdr = open(fname+'.h', 'w')
    o_hdr.writelines(hdr%d)
    o_hdr.flush()
    o_hdr.close()

    if os.path.isfile(fname+'.cxx'):
       print(":::  ERROR %s.cxx already exists" % fname)
       return -1
    print(":::  INFO Creating %s.cxx" % fname)
    o_cxx = open(fname+'.cxx', 'w')
    o_cxx.writelines(cxx%d)
    o_cxx.flush()
    o_cxx.close()


    #now add the algorithm to the _entries.cxx file in the components folder 
    #first check they exist
    if not os.path.exists("src/components"): os.mkdir("src/components") 
    if not os.path.isfile("src/components/%s_entries.cxx"%full_pkg_name):
       print(":::  INFO Creating src/components/%s_entries.cxx"%full_pkg_name)
       loadFile = open("src/components/%s_entries.cxx"%full_pkg_name,'w')
       if len(namespace_begin)>0:
          d["namespace"] = args.algname.split("::")[0]
          loadFile.writelines("""
#include "../%(namespace_klass)s.h"
DECLARE_COMPONENT(%(namespace)s::%(klass)s )
"""%d)
       else:
          loadFile.writelines("""
#include "../%(namespace_klass)s.h"
DECLARE_COMPONENT( %(klass)s )
"""%d)
       loadFile.flush()
       loadFile.close()
    else:
       #first check algorithm not already in _entries file 
       inFile=False
       for line in open("src/components/%s_entries.cxx"%full_pkg_name):
          if len(namespace_begin)==0 and "DECLARE_COMPONENT" in line and  d["klass"] in line: inFile=True
          if len(namespace_begin)>0 and "DECLARE_COMPONENT" in line and d["klass"] in line and d["namespace"]: inFile=True
          
       if not inFile:
         print(":::  INFO Adding %s to src/components/%s_entries.cxx"% (args.algname,full_pkg_name))
         nextAdd=True
         with open("src/components/%s_entries.cxx"%full_pkg_name, "a") as f:
              if len(namespace_begin)>0:
                  f.write("""  DECLARE_COMPONENT(%(namespace)s::%(klass)s );"""%d)
              else:
                  f.write("""  DECLARE_COMPONENT( %(klass)s );"""%d)
   

    full_script_name = "run" + namespace_klass
    full_alg_name = namespace_klass
   
    print(textwrap.dedent("""\
    ::: create script [%(full_script_name)s] for alg [%(full_alg_name)s]""" %locals()))
   
    e = dict( klass=full_alg_name,
              inFile=os.environ['ASG_TEST_FILE_MC'],
              )
    fname = 'scripts/%s.py' % full_script_name
    #first check doesn't exist
    if os.path.isfile(fname):
        print(":::  WARNING %s already exists .. will not overwrite" % fname)
    else:
        o_hdr = open(fname, 'w')
        o_hdr.writelines(Templates.script_template % e)
        o_hdr.flush()
        o_hdr.close()
        os.chmod(fname, 0o755)

    #need to reconfigure cmake so it knows about the new files
    #rely on the WorkDir_DIR env var for this
    workDir = os.environ.get("WorkDir_DIR")
    if workDir is None:
        print("::: ERROR No WorkDir_DIR env var, did you forget to source the setup.sh script?")
        print("::: ERROR Please do this and reconfigure cmake manually!")
    else:
        print(":::  INFO Reconfiguring cmake %s/../." % workDir)
        res = subprocess.getstatusoutput('cmake %s/../.' % workDir)
        if res[0]!=0:
            print(":::  WARNING reconfigure unsuccessful. Please reconfigure manually!")
        

    print(":::  INFO Please ensure your CMakeLists.txt file has ")
    print(":::       atlas_add_component( %s src/component/*.cxx ... )" % full_pkg_name)
    print(":::  INFO and necessary dependencies declared ")
    print(":::  INFO Minimum dependency is: Control/AthAnalysisBaseComps")

