# ensure this script fails hard on any error
set -e

for i in test/00/*.sh; do echo $i; sh $i; done
