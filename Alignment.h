#ifndef ALIGNMENT_H
#define ALIGNMENT_H
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include "Direction.h"
using namespace std;

class Alignment
{
public:
	Alignment(){clear();_maxUnAligned=10;}
	Alignment(const string& str){clear();init(str);_maxUnAligned=10;}
	Alignment(const string& str,int slen,int tlen){clear();init(str,slen,tlen);_maxUnAligned=10;}
	Alignment(const string& str, int mu){clear();init(str);_maxUnAligned=mu;}
	Alignment(const string& str,int slen,int tlen, int mu){clear();init(str,slen,tlen);_maxUnAligned=mu;}

	void clear();
	void init(const string& str);
	void init(const string& str, int srcLen, int tarLen);

//const function
	bool isAligned(int wrdIndex, LangSide ls)const;
	bool isAlignable(const pair<int,int>& boundary, LangSide ls)const;
	bool boundaryMapping(const pair<int,int>& boundary, pair<int,int>& result, LangDirection ld)const;
	
	const vector<int>& alignment(int wrdIndex, LangDirection ld)const;
	bool alignment(int wrdIndex, vector<int>& resultWrds, LangDirection ld)const;
	bool alignment(const pair<int,int>& inputPhrase, vector<pair<int,int> >& resultPhrases, LangDirection ld)const;
	bool alignment(const pair<int,int>& inputPhrase, pair<int,int>& resultPhrase, LangDirection ld)const;

	int numOfUnAlignedBoundWords(const pair<int,int>& boundary, LangSide ls)const;

	void print(ostream& os)const;
	void printInICTFormat(ostream& os)const;
	void print_boundaryMap(ostream& os)const;
	void print_alignableBoundarys(ostream& os)const;
	void print_phrases(ostream& os)const;
	map<pair<int,int>, pair<int,int> >& s2tBoundaryMap() {return _s2tBoundaryMap;}
private:
	void setupMap(const string& str);
	void setupBoundaryMapAndVec();
	void setupBoundaryMap();
	void setupVec();
	void setupBoundaryMap(const map<int,set<int> >& s2tMap, int slen, map<pair<int,int>, pair<int,int> >& s2tbm);
	void setupVec(const map<int,set<int> >& s2tMap, int slen, vector<vector<int> >& s2tVec);

//const function
	bool phraseMapping(const pair<int,int>& phrase, vector<pair<int,int> >& resultPhrases, LangDirection ld)const;
	bool isAligned_srcPos(int srcPosition)const;
	bool isAligned_tarPos(int tarPosition)const;
	
	bool srcBoundary2tarBoundary(const pair<int,int>& srcBoundary, pair<int,int>& result)const;
	bool tarBoundary2srcBoundary(const pair<int,int>& tarBoundary, pair<int,int>& result)const;

	bool srcPhrase2tarPhrases(const pair<int,int>& srcPhrase, vector<pair<int,int> >& tarPhrases)const;
	bool tarPhrase2srcPhrases(const pair<int,int>& tarPhrase, vector<pair<int,int> >& srcPhrases)const;

	bool isAlignable_srcBoundary(const pair<int,int>& srcBoundary)const;
	bool isAlignable_tarBoundary(const pair<int,int>& tarBoundary)const;

	const vector<int>& alignedTarsForSrc(int srcPosition)const{return _s2tVec[srcPosition];}
	const vector<int>& alignedSrcsForTar(int tarPosition)const{return _t2sVec[tarPosition];}

	int _srcLen,_tarLen;
	vector<vector<int> > _s2tVec,_t2sVec;
	map<int,set<int> > _s2tMap, _t2sMap;
	map<pair<int,int>, pair<int,int> > _s2tBoundaryMap,_t2sBoundaryMap;
	int _maxUnAligned;
};

void getSentLenFromMap(const map<int,set<int> >& s2tMap, int& slen);
void string2alignment(string& str, vector<pair<int,int> >& align);

#endif
