#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <boost/range/irange.hpp>

#include <boost/tokenizer.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
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

void PhraseCutoff(const string& filename, const string& log_prefix, 
                    vector<vector<string>>* p_src_corpus, vector<vector<int>>* p_max_lens ){
    if(p_src_corpus==nullptr||p_max_lens==nullptr)return;
    bool stat=log_prefix=="";
    ifstream fsrc(filename.c_str());
    ofstream flog;
    if(log_prefix!="")flog.open(log_prefix.c_str());

    int max_phrase_length=7;
    
    vector<unordered_map<string, list<pair<int,int>>>> ngram_map(max_phrase_length); 
    vector<vector<string>>& src_corpus=*p_src_corpus;
    vector<vector<int>>& max_lens=*p_max_lens;
    
    for(string line; getline(fsrc,line);){
        trim(line);
        if(line=="")continue; 
        src_corpus.push_back(vector<string>());
        split(src_corpus.back(),line,is_any_of(" \t"));
    }
    for(int i=0; i<src_corpus.size();i++){
        for (int j=0;j< src_corpus[i].size(); j++){
            ngram_map[0][src_corpus[i][j]].push_back(make_pair(i,j));
        }
    }

    int threshold=0;
    int cutoff=5;
    if(!stat)threshold=cutoff;
    for(int len=2;len<=max_phrase_length;len++){
        cerr<<"process length "<<len<<endl;
        for(auto& item : ngram_map[len-2]){
            if(item.second.size()>threshold){
                for(auto& pos : item.second){
                    auto& sent=src_corpus[pos.first];
                    auto& ind=pos.second;
                    if(ind+len-1<sent.size()){
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
            if(item.second.size()>cutoff){
                for(auto& pos: item.second){
                    max_lens[pos.first][pos.second]=i+1;
                }
            }
       }
    }
    
    if(flog.good()){
    
    vector<vector<int>> count(5,vector<int>(max_phrase_length,0));

    for(int i=0;i<max_phrase_length;i++){
        for(auto& item : ngram_map[i]){
            for(int t =0; t<5;t++){
                if(item.second.size()>t)
                    count[t][i]++;
            }
        }
    }
    
    for(int i=0;i<max_phrase_length;i++){
        flog<<i+1<<"\t";
        for(int t=0;t<5;t++){
            //cout<<i<<"\t";
            flog<<count[t][i]<<"\t";
        }
        flog<<endl;
    }
    
    for(int t=0;t<5;t++){
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

void GenerateFactorial(vector<double>& fac){
    if(fac.size()==0)return;
    fac[0]=1;
    for(int i=1;i<fac.size();i++){
        fac[i]=fac[i-1]*i;
    }
}

bool GenerateCombinatorial(vector<vector<double>>& com){
    int dim=com.size();
    if(dim==0)return false;
    if(dim!=com[0].size())return false;
    for(int i=1;i<com.size()-1;i++){
        com[i][0]=1;
        for(int j=1;j<=i;j++){
            com[i][j]=com[i][j-1]*(i-j+1)/j;
            cout<<com[i][j]<<" ";
        }
        cout<<endl;
    }
    
}

double CalculateDenominator(vector<vector<double>>& com, vector<double>& fac, int n, int m){
    int dim=min(n,m);
    double sum=0;
    assert(dim+2<com.size()&&dim+2<fac.size());
    for(int i=0;i<=dim;i++){
        sum+=com[n][i]*com[m][i]*fac[i+1];
    }
    return sum;
}

double CalculateNumerator(vector<vector<double>>& com, vector<double>& fac, int n, int m){
    int dim=min(n,m);
    double sum=0;
    assert(dim+2<com.size()&&dim+2<fac.size());
    for(int i=0;i<=dim;i++){
        sum+=com[n][i]*com[m][i]*fac[i+2];
    }
    return sum;
}

bool GenerateDenominator(vector<vector<double>>& denom){
    int dim=denom.size();
    if(dim==0)return false;
    if(dim!=denom[0].size())return false;
    vector<vector<double>> com(dim+3,vector<double>(dim+3,0));
    vector<double> fac(dim+3,0);
    GenerateCombinatorial(com);
    GenerateFactorial(fac);
    for(int i=1;i<dim;i++){
        for(int j=1;j<dim;j++){
            denom[i][j]=CalculateDenominator(com,fac,i,j);
        }
    }
    return true;
}

bool GenerateNumerator(vector<vector<double>>& denom){
    int dim=denom.size();
    if(dim==0)return false;
    if(dim!=denom[0].size())return false;
    vector<vector<double>> com(dim+3,vector<double>(dim+3,0));
    vector<double> fac(dim+3,0);
    GenerateCombinatorial(com);
    GenerateFactorial(fac);
    for(int i=1;i<dim;i++){
        for(int j=1;j<dim;j++){
            denom[i][j]=CalculateNumerator(com,fac,i,j);
        }
    }
    return true;
}
bool ExtractPhrasePairs(const string& src, const string& tgt, const string& out ){
    if(src==""||tgt==""||out=="")return false; 
    vector<vector<string>> src_corpus,tgt_corpus;
    vector<vector<int>> src_max_lens,tgt_max_lens;
    string log_prefix="";
    string logf="",loge="";
    if(log_prefix!=""){
        logf="log.f";
        loge="log.e";
    }

    PhraseCutoff(src,logf,&src_corpus,&src_max_lens);
    PhraseCutoff(tgt,loge,&tgt_corpus,&tgt_max_lens);
    
    vector<vector<double>> denom(100,vector<double>(100,0)),numer=denom;
    GenerateDenominator(denom);
    GenerateNumerator(numer);

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

    ofstream os(out.c_str());
    for(int sid=0;sid<src_corpus.size();sid++){
        vector<string>& ssent=src_corpus[sid];
        vector<string>& tsent=tgt_corpus[sid];
        int n=ssent.size();
        int m=tsent.size();
        if(n>40||m>40)continue;
        for(int i=0;i<ssent.size();i++){
            for(int j=0;j<tsent.size();j++){
                string sphrase="";
                for(int k=0;k<src_max_lens[sid][i];k++){
                    if(sphrase!="")sphrase+=" ";
                    sphrase+=ssent[i+k];
                    string tphrase="";
                    for(int l=0;l<tgt_max_lens[sid][j];l++){
                        if((k+1)>4*(l+1)||(l+1)>4*(k+1))continue;
                        if(tphrase!="")tphrase+=" ";
                        tphrase+=tsent[j+l];
                        double prob=1E-5;
                        if(n-k-2>0&&m-l-2>0&&n>1&&m>1)prob=numer[n-k-2][m-l-2]/denom[n-1][m-1];
                        os<<sphrase<<" => "<<tphrase<<" ||| "<<prob
                            <<endl;
                    }
                }
            }
        }
    }
    os.close();
}

void usage(){
    cerr<<"proc -src=srcfile [-tar=tarfile]"<<endl;
    exit(1);
}

int main(int ac, char** av)
{
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    string src="",tgt="",log_prefix="";
    desc.add_options()
            ("help", "produce help message")
            ("src", po::value<string>(&src)->required(), "source file name")
            ("tgt", po::value<string>(&tgt), "target file name")
            ("log", po::value<string>(&log_prefix),"log prefix")
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
    ExtractPhrasePairs(src,tgt,"pt");    
}


