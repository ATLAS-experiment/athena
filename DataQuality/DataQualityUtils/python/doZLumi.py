# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def getRun(fname):
    import ROOT
    fin = ROOT.TFile.Open(fname)
    if not fin: return None
    runname = None
    for key in fin.GetListOfKeys():
        if key.GetName().startswith('run_'):
            runname = key.GetName()
            break
    if runname: return int(runname[4:])
    else: return None

def checkDirExists(fname):
    # Does DQTGlobalWZFinder directory even exist?
    import ROOT
    run = getRun(fname)
    if not run: return False
    fin = ROOT.TFile.Open(fname, 'READ')
    if not fin: return False
    dobj = fin.Get(f'run_{run}/GLOBAL/DQTGlobalWZFinder')
    if not dobj: return False
    return True

def copyPlot(infname, outfname):
    import ROOT
    run = getRun(outfname)
    if not run: return
    fin = ROOT.TFile.Open(infname, 'READ')
    if not fin: return
    fout = ROOT.TFile.Open(outfname, 'UPDATE')
    if not fout: return
    for objname in ['z_lumi', 'z_lumi_ratio']:
        obj = fin.Get(objname)
        if obj:
            d = fout.Get('run_%d/GLOBAL/DQTGlobalWZFinder' % run)
            if d:
                d.WriteTObject(obj, objname)
    fin.Close(); fout.Close()
            
def makeGRL(run, defect, fname):
    import DQUtils, DQDefects

    tag = 'HEAD'
    runs = [run]
    print('Query run information...', end='')
    from DQUtils.db import fetch_iovs
    dbinstance = 'CONDBR2'
    eor = fetch_iovs('EOR', (min(runs) << 32) | 1,
                     (max(runs) << 32) | 0xFFFFFFFF,
                     with_channel=False, what=[], database='COOLONL_TDAQ/%s' % dbinstance)
    eor = eor.trim_iovs
    eor = DQUtils.IOVSet(iov for iov in eor if iov.since.run in runs)
    print('done')
    print('Query defects...', end='')
    ddb = DQDefects.DefectsDB('COOLOFL_GLOBAL/%s' % dbinstance, tag=tag)
    ignores = {_ for _ in ddb.defect_names if 'UNCHECKED' in _}
    try:
        defectiovs = ddb.retrieve(since = min(runs) << 32 | 1,
                                  until = max(runs) << 32 | 0xffffffff,
                                  channels = [defect],
                                  evaluate_full = False,
                                  ignore=ignores)
    except Exception as e:
        print(e)
        raise
    print('Doing exclusions...', end='')
    okiovs = eor.logical_and(eor, defectiovs.logical_not())
    print('done')

    print('Generating GRL...', end='')
    data = DQUtils.grl.make_grl(okiovs, '', '2.1')
    with open(fname, 'w') as outf:
        outf.write(data)

def go(fname):
    import subprocess, os, shutil  # noqa: F401
    if 'DQ_STREAM' in os.environ:
        if (os.environ.get('DQPRODUCTION', '0') == '1'
            and os.environ['DQ_STREAM'] != 'physics_Main'):
            return
    if 'DISPLAY' in os.environ: del os.environ['DISPLAY']
    runno = getRun(fname)
    print('Seen run', runno)
    if not checkDirExists(fname):
        print('But DQTGlobalWZFinder directory does not exist: code probably did not run. Exiting')
        return
    grlcmd = []  # noqa: F841
    if runno >= 325000:
        makeGRL(runno, 'PHYS_StandardGRL_All_Good', 'grl.xml')
        grlcmd = ['--grl', 'grl.xml']  # noqa: F841
    else:
        print('Run number', runno, 'not 2017 data')

    # Temporarily comment. Run 3 version of this code will be inserted soon
    subprocess.check_call(['dqt_zlumi_pandas.py', '--infile', fname, '--dblivetime', '--outdir', '', '--campaign', 'mc21'] + grlcmd)
    subprocess.check_call(['dqt_csv_luminosity.sh', '--infile', f'run_{runno}.csv', '--outdir', '', '--absolute', '--t0'])
    if os.path.isfile(f'run_{runno}.csv'):
        shutil.move(f'run_{runno}.csv', 'zlumi.csv')
    if os.path.isfile('zlumi.root'):
        copyPlot('zlumi.root', fname)
