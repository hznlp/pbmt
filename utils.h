#ifndef UTILS_H
#define UTILS_H

#ifdef WIN32
#define isNaN(x) _isnan(x)
#define isFinite(x) _finite(x)
#define isInf(x) !_finite(x)
#define isBadNumber(x) (isNaN(x)||isInf(x))
#else
#define isNaN(x) isnan(x)
#define isFinite(x) !isinf(x)
#define isInf(x) isinf(x)
#define isBadNumber(x) (isNaN(x)||isInf(x))
#endif

#include <string>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <vector>
#include <map>
using namespace std;
string intToString(int in);
string doubleToString(double in);
string& replaceSubStr(string& input, string oldSub, string newSub);
int countSubStr(const string& in, const string& sub);
bool endsWith(char * src, char* tail);
inline void error(string str){cerr<<str<<endl;exit(1);}
inline double log(double x,double y){return log(y)/log(x);}
double digamma(double x);
bool lessBySrcPos(const pair<int,int>& p1, const pair<int,int>& p2);
bool lessByTarPos(const pair<int,int>& p1, const pair<int,int>& p2);
inline double unLogrize(double score){return pow(2.0,score);}
inline double logrize(double score){return log(score)/log(2.0);}
string current_run_spec();
extern int dlevel;
ostream* set_pdout();
string& convertNonChinesePunc(string& curline, bool deleteSpace=false);
vector<string>& stringToVector(const string& sent, vector<string>& result);
string vectorToString(vector<string>& result);

map<string,int>& sentence2Phrases(const string& sent, map<string,int>& phrases);
string& SBC2DBC(const char *pszWord, string& result);   
string& DBC2SBC(const char *pszWord, string& result);   
inline ostream& operator << (ostream& os, const vector<string>& strs){for(size_t i=0;i<strs.size();i++) {os<<strs[i]; if(i+1!=strs.size())os<<" ";}return os;}
bool isPartialLexicalizedRule(const string& curline);
bool isLexicalizedRule(const string& curline);
bool isUnLexicalizedRule(const string& curline);
bool isTreeSequenceRule(const string& curline);
void getRuleSrcLeaves(const string& curline, vector<string>& leaves);
bool onlyContainRealWords(const string& curline, map<string,int>& realTermMap);
bool containRealWords(const string& curline, map<string,int>& realTermMap);
string& trim(const string& in, string& out);
string& lowercase(const string& in, string& out);

bool lineWithNoRealWords(string line);
vector<unsigned char>& genGB2312(vector<unsigned short>& input, vector<unsigned char>& result);
vector<unsigned short>& readGB2312(const char* input, vector<unsigned short>& result);
string& genGB2312(vector<unsigned short>& input, string& result);
string& filterBadCharGB2312(string& input, string& result);
#endif
