#ifndef pbmt_mp2_h
#define pbmt_mp2_h
#include "mp1.h"
extern Specs specs;
class MP2 : public MP1 {
public:
    MP2(){ptable_.insert(ptable_.begin(),
                     specs.max_sentence_length,
                     vector<double>(specs.max_sentence_length,
                                    1.0/specs.max_sentence_length));
            }
    vector<vector<double>> ptable_; // position prob table
    vector<vector<double>> ltable_; // length prob table

    virtual void expectation(CorpusCache& cache);
    //virtual void em(CorpusCache& cache, int round, const string& out);
    //virtual void viterbi(CorpusCache& cache);
};
#endif
