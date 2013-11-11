#include "Decode.h"
//=====================HypothesisHeap=======================
bool 
HypothesisHeap::
addHypothesis(Hypothesis& hypo)
{
	if(_hypoToIndex.find(hypo.represent)==_hypoToIndex.end())
	{
		_hypoToIndex[hypo.represent]=(int)size();
		push_back(hypo);
	}
	else
	{
		int ind=_hypoToIndex[hypo.represent];
		if((*this)[ind].estimatedScore<hypo.estimatedScore)
			(*this)[ind]=hypo;
	}
	return true;
}

bool 
HypothesisHeap::
sortAndPrune(int beamsize)
{
	sort(begin(),end(),compareHypos);
	if((int)size()>beamsize)
		erase(begin()+beamsize,end());
	return true;
}

//=================AuxSearchSpace==========================

void 
AuxSearchSpace::
init(string& sent)
{
	clear();
	stringToVector(sent,_sentence);
	for(size_t i=0;i<_sentence.size();i++)
	{
		vector<pair<string,double> > tmp_vecs;
		push_back(tmp_vecs);
		for(size_t j=0;j<_sentence.size();j++)
		{
			pair<string,double> tmp_bead;
			back().push_back(tmp_bead);
		}
	}
	for(size_t i=0;i<_sentence.size();i++)
	{
		string phrase=_sentence[i];
		for(size_t j=i;j<_sentence.size();j++)
		{
			if(j>i) phrase+=" "+_sentence[j];
			(*this)[i][j].first=phrase;
			(*this)[i][j].second=-100;
		}
	}
}

void 
AuxSearchSpace::
cky(PhraseTable& pt, Features& weight)
{
	int sentLength=_sentence.size();
	for(int length=0; length<sentLength;length++)
	{
		for(int start=0;start<sentLength-length;start++)
		{
			int stop=start+length;
			pair<string,double>& bead=(*this)[start][stop];
			for(int pivot=start;pivot<stop;pivot++)
			{
				double score=(*this)[start][pivot].second+(*this)[pivot+1][stop].second;
				bead.second=max(score,bead.second);
			}
			
			string& srcPhrase=bead.first;
			vector<PhraseRuleEntry*>* p_ruleVec=pt.queryRulesVec(srcPhrase);
			if(p_ruleVec!=NULL)
			{
				vector<PhraseRuleEntry*>& ruleVec=*p_ruleVec;
				for(size_t ruleIter=0;ruleIter<ruleVec.size();ruleIter++)
				{
					PhraseRuleEntry& rule=*ruleVec[ruleIter];
					double score=calculateRuleScore(rule, weight);
					bead.second=max(score,bead.second);
				}
			}
		}
	}
}

double 
AuxSearchSpace::
queryScore(pair<int,int> span)
{
	return (*this)[span.first][span.second].second;
}

double 
AuxSearchSpace::
queryFutureCost(BITVECTOR bvec)
{
	double result=0;
	int start=0,stop=0;
	for(start=0;start<(int)bvec.size();start++)
	{
		if(bvec[start]==true)continue;
		for(stop=start;stop<(int)bvec.size();stop++)
		{
			if(bvec[stop]==true)break;
		}
		stop=stop-1;
		if(stop<start)break;
		result+=queryScore(make_pair(start,stop));
		start=stop+1;
	}
	return result;
}

void 
AuxSearchSpace::
print(ostream& os){
	for(size_t i=0;i<size();i++){
		for(size_t j=i;j<size();j++){
			cout<<"["<<i<<","<<j<<"] : "
                            <<(*this)[i][j].first<<" ||| "<<(*this)[i][j].second<<endl;
		}
	}
}

//==================SearchSpace=========================
void SearchSpace::
init(string& sent){
	stringToVector(sent,_sentence);
	for(size_t i=0;i<_sentence.size()+1;i++){
		HypothesisHeap hheap;
		_hypoHeaps.push_back(hheap);
	}
	_auxSpace.init(sent);
}

void 
SearchSpace::
copeUNK(PhraseTable& pt)
{
	for(size_t i=0;i<_sentence.size();i++)
	{
		string wrd=_sentence[i];
		if(pt.queryRulesVec(wrd)==NULL)
		{
			PhraseRuleEntry e;
			e.s2tLexScore=e.t2sLexScore=e.t2sScore=e.s2tScore=-100;
			e.srcPhrase=e.tarPhrase=wrd;
			e.tarRepresent.push_back(wrd);
			pt.add(wrd,wrd,e);
			pt.rulesInVec()[wrd].push_back(&pt.data()[wrd][wrd]);
		}
	}
}

void 
SearchSpace::
beamSearch(PhraseTable& pt, Features& weight, int beamsize, int distLimit, int tlimit, bool debug)
{
	Hypothesis tmpHypo;
	_hypoHeaps[0].push_back(tmpHypo);
	Hypothesis& initHypo=_hypoHeaps[0][0];
	initHypo.coveredWords.init(_sentence.size(),false);
	initHypo.currentScore=0;
	initHypo.estimatedScore=0;
	initHypo.features.init();
	initHypo.lastCoveredWord=-1;
	initHypo.represent="";
	initHypo.translation.clear();
	initHypo.trace.p_prev=NULL;
	initHypo.trace.p_rule=NULL;

	for(size_t heapIter=0;heapIter<_sentence.size();heapIter++)
	{
		HypothesisHeap& hHeap=_hypoHeaps[heapIter];
		hHeap.sortAndPrune(beamsize);
		for(int hypoIter=0;hypoIter<(int)hHeap.size();hypoIter++)
		{	
			Hypothesis& curHypo=hHeap[hypoIter];
			if(debug)
			{	
				cout<<"CurHypo ::"<<endl;
				curHypo.print(cout);
			}
			BITVECTOR bvec=curHypo.coveredWords;
			int firstUnCovered=bvec.firstFalse();
			for(int start=0;start<(int)bvec.size();start++)
			{
				if(bvec[start]==true)
					continue;
				for(int stop=start;
                                        stop<(int)bvec.size()&&
                                        bvec[stop]==false&&
                                        (stop<distLimit+firstUnCovered||start==firstUnCovered);
                                        stop++)
				{
					pair<int,int> phraseSpan=make_pair(start,stop);
					string candiPhrase=_auxSpace.queryPhrase(phraseSpan);
					vector<PhraseRuleEntry*>* p_rules=pt.queryRulesVec(candiPhrase);
					if(p_rules==NULL)continue;
					int newLength=heapIter+stop-start+1;
					BITVECTOR newBVec;
					combine(bvec,phraseSpan,newBVec);
					//cout<<"newBVec: "<<newBVec.represent()<<endl;
					double futureScore=_auxSpace.queryFutureCost(newBVec);

					vector<PhraseRuleEntry*>& rules=*p_rules;
					for(size_t rIter=0;rIter<rules.size()&&(int)rIter<tlimit;rIter++)
					{
						PhraseRuleEntry& rule=*rules[rIter];
						Hypothesis newHypo;
						newHypo.genFromChild(curHypo,make_pair(start,stop),rule,weight,futureScore);
						if(debug)
						{
							cout<<"newHypo :: "<<endl;
							newHypo.print(cout);
						}
						_hypoHeaps[newLength].addHypothesis(newHypo);
					}
				}
			}
		}
	}
	_hypoHeaps.back().sortAndPrune(beamsize);
	if(debug)
	{
		cout<<"final hHeap"<<endl;
		for(size_t i=0;i<_hypoHeaps.back().size();i++){
			Hypothesis& hypo=_hypoHeaps.back()[i];
			hypo.print(cout);
		}
	}
}

string 
SearchSpace::
getNthTranslation(int nth){
	if(nth<1)return "";
	string result="";
	if((int)_hypoHeaps.back().size()<nth)
		return "";
	return vectorToString(_hypoHeaps.back()[nth-1].translation);
}
