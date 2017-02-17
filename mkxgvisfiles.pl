#!perl

open edges,">tierra.edges";
open linecolors,">tierra.linecolors";

open neutraldb,"neutraldb.txt";
$_=<neutraldb>;
$class=0;
while ($_=<neutraldb>)
{
    $_=<neutraldb>;
    foreach $name (split /\0/) 
    {
	$neutclass{$name}=$class; 
	#print "class($name)=$class\n";
    }
    $class++;
}

open creatdb,"sort -n -k 2 createdb|";
$j=1;
while ($_=<creatdb>)
{
    ($name,$creation)=split;
    $creat{$name}=$creation;
    $orgname[$j]=$name;
    $orglist{$name}=$j++;
}


foreach $i (keys %orglist)
{
    open src,"src/$i";
    $_=<src>; $_=<src>; $_=<src>; 
    (undef,$name,undef,undef,undef,undef,$parent)=split;
    $parentdb{$name}=$parent;
    if (exists $orglist{$parent})
    {
	# open parent sourcefile and compare genotypes
	$diff=0;
	for ($j=0; $j<7; $j++) {$_=<src>;}
	open parf,$parent;
	for ($j=0; $j<7; $j++) {$_=<parf>;}

	while (($_=<src>) && ($p=<parf>))
	{
	    (undef,undef,undef,$instr)=split;
	    (undef,undef,undef,$pinstr)=split ' ',$p;
	    $diff+=$instr==$pinstr;
	}
	
	# now add the difference in lengths
	while (($_=<src>) || ($p=<parf>)) {$diff++;}
	print edges "$orglist{$name} $orglist{$parent} $diff\n"; 
	if (exists $neutclass{$name} && exists $neutclass{$parent} && 
	    $neutclass{$name}==$neutclass{$parent})
	    {print linecolors "red\n"}
	else
	    {print linecolors "white\n"}
	close parf;
    }
    close src;
}

close edges;
close linecolors;

# now label and group nodes
open labels,">tierra.labels";
#open glyphs,">tierra.glyphs";
open colors,">tierra.colors";
#$glyphno=1;
$colourno=0;

# database of point colours
@colourname=("red","green","blue","magenta","cyan","yellow","orange");

for ($i=1; $i<=$#orgname; $i++)
{
    for ($name=$orgname[$i]; 
	 exists $orglist{$parentdb{$name}} && !exists $visited{$name}; 
	 $name=$parentdb{$name}) 
    {$visited{$name}=1}
    if (exists $visited{$name})  {$loop{$name}=1;}
    undef %visited;
# group isolated points by the colour white
    if ($name eq $orgname[$i]) {$colour{$name}=white;}
    else
    {
	if (!exists $colour{$name} || $colour{$name} eq white) 
	{
	    $colour{$name}=$colourname[$colourno%($#colourname+1)];
	    if ($colourno>$#colourname) 
	    {$colour{$name}.=int($colourno/($#colourname+1))}
	    if ($colourno<30) {$colourno++;}
	    else {print "exceeding available colours\n";}
	}    
    }
    print labels "$orgname[$i]\n";
#    print glyphs "$glyph{$name}\n";
    print colors "$colour{$name}\n";
}
close labels;
#close glyphs;
close colors;
foreach $name (keys %loop)
{print "loop detected with $name\n";}
