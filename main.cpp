#include "utils.h"
#include "Alignment.h"
#include "JKArgs.h"
#include "PhraseTable.h"
#include "Decode.h"

#ifdef WIN32
#include <Windows.h>
#include <direct.h>
#define CHDIR _chdir
#define GETCWD _getcwd
#else
#include <unistd.h>
#define CHDIR chdir
#define GETCWD getcwd
#endif

void usage()
{
	cerr<<R"(
        pbmt -extractPhrase -src=[sourcefile] -tar=[targetfile] -align=[alignment] [-s2tLex=file]
            [-t2sLex=file] [-w=weightfile]
            [-normalize=true|false] [-reverse] [-srcLengthLimit=int] [-tarLengthLimit=int]
            [-maxNumOfUnAlignedWords=int]  [-printTopEntrys=int] [-onlyonefeat] [-threshold=double] [moore=bool]
        pbmt -mtu -src=[sourcefile] -tar=[targetfile] -align=[alignment] -o=[outputfile] [-ld=S2T|T2S]
        pbmt -makeLexDic -src=[sourcefile] -tar=[targetfile] -align=[alignment] [-w=weightfile] 
            [-s2t=filename] [-t2s=filename]
        pbmt -decode -pt=[phrase table file] -dmax=[max distortion limit] -beam=[beam size] -weights=[d:d:d...]
        pbmt -qsubMFT -curDir=dir -stepLength=int -nStep=int -dConfig=trainConfig -o=ruleFile
            [-walltime=time] [-pmem=mem] [-queue=node]
        pbmt -rule_refinement -i=raw_rules
        pbmt -reprintLexDic -i=file
        pbmt -mtu -src=[sourcefile] -tar=[targetfile] -align=[alignment] -o=[output] [-maxWordSpan=3]

)";
	exit(1);
}

typedef struct{
	int ind;
	double score;
}EntryInfo;

void rule_refinement(JKArgs& args)
{
	if(!args.is_set("i"))usage();
	ifstream is(args.value("i").c_str());

	map<string,EntryInfo> srcPhrases,tarPhrases;
	while(is.good())
	{
		string curline="";
		getline(is,curline);
		if(curline=="")break;
		PhraseRuleEntry pre;
		pre.read(curline);
		if(srcPhrases.find(pre.srcPhrase)==srcPhrases.end())
		{
			EntryInfo ei;
			ei.ind=(int)srcPhrases.size();
			ei.score=pre.s2tScore;
			srcPhrases[pre.srcPhrase]=ei;
		}
		else
			srcPhrases[pre.srcPhrase].score+=pre.s2tScore;

		if(tarPhrases.find(pre.tarPhrase)==tarPhrases.end())
		{
			EntryInfo ei;
			ei.ind=(int)tarPhrases.size();
			ei.score=pre.s2tScore;
			tarPhrases[pre.tarPhrase]=ei;
		}
		else
			tarPhrases[pre.tarPhrase].score+=pre.s2tScore;
	}
	is.clear();
	is.seekg(0,ios::beg);
	PhraseTable pt;
	while(is.good())
	{
		string curline="";
		getline(is,curline);
		if(curline==""){pt.print(cout);break;}
		PhraseRuleEntry pre;
		pre.read(curline);
		pre.s2tScore/=srcPhrases[pre.srcPhrase].score;
		pre.t2sScore/=tarPhrases[pre.tarPhrase].score;
		
		if(pt.size()>0&&pt.data().find(pre.srcPhrase)==pt.data().end())
		{
			pt.print(cout);
			pt.clear();
		}
		pt.add(pre.srcPhrase,pre.tarPhrase,pre);
	}
}

void makeLexDic(string srcfilename,string tarfilename, string alignfilename, string weightFilename, Dic<double>& s2tLexDic, Dic<double>& t2sLexDic)
{
	ifstream fsrc(srcfilename.c_str()),ftar(tarfilename.c_str()),falign(alignfilename.c_str()),fweight(weightFilename.c_str());
	
	while(!falign.eof()&&!fsrc.eof()&&!ftar.eof())
	{
		string align="",src="",tar="";
		getline(falign,align);
		getline(fsrc,src);
		getline(ftar,tar);
		double fraccount=1;
		if(fweight.good())fweight>>fraccount;

		vector<pair<int,int> > alignment;
		vector<string> srcSent,tarSent;
		string2alignment(align,alignment);
		stringToVector(src,srcSent);
		stringToVector(tar,tarSent);
		map<int, int> alignedSrcPos,alignedTarPos;
		for(int i=0;i<(int)alignment.size();i++)
		{
			pair<int,int> a=alignment[i];
			string srcWord=srcSent[a.first];
			string tarWord=tarSent[a.second];
			s2tLexDic.add(srcWord,tarWord,fraccount);
			t2sLexDic.add(tarWord,srcWord,fraccount);
			alignedSrcPos[a.first];
			alignedTarPos[a.second];
		}
		for(int i=0;i<(int)srcSent.size();i++)
		{
			if(alignedSrcPos.find(i)==alignedSrcPos.end())
				t2sLexDic.add("NULL",srcSent[i],fraccount);
		}
		for(int i=0;i<(int)tarSent.size();i++)
		{
			if(alignedTarPos.find(i)==alignedTarPos.end())
				s2tLexDic.add("NULL",tarSent[i],fraccount);
		}
	}
	s2tLexDic.normalize();
	t2sLexDic.normalize();
	falign.close();
	fsrc.close();
	ftar.close();
}

void makeLexDic(JKArgs& args)
{
	if(!args.is_set("src")||!args.is_set("tar")||!args.is_set("align"))
		usage();
	string srcfilename=args.value("src");
	string tarfilename=args.value("tar");
	string alignfilename=args.value("align");
	string s2tDicName="s2t.lex";
	string t2sDicName="t2s.lex";
	string weightFilename="";
	if(args.is_set("w"))weightFilename=args.value("w");
	if(args.is_set("s2t"))
		s2tDicName=args.value("s2t");
	if(args.is_set("t2s"))
		t2sDicName=args.value("t2s");

	ofstream s2t(s2tDicName.c_str());
	ofstream t2s(t2sDicName.c_str());
	Dic<double> s2tLexDic;
	Dic<double> t2sLexDic;
	makeLexDic(srcfilename,tarfilename,alignfilename,weightFilename,s2tLexDic,t2sLexDic);
	s2tLexDic.print(s2t);
	t2sLexDic.print(t2s);
	s2t.close();
	t2s.close();
}

void extractPhrase(JKArgs& args){
	if(!args.is_set("src")||!args.is_set("tar")||!args.is_set("align"))
		usage();
	ifstream fsrc(args.value("src").c_str()),ftar(args.value("tar").c_str()),falign(args.value("align").c_str());
	ifstream fweight;
	ofstream os;
	double threshold=0;
	if(args.is_set("threshold"))threshold=atof(args.value("threshold").c_str());
	if(args.is_set("o"))os.open(args.value("o").c_str());
	if(args.is_set("w"))fweight.open(args.value("w").c_str());
	Dic<double> s2tLexDic,t2sLexDic;
	if(!args.is_set("s2tLex")||!args.is_set("t2sLex"))
	{
		cerr<<"making lex dic on the fly"<<endl;
		makeLexDic(args.value("src"),args.value("tar"),args.value("align"),args.value("w"),s2tLexDic,t2sLexDic);
	}
	else
	{
		s2tLexDic.load(args.value("s2tLex"));
		t2sLexDic.load(args.value("t2sLex"));
	}
	bool reverse=false,normalize=false;
	bool moore=false;
	if(args.is_set("moore"))if(args.value("moore")=="true")moore=true;
	if(args.is_set("reverse"))
		reverse=true;
	if(args.is_set("normalize"))
		if(args.value("normalize")=="true")
			normalize=true;
	int srcLengthLimit=8,tarLengthLimit=12;
	if(args.is_set("srcLengthLimit"))
		srcLengthLimit=atoi(args.value("srcLengthLimit").c_str());
	if(args.is_set("tarLengthLimit"))
		tarLengthLimit=atoi(args.value("tarLengthLimit").c_str());
	int maxNumOfUnAlignedWords=2;
	if(args.is_set("maxNumOfUnAlignedWords"))
		maxNumOfUnAlignedWords=atoi(args.value("maxNumOfUnAlignedWords").c_str());
	double start=1,stop=1E10;
	if(args.is_set("range"))
	{
		string range=args.value("range");
		start=atoi(range.substr(0,range.find("-")).c_str());
		stop=atof(range.substr(range.find("-")+1).c_str());
		cerr<<"start:"<<start<<",stop:"<<stop<<endl;
	}
	PhraseTable phraseTable;
	int count=0;
	while(!falign.eof())
	{
		string align="",src="",tar="";
		double weight=1;
		getline(falign,align);
		getline(fsrc,src);
		getline(ftar,tar);
		if(fweight.good())fweight>>weight;
		
		//Alignment alignment(align,maxNumOfUnAlignedWords);

		count++;
		if(count<start)continue;
		if(count>stop)break;
		if(weight==0||weight<threshold)continue;
		if(moore)weight=1;
		vector<PhraseRuleEntry> phrases;
		phraseExtractor(src,tar,align,s2tLexDic,t2sLexDic,phrases,srcLengthLimit,tarLengthLimit,maxNumOfUnAlignedWords,weight);
		for(size_t i=0;i<phrases.size();i++)
		{
			PhraseRuleEntry& pre=phrases[i];
			if(reverse)
			{
				string tmp=pre.srcPhrase;
				pre.srcPhrase=pre.tarPhrase;
				pre.tarPhrase=tmp;
			}
			if(os.good())phraseTable.add(pre.srcPhrase,pre.tarPhrase,pre);
		}
	}
	if(args.is_set("printTopEntrys"))
	{
		int topN=atoi(args.value("printTopEntrys").c_str());
		phraseTable.printTopEntrys(topN,"topEntrys.txt");
	}
	if(normalize)
		phraseTable.normalize();
	bool onlyonefeat=false;
	if(args.is_set("onlyonefeat"))
		onlyonefeat=true;
	if(os.good())phraseTable.print(os,onlyonefeat);
	string signal="mftsignal";
	if(args.is_set("signal"))signal=args.value("signal");
	ofstream o_sg(signal.c_str());
	o_sg.close();
}

string revertAlign(string& align){
    replaceSubStr(align,"-"," ");
    stringstream ss(align);
    string result="";
    while(ss.good()){
        int i=-1;
        int j=-1;
        ss>>i>>j;
        if(i>=0&&j>=0)
            result+=to_string(j)+"-"+to_string(i)+" ";
    }
    return result;
}

void mtu(JKArgs& args){
	if(!args.is_set("src")||!args.is_set("tar")||!args.is_set("align")||!args.is_set("o"))
		usage();
	ofstream os(args.value("o"));
    ifstream fsrc(args.value("src").c_str()),ftar(args.value("tar").c_str()),falign(args.value("align").c_str());
    LangDirection ld=S2T;
    if(args.value("ld")=="T2S")ld=T2S;
	double start=1,stop=1E10;
    int maxWordSpan=3;
    if(args.is_set("maxWordSpan"))maxWordSpan=atoi(args.value("maxWordSpan").c_str());
	if(args.is_set("range"))
	{
		string range=args.value("range");
		start=atoi(range.substr(0,range.find("-")).c_str());
		stop=atof(range.substr(range.find("-")+1).c_str());
		cerr<<"start:"<<start<<",stop:"<<stop<<endl;
	}
    int count=0;
	while(!falign.eof())
	{
		string align="",src="",tar="";
		getline(falign,align);
		getline(fsrc,src);
		getline(ftar,tar);

		//Alignment alignment(align,maxNumOfUnAlignedWords);
        if(ld==T2S){align=revertAlign(align);swap(src,tar);}
		count++;
		if(count<start)continue;
		if(count>stop)break;
        vector<string> mtus;
        mtuExtractor(src,tar,align,mtus,maxWordSpan);
		for(auto& m:mtus){
            cout<<m<<" ";
		}
        cout<<endl;
	}
	os.close();
	string signal="mftsignal";
	if(args.is_set("signal"))signal=args.value("signal");
	ofstream o_sg(signal.c_str());
	o_sg.close();
}

void qsubMFT(JKArgs& args)
{
	//"jkparser -qsubMFT -curDir=dir -stepLength=int -nStep=int -dConfig=trainConfig -o=ruleFile [-walltime=time] [-pmem=mem] [-queue=node]"
	string curDir="";
	int stepLength=5000;
	int nStep=6;
	string dConfig="";
	int sleepTime=5;

	if(!args.is_set("curDir")||!args.is_set("stepLength")||
		!args.is_set("nStep")||!args.is_set("dConfig")||!args.is_set("o"))
		usage();
	curDir=args.value("curDir");
	stepLength=atoi(args.value("stepLength").c_str());
	nStep=atoi(args.value("nStep").c_str());
	dConfig=args.value("dConfig");
	string output=args.value("o");
	int dlevel=atoi(args.value("debug").c_str());
	if(args.is_set("sleepTime"))sleepTime=atoi(args.value("sleepTime").c_str());

	JKArgs dArgs;
	dArgs.init(dConfig.c_str());

	string walltime="20:00:00";
	string pmem="7g";
	string queue="isi";
	if(args.is_set("walltime"))
		walltime=args.value("walltime");
	if(args.is_set("pmem"))
		pmem=args.value("pmem");
	if(args.is_set("queue"))
		queue=args.value("queue");

	cerr<<"walltime="<<walltime<<endl;

	string signal="mftsignal";
	if(dArgs.is_set("signal"))signal=dArgs.value("signal");

	string logerror=output+"-logerror";
	vector<string> v_signal,v_logerror,v_log,v_sh;

	CHDIR(curDir.c_str());

	string cmd="";
	cmd="cd "+curDir;
	if(dlevel)cerr<<cmd<<endl;
	else system(cmd.c_str());

	for(int iter=0;iter<nStep;iter++)
	{
		int start=stepLength*iter+1;
		int stop=start+stepLength-1;
		string range=intToString(start)+"-"+intToString(stop);
		string n_signal=signal+"."+range;
		v_signal.push_back(n_signal);

		string n_logerror=logerror+"."+range;
		string n_log=output+"."+range;
		v_logerror.push_back(n_logerror);
		v_log.push_back(n_log);

		string fd_cmd="( ./pbmt -config="+dConfig+" -range="+range+" -signal="+n_signal;
		fd_cmd+=" > "+n_log+" ) >& "+n_logerror;

		string fd_sh=curDir+"/mft."+range+".sh";
		v_sh.push_back(fd_sh);
		cmd="echo cd "+curDir+" > "+fd_sh;
		if(dlevel)cerr<<cmd<<endl;
		else system(cmd.c_str());
		
		cmd="echo \""+fd_cmd+" \" >> "+fd_sh;
		if(dlevel)cerr<<cmd<<endl;
		else system(cmd.c_str());
		
		cmd="qsub -l walltime="+walltime+" -l pmem="+pmem+" -q "+queue+" "+fd_sh;
		if(dlevel)cerr<<cmd<<endl;
		else system(cmd.c_str());
	}

	while(true)
	{
		bool allDone=true;
		for(int iter=0;iter<nStep;iter++)
		{
			ifstream signalFile(v_signal[iter].c_str());
			if(!signalFile.good())
				allDone=false;
			signalFile.close();
		}
		if(allDone)
			break;
#ifndef WIN32
		sleep(sleepTime);
#else
		Sleep(sleepTime*100);
#endif
	}

	cmd="rm "+output+" "+logerror;
	if(dlevel)cerr<<cmd<<endl;
	else system(cmd.c_str());
	
	cmd="cat "+vectorToString(v_log)+" > "+output;
	if(dlevel)cerr<<cmd<<endl;
	else system(cmd.c_str());
	
	cmd="cat "+vectorToString(v_logerror)+" > "+logerror;
	if(dlevel)cerr<<cmd<<endl;
	else system(cmd.c_str());

	string debugInfo=output+"-debugInfo";
	cmd="mkdir "+debugInfo;
	if(dlevel)cerr<<cmd<<endl;
	else system(cmd.c_str());
	cmd="mv "+vectorToString(v_log)+" "+vectorToString(v_logerror)+" "+vectorToString(v_sh)+" "+vectorToString(v_signal)\
		+" *sh.e* *sh.o* "+debugInfo;
	if(dlevel)cerr<<cmd<<endl;
	else system(cmd.c_str());
}

void decode(JKArgs& args)
{
	//pbmt -decode -pt=[phrase table file] -dmax=[max distortion limit] -beam=[beam size] -weights=[d:d:d...]"<<endl;
	if(!args.is_set("pt"))usage();
	
	//pt.print(cout);
	int dmax=4;
	int beamsize=100;
	Features weights;
	weights.s2t=weights.t2s=weights.s2tLex=weights.t2sLex=weights.length=weights.distort=0;
	weights.s2t=1;
	weights.distort=1;

	if(args.is_set("dmax"))dmax=atoi(args.value("dmax").c_str());
	if(args.is_set("beam"))beamsize=atoi(args.value("beam").c_str());
	if(args.is_set("weights"))weights.init(args.value("weights"));
	
	bool debug=false;
	int tlimit=10;
	if(args.is_set("tlimit"))tlimit=atoi(args.value("tlimit").c_str());
	if(args.is_set("debug")) debug=true;
	PhraseTable pt;
	pt.load(args.value("pt"));
	pt.logrize(10);
	pt.makeVector();
	pt.sortVector(weights);

	while(!cin.eof())
	{
		string curline="";
		getline(cin,curline);
		if(curline=="")continue;
		SearchSpace space;
		space.init(curline);
		space.setupAuxSpace(pt,weights);
		space.copeUNK(pt);
		space.beamSearch(pt,weights,beamsize,dmax,tlimit,debug);
		cout<<space.getNthTranslation(1)<<endl;
	}
}
void reprintLexDic(JKArgs& args)
{
	if(!args.is_set("i"))usage();
	Dic<double> s2tLexDic;
	s2tLexDic.load(args.value("i"));
	s2tLexDic.print(cout);
}

int main(int argc, char** argv)
{
	JKArgs args(argc,argv);
	if(args.is_set("config"))args.init(args.value("config").c_str());
	args.init(argc,argv);

	if(args.is_set("extractPhrase"))
		extractPhrase(args);
    else if(args.is_set("mtu"))
		mtu(args);
	else if(args.is_set("makeLexDic"))
		makeLexDic(args);
	else if(args.is_set("decode"))
		decode(args);
	else if(args.is_set("qsubMFT"))
		qsubMFT(args);
	else if(args.is_set("rule_refinement"))
		rule_refinement(args);
	else if(args.is_set("reprintLexDic"))
		reprintLexDic(args);
	else
		usage();
}

void reformArgs(int argc, char* argv[], vector<string>& lines)
{
	lines.clear();
	for(int i=1;i<argc;i++)
	{
		if(argv[i][0]=='-')
			lines.push_back(argv[i]+1);
		else
		{
			lines.back()+="=";
			lines.back()+=argv[i];
		}
	}
//for(size_t i=0;i<lines.size();i++)
//		cerr<<lines[i]<<endl;
}
/*
int main(int argc, char** argv)
{
	vector<string> argsInLines;
	reformArgs(argc,argv,argsInLines);
	
	JKArgs args;
	args.init(argsInLines);
//	args.init(argc,argv);
	decode(args);
}
*/
