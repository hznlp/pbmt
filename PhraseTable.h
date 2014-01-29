#ifndef PHRASEEXTRACTOR
#define PHRASEEXTRACTOR
#include "Alignment.h"
#include "LexProbDic.h"
#include "utils.h"

class PhraseRuleEntry {
public:
	string srcPhrase;
	string tarPhrase;
	vector<string> tarRepresent;
	double s2tLexScore;
	double t2sLexScore;
	double s2tScore;
	double t2sScore;
	void logrize(double base)
	{s2tScore=log(s2tScore);t2sScore=log(t2sScore);s2tLexScore=log(s2tLexScore);t2sLexScore=log(t2sLexScore);}
	void read(string str);
};

class Features;
class PhraseTable : public Dic<PhraseRuleEntry>
{
public:
	bool load(string filename);
	void logrize(double base);
	void print(ostream& os, bool onlyonefeat=false)const;
	void printTopEntrys(int topN, string filename);
	void makeVector();
	void sortVector(Features& weight);
	vector<PhraseRuleEntry*>* queryRulesVec(string& srcPhrase);
	map<string,vector<PhraseRuleEntry*> >& rulesInVec(){return _rulesInVec;}
	void addOneLine(string curline);
	
private:
	map<string,vector<PhraseRuleEntry*> > _rulesInVec;
};
double calculateRuleScore(PhraseRuleEntry& rule, Features& weight);

void phraseExtractor(string& src, 
	string& tar, 
	string& align, 
	Dic<double>& s2tLexDic, 
	Dic<double>& t2sLexDic, 
	vector<PhraseRuleEntry>& phraseRules,
	int srcLengthLimit=8,
	int tarLengthLimit=12,
	int maxUnAligned=2,
	double weight=1);
void mtuExtractor(string& src, string& tar, string& align,
                  vector<string>& result,int maxSpan);


PhraseRuleEntry entryCombine(PhraseRuleEntry& me1, PhraseRuleEntry& me2);
PhraseRuleEntry& updateMinEntry(PhraseRuleEntry& me, PhraseRuleEntry& minEntry);
PhraseRuleEntry& updateMaxEntry(PhraseRuleEntry& me, PhraseRuleEntry& maxEntry);

inline ostream& operator << (ostream& os, const PhraseRuleEntry& me)
{
	os<<me.t2sScore<<" "<<me.t2sLexScore<<" "<<me.s2tScore<<" "<<me.s2tLexScore<<" "<<2.718
		;
	return os;
}

string range2phrase(vector<string>& sent, pair<int,int> range, string deliminator);
void range2positions(pair<int,int> range, vector<int>& positions);
inline bool operator < (const PhraseRuleEntry& e1, const PhraseRuleEntry& e2){return e1.s2tScore<e2.s2tScore;}
#endif

