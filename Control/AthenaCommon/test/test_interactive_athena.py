# testing interactive athena scriptability
# @author Sebastien Binet
# @date April 2010

import AthenaCommon.AlgSequence as acas
job = acas.AlgSequence()

import AthenaPython.PyAthena as PyAthena
StatusCode = PyAthena.StatusCode

class TestAlg(PyAthena.Alg):

    def initialize(self):
        self.msg.info("initialize...")
        self.nevts = 0
        self.ievt = 0
        self.msg.info("initialize... [ok]")
        return StatusCode.Success

    def execute(self):
        self.msg.info("execute...")
        self.nevts += 1
        self.ievt = self.nevts - 1
        self.msg.info("--> evt: %s/%s", self.ievt, self.nevts)
        self.msg.info("execute... [ok]")
        return StatusCode.Success

    def finalize(self):
        self.msg.info("finalize...")
        self.msg.info("processed [%s] evts", self.nevts)
        self.msg.info("finalize... [ok]")
        return StatusCode.Success

    pass # class TestAlg

job += TestAlg("pyalg")

theApp.initialize()
theApp.nextEvent()

for _ in range(10):
    theApp.nextEvent()

theApp.finalize().ignore()
print ("="*80)
print ("test [OK]")
print ("="*80)
theApp.exit()

    
