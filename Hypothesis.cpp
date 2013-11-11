#include "Hypothesis.h"
bool isCombinable(const BITVECTOR& covered, const pair<int,int>& span)
{
	if(span.second>=(int)covered.size())return false;
	for(int i=span.first;i<span.second;i++)
	{
		if(covered[i]==true)return false;
	}
	return true;
}

void combine(const BITVECTOR& covered, const pair<int,int>& span, BITVECTOR& result)
{
	result.clear();
	result=covered;
	for(int i=span.first;i<=span.second;i++)
	{
		result[i]=true;
	}
}

string 
BITVECTOR::
represent()
{
	string result="";
	for(size_t i=0;i<size();i++)
	{
		if((*this)[i]==true)
			result+="1";
		else
			result+="0";
		if(i!=size()-1)
			result+=" ";
	}
	return result;
}

void 
Hypothesis::
genFromChild(Hypothesis& child, const pair<int,int>& newSpan, PhraseRuleEntry& rule, Features& weight, double futureScore)
{
	(*this)=child;
	combine(child.coveredWords,newSpan,coveredWords);
	lastCoveredWord=newSpan.second;
	represent=coveredWords.represent()+" "+intToString(lastCoveredWord);
	
	features=child.features;
	features.s2t+=rule.s2tScore;
	features.t2s+=rule.t2sScore;
	features.s2tLex+=rule.s2tLexScore;
	features.t2sLex+=rule.t2sLexScore;
	features.length+=rule.tarRepresent.size();
	features.distort+=abs(newSpan.first-child.lastCoveredWord)-1;

	currentScore=features.calulateScore(weight);
	translation.insert(translation.end(),rule.tarRepresent.begin(),rule.tarRepresent.end());

	trace.p_prev=&child;
	trace.p_rule=&rule;

	estimatedScore=currentScore+futureScore;
}

void 
Hypothesis::
print(ostream& os)
{
	os<<represent<<" : "<<vectorToString(translation)<<" ||| "<<currentScore<<" "<<estimatedScore<<endl;
	os<<"\t";features.print(os);os<<endl;
	os<<"\t";
	if(trace.p_prev!=NULL)
		os<<trace.p_prev->represent;
	os<<" ||| ";
	if(trace.p_rule!=NULL)
		os<<(*trace.p_rule).srcPhrase<<" => "<<(*trace.p_rule).tarPhrase<<" ||| "<<(*trace.p_rule);
	os<<endl;
}

void 
Features::
init(string str)
{
	replaceSubStr(str,":"," ");
	stringstream ss(str);
	ss>>t2s>>s2t>>t2sLex>>s2tLex>>distort>>length;
	length*=-1;
	distort*=-1;
}
