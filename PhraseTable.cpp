#include "PhraseTable.h"
#include "Hypothesis.h"
/*
bool induceLexScore(const vector<string>& srcSentence,
					const vector<string>& tarSentence,
					const Alignment& alignment,
					const Dic<double>& s2tLexDic,
					const Dic<double>& t2sLexDic,
					const vector<int>& srcWrdPositions,
					const vector<int>& tarWrdPositions,
					double& lexScore,
					double& reversedLexScore
					)
{
	lexScore=reversedLexScore=1;
	for(unsigned int i=0;i<srcWrdPositions.size();i++)
	{
		int sp=srcWrdPositions[i]; //the position in parse is start from 1, while in alignment it start from 0
		const string& srcWrd=srcSentence[sp];
		vector<int> tarPositions;
		alignment.alignment(sp,tarPositions,S2T);
		if(tarPositions.size()==0)continue;
		for(unsigned int tarCount=0;tarCount<tarPositions.size();tarCount++)
		{
			string tarWrd=tarSentence[tarPositions[tarCount]];
			double lexProb,reversedProb;
			
			if(!s2tLexDic.query(srcWrd,tarWrd,lexProb))
			{
				cerr<<"warning: no dic entry for "<<srcWrd<<" => "<<tarWrd<<endl;
				//lexProb=1E-5;
				;
			}
			if(!t2sLexDic.query(tarWrd,srcWrd,reversedProb))
			{
				cerr<<"warning: no dic entry for "<<tarWrd<<" => "<<srcWrd<<endl;
				//reversedProb=1E-5;
				;
			}
			lexScore*=lexProb;
			reversedLexScore*=reversedProb;
		}
	}
	for(unsigned int i=0;i<srcWrdPositions.size();i++)
	{
		int sp=srcWrdPositions[i]; //the position in parse is start from 1, while in alignment it start from 0
		const string& srcWrd=srcSentence[sp];
		vector<int> tarPositions;
		alignment.alignment(sp, tarPositions,S2T);
		double lexProb=1;
		if(tarPositions.size()==0)
		{
			if(!s2tLexDic.query(srcWrd,"NULL",lexProb))
			{
				cerr<<"warning: no dic entry for "<<srcWrd<<" => "<<"NULL"<<endl;
				lexScore*=1E-7;
			}
			else lexScore*=lexProb;		
		}
	}

	for(size_t i=0;i<tarWrdPositions.size();i++)
	{
		int sp=tarWrdPositions[i];
		const string& tarWrd=tarSentence[sp];
		vector<int> srcPositions;
		alignment.alignment(sp,srcPositions,T2S);
		double lexProb=1;
		if(srcPositions.size()==0)
		{
			if(!t2sLexDic.query(tarWrd,"NULL",lexProb))
			{
				cerr<<"warning: no dic entry for "<<tarWrd<<" => "<<"NULL"<<endl;
				reversedLexScore*=1E-7;
			}
			else reversedLexScore*=lexProb;
		}
	}
	return true;
}

bool induceLexScore(const vector<string>& srcSentence,
					const vector<string>& tarSentence,
					const Alignment& alignment,
					const Dic<double>& s2tLexDic,
					const Dic<double>& t2sLexDic,
					const vector<int>& srcWrdPositions,
					const vector<int>& tarWrdPositions,
					double& lexScore,
					double& reversedLexScore
					)
{
	lexScore=reversedLexScore=1;
	for(unsigned int i=0;i<srcWrdPositions.size();i++)
	{
		int sp=srcWrdPositions[i]; //the position in parse is start from 1, while in alignment it start from 0
		const string& srcWrd=srcSentence[sp];
		vector<int> tarPositions;
		alignment.alignment(sp,tarPositions,S2T);
		if(tarPositions.size()==0)
		{
			double lexProb=0;
			if(!t2sLexDic.query("NULL",srcWrd,lexProb))
			{
				cerr<<"warning: no dic entry for "<<srcWrd<<" => "<<"NULL"<<endl;
				lexProb=1E-7;
			}
			lexScore*=lexProb;
			continue;
		}
		double avgProb=0;
		for(unsigned int tarCount=0;tarCount<tarPositions.size();tarCount++)
		{
			string tarWrd=tarSentence[tarPositions[tarCount]];
			double lexProb=0;
			
			if(!t2sLexDic.query(tarWrd,srcWrd,lexProb))
			{
				cerr<<"warning: no dic entry for "<<tarWrd<<" => "<<srcWrd<<endl;
				//reversedProb=1E-5;
				;
			}
			avgProb+=lexProb;
		}
		lexScore*=avgProb/tarPositions.size();
	}
	
	for(size_t i=0;i<tarWrdPositions.size();i++)
	{
		int sp=tarWrdPositions[i];
		const string& tarWrd=tarSentence[sp];
		vector<int> srcPositions;
		alignment.alignment(sp,srcPositions,T2S);
		double lexProb=1;
		if(srcPositions.size()==0)
		{
			if(!s2tLexDic.query("NULL",tarWrd,lexProb))
			{
				cerr<<"warning: no dic entry for "<<tarWrd<<" => "<<"NULL"<<endl;
				lexProb=1E-7;
			}
			reversedLexScore*=lexProb;
			continue;
		}
		double avgProb=0;
		for(unsigned int srcCount=0;srcCount<srcPositions.size();srcCount++)
		{
			string srcWrd=srcSentence[srcPositions[srcCount]];
			double lexProb=0;
			
			if(!s2tLexDic.query(srcWrd,tarWrd,lexProb))
			{
				cerr<<"warning: no dic entry for "<<tarWrd<<" => "<<srcWrd<<endl;
				//reversedProb=1E-5;
				;
			}
			avgProb+=lexProb;
		}
		reversedLexScore*=avgProb/srcPositions.size();
	}
	return true;
}

*/
bool induceLexScore(const vector<string>& srcSentence,
					const vector<string>& tarSentence,
					const Alignment& alignment,
					const vector<int>& srcWrdPositions,
					const Dic<double>& t2sLexDic,
					double& lexScore,
					LangDirection LD
					)
{
	lexScore=1;
	for(unsigned int i=0;i<srcWrdPositions.size();i++)
	{
		int sp=srcWrdPositions[i]; //the position in parse is start from 1, while in alignment it start from 0
		const string& srcWrd=srcSentence[sp];
		vector<int> tarPositions;
		alignment.alignment(sp,tarPositions,LD);
		if(tarPositions.size()==0)
		{
			double lexProb=0;
			if(!t2sLexDic.query("NULL",srcWrd,lexProb))
			{
				cerr<<"warning: no dic entry for "<<srcWrd<<" => "<<"NULL"<<endl;
				lexProb=1E-7;
			}
			lexScore*=lexProb;
			continue;
		}
		double avgProb=0;
		for(unsigned int tarCount=0;tarCount<tarPositions.size();tarCount++)
		{
			string tarWrd=tarSentence[tarPositions[tarCount]];
			double lexProb=0;
			
			if(!t2sLexDic.query(tarWrd,srcWrd,lexProb))
			{
				cerr<<"warning: no dic entry for "<<tarWrd<<" => "<<srcWrd<<endl;
				//reversedProb=1E-5;
				;
			}
			avgProb+=lexProb;
		}
		lexScore*=avgProb/tarPositions.size();
	}
	return true;
}

string range2phrase(vector<string>& sent, pair<int,int> range, string deliminator=" ")
{
	string result="";
	for(int i=range.first;i<=range.second;i++)
	{
		result+=sent[i];
		if(i!=range.second)result+=deliminator;
	}
	return result;
}

void phraseExtractor(string& src, string& tar, string& align, Dic<double>& s2tLexDic, Dic<double>& t2sLexDic, vector<PhraseRuleEntry>& phraseRules,int srcLengthLimit,	int tarLengthLimit, int maxUnAligned, double weight)
{
	if(weight<=0)return;
	phraseRules.clear();
	vector<string> srcSent,tarSent;
	stringToVector(src,srcSent);
	stringToVector(tar,tarSent);
	Alignment alignment(align,srcSent.size(),tarSent.size(), maxUnAligned);
	
	map<pair<int,int>, pair<int,int> >::const_iterator iter;
	for(iter=alignment.s2tBoundaryMap().begin();iter!=alignment.s2tBoundaryMap().end();iter++)
	{
		const pair<int,int>& sb=iter->first;
		vector<pair<int,int> > tarPhrases;
		if(sb.second-sb.first>=srcLengthLimit) continue;
		alignment.alignment(sb, tarPhrases,S2T);
		string srcPhrase=range2phrase(srcSent,sb);
		for(unsigned int i=0;i<tarPhrases.size();i++)
		{
			if(tarPhrases[i].second-tarPhrases[i].first>=tarLengthLimit) continue;

			string tarPhrase=range2phrase(tarSent,tarPhrases[i]);
			PhraseRuleEntry tmp_pre;
			phraseRules.push_back(tmp_pre);
			PhraseRuleEntry& pre=phraseRules.back();
			pre.srcPhrase=srcPhrase;
			pre.tarPhrase=tarPhrase;
			pre.s2tScore=weight;
			pre.t2sScore=weight;
			vector<int> srcPositions,tarPositions;
			range2positions(sb,srcPositions);
			range2positions(tarPhrases[i],tarPositions);
			induceLexScore(srcSent,tarSent,alignment,srcPositions,t2sLexDic,pre.t2sLexScore,S2T);
			induceLexScore(tarSent,srcSent,alignment,tarPositions,s2tLexDic,pre.s2tLexScore,T2S);
			cout<<srcPhrase<<" ||| "<<tarPhrase<<" ||| "<<pre<<endl;
		}
	}
}

void mtuExtractor(string& src, string& tar, string& align, vector<string>& result, int maxSpan)
{
	vector<string> srcSent,tarSent;
	stringToVector(src,srcSent);
	stringToVector(tar,tarSent);
    if(srcSent.empty())return;
    int maxUnAligned=2;
	Alignment alignment(align,(int)srcSent.size(),(int)tarSent.size(), maxUnAligned);
    string newalign=alignment.modifiedAlignment(maxSpan);

    //Alignment alignment(newalign,(int)srcSent.size(),(int)tarSent.size(), maxUnAligned);

    for(int start=0;start<(int)srcSent.size();start++){
        for(int stop=start;stop<(int)srcSent.size();stop++){
            pair<int,int> srcSpan=make_pair(start,stop);
            pair<int,int> tarSpan;
            if(alignment.alignment(srcSpan, tarSpan, S2T)==true){
                string srcPhrase=range2phrase(srcSent,srcSpan,"_");
                string tarPhrase=range2phrase(tarSent,tarSpan,"_");
                result.push_back(srcPhrase+"=>"+tarPhrase);
                //cout<<srcPhrase+"=>"+tarPhrase<<endl;
                start=stop;
                break;
            }
        }
    }
}


void range2positions(pair<int,int> range, vector<int>& positions)
{
	positions.clear();
	for(int i=range.first;i<=range.second;i++)
	{
		positions.push_back(i);
	}
}


PhraseRuleEntry entryCombine(PhraseRuleEntry& me1, PhraseRuleEntry& me2)
{
	PhraseRuleEntry me;
	me=me1;
	me.s2tLexScore=max(me1.s2tLexScore,me2.s2tLexScore);
	me.t2sLexScore=max(me1.t2sLexScore,me2.t2sLexScore);
	me.t2sScore=me1.t2sScore+me2.t2sScore;
	me.s2tScore=me1.s2tScore+me2.s2tScore;
	return me;
}

PhraseRuleEntry& updateMinEntry(PhraseRuleEntry& me, PhraseRuleEntry& minE)
{
	minE.s2tLexScore=min(me.s2tLexScore,minE.s2tLexScore);
	minE.t2sLexScore=min(me.t2sLexScore,minE.t2sLexScore);
	minE.t2sScore=min(me.t2sScore,minE.t2sScore);
	minE.s2tScore=min(me.s2tScore,minE.s2tScore);
	return minE;
}

PhraseRuleEntry& updateMaxEntry(PhraseRuleEntry& me, PhraseRuleEntry& maxE)
{
	maxE.s2tLexScore=max(me.s2tLexScore,maxE.s2tLexScore);
	maxE.t2sLexScore=max(me.t2sLexScore,maxE.t2sLexScore);
	maxE.t2sScore=max(me.t2sScore,maxE.t2sScore);
	maxE.s2tScore=max(me.s2tScore,maxE.s2tScore);
	return maxE;
}

//be aware of vector2string
template<>
int
Dic<PhraseRuleEntry>::
normalize()
{
	map<string,double> targetCount;
	map<string,double> sourceCount;
	
	map<string, map<string, PhraseRuleEntry> >::iterator iter1;
	map<string, PhraseRuleEntry>::iterator iter2;
	for(iter1=_dic.begin();iter1!=_dic.end();iter1++)
	{
		const string& source=iter1->first;
		map<string, PhraseRuleEntry>& entryMap=iter1->second;
		for(iter2=entryMap.begin();iter2!=entryMap.end();iter2++)
		{
			PhraseRuleEntry& e=iter2->second;
			if(sourceCount.find(source)==sourceCount.end()) sourceCount[source]=0;
			sourceCount[source]+=e.s2tScore;

			//calculate target count
			string target="";
			//vector2string(e.tarRepresent,target);
			target=iter2->first;
			target=target.substr(0,target.find("!!!"));
			//cerr<<"target "<<target<<" "<<e.t2sScore<<endl;
			if(targetCount.find(target)==targetCount.end())	
				targetCount[target]=0;
			targetCount[target]+=e.t2sScore;
		}
	}
	//normalize t2sScore and structProb
	for(iter1=_dic.begin();iter1!=_dic.end();iter1++)
	{
		const string& source=iter1->first;
		map<string, PhraseRuleEntry>& entryMap=iter1->second;
		for(iter2=entryMap.begin();iter2!=entryMap.end();)
		{
			PhraseRuleEntry& e=iter2->second;
			string target="";
			//vector2string(e.tarRepresent,target);
			target=iter2->first;
			target=target.substr(0,target.find("!!!"));
			e.t2sScore/=targetCount[target];
			e.s2tScore/=sourceCount[source];
			
			if(e.s2tScore<=1E-10)e.s2tScore=1E-10;
			if(e.t2sScore<=1E-10)e.t2sScore=1E-10;

			if(e.s2tScore<=0||e.t2sScore<=0||isBadNumber(e.t2sScore)||isBadNumber(e.s2tScore))
			{
				cerr<<"src: "<<source<<" , tar: "<<target<<" score error: "<<e.s2tScore<<" , "<<e.t2sScore<<endl;
				map<string, PhraseRuleEntry>::iterator iter_temp=iter2;
				iter_temp++;
				entryMap.erase(iter2);
				iter2=iter_temp;
			}
			else iter2++;
		}
	}
	return 0;
}

void 
PhraseTable::
print(ostream& os, bool onlyonefeat)const
{
	map<string, map<string,PhraseRuleEntry> >::const_iterator iter;
	for(iter=_dic.begin();iter!=_dic.end();iter++)
	{
		const string& s=iter->first;
		const map<string, PhraseRuleEntry>& entryMap=iter->second;
		map<string,PhraseRuleEntry>::const_iterator iter2;
		for(iter2=entryMap.begin();iter2!=entryMap.end();iter2++)
		{
			const string& t=iter2->first;
			const PhraseRuleEntry& entry=iter2->second;
			if(onlyonefeat)
				os<<s<<" ||| "<<t<<" ||| "<<entry.t2sScore<<endl;
			else
				os<<s<<" ||| "<<t<<" ||| "<<entry<<endl;
		}
	}
}


void 
PhraseTable::
logrize(double base)
{
	map<string, map<string,PhraseRuleEntry> >::iterator iter;
	for(iter=_dic.begin();iter!=_dic.end();iter++)
	{
		//const string& s=iter->first;
		map<string, PhraseRuleEntry>& entryMap=iter->second;
		map<string,PhraseRuleEntry>::iterator iter2;
		for(iter2=entryMap.begin();iter2!=entryMap.end();iter2++)
		{
			//const string& t=iter2->first;
			PhraseRuleEntry& entry=iter2->second;
			entry.logrize(base);
		}
	}
}


bool 
PhraseTable::
load(string filename)
{
	ifstream is(filename.c_str());
	if(!is.good())return false;
	while(!is.eof())
	{
		string curline="";
		getline(is,curline);
		if(curline.find("|||")==string::npos)continue;
		addOneLine(curline);
	}
	return true;
}

void 
PhraseRuleEntry::read(string curline)
{
	string src="",tar="";
	trim(curline,curline);
	src=curline.substr(0,curline.find("|||"));
	trim(src,src);
	curline.erase(0, curline.find("|||")+3);
	tar=curline.substr(0,curline.find("|||"));
	trim(tar,tar);
	curline.erase(0, curline.find("|||")+3);
	//cerr<<curline<<endl;
	stringstream ss(curline);
	ss>>t2sScore>>s2tScore>>t2sLexScore>>s2tLexScore;
	srcPhrase=src;
	tarPhrase=tar;
	//stringToVector(tar,tarRepresent);
}

void 
PhraseTable::
addOneLine(string curline)
{
	string src="",tar="";
	PhraseRuleEntry entry;
	trim(curline,curline);
	src=curline.substr(0,curline.find("|||"));
	trim(src,src);
	curline.erase(0, curline.find("|||")+3);
	tar=curline.substr(0,curline.find("|||"));
	trim(tar,tar);
	curline.erase(0, curline.find("|||")+3);
	//cerr<<curline<<endl;
	stringstream ss(curline);
	ss>>entry.t2sScore>>entry.s2tScore>>entry.t2sLexScore>>entry.s2tLexScore;
	entry.srcPhrase=src;
	entry.tarPhrase=tar;
	stringToVector(tar,entry.tarRepresent);
	add(src,tar,entry);
}

void 
PhraseTable::
printTopEntrys(int topN, string filename)
{
	ofstream os(filename.c_str());
	vector<double> probs;
	map<string, map<string,PhraseRuleEntry> >::const_iterator iter;
	for(iter=_dic.begin();iter!=_dic.end();iter++)
	{
		//const string& s=iter->first;
		const map<string, PhraseRuleEntry>& entryMap=iter->second;
		map<string,PhraseRuleEntry>::const_iterator iter2;
		for(iter2=entryMap.begin();iter2!=entryMap.end();iter2++)
		{
			//const string& t=iter2->first;
			const PhraseRuleEntry& entry=iter2->second;
			probs.push_back(entry.s2tScore);
		}
	}
	sort(probs.rbegin(),probs.rend());
	double threshold=0;
	if((int)probs.size()>topN) threshold=probs[topN];
	
	vector<PhraseRuleEntry> entrys;
	
	for(iter=_dic.begin();iter!=_dic.end();iter++)
	{
		//const string& s=iter->first;
		const map<string, PhraseRuleEntry>& entryMap=iter->second;
		map<string,PhraseRuleEntry>::const_iterator iter2;
		for(iter2=entryMap.begin();iter2!=entryMap.end();iter2++)
		{
			//const string& t=iter2->first;
			const PhraseRuleEntry& entry=iter2->second;
			if(entry.s2tScore>=threshold)
				entrys.push_back(entry);
		}
	}
	sort(entrys.rbegin(),entrys.rend());
	if((int)entrys.size()>topN)
		entrys.erase(entrys.begin()+topN,entrys.end());
	for(int i=0;i<topN&&i<(int)entrys.size();i++)
	{
		PhraseRuleEntry& entry=entrys[i];
		os<<entry.srcPhrase<<" ||| "<<entry.tarPhrase<<" ||| "<<entry<<endl;
	}
	os.close();
}

template<>
int
Dic<double>::
normalize()
{
	map<string, double> srcCount;
	map<string, map<string,double> >::iterator iter;
	for(iter=_dic.begin();iter!=_dic.end();iter++)
	{
		const string& s=iter->first;
		const map<string, double>& entryMap=iter->second;
		map<string,double>::const_iterator iter2;
		for(iter2=entryMap.begin();iter2!=entryMap.end();iter2++)
		{
			//const string& t=iter2->first;
			double entry=iter2->second;
			srcCount[s]+=entry;
		}
	}
	for(iter=_dic.begin();iter!=_dic.end();iter++)
	{
		const string& s=iter->first;
		map<string, double>& entryMap=iter->second;
		map<string, double>::iterator iter2;
		for(iter2=entryMap.begin();iter2!=entryMap.end();iter2++)
		{
			//const string& t=iter2->first;
			double& entry=iter2->second;
			entry=entry/srcCount[s];
		}
	}
	return 1;
}

void 
PhraseTable::
makeVector()
{
	_rulesInVec.clear();
	for(map<string, map<string, PhraseRuleEntry> >::iterator iter1=_dic.begin();iter1!=_dic.end();iter1++)
	{
		map<string, PhraseRuleEntry>& smallDic=iter1->second;
		vector<PhraseRuleEntry*>& smallVec=_rulesInVec[iter1->first];
		for(map<string, PhraseRuleEntry>::iterator iter2=smallDic.begin();iter2!=smallDic.end();iter2++)
		{
			PhraseRuleEntry& entry=iter2->second;
			smallVec.push_back(&entry);
		}
	}
}
double calculateRuleScore(PhraseRuleEntry& rule, Features& weight)
{
	double score=rule.s2tScore*weight.s2t+rule.t2sScore*weight.t2s+rule.s2tLexScore*weight.s2tLex+
		rule.t2sLexScore*weight.t2sLex+rule.tarRepresent.size()*weight.length;
	return score;
}
void 
PhraseTable::
sortVector(Features& weight)
{
	for(map<string, vector<PhraseRuleEntry*> >::iterator iter1=_rulesInVec.begin();iter1!=_rulesInVec.end();iter1++)
	{
		vector<PhraseRuleEntry*>& ruleVec=iter1->second;
		vector<pair<double,PhraseRuleEntry*> > tmpVec;
		for(size_t i=0;i<ruleVec.size();i++)
		{
			PhraseRuleEntry* p_rule=ruleVec[i];
			double negScore=-calculateRuleScore(*p_rule,weight);
			tmpVec.push_back(make_pair(negScore,p_rule));
		}
		sort(tmpVec.begin(),tmpVec.end());
		ruleVec.clear();
		for(size_t i=0;i<tmpVec.size();i++)
		{
			ruleVec.push_back(tmpVec[i].second);
		}
	}
}

vector<PhraseRuleEntry*>*
PhraseTable::
queryRulesVec(string& srcPhrase)
{
	if(_rulesInVec.find(srcPhrase)==_rulesInVec.end())
		return NULL;
	else
		return &_rulesInVec[srcPhrase];
}
