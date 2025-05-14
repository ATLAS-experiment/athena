# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys, os, glob, subprocess, tarfile, fnmatch, smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText


def nextstep(text):
    print("\n"+"#"*100)
    print("#")
    print("#    %s" % (text))
    print("#")
    print("#"*100,"\n")
    
def tryError(command, error):
    try:
        print(" Running: %s\n" % (command))
        stdout, stderr = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
        print("OUTPUT: \n%s" % (stdout.decode('ascii')))
        print("ERRORS: %s" % ("NONE" if stderr.decode('ascii')=='' else "\n"+stderr.decode('ascii')))
        if stderr:
            exit(1)        
    except OSError as e:
        print(error,e)
        sys.exit(e.errno)       

def send_statusmail(itera, runNumber, mto, outdir, a, b, c, d, e, f, g, h):

    thres_Res_Ar = 140
    thres_Res_Xe = 130
    res_tolerance = 10
    thres_Res_time = 0
    res_time_tolerance = 0.5

    mserver = 'cernmx.cern.ch'
    mfrom = 'no-reply@cern.ch'
    msubject = "TRT CALIB TIER0 - Exit Status for Run %d" % runNumber

    def color(cond):
        if cond:
            return '#90EE90'
        else:
            return '#FF5733'

    # Build HTML message body
    mbody_html = f"""
    <html>
        <head>
            <style>
                table, th, td {{
                    border: 1px solid white;
                    border-collapse: collapse;
                    font-weight:bold;
                    margin-left:20px
                }}
            </style>
        </head>
        <body>
            <p><b>Calibration job finished</b> for run <b>{runNumber}</b>, iteration: <b>{itera}</b></p>
            <p>Here are the residuals obtained at detector level with currently used constants:</p>

            <table>
                <tr>
                    <td>Residual-Barrel A: </td>
                    <td style="background-color:{color(abs(a-thres_Res_Ar)<res_tolerance)};">{a:.1f}μm</td>
                    <td>&nbsp&nbsp</td>
                    <td>Time-Residual-Barrel A: </td>
                    <td style="background-color:{color(abs(b-thres_Res_time)<res_time_tolerance)};">{b:.2f}ns</td>
                </tr>
                <tr>
                    <td>Residual-Barrel C: </td>
                    <td style="background-color:{color(abs(c-thres_Res_Ar)<res_tolerance)};"> {c:.1f}μm</td>
                    <td>&nbsp&nbsp</td>
                    <td>Time-Residual-Barrel C: </td>
                    <td style="background-color:{color(abs(d-thres_Res_time)<res_time_tolerance)};"> {d:.2f}ns</td>
                </tr>
                <tr>
                    <td>Residual-Endcap A: </td>
                    <td style="background-color:{color(abs(e-thres_Res_Xe)<res_tolerance)};"> {e:.1f}μm</td>
                    <td>&nbsp&nbsp</td>
                    <td>Time-Residual-Endcap A: </td>
                    <td style="background-color:{color(abs(f-thres_Res_time)<res_time_tolerance)};"> {f:.2f}ns</td>
                </tr>
                <tr>
                    <td>Residual-Endcap C: </td>
                    <td style="background-color:{color(abs(g-thres_Res_Xe)<res_tolerance)};">{g:.1f}μm</td>
                    <td>&nbsp&nbsp</td>
                    <td>Time-Residual-Endcap C: </td>
                    <td style="background-color:{color(abs(h-thres_Res_time)<res_time_tolerance)};">{h:.2f}ns</td>
                </tr>
            </table>

            <p><b>Expect:</b> Residual 140 ± 10 μm in the barrel (Ar) and 130 ± 10 μm in EC (Xe). Time-Residual 0.0 ± 0.5 ns.</p>
            <p><span style="color:{color(True)}; font-weight:bold;">Green</span> is within the threshold, <span style="color:{color(False)}; font-weight:bold;">Red</span> otherwise</p>
            <p>Please check the histograms of the current run to decide whether to upload new constants.<br>
            Histograms can be found on AFS, directory: <b>{outdir}</b></p>
        </body>
    </html>
    """

    # Create MIME email
    msg = MIMEMultipart("alternative")
    msg["Subject"] = msubject
    msg["From"] = mfrom
    msg["To"] = ", ".join(mto) if isinstance(mto, list) else mto

    msg.attach(MIMEText(mbody_html, "html"))

    print("Email body (HTML):\n\n", mbody_html)
    print("Email sent to:")

    try:
        with smtplib.SMTP(mserver) as con:
            if isinstance(mto, str):
                print(f"\t- {mto}")
                con.sendmail(mfrom, mto, msg.as_string())
            elif isinstance(mto, list):
                for onemto in mto:
                    print(f"\t- {onemto}")
                    con.sendmail(mfrom, onemto, msg.as_string())
    except OSError as e:
        print("ERROR: Failed sending email notification\n", e)
        exit(e.errno)



def fromRunArgs(runArgs):
    
    ##################################################################################################
    nextstep("UNTAR files")
    ##################################################################################################
    
    print("Uncompressing files:")
    try:
        for file in runArgs.inputTARFile:
            print("\t-",file)
            tarfile.open(file).extractall(".") 
    except OSError as e:
        print("ERROR: Failed uncompressing TAR file\n",e)
        sys.exit(e.errno)    
    
    ##################################################################################################
    nextstep("Filtering files")
    ##################################################################################################
    
    # RT files (nor merging barrel)
    files_list_rt =[item for item in glob.glob("*_rt.txt") if "barrel" not in item]
    files_list_t0 =[item for item in glob.glob("*_t0.txt") if "barrel" not in item]
    files_list_cal=[item for item in glob.glob("*calibout.root") if "barrel" not in item]
    # tracks
    files_list_trk=glob.glob("*tracktuple.root")
    # straws
    files_list_stw=glob.glob("*merged.straw.txt")
    # constants
    files_list_ctn=glob.glob("*calib_constants_out.txt")
    
    def listFiles(v,txt=""):
        print("Number of files for %s: %i" % (txt,len(v)))
        
        if not v:
            print("ERROR: list of files for %s is empty" % (txt) )
            sys.exit(1)
            
        for i in v:
            print("\t-%s: %s" % (txt,i))
            
    listFiles(files_list_rt ,"RT")
    listFiles(files_list_t0 ,"T0")
    listFiles(files_list_cal,"Cal")
    listFiles(files_list_trk,"Track")
    listFiles(files_list_stw,"Straws")
    listFiles(files_list_ctn,"Constants")
    
    ##################################################################################################
    nextstep("Merging Rt and t0 files")
    ##################################################################################################  
    
    command  = "touch calibout.txt ;"  
    command += "  echo '# Fileformat=2' >> calibout.txt ; "
    command += "  echo '# RtRelation'   >> calibout.txt ; "
    
    # Merging Rt files
    for _f in files_list_rt :
        command += "cat %s >> calibout.txt ; " % (_f)
        
    command += "  echo '# StrawT0' >> calibout.txt ; "
    
    # Merging t0 files
    for _f in files_list_t0 :      
        command += "cat %s >> calibout.txt ; " % (_f)

    # add footer
    command += "  echo '#GLOBALOFFSET 0.0000' >> calibout.txt ; "
    
    if os.path.isfile("calibout.txt"):
        print("calibout.txt.. already exists. Removed.")
        os.remove("calibout.txt")
            
    tryError(command,"ERROR: Not able to merge the Rt and t0 files\n") 
      
    
    ##################################################################################################
    nextstep("Merging constants files")
    ##################################################################################################

    command = " touch oldt0s.txt ;"
    for _f in files_list_ctn :
        command += "cat %s >> oldt0s.txt ; " % (_f)    

    if os.path.isfile("oldt0s.txt"):
        print("oldt0s.txt.. already exists. Removed.")
        os.remove("oldt0s.txt")

    tryError(command,"ERROR: Not able to merge the Rt and t0 files\n")   
          
    
    ##################################################################################################
    nextstep("Shift text file")
    ##################################################################################################
    
    command = 'TRTCalib_cfilter.py calibout.txt calibout.txt oldt0s.txt shiftrt'  
      
    tryError(command,"ERROR: Failed in process TRTCalib_cfilter\n")    

    ##################################################################################################
    nextstep("Run hadd to merge root files")
    ##################################################################################################
    
    # Removing the output file if it exists to avoid problems
    if os.path.isfile("merge.root"):
        print("merge.root.. already exists. Removed.")
        os.remove("merge.root")  
        
    command  = 'hadd merge.root '
    # merge all the files
    for _f in files_list_cal :
        command += "%s " % (_f)     
    # and track tuple, we just need the first one since they are all the same
    command += "%s " % files_list_trk[0]       
    
    tryError(command,"ERROR: Failed in process hadd\n")         
    
    ##################################################################################################
    nextstep("Rename root files")
    ##################################################################################################
    
    outputFile = runArgs.outputTAR_MERGEDFile
    runNumber = int(runArgs.inputTARFile[0].split('.')[1])
    
    # Rename calibout.txt and merge.root
    command  = "mv -v calibout.txt %s.calibout.txt ; " % outputFile 
    command += "mv -v merge.root %s.merge.root ; " % outputFile 
    # only one straw file is needed
    command += "mv -v %s straws.%d.txt ; " % (files_list_stw[0], runNumber)
    # copy also db const
    command += "mv -v dbconst.txt %s.dbconst.txt" % outputFile    
    
    tryError(command,"ERROR: Failed in process hadd\n")  

    ##################################################################################################
    nextstep("Make all plots")
    ##################################################################################################

    # generating the .ps file
    command  = "TRTCalib_makeplots itersum %s.merge.root %s/lastconfigfile" % (outputFile, os.path.abspath('.'))
    
    tryError(command,"ERROR: Failed in creating plots (itersum.ps file)\n")  

    ##################################################################################################
    nextstep("converting ps to pdf")
    ##################################################################################################
    
    command  = "ps2pdf itersum.ps %s.itersum.pdf" % (outputFile)
    
    tryError(command,"ERROR: Failed in creating itersum.pdf from itersum.ps)\n")

    ##################################################################################################
    nextstep("Straw status Report")
    ##################################################################################################
    
    command  = "mkdir -p -v output/ ; TRTCalib_StrawStatusReport %d" % (runNumber)
    
    tryError(command,"ERROR: Failed running TRTCalib_StrawStatusReport.cxx\n")
 
    ##################################################################################################
    nextstep("Straw status plots (.cxx)")
    ##################################################################################################
       
    command  = "TRTCalib_StrawStatusPlots"
    
    tryError(command,"ERROR: Failed running root macro TRTCalib_StrawStatusPlots\n")
 
    ##################################################################################################
    nextstep("TAR'ing files")
    ##################################################################################################
       
    try:
        # Getting list of files to be compressed
        files_list=glob.glob(outputFile+".*")
        # Compressing
        tar = tarfile.open(outputFile, "w:gz")
        print("\nCompressing files in %s output file:" % outputFile)
        for file in files_list:
            print("\t-",file)
            tar.add(file)
        tar.close()
    except OSError as e:
        print("ERROR: Failed compressing the output files\n",e)
        sys.exit(e.errno)   

 
    ##################################################################################################
    nextstep("Copying files to AFS Directory")
    ################################################################################################## 
    
    outDIR = "%s/run_%d" % (runArgs.attrtcal_dir, runNumber)
    
    # Extracting iteration from Tier0 and for emails
    outputFile_split = outputFile.split('.')
    itera = '9999' # default iteration for testing only
    if len(outputFile_split) > 5:
        if "iter" in outputFile_split[5]:
            itera = (outputFile_split[5])[-1:]    # e.g. data14_cos.00247236.express_express.trtcal.TXT.iter1 --> '1'
    
    command  = "mkdir -p -v %s ; " % (outDIR)
    command += "cp -v %s.merge.root %s/trtcalib_0%s_histograms.root ; " % (outputFile, outDIR, itera)
    command += "cp -v %s.calibout.txt %s/calibout.%d.NoShifted.txt ; " % (outputFile, outDIR, runNumber)
    command += "cp -v %s.itersum.pdf %s/Plots.%d.pdf ; " % (outputFile, outDIR, runNumber)
    command += "cp -v %s.dbconst.txt %s/dbconst.%d.txt ; " % (outputFile, outDIR, runNumber)
    command += "cp -v %s.dbconst.txt %s/calibout.%d.txt ; " % (outputFile, outDIR, runNumber)
    command += "cp -v straws.%d.txt %s/straws.%d.txt; " % (runNumber, outDIR, runNumber)
    command += "cp -v TRT_StrawStatusReport.txt %s/TRT_StrawStatusReport.%d.txt; " % (outDIR, runNumber)
    command += "cp -v allPlots.pdf %s/TRT_StrawStatusReport.%d.pdf; " % (outDIR, runNumber)
    
    tryError(command,"ERROR: Files cannot be copied to the chosen directory\n")
    
    
    
    ##################################################################################################
    nextstep("Extractor information")
    ##################################################################################################
    
    command  = "python -m TRT_CalibAlgs.TRTCalib_Extractor %s/trtcalib_0%s_histograms.root > %s/extraction.txt" % (outDIR, itera, outDIR)
    
    tryError(command,"ERROR: Extracting information for email notification\n")
    
    ##################################################################################################
    nextstep("Email notification")
    ##################################################################################################   

    res_ba  = 0 ; tres_ba = 0
    res_bc  = 0 ; tres_bc = 0
    res_ea  = 0 ; tres_ea = 0
    res_ec  = 0 ; tres_ec = 0
    
    try:
        with open("%s/extraction.txt" % (outDIR)) as exfile:
            for line in exfile :
                line = line.strip()
                if fnmatch.fnmatch(line,'* res *') and fnmatch.fnmatch(line, '*part 1*') :
                        res_ba = float(line.split()[6])*1000
                if fnmatch.fnmatch(line,'* tresmean *') and fnmatch.fnmatch(line, '*part 1*') :
                        tres_ba = float(line.split()[6])
                if fnmatch.fnmatch(line,'* res *') and fnmatch.fnmatch(line, '*part -1*') :
                        res_bc = float(line.split()[6])*1000
                if fnmatch.fnmatch(line,'* tresmean *') and fnmatch.fnmatch(line, '*part -1*') :
                        tres_bc = float(line.split()[6])
                if fnmatch.fnmatch(line,'* res *') and fnmatch.fnmatch(line, '*part 2*') :
                        res_ea = float(line.split()[6])*1000
                if fnmatch.fnmatch(line,'* tresmean *') and fnmatch.fnmatch(line, '*part 2*') :
                        tres_ea = float(line.split()[6])
                if fnmatch.fnmatch(line,'* res *') and fnmatch.fnmatch(line, '*part -2*') :
                        res_ec = float(line.split()[6])*1000
                if fnmatch.fnmatch(line,'* tresmean *') and fnmatch.fnmatch(line, '*part -2*') :
                        tres_ec = float(line.split()[6])
    except OSError as e:
        print("ERROR: Failed reading %s/extraction.txt file\n" % (outDIR) ,e)
        sys.exit(e.errno)    

    if runArgs.sendNotification and runArgs.emailList :
        send_statusmail(itera, runNumber, runArgs.emailList, outDIR, res_ba, tres_ba, res_bc, tres_bc, res_ea, tres_ea, res_ec, tres_ec)
    else:
        print("INFO: No email notification sent since --sendNotification=%r or empty --emailList=" % (runArgs.sendNotification), runArgs.emailList)
    