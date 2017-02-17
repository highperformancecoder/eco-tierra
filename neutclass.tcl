#!etierra
# write out the neutral database on standard output as pairs (orgname, neutclass) 
use_namespace etierra
neutdb.Init $argv(1) r
etierra.create_inv_neutdb
foreach {name class} [array get neut_class] {
    puts stdout "$name $class"
}

