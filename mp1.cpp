#include "mp1.h"
extern Specs specs;

void MP1::
init(string src, string tgt, string lex_s2t, string lex_t2s, CorpusCache& cache){
    SimplePhraseTable& pt_=*ppt_;
    LexDic* p_lex_s2t=NULL;
    LexDic* p_lex_t2s=NULL;
    if(lex_s2t!="")p_lex_s2t=new LexDic(lex_s2t);
    if(lex_t2s!="")p_lex_t2s=new LexDic(lex_t2s);

    ExtractPhrasePairs(src,tgt,"",
                       specs.max_sentence_length,
                       specs.max_phrase_length,
                       specs.max_length_ratio,
                       specs.min_phrase_count,
                       true,
                       p_lex_s2t,
                       p_lex_t2s,
                       pt_,
                       &cache);
}

void MP1::
expectation(CorpusCache& cache){
    double& alpha=alpha_;
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
                if(target_probs[j][0]==0){
                    target_probs[j][0]=1E-7;
                    //cerr<<"reset error target["<<j<<",0]"<<endl;
                }
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
                            //cerr<<"before update"<<endl;
                            //sp(i,ilen,j,jlen)->fractype.print(cerr);
                            sp(i,ilen,j,jlen)->fractype.updateLog(count);
                            //cerr<<"after update"<<endl;
                            //sp(i,ilen,j,jlen)->fractype.print(cerr);
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

void
MP1::em(CorpusCache& cache, int round, const string& out, bool knsmoothing)
{
    SimplePhraseTable& pt_=*ppt_;
    alpha_=0.5;
    for(int i=0;i<round;i++){
        if(out!=""){
            pt_.print(out+".m1."+to_string(i));
        }
        cerr<<"round "<<i<<", alpha:"<<alpha_<<endl;
        expectation(cache);
        if(!knsmoothing)
            pt_.normalize();
        else
            pt_.knsmoothing();
    }
}

void
MP1::
viterbi(CorpusCache& cache){
    double& alpha=alpha_;
    for(auto& sp: cache){
        vector<vector<double>> target_probs(sp.m,vector<double>(sp.l,0.0));
        vector<vector<pair<int,int>>> target_best(sp.m,
                                                  vector<pair<int,int>>(sp.m));
        alpha/=sp.n*sp.l;
        for(int j=0;j<sp.m;j++){
            for(int jlen=0;jlen<sp.l;jlen++){
                for(int i=0;i<sp.n;i++){
                    for(int ilen=0;ilen<sp.l;ilen++){
                        if(sp(i,ilen,j,jlen)!=(void*)0){
                            if(target_probs[j][jlen]<
                               sp(i,ilen,j,jlen)->prob*alpha){
                                target_probs[j][jlen]=
                                sp(i,ilen,j,jlen)->prob*alpha;
                                target_best[j][jlen]=make_pair(i,ilen);
                            }
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
                cerr<<"("<<j<<","<<jlen<<")=>("
                <<target_best[j][jlen].first
                <<" "<<target_best[j][jlen].second<<") ";
            }
            cerr<<endl;
        }

        //viterbi
        vector<pair<double,int> > viterbi(sp.m,pair<double,int>(0.0,0));
        for(int i=0;i<sp.l&&i<sp.m;i++){
            viterbi[i]=pair<double,int>(target_probs[0][i],-1);
        }
        for(int i=1;i<(int)viterbi.size();i++){
            for(int j=1;j<=sp.l&&i-j>=0;j++){
                if(viterbi[i-j].first*target_probs[i-j+1][j-1]>viterbi[i].first)
                    viterbi[i]=
                    make_pair(viterbi[i-j].first*target_probs[i-j+1][j-1],i-j);
            }
        }
        int pos=sp.m-1;
        string sequence="";
        string source="";
        while(pos>=0){
            sequence=to_string(viterbi[pos].second+1)+","+to_string(pos)
            +" "+sequence;
            cout<<viterbi[pos].second+1<<","<<pos-viterbi[pos].second<<endl;
            pair<int,int> srcpair=
            target_best[viterbi[pos].second+1][pos-viterbi[pos].second];
            source=to_string(srcpair.first)+","+
            to_string(srcpair.second)
            +" "+source;
            pos=viterbi[pos].second;
        }
        cout<<"best seg:"<<sequence<<endl;
        cout<<"     src:"<<source<<endl;
    }
}

