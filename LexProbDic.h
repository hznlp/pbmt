#ifndef LEXPROBDIC_H
#define LEXPROBDIC_H
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "Direction.h"
using namespace std;

inline vector<double> entryCombine(const vector<double>& d1, const vector<double>& d2){vector<double> tmp; tmp.clear(); for(size_t i=0;i<d1.size()&&i<d2.size();i++)tmp.push_back(d1[i]+d2[i]); return tmp;}
inline vector<double>& updateMinEntry(const vector<double>& d1, vector<double>& minD){for(size_t i=0;i<d1.size()&&i<minD.size();i++)minD[i]=min(d1[i],minD[i]); return minD;}
inline vector<double>& updateMaxEntry(const vector<double>& d1, vector<double>& maxD){for(size_t i=0;i<d1.size()&&i<maxD.size();i++)maxD[i]=max(d1[i],maxD[i]); return maxD;}
inline ostream& operator << (ostream& os, const vector<double>& ds){for(size_t i=0;i<ds.size();i++){os<<ds[i];if(i!=ds.size()-1)os<<" ";}return os;}

inline double entryCombine(double& d1, double& d2){return d1+d2;}
inline double& updateMinEntry(double& d1, double& minD){minD=min(d1,minD); return minD;}
inline double& updateMaxEntry(double& d1, double& maxD){maxD=max(d1,maxD); return maxD;}

template<class Entry>
class Dic
{
public:
	Dic(){_dic.clear();_nullEntry.clear();}
	~Dic(){clear();}
	bool load(string filename)
	{
		ifstream is(filename.c_str());
		if(!is.good())return false;
		while(!is.eof())
		{
			string curline="";
			getline(is,curline);
			stringstream ss(curline);
			string s="",t="";
			Entry entry;
			string tmp;
			ss>>s>>t>>entry;
			if(t=="")break;
			//cout<<s<<" "<<t<<" "<<entry<<endl;
			add(s,t,entry);
		}
		return true;
	}
	bool query(const string& s, const string& t, Entry& entry)const
	{
		bool result=false;
		typename map<string, map<string,Entry> >::const_iterator iter=_dic.find(s);
		if(iter==_dic.end())
			result=false;
		else
		{
			const map<string, Entry>& entryMap=iter->second;
			typename map<string, Entry>::const_iterator iter2=entryMap.find(t);
			if(iter2==entryMap.end())
				result=false;
			else
			{
				entry=iter2->second;
				result=true;
			}
		}
		//if(result==false)
		//	cout<<s<<" -- "<<t<<endl;
		//if(result==false)
		//	entry=_minEntry;
		return result;
	}
	
	/*
	bool add(const string& s, const string& t, Entry entry)
	{
		_dic[s][t]=entry;
		return true;
	}*/
	
	bool add(const string& s, const string& t, Entry& entry)
	{
		if(_dic.size()==0)
		{
			_minEntry=entry;
			_maxEntry=entry;
		}
		if(_dic.find(s)==_dic.end())
		{
			_dic[s];
		}
		map<string,Entry>& entryMap=_dic[s];
		if(entryMap.find(t)==entryMap.end())
			entryMap[t]=entry;
		else
		{
			Entry& e=entryMap[t];
			e=entryCombine(e,entry);
		}
		updateMinEntry(entry,_minEntry);
		updateMaxEntry(entry,_maxEntry);
		return true;
	}

	void print(ostream& os)const
	{
		typename map<string, map<string,Entry> >::const_iterator iter;
		for(iter=_dic.begin();iter!=_dic.end();iter++)
		{
			const string& s=iter->first;
			const map<string, Entry>& entryMap=iter->second;
			typename map<string,Entry>::const_iterator iter2;
			for(iter2=entryMap.begin();iter2!=entryMap.end();iter2++)
			{
				const string& t=iter2->first;
				Entry entry=iter2->second;
				os<<t<<" "<<s<<" "<<entry<<endl;
			}
		}
	}

	void normalPrint(ostream& os)const
	{
		typename map<string, map<string,Entry> >::const_iterator iter;
		for(iter=_dic.begin();iter!=_dic.end();iter++)
		{
			const string& s=iter->first;
			const map<string, Entry>& entryMap=iter->second;
			typename map<string,Entry>::const_iterator iter2;
			for(iter2=entryMap.begin();iter2!=entryMap.end();iter2++)
			{
				const string& t=iter2->first;
				Entry entry=iter2->second;
				os<<s<<" ||| "<<t<<" ||| "<<entry<<endl;
			}
		}
	}

	virtual int normalize(void);
	void clear()
	{
		typename map<string, map<string, Entry> >::iterator iter=_dic.begin(); 
		for(;iter!=_dic.end();iter++)
			iter->second.clear();
		_dic.clear();
		_nullEntry.clear();
	}
	size_t size(){return _dic.size();}
	const Entry& minEntry()const{return _minEntry;}
	map<string, map<string, Entry> >& data(){return _dic;}
protected:
	map<string, map<string, Entry> > _dic;
	map<string, Entry> _nullEntry;
	Entry _minEntry,_maxEntry;
};

template<class Entry>
class BothWayDic
{
public:
	void load(string s2t_file, string t2s_file){load_s2t(s2t_file),load_t2s(t2s_file);}
	void load(string filename, LangDirection ld)
	{
		if(ld==S2T)
			load_s2t(filename);
		else
			load_t2s(filename);
	}
	bool add(string& from, string& to, Entry& entry, LangDirection ld)
	{
		if(ld==S2T)
			return _s2t.add(from,to,entry);
		else
			return _t2s.add(from,to,entry);
	}
	bool query(const string& from, const string& to, Entry& entry, LangDirection ld)const
	{
		if(ld==S2T)
			return _s2t.query(from,to,entry);
		else
			return _t2s.query(from,to,entry);
	}
	const map<string, Entry>& query(const string& from, LangDirection ld)const
	{
		if(ld==S2T)
			return _s2t.query(from);
		else
			return _t2s.query(from);
	}

	void print(ostream& os2t, ostream& ot2s)const{_s2t.print(os2t),_t2s.print(ot2s);}
	void print_s2t(ostream& os)const{_s2t.print(os);}
	void print_t2s(ostream& os)const{_t2s.print(os);}
	int normalize(){_s2t.normalize();_t2s.normalize();return 0;}
	
	Dic<Entry>& s2t(){return _s2t;}
	Dic<Entry>& t2s(){return _t2s;}
	void clear(){_s2t.clear();_t2s.clear();}
private:
	void load_s2t(string filename){_s2t.load(filename);}
	void load_t2s(string filename){_t2s.load(filename);}
	bool add_s2t(string& s, string& t, Entry entry){return _s2t.add(s,t,entry);}
	bool add_t2s(string& t, string& s, Entry entry){return _t2s.add(t,s,entry);}

//const function
	bool query_s2t(const string& s, const string& t, Entry& entry)const{return _s2t.query(s,t,entry);}
	bool query_t2s(string& t, string& s, Entry& entry)const{return _t2s.query(t,s,entry);}
	const map<string, Entry>& query_s2t(const string& s)const{return _s2t.query(s);}
	const map<string, Entry>& query_t2s(const string& t)const{return _t2s.query(t);}
	Dic<Entry> _s2t;
	Dic<Entry> _t2s;
};



#endif
