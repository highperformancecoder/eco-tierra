#PBS -A rks -l walltime=6:0:0 -q single
cd /scratch/$PBS_JOBID
mkdir src
cd src
cp $PBS_O_WORKDIR/gb0/opcode.map .
for i in $PBS_O_WORKDIR/gb0/*.gen; do 
    $HOME/Tierra/tierra/arg1 x $i>/dev/null
done
rm opcode.map
cd ..
$PBS_O_WORKDIR/etierra $PBS_O_WORKDIR/setup.tcl
rm -rf src
cp -r * $PBS_O_WORKDIR
