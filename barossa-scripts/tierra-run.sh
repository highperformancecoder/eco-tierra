#PBS -lnodes=1 -lwalltime=48:0:0 -A rks -q single
cd /scratch/$PBS_JOBID
cp -r $PBS_O_WORKDIR/gb0 .
$PBS_O_WORKDIR/tierra1 $PBS_O_WORKDIR/si0
cp -r gb0 $PBS_O_WORKDIR 
