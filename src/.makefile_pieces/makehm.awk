(NR == 1) {
    filename = FILENAME;
}

# all lines
{
    inp = substr($0,length($1)+1);
}

/^(\/\/HV|\/\/VO|\/\*HV\*\/) / {
    outp1[filename] = outp1[filename] inp;
    if(length(inp)==1) {
        outp1[filename] = outp1[filename] "\n";
    }
}

/^(\/\/HV|\/\/HO|\/\*HV\*\/) / {
    outp2[filename] = outp2[filename] inp;
    if(length(inp)==1){
        outp2[filename] = outp2[filename] "\n";
    }
}

/^\/\/PV / {
    outp3[filename] = outp3[filename] inp;
    if(length(inp)==1){
        outp3[filename] = outp3[filename] "\n";
    }
}
/^(\/\/INC|\/\*INC\*\/) / {
    outp4[filename] = outp4[filename] inp;
    if(length(inp)==1){
        outp4[filename] = outp4[filename] "\n";
    }
}

(FILENAME != filename) {
    if(length(outp2[filename]) > 0)
    {
       writeout(filename);
    }
    filename = FILENAME;
}

END {
    if(length(outp2[filename]) > 0)
    {
       writeout(filename);
    }
}

function rindex(str,c)
{
    return match(str,"\\" c "[^" c "]*$")? RSTART : 0
}

function writeout(filename) {
    filename_stem = substr(filename,1,rindex(filename,".")-1);
    print outp1[filename] > filename_stem ".virt.hm";
    print outp2[filename] > filename_stem ".autoheader.hm";
    print outp3[filename] > filename_stem ".priVar.hm";
    print outp4[filename] > filename_stem ".includes.hm";
}
