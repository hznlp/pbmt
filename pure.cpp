#include "pure.h"
#include "mp.h"

void usage(){
    cerr<<
    R"(
    proc -extract -src=source_file -tgt=target_file -o=output_file -maxlen=int
    -cutoff=int -inmemory=true
    -score -i=input -o=output [-nbest=int] [-lengthsort]
    [-lex_s2t=file] [-lex_t2s=file] [-lexcond=false]
    [-scoring=Frac|Count|CountLex|OnlyLex|OnlyFrac|OnlyCount]
    -readlog -i=file -m=message
    -em -src=sourcefile -tgt=targetfile [-lex=lex_s2t] [-round=int]
        [-init=frac|count] [-o=output_prefix] [-maxlen] [-pt=phrase-table]
        [-reverse]
    -combine -ps2t=pt_s2t -pt2s=pt_t2s -o=out
    -filter -i=input -block=src|tgt

    )";
    exit(-1);
}

int main(int ac, char** av)
{
    JKArgs args(ac,av);
    if(args.count("extract"))
        ExtractPhrasePairs(args);
    else if(args.count("score"))
        Score(args);
    else if(args.count("readlog"))
        Readlog(args);
    else if(args.count("em"))
        em(args);
    else if(args.count("combine"))
        combine(args);
    else if(args.count("filter"))
        filter(args);
    else usage();
}
