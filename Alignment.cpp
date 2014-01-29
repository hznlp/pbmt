#include "Alignment.h"
#include <math.h>
//0-1 1-2 2-2 3-2 3-3 3-4 4-5 4-6 4-7 5-8 6-9 7-10 9-11 11-12
#pragma warning(disable:4996)
#define min(x,y) x<y?x:y;
#define max(x,y) x>y?x:y;

void string2alignment(string& str, vector<pair<int,int> >& align)
{
	align.clear();
	stringstream ss(str.c_str());
	while(ss.good())
	{
		string tmp="";
		int a,b;
		ss>>tmp;
		if(tmp.find("-")==string::npos)break;		
		sscanf(tmp.c_str(),"%d-%d",&a,&b);
		align.push_back(make_pair(a,b));
	}
}

void
Alignment::
clear()
{
	_s2tMap.clear();_t2sMap.clear();
	_s2tVec.clear();_t2sVec.clear();
	_s2tBoundaryMap.clear();_t2sBoundaryMap.clear();
	_srcLen=_tarLen=-1;
}

void
Alignment::
init(const string& str)
{
	clear();
	setupMap(str);
	setupBoundaryMapAndVec();
}

void
Alignment::
init(const string& str, int srcLen, int tarLen)
{
	clear();
	setupMap(str);
	_srcLen=srcLen, _tarLen=tarLen;
	setupBoundaryMapAndVec();
}

void
Alignment::
setupMap(const string& str)
{
	_s2tMap.clear();_t2sMap.clear();
	stringstream ss(str.c_str());
	while(ss.good())
	{
		string tmp="";
		int a,b;
		ss>>tmp;
		if(tmp.find("-")==string::npos)break;
		
		sscanf(tmp.c_str(),"%d-%d",&a,&b);
		_s2tMap[a].insert(b);
		_t2sMap[b].insert(a);
	}
	getSentLenFromMap(_s2tMap,_srcLen);
	getSentLenFromMap(_t2sMap,_tarLen);
}

void
getSentLenFromMap(const map<int,set<int> >& s2tMap, int& slen)
{
	for(map<int,set<int> >::const_iterator iter=s2tMap.begin();iter!=s2tMap.end();iter++)
	{
		int len=iter->first+1;
		slen=slen>len?slen:len;
	}
}

void getMinMaxFromSet(const set<int>& aset, pair<int,int>& minAndMax)
{
	int min=10000000,max=-min;
	for(set<int>::const_iterator iter=aset.begin();iter!=aset.end();iter++)
	{
		int pos=*iter;
		min=min<pos?min:pos;
		max=max>pos?max:pos;
	}
	minAndMax.first=min;
	minAndMax.second=max;
	if(max<0)
		minAndMax.first=minAndMax.second=-1;
}

void combineMinMax(const pair<int,int>& mm1,const pair<int,int>& mm2, pair<int,int>& result)
{
	if(mm1.first==-1)
	{
		result=mm2;
		return;
	}
	if(mm2.first==-1)
	{
		result=mm1;
		return;
	}
	result.first=min(mm1.first,mm2.first);
	result.second=max(mm1.second,mm2.second);
	return;
}

void 
Alignment::
setupBoundaryMap(const map<int,set<int> >& s2tMap, int slen, map<pair<int,int>, pair<int,int> >& s2tbm)
{
	int i=0;
	for(i=0;i<slen;i++)
	{
		pair<int,int> minAndMax;
		minAndMax.first=minAndMax.second=-1;
		if(s2tMap.find(i)!=s2tMap.end())
		{
			const set<int>& tmpSet=s2tMap.find(i)->second;
			getMinMaxFromSet(tmpSet,minAndMax);
		}
		s2tbm[make_pair(i,i)]=minAndMax;
	}
	for(i=0;i<slen-1;i++)
	{
		for(int j=i+1;j<slen;j++)
		{
			combineMinMax(s2tbm[make_pair(i,j-1)],s2tbm[make_pair(j,j)],s2tbm[make_pair(i,j)]);
		}
	}
}

void 
Alignment::
setupBoundaryMap()
{
	setupBoundaryMap(_s2tMap,_srcLen,_s2tBoundaryMap);
	setupBoundaryMap(_t2sMap,_tarLen,_t2sBoundaryMap);
}

void 
Alignment::
setupVec(const map<int,set<int> >& s2tMap, int slen, vector<vector<int> >& s2tVec)
{
	s2tVec.clear();
	for(int i=0;i<slen;i++)
	{
		vector<int> tmp;
		s2tVec.push_back(tmp);
	}
	map<int,set<int> >::const_iterator iter;
	for(iter=s2tMap.begin();iter!=s2tMap.end();iter++)
	{
		int srcpos=iter->first;
		const set<int>& tarSet=iter->second;
		for(set<int>::const_iterator setIter=tarSet.begin();setIter!=tarSet.end();setIter++)
		{
			s2tVec[srcpos].push_back(*setIter);
		}
		sort(s2tVec[srcpos].begin(),s2tVec[srcpos].end());
	}
}

void
Alignment::
setupVec()
{
	setupVec(_s2tMap,_srcLen,_s2tVec);
	setupVec(_t2sMap,_tarLen,_t2sVec);
}

void 
Alignment::
setupBoundaryMapAndVec()
{
	setupBoundaryMap();
	setupVec();
}

void 
Alignment::
print(ostream& os)const
{
	for(int i=0;i<_srcLen;i++)
	{
		const vector<int>& vec=_s2tVec[i];
		if(vec.size()==0)continue;
		for(unsigned int j=0;j<vec.size();j++)
			os<<i<<"-"<<vec[j]<<" ";
	}
	os<<endl;
}

void 
Alignment::
printInICTFormat(ostream& os)const
{
	for(int i=0;i<_srcLen;i++)
	{
		const vector<int>& vec=_s2tVec[i];
		if(vec.size()==0)continue;
		for(unsigned int j=0;j<vec.size();j++)
			os<<i+1<<":"<<vec[j]+1<<"/1 ";
	}
}

void 
Alignment::
print_boundaryMap(ostream& os)const
{
	map<pair<int,int>, pair<int,int> >::const_iterator iter;
	os<<"\nbounary map source to target"<<endl;
	for(iter=_s2tBoundaryMap.begin();iter!=_s2tBoundaryMap.end();iter++)
	{
		const pair<int,int>& sb=iter->first;
		const pair<int,int>& tb=iter->second;
		os<<"["<<sb.first<<","<<sb.second<<"]";
		os<<" --> ";
		os<<"["<<tb.first<<","<<tb.second<<"]";
		os<<endl;
	}
	os<<"\nboundary map target to source"<<endl;
	for(iter=_t2sBoundaryMap.begin();iter!=_t2sBoundaryMap.end();iter++)
	{
		const pair<int,int>& sb=iter->first;
		const pair<int,int>& tb=iter->second;
		os<<"["<<sb.first<<","<<sb.second<<"]";
		os<<" --> ";
		os<<"["<<tb.first<<","<<tb.second<<"]";
		os<<endl;
	}
}

bool 
Alignment::
srcBoundary2tarBoundary(const pair<int,int>& srcBoundary, pair<int,int>& result)const
{
	if(_s2tBoundaryMap.find(srcBoundary)==_s2tBoundaryMap.end())
	{
		result.first=result.second=-1;
		return false;
	}
	else
	{
		result=_s2tBoundaryMap.find(srcBoundary)->second;
		return true;
	}
}

bool 
Alignment::
tarBoundary2srcBoundary(const pair<int,int>& tarBoundary, pair<int,int>& result)const
{
	if(_t2sBoundaryMap.find(tarBoundary)==_t2sBoundaryMap.end())
	{
		result.first=result.second=-1;
		return false;
	}
	else
	{
		result=_t2sBoundaryMap.find(tarBoundary)->second;
		return true;
	}
}

bool 
Alignment::
srcPhrase2tarPhrases(const pair<int,int>& srcPhrase, vector<pair<int,int> >& tarPhrases)const
{
	if(!isAlignable_srcBoundary(srcPhrase))return false;
	pair<int,int> tarAlignedBoundary;
	srcBoundary2tarBoundary(srcPhrase,tarAlignedBoundary);
	int left=tarAlignedBoundary.first;
	int right=tarAlignedBoundary.second;
	
	int leftLimit=left;
	int rightLimit=right;
	
	while(leftLimit>0&&left-leftLimit<_maxUnAligned)
	{
		if(!isAligned_tarPos(leftLimit-1))
			leftLimit--;
		else
			break;
	}

	while(rightLimit<_tarLen-2&&rightLimit-right<_maxUnAligned)
	{
		if(!isAligned_tarPos(rightLimit+1))
			rightLimit++;
		else
			break;
	}
	for(int leftPos=leftLimit;leftPos<=left;leftPos++)
	{
		for(int rightPos=right;rightPos<=rightLimit;rightPos++)
		{
			tarPhrases.push_back(make_pair(leftPos,rightPos));
		}
	}
	return true;
}

bool 
Alignment::
tarPhrase2srcPhrases(const pair<int,int>& tarPhrase, vector<pair<int,int> >& srcPhrases)const
{
	if(!isAlignable_tarBoundary(tarPhrase))return false;
	pair<int,int> srcAlignedBoundary;
	tarBoundary2srcBoundary(tarPhrase,srcAlignedBoundary);
	int left=srcAlignedBoundary.first;
	int right=srcAlignedBoundary.second;
	
	int leftLimit=left;
	int rightLimit=right;
	
	while(leftLimit>0&&left-leftLimit<_maxUnAligned)
	{
		if(!isAligned_srcPos(leftLimit-1))
			leftLimit--;
		else
			break;
	}
	while(rightLimit<_tarLen-2&&rightLimit-right<_maxUnAligned)
	{
		if(!isAligned_srcPos(rightLimit+1))
			rightLimit++;
		else
			break;
	}
	for(int leftPos=leftLimit;leftPos<=left;leftPos++)
	{
		for(int rightPos=right;rightPos<=rightLimit;rightPos++)
		{
			srcPhrases.push_back(make_pair(leftPos,rightPos));
		}
	}
	return true;
}

bool 
Alignment::
isAlignable_srcBoundary(const pair<int,int>& srcBoundary)const
{
	pair<int,int> tmpSrcBoundary,tmpTarBoundary;
	if(!srcBoundary2tarBoundary(srcBoundary,tmpTarBoundary))return false;
	if(!tarBoundary2srcBoundary(tmpTarBoundary,tmpSrcBoundary))return false;
/*	cerr<<"tar"<<endl;
	cerr<<tmpTarBoundary.first<<","<<tmpTarBoundary.second<<endl;
    cerr<<"tar 2 src"<<endl;
	cerr<<tmpSrcBoundary.first<<","<<tmpSrcBoundary.second<<endl;
	cerr<<"src"<<endl;
	cerr<<srcBoundary.first<<","<<srcBoundary.second<<endl;
	*/
	int leftGap=tmpSrcBoundary.first-srcBoundary.first;
	int rightGap=srcBoundary.second-tmpSrcBoundary.second;
	if(leftGap>=0&&rightGap>=0&&leftGap<=_maxUnAligned&&rightGap<=_maxUnAligned)
		return true;
	else
		return false;
}

bool 
Alignment::
isAlignable_tarBoundary(const pair<int,int>& tarBoundary)const
{
	pair<int,int> tmpSrcBoundary,tmpTarBoundary;
	if(!tarBoundary2srcBoundary(tarBoundary,tmpSrcBoundary))return false;
	if(!srcBoundary2tarBoundary(tmpSrcBoundary,tmpTarBoundary))return false;
	
	int leftGap=tmpTarBoundary.first-tarBoundary.first;
	int rightGap=tarBoundary.second-tmpTarBoundary.second;
	if(leftGap>=0&&rightGap>=0&&leftGap<=_maxUnAligned&&rightGap<=_maxUnAligned)
		return true;
	else
		return false;
}

void 
Alignment::
print_alignableBoundarys(ostream& os)const
{
	map<pair<int,int>, pair<int,int> >::const_iterator iter;
	os<<"\nalignable boundary map source to target"<<endl;
	for(iter=_s2tBoundaryMap.begin();iter!=_s2tBoundaryMap.end();iter++)
	{
		const pair<int,int>& sb=iter->first;
		const pair<int,int>& tb=iter->second;
		if(!isAlignable_srcBoundary(sb))continue;
		os<<"["<<sb.first<<","<<sb.second<<"]";
		os<<" --> ";
		os<<"["<<tb.first<<","<<tb.second<<"]";
		os<<endl;
	}
	
	os<<"\nalignable boundary map target to source"<<endl;
	for(iter=_t2sBoundaryMap.begin();iter!=_t2sBoundaryMap.end();iter++)
	{
		const pair<int,int>& sb=iter->first;
		const pair<int,int>& tb=iter->second;
		if(!isAlignable_tarBoundary(sb))continue;
		os<<"["<<sb.first<<","<<sb.second<<"]";
		os<<" --> ";
		os<<"["<<tb.first<<","<<tb.second<<"]";
		os<<endl;
	}
}

bool 
Alignment::
isAligned_srcPos(int srcPosition)const
{
	if(_s2tMap.find(srcPosition)==_s2tMap.end())
		return false;
	else
		return true;
}

bool 
Alignment::
isAligned_tarPos(int tarPosition)const
{
	
	if(_t2sMap.find(tarPosition)==_t2sMap.end())
		return false;
	else
		return true;
}

void 
Alignment::
print_phrases(ostream& os)const
{
	map<pair<int,int>, pair<int,int> >::const_iterator iter;
	os<<"\nphrases source to target"<<endl;
	for(iter=_s2tBoundaryMap.begin();iter!=_s2tBoundaryMap.end();iter++)
	{
		const pair<int,int>& sb=iter->first;
		if(!isAlignable_srcBoundary(sb))continue;
		vector<pair<int,int> > tarPhrases;
		srcPhrase2tarPhrases(sb, tarPhrases);
		os<<"["<<sb.first<<","<<sb.second<<"]";
		os<<" --> ";
		for(unsigned int i=0;i<tarPhrases.size();i++)
			os<<"["<<tarPhrases[i].first<<","<<tarPhrases[i].second<<"]";
		os<<endl;
	}
	
	os<<"\nphrases target to source"<<endl;
	for(iter=_t2sBoundaryMap.begin();iter!=_t2sBoundaryMap.end();iter++)
	{
		const pair<int,int>& tb=iter->first;
		if(!isAlignable_tarBoundary(tb))continue;

		vector<pair<int,int> > srcPhrases;
		tarPhrase2srcPhrases(tb, srcPhrases);
		
		os<<"["<<tb.first<<","<<tb.second<<"]";
		os<<" --> ";
		for(unsigned int i=0;i<srcPhrases.size();i++)
			os<<"["<<srcPhrases[i].first<<","<<srcPhrases[i].second<<"]";
		os<<endl;
	}	
}

bool 
Alignment::
isAligned(int wrdIndex, LangSide ls)const
{
	if(ls==SRC)
	{
		return isAligned_srcPos(wrdIndex);
	}
	else
	{
		return isAligned_tarPos(wrdIndex);
	}
}

bool 
Alignment::
isAlignable(const pair<int,int>& boundary, LangSide ls)const
{
	if(ls==SRC)
	{
		return isAlignable_srcBoundary(boundary);
	}
	else
	{
		return isAlignable_tarBoundary(boundary);
	}
}

bool 
Alignment::
boundaryMapping(const pair<int,int>& inputBoundary, pair<int,int>& result, LangDirection ld)const
{
	if(ld==S2T)
	{
		return srcBoundary2tarBoundary(inputBoundary,result);
	}
	else
	{
		return tarBoundary2srcBoundary(inputBoundary,result);
	}
}

bool 
Alignment::
phraseMapping(const pair<int,int>& inputPhrase, vector<pair<int,int> >& resultPhrases, LangDirection ld)const
{
	if(ld==S2T)
	{
		return srcPhrase2tarPhrases(inputPhrase,resultPhrases);
	}
	else
	{
		return tarPhrase2srcPhrases(inputPhrase,resultPhrases);
	}
}

bool 
Alignment::
alignment(const pair<int,int>& inputPhrase, vector<pair<int,int> >& resultPhrases, LangDirection ld)const
{
	if(ld==S2T)
	{
		return srcPhrase2tarPhrases(inputPhrase,resultPhrases);
	}
	else
	{
		return tarPhrase2srcPhrases(inputPhrase,resultPhrases);
	}
}

bool 
Alignment::
alignment(const pair<int,int>& inputPhrase, pair<int,int>& resultPhrase, LangDirection ld)const
{
	LangSide ls;
	if(ld==S2T)
		ls=SRC;
	else ls=TAR;
	//cout<<"query ["<<inputPhrase.first<<","<<inputPhrase.second<<"]"<<endl;
	if(!isAlignable(inputPhrase,ls))return false;
	//cout<<"yes we come here"<<endl;
	return boundaryMapping(inputPhrase,resultPhrase,ld);
}

const vector<int>& 
Alignment::
alignment(int wrdIndex,LangDirection ld)const
{
	if(ld==S2T)
	{
		return alignedTarsForSrc(wrdIndex);
	}
	else
	{
		return alignedSrcsForTar(wrdIndex);
	}
}

bool 
Alignment::
alignment(int wrdIndex, vector<int>& resultWrds, LangDirection ld)const
{
	if(ld==S2T)
	{
		if(wrdIndex>=_srcLen)return false;
		resultWrds=alignedTarsForSrc(wrdIndex);
	}
	else
	{
		if(wrdIndex>=_tarLen)return false;
		resultWrds=alignedSrcsForTar(wrdIndex);
	}	
	return true;
}

int 
Alignment::
numOfUnAlignedBoundWords(const pair<int,int>& boundary, LangSide ls)const
{
	int count=0;
	int leftAlignedPos=boundary.first;
	int rightAlignedPos=boundary.second;
	for(;leftAlignedPos<=rightAlignedPos;leftAlignedPos++)
	{	
		if(isAligned(leftAlignedPos,ls))
			break;
		else 
		{
			leftAlignedPos++;
			count++;
		}
	}
	for(;rightAlignedPos>=leftAlignedPos;rightAlignedPos--)
	{
		if(isAligned(rightAlignedPos,ls))
			break;
		else
		{
			rightAlignedPos--;
			count++;
		}
	}
	return count;
}

string
Alignment::
modifiedAlignment(int maxSpan){
    set<pair<int,int> > a;
    for(size_t i=0;i<_s2tVec.size();i++){
        for(int& j : _s2tVec[i]){
            a.insert(make_pair(i,j));
        }
    }
    //delete links which cause too long span
    for(size_t i=0;i<_s2tVec.size();i++){
        vector<int>& v=_s2tVec[i];
        //delete links
        for(int& j : v){
            if(j-v.front()>maxSpan){
                a.erase(make_pair(i,j));
            }
        }
    }
    for(size_t i=0;i<_t2sVec.size();i++){
        vector<int>& v=_t2sVec[i];
        //delete links
        for(int& j : v){
            if(j-v.front()>maxSpan){
                a.erase(make_pair(j,i));
            }
        }
    }
    //delete links which cause failure in a greedy mtu extraction process
    string result="";
    for(auto& p:a){
        result+=to_string(p.first)+"-"+to_string(p.second)+" ";
    }
    init(result,_srcLen,_tarLen);
    for(int start=0;start<_srcLen;start++){
        bool success=false;
        for(int stop=start;stop<_srcLen;stop++){
            pair<int,int> srcSpan=make_pair(start,stop);
            if(isAlignable(srcSpan, SRC)==true){
                success=true;
                start=stop;
                break;
            }
        }
        if(!success){
            vector<int>& v=_s2tVec[start];
            for(int& j : v){
                a.erase(make_pair(start,j));
            }
        }
    }
    if(a.size()==0)return "";
    //assign alignment for source unaligned words
    vector<pair<int,int>> aVec;
    for(auto& p:a)
        aVec.push_back(p);
    sort(aVec.begin(),aVec.end());
    for(int i=0;i<aVec[0].first;i++)
        a.insert(make_pair(i,aVec[0].second));
    for(int i=aVec.back().first;i<_srcLen;i++)
        a.insert(make_pair(i,aVec.back().second));
    for(int i=1;i<(int)aVec.size();i++){
        for(int j=aVec[i-1].first+1;j<aVec[i].first;j++){
            a.insert(make_pair(j,aVec[i-1].second));
        }
    }

    //assign alignment for target unaligned words
    aVec.clear();
    for(auto& p:a)
        aVec.push_back(make_pair(p.second,p.first));
    sort(aVec.begin(),aVec.end());
    for(int i=0;i<aVec[0].first;i++)
        a.insert(make_pair(aVec[0].second,i));
    for(int i=aVec.back().first;i<_tarLen;i++)
        a.insert(make_pair(aVec.back().second,i));
    for(int i=1;i<(int)aVec.size();i++){
        for(int j=aVec[i-1].first+1;j<aVec[i].first;j++){
            a.insert(make_pair(aVec[i-1].second,j));
        }
    }

    result="";
    for(auto& p:a){
        result+=to_string(p.first)+"-"+to_string(p.second)+" ";
    }
    init(result,_srcLen,_tarLen);
    return result;
}
