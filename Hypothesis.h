#ifndef HYPOTHESIS_H
#define HYPOTHESIS_H
#include "PhraseTable.h"
#include <sstream>
using namespace std;

class Features
{
public:
	double calulateScore(Features& weight)
	{
		return s2t*weight.s2t
				+t2s*weight.t2s
				+s2tLex*weight.s2tLex
				+t2sLex*weight.t2sLex
				+length*weight.length
				+distort*weight.distort;
	}
	void init(){s2t=t2s=s2tLex=t2sLex=length=distort=0;}
	void init(string str);
	double s2t,t2s,s2tLex,t2sLex,distort,length;
	void print(ostream& os){os<<s2t<<","<<t2s<<","<<s2tLex<<","<<t2sLex<<","<<distort<<","<<length;}
};

class Hypothesis;

class Trace
{
public:
	Hypothesis* p_prev;
	PhraseRuleEntry* p_rule;
};

class BITVECTOR : public vector<bool>
{
public:
	void init(int num, bool val){clear();for(int i=0;i<num;i++)push_back(val);}
	int firstFalse(){
		int result=-1; 
		for(size_t i=0;i<size();i++)
			if((*this)[i]==false)
			{result=i;break;}
		return result;
	}
	string represent();
};

class Hypothesis
{
public:
	string represent;
	int lastCoveredWord;
	BITVECTOR coveredWords;
	Features features;
	double estimatedScore;
	double currentScore;
	vector<string> translation;
	Trace trace;
	void genFromChild(Hypothesis& child, const pair<int,int>& newSpan, PhraseRuleEntry& rule, Features& weight, double futureScore);
	void print(ostream& os);
private:
};

inline bool compareHypos(const Hypothesis& hypo1, const Hypothesis& hypo2){return hypo1.estimatedScore > hypo2.estimatedScore; }

void genNewHypo(Hypothesis& current, const pair<int,int>& span, PhraseRuleEntry& rule, Hypothesis& result);

bool isCombinable(const BITVECTOR& covered, const pair<int,int>& span);
void combine(const BITVECTOR& covered, const pair<int,int>& span, BITVECTOR& result);

#endif
