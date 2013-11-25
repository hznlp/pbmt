#include "mp2.h"
void
MP2::
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
