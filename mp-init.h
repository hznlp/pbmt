#ifndef pbmt_mp_init_h
#define pbmt_mp_init_h

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
#include <cassert>
#include <boost/range/irange.hpp>
#include <boost/tokenizer.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include "JKArgs.h"
using namespace boost;
using namespace std;

#define MAX_PHRASE_LEN 7
#define MAX_CUTOFF 5

struct PhraseInfo {
    double ps2t,pt2s,ls2t,lt2s;
    PhraseInfo(){ps2t=pt2s=ls2t=lt2s=1;};
    PhraseInfo(double ls,double ps, double lt, double pt)
    :ps2t(ps),pt2s(pt),ls2t(ls),lt2s(lt){}
};

class RichPhraseTable: public unordered_map<string,map<string,PhraseInfo>>{
public:
    void read(string filename);
    void print(string filename);
};

struct SimplePhraseInfo {
    double prob,count;
    SimplePhraseInfo()
        :prob(0.0),count(0.0){}
    SimplePhraseInfo(double p,double c)
        :prob(p),count(c){}
};

class LexDic : public unordered_map<string,map<string,double>>{
public:
    bool read(const string& in);
};

double ScoreLex(vector<string>& src, vector<string>& tgt, LexDic& lex_s2t);

class SimplePhraseTable : public
    map<string,map<string,SimplePhraseInfo>>{
    public:
        void normalize();
        void normalize_using_prob();
        void reset_count(double value);
        void ibm1_scoring(LexDic& ibm1);
        void print(ostream& os);
        void read(string filename, bool direction);
};

enum Scoring { Frac,Count,CountLex,OnlyLex,OnlyFrac,OnlyCount};

void usage();
void em(JKArgs& args);
void combine(JKArgs& args);

/* Get the A[i][j][ilen][jlen] for arrary A[n][m][l][l] */
int index(int i, int ilen, int j, int jlen, int n, int m, int l);
typedef SimplePhraseInfo* PSimplePhraseInfo;
struct SentenceCache{
    PSimplePhraseInfo* p;
    int n,m,l;
    SentenceCache(int n,int m, int l);
    PSimplePhraseInfo& operator()(int i, int ilen, int j, int jlen);
};

typedef vector<SentenceCache> CorpusCache;

void expectation(CorpusCache& cache, SimplePhraseTable& pt);

void PhraseCutoff(const string& filename,
                  const string& log_prefix,
                  vector<vector<string>>* p_src_corpus,
                  vector<vector<int>>* p_max_lens,
                  int max_phrase_length,
                  int cutoff);

bool ExtractPhrasePairs(const string& src,
                        const string& tgt,
                        const string& out,
                        int max_sentence_length,
                        int max_phrase_length,
                        double max_length_ratio,
                        int min_phrase_count,
                        bool inmemory);

void Readlog(JKArgs& args);

bool Score(JKArgs& args);
void filter(JKArgs& args);
void ExtractPhrasePairs(JKArgs& args);

#endif
