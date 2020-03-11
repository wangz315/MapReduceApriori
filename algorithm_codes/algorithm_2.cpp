#include "bits/stdc++.h"
#include <string>
#include <iostream>
#include <sstream>
#include "mpi.h"

using namespace std;

#define structure map<vector<int>, int>
#define FOR_MAP(ii,T) for(structure::iterator (ii)=(T).begin();(ii)!=(T).end();(ii)++)
#define FOR_next_MAP(jj,ii,T) for(structure::iterator (jj)=(ii);(jj)!=(T).end();(jj)++)
#define VI vector<int>

const int MIN_SUP = 5000;
const double RULE = 1.0;

// strings
const char* data_base_name = "input";
const char* tunnel_name = "out_tunnel";

//mined data
structure C;
structure L;
structure R;

// MPI var
int my_rank;
int num_procs;


// prototypes

// mining data from database. C & L is used for each round, and R is the final result of all mined data
void mining();
// gather supports of elements in structure
void gather_result(ifstream &fin);
// mine C1
void C1();
// get L1
void L1();
// mine Cx, x is the round of iteration
void generate_C();
// get L based on C. Has to call generate_C() first
void generate_L();
// scan data base
void scan_D();
void scan_D(ifstream &fin);
// set the count of VI to structure, flag: set count of R
void set_count(VI );
void set_count(VI , int flag);
// prune by apripori
void prune();
// check compatibility
bool check_compatibility(VI ,VI );
// output results to a file
void output(structure , ofstream &fout);


int main(int argc, char const *argv[])
{
	// init MPI
	MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // IO var
    ofstream fout;
	ifstream fin;
    ostringstream file_name;


	C.clear();
	L.clear();
	R.clear();

	double start_time;
    double stop_time;

    if(!my_rank)
    {
    	start_time = MPI_Wtime();
    }
    
	// results in R
	mining();

	// send to server: write data to a txt file as simulation
	file_name.str(string());
	file_name << tunnel_name << my_rank << "_0.txt";
	fout.open(file_name.str().c_str(), ios_base::app);
	output(R, fout);
	fout.close();
	R.clear();

	// wait until all sent
	MPI_Barrier(MPI_COMM_WORLD);


	// server recive
	if(!my_rank)
	{
		int i;

		stop_time = MPI_Wtime();
        printf("\nFirst return time (sec): %f\n", stop_time - start_time);

		// get result from each data base, once an iteration
		for(i = 0; i < num_procs; i++)
		{
			file_name.str(string());
			file_name << tunnel_name << i << "_0.txt";
			fin.open(file_name.str().c_str());
			gather_result(fin);
			fin.close();
		}

		// prune
		structure temp;
		temp.clear();
		FOR_MAP(ii,R)
		{
			if (ii->second >= (MIN_SUP * num_procs * RULE))
			{
				temp[ii->first]=ii->second;
			}
		}

		R.clear();
		R=temp;
		temp.clear();

		// output total result
		file_name.str(string());
		file_name << "out_total.txt";
		fout.open(file_name.str().c_str(), ios_base::app);
		output(R, fout);
		R.clear();
		fout.close();

		stop_time = MPI_Wtime();
        printf("\nTotal time (sec): %f\n", stop_time - start_time);
	}

	MPI_Finalize();

	return 0;
}

void mining()
{
	// first iteration
	bool mv=true;
	int index=2;

	while(true)
	{
		if (mv)
		{
			C1();

			L1();
			
			mv=!mv;
		}
		else
		{
			generate_C();

			if(C.size()==0)
				break;

			prune();

			if (C.size()==0)
			{
				break;
			}

			scan_D();
			generate_L();

			if (L.size()==0)
			{
				break;
			}
			index++;
		}
	}
}

void gather_result(ifstream &fin)
{
	int n;
	VI v;

	while(fin>>n)
	{
		if(n != -1)
		{
			v.push_back(n);
		}
		else
		{
			fin>>n;
			R[v] += n;
			v.clear();
		}
	}
}

void C1()
{
	ifstream fin;
    ostringstream file_name;
	file_name.str(string());
	file_name << data_base_name << my_rank << ".txt";
	fin.open(file_name.str().c_str());

	int n;
	VI v;
	while(fin>>n)
	{
		v.clear();
		if (n==-1)
		{
			continue;
		}
		v.push_back(n);
		if(C.count(v)>0)
			C[v]++;
		else
			C[v]=1;
	}
	fin.close();
}

void L1()
{

	FOR_MAP(ii,C)
	{
		if (ii->second >= MIN_SUP)
		{
			L[ii->first]=ii->second;
			R[ii->first]=ii->second;
		}
	}

}

void generate_C()
{
	C.clear();
	FOR_MAP(ii,L)
	{

		FOR_next_MAP(jj,ii,L)
		{
			if(jj==ii)
				continue;
			VI a,b;
			a.clear();
			b.clear();
			a=ii->first;
			b=jj->first;
			if(check_compatibility(a,b))	
			{
				a.push_back(b.back());
				sort(a.begin(), a.end());
				C[a]=0;
			}
		}

	}
}

void generate_L()
{
	L.clear();

	FOR_MAP(ii,C)
	{
		if(ii->second >= MIN_SUP)
		{
			L[ii->first]=ii->second;
			R[ii->first]=ii->second;
		}
	}
}

void scan_D()
{
	ifstream fin;
	ostringstream file_name;
	file_name << "input" << my_rank << ".txt";
	fin.open(file_name.str().c_str());

	int n;
	VI a;
	while(fin>>n)
	{
		if(n==-1 && a.size()>0)
		{
			set_count(a);
			a.clear();
		}else if(n!=-1)
		{
			a.push_back(n);
		}
		
	}

	fin.close();
}

void scan_D(ifstream &fin)
{
	int n;
	VI a;
	while(fin>>n)
	{
		if(n==-1 && a.size()>0)
		{
			set_count(a, 0);
			a.clear();
		}else if(n!=-1)
		{
			a.push_back(n);
		}
		
	}
}

void set_count(VI a)
{
	FOR_MAP(ii,C)
	{
		VI b;
		b.clear();
		b=ii->first;
		int true_count=0;
		if (b.size()<=a.size())
		{
			for (int i = 0; i < (int)b.size(); ++i)
			{
				for (int j = 0; j < (int)a.size(); ++j)
				{
					if(b[i]==a[j])
					{
						true_count++;
						break;
					}
				}
			}
		}

		if (true_count==(int)b.size())
		{
			ii->second++;
		}
	}
}

void set_count(VI a, int n)
{
	FOR_MAP(ii,R)
	{
		VI b;
		b.clear();
		b=ii->first;
		int true_count=0;
		if (b.size()<=a.size())
		{
			for (int i = 0; i < (int)b.size(); ++i)
			{
				for (int j = 0; j < (int)a.size(); ++j)
				{
					if(b[i]==a[j])
					{
						true_count++;
						break;
					}
				}
			}
		}

		if (true_count==(int)b.size())
		{
			ii->second++;
		}
	}
}

void prune()
{
	VI a,b;
	
	FOR_MAP(ii,C)
	{
		a.clear();
		b.clear();

		a=ii->first;
		for(int i = 0;i<(int)a.size();i++)
		{
			b.clear();
			for (int j = 0; j < (int)a.size(); ++j)
			{
				if(j==i)
					continue;
				b.push_back(a[j]);
			}
			if(L.find(b)==L.end())
				{
					ii->second=-1;
					break;
				}
			
		}

		
	}

	structure temp;
	temp.clear();
	FOR_MAP(ii,C)
	{
		if (ii->second != -1)
		{
			temp[ii->first]=ii->second;
		}
	}
	
	C.clear();
	C=temp;
	temp.clear();
}

bool check_compatibility(VI a,VI b)
{
	bool compatible=true;
	for (int i = 0; i < (int)a.size()-1; ++i)
	{
		if (a[i]!=b[i])
		{
			compatible=false;
			break;
		}
	}

	return compatible;
}

void output(structure T, ofstream &fout)
{
	fout<<"\n";
	VI v;
	FOR_MAP(ii,T)
	{
		v.clear();
		v=ii->first;
		for (int i = 0; i < (int)v.size(); ++i)
		{
			fout<<v[i]<<" ";
		}

		// -1 used as spliter
		fout<<" -1 "<<ii->second;
		fout<<"\n";

	}

}









