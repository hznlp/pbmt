//
//  mp.h
//  test
//
//  Created by Hui on 13-10-27.
//  Copyright (c) 2013年 Hui. All rights reserved.
//

#ifndef pbmt_mp1_h
#define pbmt_mp1_h
#include "mp-init.h"

class MP1{
public:
    MP1(){ppt_=new SimplePhraseTable();}
    void init(string src, string tgt, CorpusCache& cache);
    virtual void expectation(CorpusCache& cache);
    virtual void em(CorpusCache& cache, int round,
                    const string& out, bool knsmoothing);
    virtual void viterbi(CorpusCache& cache);
    SimplePhraseTable& pt(){return *ppt_;}
    SimplePhraseTable *ppt_;
    double alpha_;
};

#endif
