from PyKernel import PyKernel

from PyKernel.PyKernel import plot,plot2,fill,fill2,dumpSG
from PyKernel.PyKernel import plotProf,fillProf,unregister,dumpHist

import cppyy
cppyy.load_library("PyKernelDict")

# Bind the C++ global namespace to the Python variable g 
g = cppyy.gbl
