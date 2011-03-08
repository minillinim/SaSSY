BEGIN {
	useline = 0;
}

(NF == 0) {
	useline = 0;
}

(useline) {
	filename_stem = substr($1,1,rindex($1,".")-1);
    print filename_stem ;
}

/^SPECCASES / {
	useline = 1;
}

function rindex(str,c)
{
    return match(str,"\\" c "[^" c "]*$")? RSTART : 0
}
