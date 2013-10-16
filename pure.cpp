#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <array>
#include <map>
#include <boost/range/irange.hpp>

#include <boost/tokenizer.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include "JKArgs.h"

using std::map;
using std::min;
using std::list;
using std::unordered_map;
using std::string;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::vector;
using std::pair;
using std::make_pair;
using std::cerr;
using boost::tokenizer;
using boost::char_separator;
using boost::split;
using boost::is_any_of;
using boost::trim;
using boost::irange;
using boost::split_regex;
using namespace boost;
using namespace std;
using std::array;

void usage();
void PhraseCutoff(const string& filename, 
        const string& log_prefix, 
        vector<vector<string>>* p_src_corpus, 
        vector<vector<int>>* p_max_lens,
        int max_phrase_length,
        int cutoff)
{
    if(p_src_corpus==nullptr||p_max_lens==nullptr)return;
    bool stat=(log_prefix=="");
    ifstream fsrc(filename.c_str());
    ofstream flog;
    if(log_prefix!="")flog.open(log_prefix.c_str());

    
    vector<unordered_map<string, list<pair<int,int>>>> ngram_map(max_phrase_length); 
    vector<vector<string>>& src_corpus=*p_src_corpus;
    vector<vector<int>>& max_lens=*p_max_lens;
    
    for(string line; getline(fsrc,line);){
        trim(line);
        if(line=="")continue; 
        src_corpus.push_back(vector<string>());
        split(src_corpus.back(),line,is_any_of(" \t"));
    }
    for(uint64_t i=0; i<src_corpus.size();i++){
        for (uint64_t j=0;j< src_corpus[i].size(); j++){
            ngram_map[0][src_corpus[i][j]].push_back(make_pair(i,j));
        }
    }

    int threshold=0;
    if(!stat)threshold=cutoff;
    for(int len=2;len<=max_phrase_length;len++){
        cerr<<"process length "<<len<<endl;
        for(auto& item : ngram_map[len-2]){
            if(item.second.size()>(uint64_t)threshold){
                for(auto& pos : item.second){
                    auto& sent=src_corpus[pos.first];
                    auto& ind=pos.second;
                    if(ind+len-1<(int64_t)sent.size()){
                        ngram_map[len-1][item.first+" "+sent[ind+len-1]].push_back(pos);
                    }
                }
            }
        }
    }
    
    for(auto& sent : src_corpus){
        max_lens.push_back(vector<int>(sent.size(),1));
    }
    for(int i=1;i<max_phrase_length;i++){
        for(auto& item : ngram_map[i]){
            if(item.second.size()>(uint64_t)cutoff){
                for(auto& pos: item.second){
                    max_lens[pos.first][pos.second]=i+1;
                }
            }
       }
    }
    
    if(flog.good()){
    
    vector<vector<int>> count(cutoff,vector<int>(max_phrase_length,0));

    for(int i=0;i<max_phrase_length;i++){
        for(auto& item : ngram_map[i]){
            for(int t =0; t<cutoff;t++){
                if(item.second.size()>(uint64_t)t)
                    count[t][i]++;
            }
        }
    }
    
    for(int i=0;i<max_phrase_length;i++){
        flog<<i+1<<"\t";
        for(int t=0;t<cutoff;t++){
            //cout<<i<<"\t";
            flog<<count[t][i]<<"\t";
        }
        flog<<endl;
    }
    
    for(int t=0;t<cutoff;t++){
        double length=0;
        double num=0;
        for(int i=0;i<max_phrase_length;i++){
           length+=count[t][i]*(i+1); 
           num+=count[t][i];
        }
        flog<<"cutoff "<<t<<", size "<<num<<", avg length "<<length/num<<endl;
    }
    }
}

void GenerateFactorial(vector<double>& fac)
{
    if(fac.size()==0)return;
    fac[0]=1;
    for(uint64_t i=1;i<fac.size();i++){
        fac[i]=fac[i-1]*i;
    }
}

bool GenerateCombinatorial(vector<vector<double>>& com)
{
    uint64_t dim=com.size();
    if(dim==0)return false;
    if(dim!=com[0].size())return false;
    for(uint64_t i=1;i<com.size()-1;i++){
        com[i][0]=1;
        for(uint64_t j=1;j<=i;j++){
            com[i][j]=com[i][j-1]*(i-j+1)/j;
            cout<<com[i][j]<<" ";
        }
        cout<<endl;
    }
    return true; 
}

double CalculateDenominator(vector<vector<double>>& com, vector<double>& fac, int n, int m)
{
    uint64_t dim=min(n,m);
    double sum=0;
    assert(dim+2<com.size()&&dim+2<fac.size());
    for(uint64_t i=0;i<=dim;i++){
        sum+=com[n][i]*com[m][i]*fac[i+1];
    }
    return sum;
}

double CalculateNumerator(vector<vector<double>>& com, vector<double>& fac, int n, int m)
{
    uint64_t dim=min(n,m);
    double sum=0;
    assert(dim+2<com.size()&&dim+2<fac.size());
    for(uint64_t i=0;i<=dim;i++){
        sum+=com[n][i]*com[m][i]*fac[i+2];
    }
    return sum;
}

bool GenerateDenominator(vector<vector<double>>& denom)
{
    uint64_t dim=denom.size();
    if(dim==0)return false;
    if(dim!=denom[0].size())return false;
    vector<vector<double>> com(dim+3,vector<double>(dim+3,0));
    vector<double> fac(dim+3,0);
    GenerateCombinatorial(com);
    GenerateFactorial(fac);
    for(uint64_t i=1;i<dim;i++){
        for(uint64_t j=1;j<dim;j++){
            denom[i][j]=CalculateDenominator(com,fac,(int)i,(int)j);
        }
    }
    return true;
}

bool GenerateNumerator(vector<vector<double>>& denom)
{
    uint64_t dim=(denom.size());
    if(dim==0)return false;
    if(dim!=denom[0].size())return false;
    vector<vector<double>> com(dim+3,vector<double>(dim+3,0));
    vector<double> fac(dim+3,0);
    GenerateCombinatorial(com);
    GenerateFactorial(fac);
    for(int i=1;i<(int64_t)dim;i++){
        for(int j=1;j<(int64_t)dim;j++){
            denom[i][j]=CalculateNumerator(com,fac,i,j);
        }
    }
    return true;
}
bool ExtractPhrasePairs(const string& src, 
        const string& tgt, 
        const string& out, 
        int max_sentence_length,
        int max_phrase_length,
        double max_length_ratio,
        int min_phrase_count,
        bool inmemory)
{    
    if(src==""||tgt==""||out=="")return false; 
    vector<vector<string>> src_corpus,tgt_corpus;
    vector<vector<int>> src_max_lens,tgt_max_lens;
    string log_prefix="";
    string logf="",loge="";
    if(log_prefix!=""){
        logf="log.f";
        loge="log.e";
    }

    PhraseCutoff(src,logf,&src_corpus,&src_max_lens,max_phrase_length,min_phrase_count);
    PhraseCutoff(tgt,loge,&tgt_corpus,&tgt_max_lens,max_phrase_length,min_phrase_count);
    
    vector<vector<double>> denom(100,vector<double>(100,0)),numer=denom;
    GenerateDenominator(denom);
    GenerateNumerator(numer);
/*
    cout<<"denom:\n";
    for(auto& item:denom){
        for(auto& j:item)
            cout<<j<<" ";
        cout<<endl;
    }
    cout<<"numer:\n";
    for(auto& item:numer){
        for(auto& j:item)
            cout<<j<<" ";
        cout<<endl;
    }
*/
    ofstream os(out.c_str());
    map<string,map<string,pair<double,double>>> pt;
    for(uint64_t sid=0;sid<src_corpus.size();sid++){
        vector<string>& ssent=src_corpus[sid];
        vector<string>& tsent=tgt_corpus[sid];
        int n=static_cast<int>(ssent.size());
        int m=static_cast<int>(tsent.size());
        if(n>max_sentence_length||m>max_sentence_length)continue;
        for(uint64_t i=0;i<ssent.size();i++){
            for(uint64_t j=0;j<tsent.size();j++){
                string sphrase="";
                for(int k=0;k<src_max_lens[sid][i];k++){
                    if(sphrase!="")sphrase+=" ";
                    sphrase+=ssent[i+k];
                    string tphrase="";
                    for(int l=0;l<tgt_max_lens[sid][j];l++){
                        if((k+1)>max_length_ratio*(l+1)||(l+1)>max_length_ratio*(k+1))continue;
                        if(tphrase!="")tphrase+=" ";
                        tphrase+=tsent[j+l];
                        double prob=1E-5;
                        if(n-k-2>0&&m-l-2>0&&n>1&&m>1)
                            prob=numer[n-k-2][m-l-2]/denom[n-1][m-1];
                        if(!inmemory)
                            os<<sphrase<<" ||| "<<tphrase<<" ||| "<<prob<<" 1"<<endl;
                        else{
                            auto& item=pt[sphrase][tphrase];
                            item.first+=prob;
                            item.second+=1;
                        }
                    }
                }
            }
        }
    }
    if(inmemory){
        for(auto& m: pt)
            for(auto& i: m.second)
                os<<m.first<<" ||| "<<i.first<<" ||| "<<i.second.first<<" "<<i.second.second<<endl;
    }
    os.close();
    return 1;
}
void ExtractPhrasePairs(JKArgs& args){
    string src="",tgt="",log_prefix="",out="",in="";
    int maxlen=7;
    int cutoff=5;
    /*
     namespace po = boost::program_options;
     po::options_description desc("Allowed options");
     desc.add_options()
     ("help,h", "produce help message")
     ("src,s", po::value<string>(&src)->required(), "source file name")
     ("tgt,t", po::value<string>(&tgt), "target file name")
     ("out,o",po::value<string>(&out), "output file" )
     //       ("log", po::value<string>(&log_prefix),"log prefix")
     //       ("mlen",po::value<string>(), "max phrase len" )
     //       ("cutoff,c",po::value<int>(&cutoff),"phrase count cutoff" )
     ;
     po::variables_map vm;
     
     try{
     po::store(po::parse_command_line(ac, av, desc), vm);
     po::notify(vm);
     }
     catch(po::error& e){
     cerr << "ERROR: " << e.what() << endl << endl;
     cerr << desc << endl;
     return -1;
     }
     if (vm.count("help")) {
     cout << desc << endl;
     return 1;
     }
     if(vm.count("mlen")){
     //maxlen=vm["mlen"].as<int>();
     }
     */
    bool inmemory=true;
    if(args.count("src"))src=args["src"];
    if(args.count("tgt"))tgt=args["tgt"];
    if(args.count("log"))log_prefix=args["log"];
    if(args.count("o"))out=args["o"];
    if(args.count("maxlen"))maxlen=stoi(args["maxlen"]);
    if(args.count("cutoff"))cutoff=stoi(args["cutoff"]);
    if(args["inmemory"]=="false")inmemory=false;
    ExtractPhrasePairs(src,tgt,out,40,maxlen,4,cutoff,inmemory);
}

typedef map<string,map<string,double>> LexDic;

bool LoadLex(ifstream& is, LexDic& lex){
    for(;!is.eof();){
        string src="",tgt="";
        double score;
        is>>src>>tgt>>score;
        if(src=="")break;
        lex[src][tgt]=score;
        //cout<<"load: "<<src<<" => "<<tgt<<" "<<score<<endl;
    }
    if(lex.size()==0)return false;
    else return true;
}

double ScoreLex(vector<string>& src, vector<string>& tgt, LexDic& lex_s2t){
    double result=1;
    for(auto& t: tgt){
        double s2t=lex_s2t["<NULL>"][t];
        for(auto& s: src){
            s2t+=lex_s2t[s][t];
            
        }
        result*=s2t;
    }
    return result;
}

struct PhraseInfo {
    double ps2t,pt2s,ls2t,lt2s;
    PhraseInfo(){ps2t=pt2s=ls2t=lt2s=1;};
    PhraseInfo(double ls,double ps, double lt, double pt){ps2t=ps;pt2s=pt;ls2t=ls;lt2s=lt;}
};
typedef map<string,map<string,PhraseInfo>> PhraseTable;
enum Scoring { Frac,Count,CountLex,OnlyLex};

bool Score(JKArgs& args){
    if(!args.count("i")||!args.count("o"))usage();
    const string& in=args["i"];
    const string& out=args["o"];
    int nbest=args.count("nbest")?stoi(args["nbest"]):100;
    bool uselex=false;
    ifstream flex_s2t,flex_t2s;
    LexDic lex_s2t,lex_t2s;
    PhraseTable pt;
    
    if(args.count("lex_s2t"))flex_s2t.open(args["lex_s2t"]);
    if(args.count("lex_t2s"))flex_t2s.open(args["lex_t2s"]);
    if(args.count("lex_s2t")||args.count("lex_t2s"))uselex=true;
    Scoring scoring=Frac;
    if(args["scoring"]=="CountLex")scoring=CountLex;
    else if(args["scoring"]=="Count")scoring=Count;
    else if(args["scoring"]=="OnlyLex")scoring=OnlyLex;
    
        
    ifstream fin(in);
    ofstream fout(out);
    string prev_src="",prev_tgt="";
    for(string line; getline(fin,line);){
        trim(line);
        if(line=="")continue;
        vector<string> content;
        split_regex(content,line,regex(" => | \\|\\|\\| "));
        if(content.size()<3)continue;
        vector<string> features;
        split_regex(features,content[2],regex(" "));
        /*for(auto& f : features)
            cout<<f<<" ";
        cout<<endl;*/
        if(features.size()==1){
            double fraccount=(double)stod(features[0]);
            pt[content[0]][content[1]]=PhraseInfo((double)1.0,fraccount,(double)1.0,fraccount);
        }
        else if(features.size()==2){
            double count=(double)stod(features[1]);
            double fraccount=(double)stod(features[0]);
            pt[content[0]][content[1]]=PhraseInfo(count,fraccount,(double)1.0,fraccount);
        }
        else if(features.size()>=4){
            pt[content[0]][content[1]]=PhraseInfo(stod(features[0]),stod(features[1]),stod(features[2]),stod(features[3]));
        }
    }
    map<string,double> src_sum,tgt_sum;
    if(uselex){
        if(LoadLex(flex_s2t, lex_s2t)==false)return false;
        if(LoadLex(flex_t2s, lex_t2s)==false)return false;
        
        for(auto& m: pt){
            vector<string> src;
            split(src,m.first,is_any_of(" \t"));
            vector<pair<string,PhraseInfo>> phrases;
            vector<double> scores;
            for(auto& i: m.second){
                vector<string> tgt;
                split(tgt,i.first,is_any_of(" \t"));
                double lex_s2t_score=ScoreLex(src, tgt, lex_s2t);
                double lex_t2s_score=ScoreLex(tgt, src, lex_t2s);
                if(scoring==CountLex){i.second.ps2t=i.second.pt2s=i.second.ls2t*(lex_s2t_score+lex_t2s_score)/(double)2.0;}
                else if(scoring==Count){i.second.ps2t=i.second.pt2s=i.second.ls2t;}
                else if(scoring==Frac){i.second.pt2s=i.second.ps2t;}
                phrases.push_back(
                        make_pair(i.first,
                        PhraseInfo(lex_s2t_score,i.second.ps2t,lex_t2s_score,i.second.pt2s)));
                scores.push_back(lex_s2t_score+lex_t2s_score);
            }
            pt[m.first].clear();
            sort(scores.rbegin(),scores.rend());
            double threshold=scores[scores.size()>(uint64_t)nbest?nbest:scores.size()-1];
            for(auto& p:phrases){
                if(p.second.ls2t+p.second.lt2s>=threshold){
                    pt[m.first][p.first]=p.second;
                }
            }
        }
    }
    if(scoring==OnlyLex){
        for(auto& m: pt)
            for(auto& p: m.second)
                fout<<m.first<<" ||| "<<p.first<<" ||| "<<p.second.ls2t<<" 1 "<<p.second.lt2s<<" 1 2.718"<<endl;
        return true;
    }
    else{
        for(auto& m: pt){
            for(auto& i: m.second){
                src_sum[m.first]+=i.second.ps2t;
                tgt_sum[i.first]+=i.second.ps2t;
            }
        }
        for(auto& m: pt){
            double ssum=src_sum[m.first];
            for(auto& p: m.second){
                auto tsum=tgt_sum[p.first];
                fout<<m.first<<" ||| "<<p.first<<" ||| "<<p.second.ls2t<<" "
                <<p.second.ps2t/ssum<<" "<<p.second.lt2s<<" "<<p.second.pt2s/tsum<<" 2.718"<<endl;
            }
        }
    }
    fout.close();
    return true;
}

void Readlog(JKArgs& args){
    if(!args.count("i"))usage();
    ifstream is(args["i"]);
    
    vector<vector<double>> scores(8,vector<double>(6,0));
    for(string line; getline(is,line);){
        vector<string> words;
        split_regex(words,line,regex(" |-pt\\.|\\.lex|/"));
        int maxlen=0;
        int cutoff=0;
        for(size_t i=0;i<words.size();i++){
            if(words[i]=="evaluation"){
                maxlen=words[i+1][1]-'0';
                cutoff=words[i+1][3]-'0';
                //cerr<<maxlen<<","<<cutoff<<endl;
            }
            else if(words[i]=="BLEUr4n4[%]")
                scores[maxlen][cutoff]=stod(words[i+1]);
            
        }
    }
    cout<<R"(
\begin{table}[H]
\centering
\begin{tabular}{ c c | c | c | c | c|c }
& \multicolumn{5}{c}{\bf{ count cutoff}}  \\
&  & 1 &	2 &	3 &	4 &	5\\
\hline
\multirow{7}{*}{\bf{max length}}
)";
     for(int i=1;i<8;i++){
        cout<<"& "<<i<<"";
        for(int j=1;j<6;j++){
            cout<<" &\t";
            if(scores[i][j]==0)
                cout<<"\\bf{N.A.}";
            else cout<<scores[i][j];
        }
         cout<<"\\\\"<<endl;
         if(i<7)cout<<R"(\cline{2-7})"<<endl;
    }
    cout<<R"(\hline
\end{tabular}
\caption{bleu of different maxlen and cutoff}
\end{table}
    
)";
    
}

void usage(){
    cerr<<
    R"(
    proc -extract -src=source_file -tgt=target_file -o=output_file -maxlen=int -cutoff=int -inmemory=true
    -score -i=input -o=output [-nbest=int] [-lengthsort] [-lex_s2t=file] [-lex_t2s=file] [-lexcond=false]
            [-scoring=Frac|Count|CountLex] [-zeroP]
    -readlog -i=file
    
    )";
    exit(-1);
}
//
//
int main(int ac, char** av)
{
    JKArgs args(ac,av);
    if(args.count("extract"))
        ExtractPhrasePairs(args);
    else if(args.count("score"))
        Score(args);
    else if(args.count("readlog"))
        Readlog(args);
    else usage();
}
