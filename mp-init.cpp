#include "mp-init.h"
#include "pure.h"

/*Fractional number generator*/
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
            //cout<<com[i][j]<<" ";
        }
        //cout<<endl;
    }
    return true;
}
double CalculateDenominator(vector<vector<double>>& com,
                            vector<double>& fac,
                            int n, int m)
{
    uint64_t dim=min(n,m);
    double sum=0;
    assert(dim+2<com.size()&&dim+2<fac.size());
    for(uint64_t i=0;i<=dim;i++){
        sum+=com[n][i]*com[m][i]*fac[i+1];
    }
    return sum;
}
double CalculateNumerator(vector<vector<double>>& com,
                          vector<double>& fac, int n, int m)
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

/*Cutoff phrases using max length and count cutoff constraints,
    return a group of vectors:
    showing the max phrase length for each position in each sentence.*/
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


    vector<unordered_map<string, list<pair<int,int>>>>
        ngram_map(max_phrase_length);
    vector<vector<string>>& src_corpus=*p_src_corpus;
    vector<vector<int>>& max_lens=*p_max_lens;

    for(string line; getline(fsrc,line);){
        trim(line);
        if(line=="")continue;
        src_corpus.push_back(vector<string>());
        split(src_corpus.back(),line,is_any_of(" \t"));
        //for(int i=0;i<src_corpus.back().size();i++)
        //    cout<<i<<": "<<src_corpus.back()[i]<<endl;
    }

    for(auto& sent : src_corpus){
        max_lens.push_back(vector<int>(sent.size(),max_phrase_length));
        for(int i=0;i<(int)sent.size();i++){
            max_lens.back()[i]=
                min(max_phrase_length, (int)sent.size()-i);
        }
    }

    if(cutoff==0)return;
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
                        ngram_map[len-1][item.first+" "
                                         +sent[ind+len-1]].push_back(pos);
                    }
                }
            }
        }
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

/* Get the A[i][j][ilen][jlen] for arrary A[n][m][l][l] */
int index(int i, int ilen, int j, int jlen, int n, int m, int l){
    if(i<0||j<0||ilen<0||jlen<0||i>=n||j>=m||ilen>=l||jlen>=l)return -1;
    return i*m*l*l+j*l*l+ilen*l+jlen;
}

SentenceCache::
SentenceCache(int n_, int m_, int l_):n(n_),m(m_),l(l_){
    p=new PSimplePhraseInfo[n*m*l*l];
    memset(p,0,n*m*l*l*sizeof(void*));
}

PSimplePhraseInfo&
SentenceCache::
operator ()(int i, int ilen, int j, int jlen){
    int ind=index(i,ilen,j,jlen,n,m,l);
    return p[ind];
}

bool
LexDic::
read(const string& in){
    ifstream is(in.c_str());
    for(;!is.eof();){
        string src="",tgt="";
        double score;
        is>>src>>tgt>>score;
        if(src=="")break;
        (*this)[src][tgt]=score;
    }
    if(this->size()==0)return false;
    else return true;
}


void
SimplePhraseTable::
normalize(){
    for(auto& omap: *this){
        double total=0;
        auto& imap=omap.second;
        for(auto& item : imap){
            total+=item.second.count;
        }
        for(auto& item : imap){
            item.second.prob=item.second.count/total;
            item.second.count=0.0;
        }
    }
}

void
SimplePhraseTable::
reset_count(double value){
    for(auto& omap: *this){
        for(auto& item : omap.second){
            item.second.count=value;
        }
    }
}

void
SimplePhraseTable::
normalize_using_prob(){
    for(auto& omap: *this){
        double total=0;
        for(auto& item : omap.second){
            total+=item.second.prob;
        }
        for(auto& item : omap.second){
            item.second.prob/=total;
            item.second.count=0.0;
        }
    }
}

void
SimplePhraseTable::
ibm1_scoring(LexDic& ibm1){
    for(auto& omap: *this){
        vector<string> src;
        split(src,omap.first,is_any_of(" \t"));
        for(auto& item : omap.second){
            vector<string> tgt;
            split(tgt,item.first,is_any_of(" \t"));
            item.second.prob=ScoreLex(src, tgt, ibm1);
        }
    }
}

void
SimplePhraseTable::
print(ostream& os){
    for(auto& omap: *this){
        for(auto& item : omap.second){
            os<<omap.first<<" ||| "<<item.first<<" ||| "
            <<item.second.prob<<" "<<item.second.count<<endl;
        }
    }
}

void SimplePhraseTable::
read(string filename, bool reverse){
    ifstream is(filename.c_str());
    for(string line; getline(is,line);){
        trim(line);
        if(line=="")continue;
        vector<string> content;
        split_regex(content,line,regex(" => | \\|\\|\\| "));
        if(content.size()<3)continue;
        vector<string> features;
        split_regex(features,content[2],regex(" "));
        if(features.size()==1){
            double fraccount=(double)stod(features[0]);
            (*this)[content[0]][content[1]]=
            SimplePhraseInfo(fraccount,fraccount);
        }
        else if(features.size()==2){
            double count=(double)stod(features[1]);
            double fraccount=(double)stod(features[0]);
            (*this)[content[0]][content[1]]=
            SimplePhraseInfo(fraccount,fraccount);
        }
        else if(features.size()>=4){
            if(reverse){
                (*this)[content[1]][content[0]]=
                SimplePhraseInfo(stod(features[3]),stod(features[3]));
            }
            else{
                (*this)[content[0]][content[1]]=
                SimplePhraseInfo(stod(features[1]),stod(features[1]));
            }
        }
    }
    is.close();
}

/*
 Extract initial phrase pairs and set cache pointers for each sentence pair
 Each entry in the phrase table contains two features:
 (prob,count) where prob = fractional count and count = absolute count*/
bool ExtractPhrasePairs(const string& src,
                        const string& tgt,
                        const string& out,
                        int max_sentence_length,
                        int max_phrase_length,
                        double max_length_ratio,
                        int min_phrase_count,
                        bool inmemory,
                        SimplePhraseTable& pt,
                        CorpusCache* cache){

    bool create_pt=pt.empty();
    if(src==""||tgt=="")return false;
    vector<vector<string>> src_corpus,tgt_corpus;
    vector<vector<int>> src_max_lens,tgt_max_lens;
    string log_prefix="";
    string logf="",loge="";
    if(log_prefix!=""){
        logf="log.f";
        loge="log.e";
    }

    PhraseCutoff(src,logf,&src_corpus,&src_max_lens,
                 max_phrase_length,min_phrase_count);
    PhraseCutoff(tgt,loge,&tgt_corpus,&tgt_max_lens,
                 max_phrase_length,min_phrase_count);

    vector<vector<double>> denom(100,vector<double>(100,0)),numer=denom;
    GenerateDenominator(denom);
    GenerateNumerator(numer);

    ofstream os(out.c_str());
    for(uint64_t sid=0;sid<src_corpus.size();sid++){
        vector<string>& ssent=src_corpus[sid];
        vector<string>& tsent=tgt_corpus[sid];
        int n=static_cast<int>(ssent.size());
        int m=static_cast<int>(tsent.size());
        if(n>max_sentence_length||m>max_sentence_length){
            continue;
        }
        if(cache!=nullptr)
            cache->push_back(SentenceCache((int)ssent.size(),(int)tsent.size(),5));
        for(int i=0;i<(int)ssent.size();i++){
            for(int j=0;j<(int)tsent.size();j++){
                string sphrase="";
                for(int k=0;k<src_max_lens[sid][i];k++){
                    if(sphrase!="")sphrase+=" ";
                    sphrase+=ssent[i+k];
                    string tphrase="";
                    for(int l=0;l<tgt_max_lens[sid][j];l++){
                        if(tphrase!="")tphrase+=" ";
                        tphrase+=tsent[j+l];
                        if((k+1)>(((int)max_length_ratio)*(l+1))||
                           (l+1)>(((int)max_length_ratio)*(k+1))){
                            //cerr<<i<<" "<<k<<" "<<j<<" "<<l<<endl;
                            continue;
                        }
                        //cerr<<sphrase <<" ||| "<<tphrase<<" ||| "<<k+1<<" "<<l+1<<endl;
                        double prob=1E-5;
                        if(n-k-2>0&&m-l-2>0&&n>1&&m>1)
                            prob=numer[n-k-2][m-l-2]/denom[n-1][m-1];
                        if(!inmemory&&os.good())
                            os<<sphrase<<" ||| "<<tphrase
                            <<" ||| "<<prob<<" 1"<<endl;
                        else{
                            if(create_pt){
                                auto& item=pt[sphrase][tphrase];
                                if(cache!=nullptr){
                                    cache->back()(i,k,j,l)=&item;
                                }
                                item.prob+=prob;
                                item.count+=1.0;
                            }
                            else{
                                if(k==0&&l==0){
                                    auto& item=pt[sphrase][tphrase];
                                    if(cache!=nullptr){
                                        cache->back()(i,k,j,l)=&item;
                                    }
                                    if(item.prob==0){
                                        item.prob=0.001;
                                        item.count=0.001;
                                    }
                                }
                                else{
                                    if(pt.find(sphrase)==pt.end())continue;
                                    auto& omap=pt[sphrase];
                                    auto iter=omap.find(tphrase);
                                    if(iter==omap.end())continue;
                                    /*
                                    cerr<<"found "<<sphrase<<" ||| "<<tphrase
                                        <<" ||| "<<iter->second.prob<<" "
                                        <<iter->second.count<<endl;*/
                                    cache->back()(i,k,j,l)=&(iter->second);
                                }
                            }
                        }
                    }
                }
            }
        }
        //cerr<<cache->back()(ssent.size(),4,tsent.size(),4)<<endl;
        //cerr<<sid<<endl;
    }
    if(inmemory&&os.good()){
        for(auto& m: pt)
            for(auto& i: m.second)
                os<<m.first<<" ||| "<<i.first<<" ||| "<<i.second.prob
                <<" "<<i.second.count<<endl;
    }
    os.close();
    return true;
}


void expectation(CorpusCache& cache, double& alpha){
    double alphaCount=0;
    for(auto& sp: cache){
        vector<vector<double>> target_probs(sp.m,vector<double>(sp.l,0.0));
        alpha/=sp.n*sp.l;
        for(int j=0;j<sp.m;j++){
            for(int jlen=0;jlen<sp.l;jlen++){
                for(int i=0;i<sp.n;i++){
                    for(int ilen=0;ilen<sp.l;ilen++){
                        if(sp(i,ilen,j,jlen)!=(void*)0){
                            target_probs[j][jlen]+=
                                (sp(i,ilen,j,jlen)->prob*alpha);
                            if(sp(i,ilen,j,jlen)->prob>1){
                                cerr<<"wth prob>1 : "
                                <<sp(i,ilen,j,jlen)->prob<<endl;
                            }
                        }
                    }
                }
                //cout<<target_probs[j][jlen]<<" ";
            }
            //cout<<endl;
        }
        //cout<<endl;

        for(int j=0;j<sp.m;j++){
            for(int jlen=0;jlen<sp.l;jlen++){
                if(target_probs[j][jlen]>1)
                    cerr<<"error :"<<target_probs[j][jlen]<<endl;
            }
        }

        vector<double> forward(sp.m,0.0),backward(sp.m,0.0);
        //forward[i] is the posterior probability of target words of 1...i+1
        for(int i=0;i<sp.l&&i<sp.m;i++)
            forward[i]=target_probs[0][i];
        for(int i=1;i<(int)forward.size();i++){
            for(int j=1;j<=sp.l&&i-j>=0;j++){
                forward[i]+=forward[i-j]*target_probs[i-j+1][j-1];
            }
        }
        //backward[i] is the posterior probability of target words of i+1...m
        for(int i=0;i<sp.l&&i<sp.m;i++)
            backward[sp.m-i-1]=target_probs[sp.m-i-1][i];
        for(int i=sp.m-2;i>=0;i--){
            for(int j=1;j<=sp.l&&i+j<sp.m;j++){
                backward[i]+=target_probs[i][j-1]*backward[i+j];
            }
        }

        //viterbi
        vector<pair<double,int> > viterbi(sp.m,pair<double,int>(0.0,0));
        for(int i=0;i<sp.l&&i<sp.m;i++)
            viterbi[i]=pair<double,int>(target_probs[0][i],-1);
        for(int i=1;i<(int)forward.size();i++){
            for(int j=1;j<=sp.l&&i-j>=0;j++){
                if(viterbi[i-j].first*target_probs[i-j+1][j-1]>viterbi[i].first)
                viterbi[i]=
                    make_pair(viterbi[i-j].first*target_probs[i-j+1][j-1],i-j);
            }
        }
        int pos=sp.m-1;
        string sequence="";
        while(pos>=0){
            sequence=to_string(pos)+" "+sequence;
            pos=viterbi[pos].second;
        }
        //cout<<"best seg:"<<sequence<<endl;

        //make sure forward[sp.m-1]==backward[0];
        assert(backward[0]>0);
        if(abs(forward[sp.m-1]-backward[0])>=1e-5*backward[0])
            cerr<<forward[sp.m-1]<<", "<<backward[0]<<endl;
        assert(abs(forward[sp.m-1]-backward[0])<1e-5*backward[0]);
        //cerr<<"backward[0]:"<<backward[0]<<endl;
        //collect fractional count for each phrase pair
        //fraccount=forward[j]*backward[j+jlen]*p(t|s)/backward[0];

        for(int j=0;j<sp.m;j++){
            for(int jlen=0;jlen<sp.l&&j+jlen+1<=sp.m;jlen++){
                double segprob=0;
                double before=1;
                double after=1;
                if(j>0)before=forward[j-1];
                if(j+jlen+1<sp.m)after=backward[j+jlen+1];

                segprob=before*after*target_probs[j][jlen]/backward[0];

                if(segprob>1||segprob<=0){
                    //cerr<<"segprob "<<segprob<<","<<j<<","<<jlen<<endl;
                }
                if(segprob<=0)continue;
                for(int i=0;i<sp.n;i++){
                    for(int ilen=0;ilen<sp.l&&ilen+i+1<=sp.n;ilen++){
                        if(sp(i,ilen,j,jlen)!=(void*)0){
                            double count=sp(i,ilen,j,jlen)->prob*segprob*alpha
                                    /target_probs[j][jlen];
                            sp(i,ilen,j,jlen)->count+=count;
                            alphaCount+=count;
                            if(count>1)
                            cerr<<i<<","<<ilen<<","<<j
                                <<","<<jlen<<" ["<<sp.m<<","<<sp.n<<"]"
                                <<",count "<<count<<endl;
                        }
                    }
                }
            }
        }
        alpha*=sp.n*sp.l;
    }
    //cerr<<alphaCount<<","<<cache.size()<<endl;
    alpha=alphaCount/(alphaCount+cache.size());
}

void em(CorpusCache& cache, SimplePhraseTable& pt, int round, const string& out)
{
    double alpha=0.5;
    for(int i=0;i<round;i++){
        if(out!=""){
            string o=out+"."+to_string(i);
            ofstream os(o.c_str());
            pt.print(os);
            os.close();
        }
        cerr<<"round "<<i<<", alpha:"<<alpha<<endl;
        expectation(cache,alpha);
        /*if(out!=""){
            string o=out+"."+to_string(i)+".count";
            ofstream os(o.c_str());
            pt.print(os);
            os.close();
        }*/
        pt.normalize();

    }
}

void em(JKArgs& args){
    const string& src=args["src"];
    const string& tgt=args["tgt"];
    const string& out=args["o"];
    const string& lex=args["lex"];
    int round=5;
    if(args.count("round"))round=stoi(args["round"]);
    int max_sentence_length=100;
    int max_phrase_length=args.count("maxlen")?stoi(args["maxlen"]):5;
    double max_length_ratio=4;
    int min_phrase_count=0;
    SimplePhraseTable pt;
    if(args.count("pt"))pt.read(args["pt"],args.count("reverse"));
    CorpusCache cache;
    ExtractPhrasePairs(src,tgt,"",
                       max_sentence_length,
                       max_phrase_length,
                       max_length_ratio,
                       min_phrase_count,
                       true,
                       pt,
                       &cache);
    if(lex!=""){
        LexDic lex_s2t;
        lex_s2t.read(lex);
        pt.ibm1_scoring(lex_s2t);
    }
    else if(args["init"]=="frac")
        pt.normalize_using_prob();
    else
        pt.normalize();
    em(cache,pt,round,out);
}

/*Extract initial phrase pairs*/
bool ExtractPhrasePairs(const string& src,
                        const string& tgt,
                        const string& out,
                        int max_sentence_length,
                        int max_phrase_length,
                        double max_length_ratio,
                        int min_phrase_count,
                        bool inmemory)
{
    SimplePhraseTable pt;
    return ExtractPhrasePairs(src,tgt,out,max_sentence_length,
                               max_phrase_length,
                               max_length_ratio,
                               min_phrase_count,
                               inmemory, pt, nullptr);
}

void ExtractPhrasePairs(JKArgs& args){
    string src="",tgt="",log_prefix="",out="",in="";
    int max_phrase_len=MAX_PHRASE_LEN;
    int max_sent_len=100;
    int cutoff=MAX_CUTOFF;
    bool inmemory=true;
    if(args.count("src"))src=args["src"];
    if(args.count("tgt"))tgt=args["tgt"];
    if(args.count("log"))log_prefix=args["log"];
    if(args.count("o"))out=args["o"];
    if(args.count("max_phrase_len"))max_phrase_len=stoi(args["max_phrase_len"]);
    if(args.count("cutoff"))cutoff=stoi(args["cutoff"]);
    if(args["inmemory"]=="false")inmemory=false;
    if(args.count("max_sent_len"))max_sent_len=stoi(args["max_sent_len"]);
    ExtractPhrasePairs(src,tgt,out,max_sent_len,
                       max_phrase_len,4,cutoff,inmemory);
}

void combine(JKArgs& args){
    string ps2t=args["ps2t"];
    string pt2s=args["pt2s"];
    string out=args["o"];
    if(ps2t==""||pt2s==""||out=="")usage();
    LexDic lex_s2t,lex_t2s;
    bool uselex=false;
    if(args.count("lex_s2t")||args.count("lex_t2s"))uselex=true;
    if(uselex&&lex_s2t.read(args["lex_s2t"])==false)return;
    if(uselex&&lex_t2s.read(args["lex_t2s"])==false)return;

    RichPhraseTable pt;
    pt.read(ps2t);
    ifstream is(pt2s.c_str());
    for(string line; getline(is,line);){
        trim(line);
        if(line=="")continue;
        vector<string> content;
        split_regex(content,line,regex(" => | \\|\\|\\| "));
        if(content.size()<3)continue;
        vector<string> features;
        split_regex(features,content[2],regex(" "));
        auto& item=pt[content[1]][content[0]];
        item.pt2s=stod(features[0]);
        if(uselex){
            vector<string> src,tgt;
            split(src,content[1],is_any_of(" \t"));
            split(tgt,content[0],is_any_of(" \t"));
            item.ls2t=ScoreLex(src, tgt, lex_s2t);
            item.lt2s=ScoreLex(tgt, src, lex_t2s);
        }
        else item.ls2t=item.lt2s=1;
    }
    pt.print(out);
}

/*Score the phrase pair*/
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


void RichPhraseTable::
read(string filename){
    ifstream is(filename.c_str());
    for(string line; getline(is,line);){
        trim(line);
        if(line=="")continue;
        vector<string> content;
        split_regex(content,line,regex(" => | \\|\\|\\| "));
        if(content.size()<3)continue;
        vector<string> features;
        split_regex(features,content[2],regex(" "));
        if(features.size()==1){
            double fraccount=(double)stod(features[0]);
            (*this)[content[0]][content[1]]=
            PhraseInfo((double)1.0,fraccount,(double)1.0,fraccount);
        }
        else if(features.size()==2){
            double count=(double)stod(features[1]);
            double fraccount=(double)stod(features[0]);
            (*this)[content[0]][content[1]]=
            PhraseInfo(count,fraccount,(double)1.0,fraccount);
        }
        else if(features.size()>=4){
            (*this)[content[0]][content[1]]=
            PhraseInfo(stod(features[0]),stod(features[1]),
                       stod(features[2]),stod(features[3]));
        }
    }
    is.close();
}

void RichPhraseTable::
print(string filename){
    ofstream os(filename.c_str());
    for(auto& omap:*this){
        for(auto& item:omap.second){
            os<<omap.first<<" ||| "<<item.first<<" ||| "<<item.second.ls2t
                <<" "<<item.second.ps2t<<" "<<item.second.lt2s<<" "
            <<item.second.pt2s<<" 2.718"<<endl;
        }
    }
    os.close();
}

/*Score the raw phrase table*/
bool Score(JKArgs& args){
    if(!args.count("i")||!args.count("o"))usage();
    const string& in=args["i"];
    const string& out=args["o"];
    int nbest=args.count("nbest")?stoi(args["nbest"]):100;
    bool uselex=false;
    LexDic lex_s2t,lex_t2s;
    RichPhraseTable pt;

    if(args.count("lex_s2t")||args.count("lex_t2s"))uselex=true;
    Scoring scoring=Frac;
    if(args["scoring"]=="CountLex")scoring=CountLex;
    else if(args["scoring"]=="Count")scoring=Count;
    else if(args["scoring"]=="OnlyLex")scoring=OnlyLex;
    else if(args["scoring"]=="OnlyFrac")scoring=OnlyFrac;
    else if(args["scoring"]=="OnlyCount")scoring=OnlyCount;

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
            pt[content[0]][content[1]]=
            PhraseInfo((double)1.0,fraccount,(double)1.0,fraccount);
        }
        else if(features.size()==2){
            double count=(double)stod(features[1]);
            double fraccount=(double)stod(features[0]);
            pt[content[0]][content[1]]=
            PhraseInfo(count,fraccount,(double)1.0,fraccount);
        }
        else if(features.size()>=4){
            pt[content[0]][content[1]]=
            PhraseInfo(stod(features[0]),stod(features[1]),
                       stod(features[2]),stod(features[3]));
        }
    }
    map<string,double> src_sum,tgt_sum;
    if(uselex){
        if(lex_s2t.read(args["lex_s2t"])==false)return false;
        if(lex_t2s.read(args["lex_t2s"])==false)return false;

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
                if(scoring==CountLex){
                    i.second.ps2t=i.second.pt2s
                    =i.second.ls2t*(lex_s2t_score+lex_t2s_score)/(double)2.0;
                }
                else if(scoring==Count||scoring==OnlyCount){
                    i.second.ps2t=i.second.pt2s=i.second.ls2t;
                }
                else
                    i.second.pt2s=i.second.ps2t;
                phrases.push_back(
                    make_pair(i.first,
                              PhraseInfo(lex_s2t_score,i.second.ps2t,
                                         lex_t2s_score,i.second.pt2s)));
                scores.push_back(lex_s2t_score+lex_t2s_score);
            }
            pt[m.first].clear();
            sort(scores.rbegin(),scores.rend());
            double threshold=
            scores[scores.size()>(uint64_t)nbest?nbest:scores.size()-1];
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
                fout<<m.first<<" ||| "<<p.first<<" ||| "
                <<p.second.ls2t<<" 1 "<<p.second.lt2s<<" 1 2.718"<<endl;
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
                if(scoring==OnlyFrac||scoring==OnlyCount){
                    fout<<m.first<<" ||| "<<p.first<<" ||| 1 "
                    <<p.second.ps2t/ssum<<" 1 "
                    <<p.second.pt2s/tsum<<" 2.718"<<endl;
                }
                else{
                    fout<<m.first<<" ||| "<<p.first<<" ||| "<<p.second.ls2t<<" "
                    <<p.second.ps2t/ssum<<" "<<p.second.lt2s
                    <<" "<<p.second.pt2s/tsum<<" 2.718"<<endl;
                }
            }
        }
    }
    fout.close();
    return true;
}

/*Read log file, which summarize bleu scores by using different initial 
    phrase tables(with different max length and cutoff)*/
void Readlog(JKArgs& args){
    if(!args.count("i"))usage();
    ifstream is(args["i"]);
    string message=args["m"];

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
    &  & 1 &    2 &	3 &	4 &	5\\
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
    if(message!="")message=", "+message;
    string tail="\\hline\n\\end{tabular}\n\\caption\
    {BLEU of different maxlen and cutoff "+message+"}\n\\end{table}\n";
    cout<<tail<<endl;
}

