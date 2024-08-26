from DataQualityUtils.doZLumi import makeGRL
import sys

mydir = sys.argv[1]
run_list = sys.argv[2:]
print("Targetdir = ", mydir)
print("Run List = ", run_list)
for i in run_list:
    run = int(i)
    outfile = mydir+'/'+str(run)+'_grl.xml'
    makeGRL(run, 'PHYS_StandardGRL_All_Good', outfile)
    print(run)
