#ifndef DECODE_H
#define DECODE_H
#include "PhraseTable.h"
#include "Hypothesis.h"

class HypothesisHeap : public vector<Hypothesis>
{
public:
	bool addHypothesis(Hypothesis& hypo);
	bool sortAndPrune(int beamsize);

private:
	map<string,int> _hypoToIndex;
};

class AuxSearchSpace: public vector<vector<pair<string,double> > >
{
public:
	void init(string& sent);
	void cky(PhraseTable& pt, Features& weight);
	double queryScore(pair<int,int> span);
	string queryPhrase(pair<int,int>& span){return (*this)[span.first][span.second].first;}
	double queryFutureCost(BITVECTOR bvec);
	void print(ostream& os);
private:
	vector<string> _sentence;
};

class SearchSpace
{
public:
	void init(string& sent);
	void copeUNK(PhraseTable& pt);
	void setupAuxSpace(PhraseTable& pt, Features& weight){_auxSpace.cky(pt,weight);}
	void beamSearch(PhraseTable& pt, Features& weight, int beamsize, int distLimit, int tlimit=10, bool debug=false);
	string getNthTranslation(int nth);
	AuxSearchSpace& auxSpace(){return _auxSpace;}

private:
	vector<string> _sentence;
	AuxSearchSpace _auxSpace;
	vector<HypothesisHeap> _hypoHeaps;
};

#endif
