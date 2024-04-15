# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#########################################################
#                                                       #
# TRT_Digitization/postInclude.OverrideTRTparameters.py #
#                                                       #
# Overrride default settings in TRTDigSettings.cxx      #
#                                                       #
# This is not a full list, just the parameters likely   #
# to be used in basic tuning...                         #
#                                                       #
# For a full list of overrridable parameters            #
# and a list of setttings used in the job:              #
#                                                       #
#   grep TRTDigSettings <job output>                    #
#                                                       #
#########################################################

from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()
from Digitization.DigitizationFlags import digitizationFlags
trt = None
if not digitizationFlags.doXingByXingPileUp() and hasattr(job, 'TRTDigitization'):
    #Back-compatibility for 17.X.Y.Z releases
    if hasattr(job.TRTDigitization, 'DigitizationTool'):
        trt = job.TRTDigitization.DigitizationTool
    else:
        trt = job.TRTDigitization
else:
     from AthenaCommon.CfgGetter import getPublicTool
     trt = getPublicTool("TRTDigitizationTool")
if None == trt:
   raise AttributeError("TRTDigitization(Tool) not found.")

# Units are CLHEP: MeV, ns, mm
# Xenon threshold tunes
trt.Override_highThresholdBarShort  = 0.00129875
trt.Override_highThresholdBarLong   = 0.00118775
trt.Override_highThresholdECAwheels = 0.001185591
trt.Override_highThresholdECBwheels = 0.001145376

# Argon thresholds (July 2014 tuning from Artem) TRT_Digitization-01-00-09  
trt.Override_highThresholdBarShortArgon  = 0.000468802
trt.Override_highThresholdBarLongArgon   = 0.000456754
trt.Override_highThresholdECAwheelsArgon = 0.0006035
trt.Override_highThresholdECBwheelsArgon = 0.00057375

# Krypton thresholds
trt.Override_highThresholdBarShortKrypton  = 0.003070
trt.Override_highThresholdBarLongKrypton   = 0.002900
trt.Override_highThresholdECAwheelsKrypton = 0.003150
trt.Override_highThresholdECBwheelsKrypton = 0.003020
