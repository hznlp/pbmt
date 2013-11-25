#ifndef FRACTYPE_H
#define FRACTYPE_H
#include <iostream>
#include <array>
#include <cmath>
#include <vector>
#define MAX_DISCOUNT_ORDER 5
extern int _debug;
using namespace std;

class FracType : public array<double,5>
{
public:	
	FracType(){resetToLog();}
	~FracType(){init=true;}
	void reset(){fill(0.0);init=true;}
	void resetToLog(){fill(-1E5);init=true;}
	void changeLogToReal();
	void print(ostream& os){for(int i=0;i<5;i++)os<<(*this)[i]<<"\t";os<<endl;}
	void printExp(ostream& os){for(int i=0;i<5;i++)os<<exp((*this)[i])<<"\t";os<<endl;}
	void update(double p);
	void updateLog(double p);
	int compare(FracType& f);
	bool init;
};
double discountMass(FracType& ft, vector<double>& discount);
void calculateDiscount(vector<double>& coc, vector<double>& discount);
#endif
