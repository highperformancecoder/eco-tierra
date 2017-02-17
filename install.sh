#!/bin/bash
# run this command from the directory you wish to install eco-tierra into
# works out where eco-tierra source directory is from the basename

src=${0%%/install.sh}

if [ $src = `pwd` -o $src = "." ]; then
    echo "Please run this script from install location"
    exit
fi

ln -sf $src/*.tcl $src/*.sh $src/*.pl $src/etierra $src/BDBmerge .
