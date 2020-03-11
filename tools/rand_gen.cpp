#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

int elements[10];

const int length = 10;
const int trans = 160000;

int contain(int n)
{
	for(int i = 0; i < 10; i++)
	{
		if(n == elements[i])
		{
			return 1;
		}
	}
	return 0;
}


int main(int argc, char const *argv[])
{
	ofstream fout;
    ostringstream file_name;

    srand(time(NULL));

    for(int k = 0; k < 4; k++)
    {
	    file_name.str(string());
		file_name << "input" << k << ".txt";
		fout.open(file_name.str().c_str(), ios_base::app);

		for(int i = 0; i < 10; i++)
		{
			elements[i] = -1;
		}
		
		
		int n;
		for(int j = 0; j < trans; j++)
		{
			for(int i = 0; i < length; i++)
			{
				n = rand() % 10;
				if(!contain(n))
				{
					fout << n << " ";
					elements[i] = n;
				}
			}
			for(int i = 0; i < 10; i++)
			{
				elements[i] = -1;
			}
			fout << -1 << endl;
		}

		fout.close();
	}

	return 0;
}