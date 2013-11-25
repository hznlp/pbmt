#include "mp.h"
extern Specs specs;
void em(JKArgs& args){
    const string& src=args["src"];
    const string& tgt=args["tgt"];
    const string& out=args["o"];
    const string& lex=args["lex"];
    bool knsmoothing=(args["kn"]=="true");
    int round=5;
    if(args.count("round"))round=stoi(args["round"]);
    specs.max_phrase_length=args.count("maxlen")?stoi(args["maxlen"]):5;
    MP1 mp1;
    if(args.count("pt"))mp1.pt().read(args["pt"],args.count("reverse"));
    CorpusCache cache;
    mp1.init(src,tgt,cache);
    if(lex!=""){
        LexDic lex_s2t;
        lex_s2t.read(lex);
        mp1.pt().ibm1_scoring(lex_s2t);
    }
    else if(args["init"]=="frac")
        mp1.pt().normalize_using_prob();
    else
        mp1.pt().normalize();
    if(args.count("viterbi"))
        mp1.viterbi(cache);
    else
        mp1.em(cache,round,out,knsmoothing);

}
