#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


import subprocess, os, shlex, re, yaml

from AthenaCommon import Logging

## Get handle to Athena logging
logger = Logging.logging.getLogger("Geneva_i")


class GenevaConfig:

    def __init__(self, runArgs):
        self.genevapath = os.environ['GENEVA_DATA_DIR']
        self.exe = 'geneva'
        self.points = 2000
        self.iterations = 4
        self.factor = 250
        self.integrationnumruns = 16
        self.numruns = 1
        #Geneva specific variables for input.DAT, see writeInputDAT function for more elaboration
        self.rts = 13000. #collision energy (GeV)
        if hasattr(runArgs,"ecmEnergy"):
            self.rts = runArgs.ecmEnergy

        self.card=''
        self.yaml=yaml.safe_load(self.card)
        self.cardname='input.yml'
        self.sigmabelow=''

        self.iseed = 34
        if hasattr(runArgs,"randomSeed"):
            self.iseed  = runArgs.randomSeed


        self.nev = "500"
        if hasattr(runArgs,"maxEvents"):
            self.nev = runArgs.maxEvents

    def parseCard(self):
        self.yaml=yaml.safe_load(self.card)
        self.yaml['global']['run_name']="tutorial"
        self.yaml['global']['num_events']= self.nev*self.factor
        for a in self.yaml['process'].keys():
          self.yaml['process'][a]['initial_state']['beams']='pp'
          self.yaml['process'][a]['initial_state']['Ecm']=self.rts
          self.yaml['process'][a]['initial_state']['pdf_provider']['LHAPDF']['set']="PDF4LHC15_nnlo_100"
        return

    def outputLHEFile(self):
        return "generate/tutorial_1.lhe.gz"


def writeInputDAT(Init):
    with open(Init.cardname, "w") as outF:
        yaml.dump(Init.yaml,outF)
    return


def run_command(command, stdin = None):
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


def GenevaInitialize(Init, stdin=None):

    logger.info("Starting Geneva Initialization")

    try:
        open(Init.cardname)
    except IOError:
        raise Exception("problem with file IO; potentially input.DAT not created correctly")
        return
    rc = 0 
    try: 
            os.environ['OpenLoopsPath']=os.environ['OPENLOOPSPATH']
            print(Init.exe + ' setup ' + Init.cardname + ' --points '+ str(Init.points) + ' --iterations '+ str(Init.iterations) + ' --num-runs ' + str(Init.numruns))
            rc = run_command(Init.exe + ' setup ' + Init.cardname + ' --points '+ str(Init.points) + ' --iterations '+ str(Init.iterations) + ' --num-runs ' + str(Init.integrationnumruns)) 

    except OSError:
            raise Exception("init executable or file not found")

    except Exception:
            raise Exception("Non-OS Error or IOError in init execution block")

    if rc:
        raise Exception('Unexpected error in geneva init execution')
        
    return


def GenevaExecute(Init):

    logger.info("Starting Geneva Itself")

    try:
        open(Init.cardname)
    except IOError:
        raise  Exception ("problem with IO; potentially input.DAT not created correctly")
        return
    rc=0
    try: 
            os.environ['OpenLoopsPath']=os.environ['OPENLOOPSPATH']
            rc = run_command(Init.exe + ' generate ' + Init.cardname + ' --num-runs ' + str(Init.numruns)) 
            rc = run_command(Init.exe + ' reweight ' + Init.cardname + ' --sigma-below  '+ Init.sigmabelow + ' --num-runs  '+ str(Init.numruns)) 

    except OSError:
            raise Exception("geneva executable or file not found")

    except Exception:
            raise Exception("Non-OS Error or IOError in Superchic execution block")

    if rc:
        raise Exception('Unexpected error in geneva execution')
    return

def GenevaRun(Init, genSeq):
    genSeq.GenevaConfig = Init
    writeInputDAT(Init)
    GenevaInitialize(Init)
    GenevaExecute(Init)
    return
