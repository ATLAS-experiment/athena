PATH=/cvmfs/sft.cern.ch/lcg/external/texlive/2023/bin/x86_64-linux:$PATH

year=24
plotdir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/Plots/"
inputdir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/"

echo "INFO: Prepare LaTeX source code"

runlist=`\ls ${plotdir}/data${year}_13p6TeV | grep ^[0-9] | tr [\\\n] [,] | sed s/,$//`
cat slide_template.tex |\
    sed "s/YEAR/24/" |\
    sed "s/RUNLIST/${runlist}/" |\
    cat > plotdump.tex

split -n l/2 ${inputdir}/data${year}_13p6TeV/skipruns

echo "INFO: Compile run 1 (this may take a while)"
pdflatex plotdump
echo "INFO: Compile run 2 (this may take a while)"
pdflatex plotdump
# compress
newfilename=plotdump_`date "+%F"`.pdf
echo "INFO: Compress slide PDF file to $newfilename"
gs -sDEVICE=pdfwrite -dCompatibilityLevel=1.5 -dNOPAUSE -dQUIET -dBATCH -dPrinted=false -sOutputFile=${newfilename} plotdump.pdf
echo "INFO: $newfilename ready (hopefully)"

\rm plotdump.* xaa xab
