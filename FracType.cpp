#include "FracType.h"

double logAdd(double logx, double logy)
{
	if(abs(logx)>308)return logy;
	if(abs(logy)>308)return logx;
	return logx+log1p(exp(logy-logx));
}

double discountMass(FracType& ft, vector<double>& discount)
{
	//if(abs(ft[0]-1)<1E-30)return 0;
	double mass=0;
	double n3=1-ft[0]-ft[1]-ft[2];
	if(n3<0)n3=ft[3];
	mass=discount[0]*ft[1]+discount[1]*ft[2]+discount[2]*n3;
	return mass;
}

int 
FracType::
compare(FracType& f)
{
	for(int i=0;i<5;i++)
	{
		if(abs((*this)[i]-f[i])>1E-10)
		{
			cerr<<(*this)[i]<<"!="<<f[i]<<endl;
			return i;
		}
	}
	return -1;
}

void 
FracType::
update(double p)
{
	FracType& t=*this;
	while(p>1)
	{
		update(1);
		p-=1;
	}
	if(p==0)return;
	if(t.init)
	{
		t[0]=1-p;
		t[1]=p;
		t.init=false;
	}
	else
	{
		for(int i=4;i>0;i--)
			t[i]=t[i-1]*p+t[i]*(1-p);
		t[0]*=(1-p);
	}
}

void 
FracType::
changeLogToReal()
{
    //cerr<<"fractype:"<<endl;
	for(int i=0;i<5;i++)
	{
		if((*this)[i]<-308)(*this)[i]=0;
		else (*this)[i]=exp((*this)[i]);
        //cerr<<(*this)[i]<<" ";
	}
    //cerr<<endl;
}

void 
FracType::
updateLog(double p)
{
	//update(p);
	//return;
	FracType& t=*this;
	while(p>1)
	{
		updateLog(1);
		p-=1;
	}
	if(p==0)return;
	if(p==1)
	{
		if(t.init)
		{
			t[0]=-1E5;
			t[1]=0;
			t.init=false;
		}
		else
		{
			for(int i=4;i>0;i--)
				t[i]=t[i-1];
			t[0]=-1E5;
		}
	}
	else
	{
		if(t.init)
		{
			t[0]=log1p(-p);
			t[1]=log(p);
			t.init=false;
		}
		else
		{
			for(int i=4;i>0;i--)
				t[i]=logAdd(t[i-1]+log(p),t[i]+log1p(-p));
			t[0]+=log1p(-p);
		}
	}
}

void
calculateDiscount(vector<double>& coc, vector<double>& discount)
{
	discount.clear();
	double n1=coc[0];
	double n2=coc[1];
	double n3=coc[2];
	double n4=coc[3];
	double Y=n1/(n1+2*n2);
	discount.push_back(1-2*Y*n2/n1);
	discount.push_back(2-3*Y*n3/n2);
	discount.push_back(3-4*Y*n4/n3);
	discount.push_back(3-4*Y*n4/n3);
    for(size_t i=0;i<discount.size();i++)
	{
        cerr<<"discount "<<i<<":"<<discount[i]<<endl;
        if(discount[i]<0&&i>0)
        {
			discount[i]=discount[i-1];
		}
	}
}
