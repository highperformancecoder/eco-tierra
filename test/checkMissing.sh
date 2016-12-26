# The following directories should only contain project files
# libraries/tcl* and libraries/tk* are too expensive to search, so
# just look at upper level files in library
DIRS=

EXIT=0

notAegisFile() 
{
    aels -ter $1|grep -- \---
}

checkAegis()
{
    # ignore ,D files
    if echo $1|grep ",D$" >/dev/null; then return; fi
    if aels -ter $1|grep -- \--- >/dev/null; then
        echo "$1 not project file"
        EXIT=1
        fi
}

for f in *.tcl *.sh *.pl; do 
    if [ ! -d $f ]; then 
        checkAegis $f
    fi
done

for dir in $DIRS; do
  for f in `find $dir \( ! -name "*,D" -a ! -name "*~" -a ! -name "*~" -a ! -type d \) -print`; do
      checkAegis $f
      done
done

exit $EXIT
