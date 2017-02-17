#! /bin/bash

# no. of processors set here
np=10
rm -f results*.pag results*.dir details*.log details*.pbs*


do_it()
{
# $1 = processor number
  cat >details$1.pbs <<EOF
#!/bin/bash
if [ ! -z "\$PBS_O_WORKDIR" ]; then cd \$PBS_O_WORKDIR; fi
PATH=.:$PATH
details.tcl $1 $np
EOF
    joblist=$joblist:`qsub -A rks -q single -r y -l walltime=72:0:0,ncpus=1 details$1.pbs|tail -1`
}

i=0
rr=
while [ $i -lt $np ]; do
  do_it $i 
  rr="$rr results$i"
  i=$[$i+1]
done

echo $joblist

qsub -A rks -q single  -r y -m ae -W depend=afterok$joblist -l cput=1:0:0 -l ncpus=1 <<EOF
#!/bin/bash
    if [ ! -z "\$PBS_O_WORKDIR" ]; then cd \$PBS_O_WORKDIR; fi
    PATH=.:$PATH
    merge $rr results
    rm neutraldb*
    chmod u+rw results.*
    rem-neutrals.tcl
EOF



