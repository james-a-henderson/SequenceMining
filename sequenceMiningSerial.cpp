//To compile: g++ -o3 -w sequenceMiningSerial.cpp -lpthread -o sequenceMiningSerial
//To run: ./sequenceMining <name of file> <minimum support>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <fstream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <vector>


#include "timer.h"

using namespace std;

typedef vector< vector<int> > TwoDemensionalVector;
typedef vector<int> int_vec;
typedef pair<int,int> int_pair;
typedef vector<int_pair> pair_vec;

typedef map<vector<int>, pair_vec> sequence_map;
typedef vector<pair<vector<int>,pair_vec> > sequence_vec;

//global variables
TwoDemensionalVector inputs; //number of inputs
int minSupport;	//number of lines a sequence must appear in for it to be significant
vector <int> significantValues; //values that can be added to a sequence
sequence_vec sequenceList; //list of sequences
sequence_vec currentSequences; //list of sequences that have been recently added to

//functions
void printInput(TwoDemensionalVector sequence);
void getInitalSequence();
void getAdditionalSequences();
int countNumLines(pair_vec);
void Get_args(int argc,char* argv[]);
void Usage(char* prog_name);

int main(int argc, char* argv[])
{
	
	Get_args(argc, argv);

	ifstream infile;
	
	//get input from file
	string line;
	
	//read input
	while(getline(infile, line))
	{
		vector<int> row;
		istringstream iss(line);
		int n;
		while(iss >> n)
		{
			row.push_back(n);
		}

		inputs.push_back(row);
	}

	infile.close();
	
	//output input
	//for debug purposes
    //printInput(inputs);

	double start, finish, elapsed;
	//get the initial sequence
	getInitalSequence();
	cout << "initial sequence done" << endl;
	
	GET_TIME(start);
    getAdditionalSequences();
	GET_TIME(finish);
	elapsed = finish - start;
	cout << "Output:\n";
	for(sequence_vec::iterator it = sequenceList.begin(); it != sequenceList.end(); ++it)
	{
		for(int i = 0; i < it->first.size(); i++)
		{
			cout << it->first[i] << ' ';
		}
		cout << '\t';

		double appearedPercent = (double)countNumLines(it->second) / (double) inputs.size();
		cout << (appearedPercent * 100) << "%";
		cout << endl;
	}

	printf("Elapsed time: %f \n", elapsed);
}

//prints a two demensional vector
//for testing purposes
void printInput(TwoDemensionalVector sequence)
{
	cout << "Input:\n";
	for(int i = 0; i < sequence.size(); i++)
	{
		for(int j = 0; j < sequence[i].size(); j++)
			cout << sequence[i][j] << " ";

		cout << endl;
	}
	cout << endl;
}

//gets the first sequence
//initial sequence only has a length of one
void getInitalSequence()
{
	//vector containing the initial sequences
	sequence_map fSeq;

	for(int line = 0; line < inputs.size(); line++)
	{
		for(int index = 0; index < inputs[line].size(); index++)
		{
			//make a vector of ints containing the sequence
			//sequence must be in vector to use with other code, even though it only has a length of one
			vector<int> value;
			value.push_back(inputs[line][index]);

			fSeq[value].push_back(make_pair(line,index));
		}
	}	
	
	//delete sequences not in a significant number of lines
	//if a sequence is in a significant number of lines, add it to significantValues
	for(sequence_map::iterator it = fSeq.begin(); it != fSeq.end(); ++it)
	{
	    if(countNumLines(it->second) >= minSupport)
		{
			sequenceList.push_back(make_pair(it->first,it->second));
			currentSequences.push_back(make_pair(it->first,it->second));
			significantValues.push_back(it->first[0]);
		}
	}

	sort(significantValues.begin(), significantValues.end());
}

void getAdditionalSequences()
{
	bool newSequencesAdded = true;
	while(newSequencesAdded)
	{
		sequence_map fSeq;

		newSequencesAdded = false;
		
		for(sequence_vec::iterator it = currentSequences.begin(); it != currentSequences.end(); ++it)
		{
			for(pair_vec::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			{
				
				int line = it2->first;
				int index = it2->second;

				for(int i = index + 1; i < inputs[line].size(); i++)
				{
					int num = inputs[line][i];
					if(binary_search(significantValues.begin(), significantValues.end(), num))
					{
						vector<int> values = it->first;
						values.push_back(num);
						fSeq[values].push_back(make_pair(line,i));
					}
				}
			}
		}
		sequence_vec newCurrent;
		
		for(sequence_map::iterator it = fSeq.begin(); it != fSeq.end(); ++it)
		{
			
			if(countNumLines(it->second) >= minSupport)
			{
				pair<vector<int>,pair_vec> newPair(it->first,it->second);
				newCurrent.push_back(newPair);
				sequenceList.push_back(newPair);
				
				newSequencesAdded = true;
			}
		}
		currentSequences = newCurrent;
	}
}

/*
  returns the number of lines in the pair_vec

 */
int countNumLines(pair_vec pv)
{
	vector<int> intVec;
	for(pair_vec::iterator it = pv.begin(); it != pv.end(); ++it)
	{
		intVec.push_back(it->first);
	}

	sort(intVec.begin(), intVec.end());
	vector<int>::iterator unique_end = unique(intVec.begin(), intVec.end());
	return abs(distance(unique_end, intVec.begin()));
}

/*
  get command line arguments

 */
void Get_args(int argc, char* argv[])
{
	if(argc != 3)
		Usage(argv[0]);

	string fileName = argv[1];
	infile.open(fileName.c_str(),ifstream::in);
	minSupport = strtol(argv[2], NULL, 10);
}

void Usage(char* prog_name)
{
	fprintf(stderr, "usage: %s <name of file> <minimum support>\n", prog_name);
	exit(0);
}
