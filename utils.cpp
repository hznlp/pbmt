#include "utils.h"
#pragma warning(disable:4996) //仅显示一个

string
intToString(int i)
{
  stringstream ss;
  ss<<i; 
  string ans;
  ss>>ans;
  return ans;
}
string doubleToString(double i)
{
  stringstream ss;
  //ss.precision(20);
  if(i==0)
	  return "0.0";
  ss<<i; 
  string ans;
  ss>>ans;
  return ans;
}
string& replaceSubStr(string& in, string oldSub, string newSub)
{
	size_t pos=0;
	while(true)
	{
		pos=in.find(oldSub,pos);
		if(pos!=string::npos)
		{
			in.replace(pos,oldSub.length(),newSub);
			pos+=newSub.length();
		}
		else break;
	}
	return in;
}

int countSubStr(const string& in, const string& sub)
{
	int count=0;
	size_t pos=0;
	while(true)
	{
		pos=in.find(sub,pos);
		if(pos!=string::npos)
		{
			count++;
			pos+=sub.length();
		}
		else break;
	}
	return count;
}

bool endsWith(char * src, char* tail)
{
	size_t srclen=strlen(src);
	size_t taillen=strlen(tail);
	return !strcmp(src+(srclen-taillen),tail);
}

bool lessBySrcPos(const pair<int,int>& p1, const pair<int,int>& p2)
{
	if(p1.first<p2.first)
		return true;
	else if(p1.first==p2.first&&p1.second<p2.second)
		return true;
	else return false;
}
/*
pair<int,int>
pair<int,int>::operator = (pair<int,int> p1, pair<int,int> p2)
{
	p1.first=p2.first;p1.second=p2.second;
	return p1;
}
*/
bool lessByTarPos(const pair<int,int>& p1, const pair<int,int>& p2)
{
	if(p1.second<p2.second)
		return true;
	else if(p1.second==p2.second&&p1.first<p2.first)
		return true;
	else return false;
}

string current_run_spec()
{
  string result="";
  stringstream ss;
  struct tm *local;
  time_t t;
  char *user;

  t = time(NULL);
  local = localtime(&t);

  if(local->tm_hour<10)
	  ss<<"0";
  ss<<local->tm_hour<<"-";

  if(local->tm_min<10)
	  ss<<"0";
  ss<<local->tm_min<<"-";

  if(local->tm_sec<10)
	  ss<<"0";
  ss<<local->tm_sec<<"@";

  int year=(local->tm_year%100);
  if(year<10)
	  ss<<"0";
  ss<<year<<"-";

  if(local->tm_mon+1<10)
	  ss<<"0";
  ss<<(local->tm_mon + 1)<<"-";

  if(local->tm_mday<10)
	  ss<<"0";
  ss<<local->tm_mday<<".";



  user = getenv("USERNAME");
  if(user==NULL)
	  user = getenv("USER");
  if(user==NULL)
	user="noname";
  ss<<user;
  getline(ss,result);
  return result;
}

string& convertNonChinesePunc(string& curline, bool deleteSpace)
{
	vector<string> sent;
	sent.clear();
	stringstream ss(curline);
	map<string,string> dic;
	dic[","]="，";
	//dic["."]="。"; //must in the end of sentence
	dic["("]="（";
	dic[")"]="）";
	dic[";"]="；";
	dic[":"]="：";
	dic["?"]="？";
	dic["<"]="《";
	dic[">"]="》";
	dic["!"]="！";
	//dic["&"]="＆";
	dic["「"]="“";
	dic["」"]="”";
	dic["-LRB-"]="（";
	dic["-RRB-"]="）";
	dic["-LCB-"]="{";
	dic["-RCB-"]="}";
	dic["-LSB-"]="】";
	dic["-RSB-"]="【";

	while(!ss.eof())
	{
		string wrd="";
		ss>>wrd;
		if(wrd=="")break;
		sent.push_back(wrd);
	}
	for(size_t i=0;i<sent.size();i++)
	{
		if(i==sent.size()-1&&sent[i]==".")
			sent[i]="。";
		else if(dic.find(sent[i])!=dic.end())
			sent[i]=dic[sent[i]];
	}
	curline="";
	for(size_t i=0;i<sent.size();i++)
	{
		curline+=sent[i];
		if(i!=sent.size()-1&&!deleteSpace)
			curline+=" ";
	}
	return curline;
}

vector<string>& stringToVector(const string& sent, vector<string>& result)
{
	stringstream ss(sent.c_str());
	result.clear();
	while(!ss.eof())
	{
		string word="";
		ss>>word;
		if(word=="")break;
		result.push_back(word);
	}
	return result;
}

map<string,int>& sentence2Phrases(const string& sent, map<string,int>& phrases)
{
	vector<string> words;
	stringToVector(sent,words);
	for(size_t length=0;length<words.size();length++)
	{
		for(size_t start=0;start<words.size()-length;start++)
		{
			string phrase="";
			for(size_t pos=start;pos<=start+length;pos++)
			{
				phrase+=words[pos]+" ";
			}
			phrases[phrase]=1;
		}
	}
	return phrases;
}

string& SBC2DBC(const char * src, string& result)
{
	size_t len=strlen(src);
	char * buf=new char[len+1];
	int j=0;
	for(size_t i=0;i<len;i++)
	{
		if((unsigned char)src[i] > 163)
		{
			buf[j++]=src[i++];
			buf[j++]=src[i];
			continue;
		}
		else if((unsigned char)src[i]==163)
		{
			i++;
			buf[j++]=(unsigned char)src[i]-128;
			continue;
		}
		else if((unsigned char)src[i]==161&&(unsigned char)src[i+1]==161)
		{
			i++;
			buf[j++]=' ';
			continue;
		}
		else if((unsigned char)src[i]==161&&(unsigned char)src[i+1]!=161)
		{
			buf[j++]=src[i++];
			buf[j++]=src[i];
			continue;
		}
		else 
		{
			buf[j++]=src[i];
		}
	}
	buf[j]=0;
	result=buf;
	delete buf;
	return result;
}

string& DBC2SBC(const char * src, string& result)
{
	size_t len=strlen(src);
	char * buf=new char[len*2+1];
	int j=0;
	for(size_t i=0;i<len;i++)
	{
		if((unsigned char)src[i] >= 163||(unsigned char)src[i]==161)
		{
			buf[j++]=src[i++];
			buf[j++]=src[i];
			continue;
		}
		else if((unsigned char)src[i]==32)
		{
			buf[j++]=(char)161;
			buf[j++]=(char)161;
			continue;
		}
		else if(((unsigned char)src[i]>='a'&&(unsigned char)src[i]<='z')||
			((unsigned char)src[i]>='A'&&(unsigned char)src[i]<='Z')||
			(unsigned char)src[i]>=128)
		{
			buf[j++]=src[i];
			continue;
		}
		else 
		{
			buf[j++]=(char)163;
			buf[j++]=src[i]+128;
			continue;
		}
	}
	buf[j]=0;
	result=buf;
	delete buf;
	return result;
}

vector<unsigned short>& readGB2312(const char* input, vector<unsigned short>& result)
{
	size_t len=strlen(input);
	
	size_t rIter=0;
	for(size_t i=0;i<len+1;i++)
	{
		unsigned char ch=input[i];
		//cout<<(int)ch<<" ";
		result.push_back(0);
		if(ch>=128)
		{
			result[rIter]=0;
			result[rIter]=ch;
			result[rIter]<<=8;
			result[rIter]+=(unsigned char)input[i+1];
			//cout<<(int)(result[rIter]>>8)<<" ";
			//cout<<(int)((unsigned char)input[i+1])<<" "<<(int)(result[rIter]&255)<<" ";
			i++;
			rIter++;
		}
		else
		{
			result[rIter]=0;
			result[rIter]=ch;
			rIter++;
		}
	}
	//cout<<endl;
	result.push_back(0);
	return result;
}

vector<unsigned char>& genGB2312(vector<unsigned short>& input, vector<unsigned char>& result)
{
	size_t len=0;
	for(;input[len]!=0;len++);
	for(size_t i=0;i<len;i++)
	{
		if(input[i]<128)
		{
			result.push_back((unsigned char)(input[i]&255));
		}
		else
		{
			unsigned char upper=(unsigned char)(input[i]>>8);
			unsigned char lower=(unsigned char)(input[i]&255);
			result.push_back(upper);
			result.push_back(lower);
		}
	}
	result.push_back(0);
	return result;
}

string& genGB2312(vector<unsigned short>& input, string& result)
{
	result="";
	size_t len=0;
	for(;input[len]!=0;len++);
	for(size_t i=0;i<len;i++)
	{
		if(input[i]<128)
		{
			result+=((char)(input[i]&255));
		}
		else
		{
			unsigned char upper=(unsigned char)(input[i]>>8);
			unsigned char lower=(unsigned char)(input[i]&255);
			if(!
				((upper>=0xA1&&upper<=0xF7)&&(lower>=0xA1&&lower<=0xFE))
				)
				break;
			result+=(char)(upper);
			result+=(char)(lower);
		}
	}
	
	return result;
}

string& filterBadCharGB2312(const char* input, string& result)
{
	vector<unsigned short> temp;
	readGB2312(input,temp);
	genGB2312(temp,result);
	return result;
}

string& filterBadCharGB2312(string& input, string& result)
{
	return filterBadCharGB2312(input.c_str(),result);
}


bool isPartialLexicalizedRule(const string& curline)
{
	if(isLexicalizedRule(curline))return false;
	string sub=curline.substr(0,curline.find("==>"));
	if(sub.find_first_not_of("()+ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz_ ")!=string::npos&&sub.find("(S1 (IP IP))")!=0)
		return true;
	else return false;
}

bool isLexicalizedRule(const string& curline)
{
	string subs=curline.substr(curline.find("!!!")+3);
	subs=subs.substr(0,subs.find("|||"));
	//cerr<<subs<<endl;
	if(subs.find_first_not_of(" ")==string::npos)
		return true;
	else return false;
}

bool isUnLexicalizedRule(const string& curline)
{
	if(isLexicalizedRule(curline)||isPartialLexicalizedRule(curline))return false;
	return true;
}


bool isSpace(char ch)
{
	return ch==' '||ch=='\t'||ch=='\n'||ch=='\r';
}
string& trim(const string& in, string& out)
{
	string result="";
	int start=0,stop=0;
	for(start=0;start<static_cast<int>(in.length());start++)
	{
		if(isSpace(in[start]))
			continue;
		else 
			break;
	}
	for(stop=static_cast<int>(in.length())-1;stop>=0;stop--)
	{
		if(isSpace(in[stop]))
			continue;
		else
			break;
	}
	for(int i=start;i<=stop;i++)
	{
		result+=in[i];
	}
	out=result;
	return out;
}

bool lineWithNoRealWords(string line)
{
	stringstream tmp_ss(line.c_str());
	string wrd="";tmp_ss>>wrd;
	return (wrd=="");
}	

string& lowercase(const string& in, string& out)
{
	string result=in;
	for(size_t i=0;i<result.length();i++)
	{
		if(result[i]>='A'&&result[i]<='Z')
			result[i]+='a'-'A';
	}
	out=result;
	return out;
}

string vectorToString(vector<string>& words)
{
	string result="";
	int len=(int)words.size();
	for(int i=0;i<len;i++)
	{
		result+=words[i];
		if(i!=len-1)
			result+=" ";
	}
	return result;
}
/* digamma.c
 *
 * Mark Johnson, 2nd September 2007
 *
 * Computes the 唯(x) or digamma function, i.e., the derivative of the 
 * log gamma function, using a series expansion.
 *
 * Warning:  I'm not a numerical analyst, so I may have made errors here!
 *
 * The parameters of the series were computed using the Maple symbolic
 * algebra program as follows:
 *
 * series(Psi(x+1/2), x=infinity, 21);
 *
 * which produces:
 *
 *  ln(x)+1/(24*x^2)-7/960/x^4+31/8064/x^6-127/30720/x^8+511/67584/x^10-1414477/67092480/x^12+8191/98304/x^14-118518239/267386880/x^16+5749691557/1882718208/x^18-91546277357/3460300800/x^20+O(1/(x^21)) 
 *
 * It looks as if the terms in this expansion *diverge* as the powers
 * get larger.  However, for large x, the x^-n term will dominate.
 *
 * I used Maple to examine the difference between this series and
 * Digamma(x+1/2) over the range 7 < x < 20, and discovered that the
 * difference is less that 1e-8 if the terms up to x^-8 are included.
 * This determined the power used in the code here.  Of course,
 * Maple uses some kind of approximation to calculate Digamma,
 * so all I've really done here is find the smallest power that produces
 * the Maple approximation; still, that should be good enough for our
 * purposes.
 *
 * This expansion is accurate for x > 7; we use the recurrence 
 *
 * digamma(x) = digamma(x+1) - 1/x
 *
 * to make x larger than 7.
 */


double digamma(double x) {
  double result = 0, xx, xx2, xx4;
  if(x<0.5)return x;
//assert(x > 0);
  for ( ; x < 7; ++x)
    result -= 1/x;
  x -= 1.0/2.0;
  xx = 1.0/x;
  xx2 = xx*xx;
  xx4 = xx2*xx2;
  result += log(x)+(1./24.)*xx2-(7.0/960.0)*xx4+(31.0/8064.0)*xx4*xx2-(127.0/30720.0)*xx4*xx4;
  return result;
}

/*
#ifdef TEST
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  double x;
  for (x = 0.1; x < 10; x += 0.1)
    printf("digamma(%g) = %g, exp(digamma(%g)) = %g\n", x, digamma(x), x, exp(digamma(x)));
  exit(EXIT_SUCCESS);
}

#endif  TEST */

