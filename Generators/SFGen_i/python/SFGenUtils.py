#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


import subprocess, os, shlex, re

from AthenaCommon import Logging

## Get handle to Athena logging
logger = Logging.logging.getLogger("SFGen_i")

# This module defines the SFGen Utilities in the Athena framework

class SFGenConfig:

# Init definition

    def __init__(self, runArgs):
        self.sfgenpath = os.environ['SFGENPATH']
        
        #SFGen specific variables for input.DAT, see writeInputDAT function for more elaboration

        self.rts = 13000 #collision energy (GeV)
        if hasattr(runArgs,"ecmEnergy"):
            self.rts = runArgs.ecmEnergy

        self.coll = False
        self.PDFname = 'SF_MSHT20qed_nnlo'
        self.PDFmember = 0
        self.proc = 1 #Please consult SFGen Manual https://sfgen.hepforge.org/
        self.outtag = 'out'
        self.diff = 'tot' #Interaction: elastic ('el'), single ('sd','sda','sdb') and double ('dd') dissociation.
        self.SFerror = False
        self.mixed = False
        self.Zinit = False
        self.subt = False
        self.lep1 = 'mu+'
        self.lep2 = 'mu+'
        self.kmu = 1.
        self.ncall = 50000
        self.itmx = 10
        self.prec = 1
        self.ncall1 = 10000
        self.inccall = 10000
        self.itend = 1000

        self.iseed = 13
        if hasattr(runArgs,"randomSeed"):
            self.iseed  = runArgs.randomSeed

        self.genunw = False

        self.nev = "10"
        if hasattr(runArgs,"maxEvents"):
            self.nev = runArgs.maxEvents

        self.erec = 'lhe' #output file type
        self.ymin = -2.4 # Minimum dilepton rapidity
        self.ymax = 2.4 # Maximum dilepton rapidity
        self.mmin = 10. # Minimum dilepton mass
        self.mmax = 1000. # Maximum dilepton mass

        self.gencuts = True # Generate cuts below

        self.ptamin = 20. # Minimum pT of outgoing object a
        self.ptbmin = 20. # Minimum pT of outgoing object b
        self.etaamin = -2.4 # Minimum eta of outgoing object a
        self.etaamax = 2.4 # Maximum eta of outgoing object a
        self.etabmin = -2.4 # Minimum eta of outgoing object b
        self.etabmax = 2.4 # Maximum eta of outgoing object b
        self.ptllmin = 0. # Minimun pT of dilepton pair

# Writing of the input.DAT file 

    def toFortran(self):

        def fortDouble(x):
            return str(x)+"d0"
        def fortInt(x):
            return str(x)
        def fortBool(x):
            return '.true.' if x else '.false.'
        def fortStr(x):
            return "'{}'".format(x)

        conf = ""
        conf+="***********************************************************************************\n"
        conf+="***********************************************************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortDouble(self.rts) + "                     ! [rts] : CMS collision energy (GeV) \n"
        conf+="***********************************************************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortInt(self.proc) + "                             ! [proc] : Process number \n"
        conf+=fortStr(self.outtag) + "                           ! [outtag] : for output file \n"
        conf+=fortStr(self.diff) + "                          ! [diff] : elastic ('el'), single/double dissociation ('sd'/'dd') \n"
        conf+=fortBool(self.SFerror) + "                             ! [SFerror] : Include error from SF input - increases run time \n"
        conf+=fortBool(self.mixed) + "                             ! [mixed] : include mixed gam/Z + q diagrams \n"
        conf+=fortBool(self.Zinit) + "                             ! [Zinit] : include Z bosons in initial state \n"
        conf+=fortBool(self.subt) + "                             ! [subt] : calculate *only* (positive) subtraction term \n"
        conf+=fortStr(self.lep1) + "                ! [lep1] : for lepton-lepton scattering (proc=4) \n"
        conf+=fortStr(self.lep2) + "                ! [lep2] : for lepton-lepton scattering (proc=4) \n"
        conf+=fortDouble(self.kmu) + "                     ! [kmu] :  = mu(f,r)/mu0 \n"
        conf+="***********************************************************************************\n"
        conf+="**************************  To run in collinear mode  *****************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortBool(self.coll) + "                ! [coll] : use collinear approach + lhapdf \n"
        conf+=fortStr(self.PDFname) + "                ! [PDFname] : PDF set \n"
        conf+=fortInt(self.PDFmember)  + "                       ! [PDFmember] : PDF member \n"
        conf+="***********************************************************************************\n"
        conf+="*************Integration parameters************************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortInt(self.ncall) + "                              ! [ncall] : Number of calls for preconditioning \n"
        conf+=fortInt(self.itmx) + "                                  ! [itmx] : Number of iterations for preconditioning \n"
        conf+=fortDouble(self.prec) + "                                 ! [prec] :  Relative accuracy (in %) in main run \n"
        conf+=fortInt(self.ncall1) + "                               ! [ncall1] : Number of calls in first iteration \n"
        conf+=fortInt(self.inccall) + "                              ! [inccall] : Number of increase calls per iteration \n"
        conf+=fortInt(self.itend) + "                                ! [itend] : Maximum number of iterations \n"
        conf+=fortInt(self.iseed) + "                                 ! [iseed] : Random number seed (integer > 0) \n"
        conf+="***********************************************************************************\n"
        conf+="********************Unweighted events**********************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortBool(self.genunw) + "                              ! [genunw] : Generate unweighted events \n"
        conf+=fortInt(int(self.nev)) + "                                 ! [nev] : Number of events (preferably controlled by maxEvents option in Gen_tf command) \n"
        conf+=fortStr(self.erec) + "                               ! [erec] : Event record format ('hepmc','lhe','hepevt') \n"
        conf+="***********************************************************************************\n"
        conf+="*******************   general cuts ************************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortDouble(self.ymin) + "                               ! [ymin] : Minimum dilepton rapidity \n"
        conf+=fortDouble(self.ymax) + "                               ! [ymax] : Maximum dilepton rapidity \n"
        conf+=fortDouble(self.mmin) + "                               ! [mmin] : Minimum dilepton mass \n"
        conf+=fortDouble(self.mmax) + "                               ! [mmax] : Maximum dilepton mass \n"
        conf+=fortBool(self.gencuts) + "                             ! [gencuts] : Generate cuts below \n"
        conf+="***********************************************************************************\n"
        conf+="********** 2 body final states : p(a) + p(b) **************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortDouble(self.ptamin) + "                               ! [ptamin] \n"
        conf+=fortDouble(self.ptbmin) + "                               ! [ptbmin] \n"
        conf+=fortDouble(self.etaamin) + "                              ! [etaamin] \n"
        conf+=fortDouble(self.etaamax) + "                              ! [etaamax] \n"
        conf+=fortDouble(self.etabmin) + "                              ! [etabmin] \n"
        conf+=fortDouble(self.etabmax) + "                              ! [etabmax] \n"
        conf+=fortDouble(self.ptllmin) + "                             ! [ptllmin] \n"
        conf+="***********************************************************************************\n"
        conf+="***********************************************************************************\n"

        return conf 

    def outputLHEFile(self):
        return "evrecs/evrec"+self.outtag+".dat"


def writeInputDAT(Init):
    
    with open("input.DAT", "w") as outF:
        outF.write(Init.toFortran())
    
    return


def run_command(command, stdin = None ):
    """
    Run a command and print output continuously
    """
    process = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE, stdin=stdin)
    while True:
        output = process.stdout.readline().decode("utf-8")
        if output == '' and process.poll() is not None:
            break
        if output:
            # remove ANSI escape formatting characters
            reaesc = re.compile(r'(\x9B|\x1B\[)[0-?]*[ -\/]*[@-~]')
            text = reaesc.sub('', output.strip())
            logger.info(text)

    rc = process.poll()
    return rc

# SFGen initialization

def SFGenInitialize(Init, stdin=None):

    logger.info("Starting SFGen Initialization")

    if not os.path.exists('evrecs'):
        os.makedirs('evrecs')
    if not os.path.exists('outputs'):
        os.makedirs('outputs')


    try:
        inputDAT = open('input.DAT')

    except IOError:
        raise Exception("Problem with file IO; potentially input.DAT not created correctly")
    else:

        try: 
            rc = run_command(Init.sfgenpath+"/bin/SFGen", inputDAT)

        except OSError:
            raise Exception("File not found")

        except Exception:
            raise Exception("Non-OSError or IOError in execution block")

    if rc:
        raise Exception('Unexpected error in sfgen execution in SFGenInitialize')
       
    return

# SFGen execution

def SFGenExecute(Init):

    logger.info("Starting SFGen Itself")

    if not os.path.exists('evrecs'):
        os.makedirs('evrecs')
    if not os.path.exists('outputs'):
        os.makedirs('outputs')

    
    try:
        inputDAT = open('input.DAT')
    except IOError:
        raise  Exception ("Problem with IO; potentially input.DAT not created correctly")
    else: 

        try:
            
            rc = run_command(Init.sfgenpath+'/bin/SFGen', stdin=inputDAT)

        except OSError:
            raise Exception("SFGen executable or file not found")

        except Exception:
            raise Exception("Non-OSError or IOError in SFGen execution block")

    if rc:
        raise Exception('Unexpected error in sfgen execution in SFGenExecute')

    return

# SFGen running

def SFGenRun(Init, genSeq):

    # dump the job configuration for fortran code
    print(Init.toFortran())

    # attach config to genSequence for later using in showering
    genSeq.SFGenConfig = Init
    
    writeInputDAT(Init)
    SFGenInitialize(Init)
    SFGenExecute(Init)
    
    return
