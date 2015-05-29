//To compile: g++ -o3 -w sequenceMining.cpp -lpthread -o sequenceMining
//To run: ./sequenceMining <number of threads> <name of file> <minimum support>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <fstream>
#include <map>
#include <pthread.h>
#include <queue>
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
ifstream infile; //input file
long lastLine; //line most recently processed

int thread_count;
pthread_mutex_t mutex_Queue;

queue<long> availableThreads;

long * lineToProcess;

//functions
void printInput(TwoDemensionalVector sequence);
void printOutput();
void getInitalSequence();
void getAdditionalSequences();
void* pth_getAdditionalSequences(void* rank);
int countNumLines(pair_vec);
void Get_args(int argc,char* argv[]);
void Usage(char* prog_name);


int main(int argc, char* argv[])
{
	//pthreads variables
	long thread;
	pthread_t* thread_handles;

	lastLine = -1;
	
	Get_args(argc, argv);
	
	thread_handles = (pthread_t*) malloc (thread_count*sizeof(pthread_t));
	pthread_mutex_init(&mutex_Queue, NULL);
	
	lineToProcess = new long [thread_count];
	fill_n(lineToProcess, thread_count, -1);

	
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
//	printInput(inputs);

	double start, finish, elapsed;
	getInitalSequence();
	cout << "initial sequence done" << endl;
	
	GET_TIME(start);

	//initialize semaphores and add available threads
	for(thread = 0; thread < thread_count; thread++)
	{
		availableThreads.push(thread);
	}
	
	for(thread = 0; thread < thread_count; thread++)
		pthread_create(&thread_handles[thread], NULL, pth_getAdditionalSequences, (void*) thread);

	while(true)
	{
		//there are no active threads
		if(availableThreads.size() == thread_count)
		{
			//there are no more lines to process
			if(lastLine == sequenceList.size() - 1)
			{
				for(long i = 0; i < thread_count; i++)
				{
					lineToProcess[i] = -2;
				}
				break;
			}
			else
			{
				long newThread = availableThreads.front();
				availableThreads.pop();
				lineToProcess[newThread] = ++lastLine;
			}
		}
		else
		{
			pthread_mutex_lock(&mutex_Queue);
			if(!availableThreads.empty() && lastLine != sequenceList.size() - 1)
			{
				long newThread = availableThreads.front();
				availableThreads.pop();
				lineToProcess[newThread] = ++lastLine;
			}
			pthread_mutex_unlock(&mutex_Queue);
		}
	}
	
	for(thread = 0; thread < thread_count; thread++)
		pthread_join(thread_handles[thread], NULL);
	GET_TIME(finish);
	elapsed = finish - start;
	cout << "Output:\n";
	
	printOutput();

	printf("Elapsed time: %f \n", elapsed);
	delete [] lineToProcess;
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

//prints the final list of sequences
void printOutput()
{
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

			//int iValue = inputs[line][index];

			//fSeq[value].insert(make_pair(line, index));
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
			significantValues.push_back(it->first[0]);
		}
	}

	sort(significantValues.begin(), significantValues.end());
}

void* pth_getAdditionalSequences(void* rank)
{
	long my_rank = (long) rank;
	
	while(true)
	{
		sequence_map fSeq;
		int sequenceNum;

		//wait until assigned a sequence
		while(lineToProcess[my_rank] == -1);

		sequenceNum = lineToProcess[my_rank];
		
		//end thread
		if(sequenceNum == -2)
			break;

		pair<vector<int>,pair_vec> thisSequence = sequenceList.at(sequenceNum);		
		
		//map sequences
		for(int j = 0; j < thisSequence.second.size(); j++)
		{
			//cout << my_rank << ' ' << sequenceNum << " yo" << endl;
			int line = thisSequence.second.at(j).first;
			int index = thisSequence.second.at(j).second;

			//cout << my_rank << ' ' << sequenceNum << ' ' << line << endl;
			
			for(int i = index + 1; i < inputs.at(line).size(); i++)
			{
				int num = inputs[line][i];
				if(binary_search(significantValues.begin(), significantValues.end(), num))
				{
					vector<int> values = sequenceList[sequenceNum].first;
					values.push_back(num);
					//fSeq[values].insert(make_pair(line, i));
					fSeq[values].push_back(make_pair(line,i));
				}
			}
		}

		sequence_vec newCurrent;

		
		//determine if sequences appear a significant number of times
		for(sequence_map::iterator it = fSeq.begin(); it != fSeq.end(); ++it)
		{
			
			if(countNumLines(it->second) >= minSupport)
			{
				pair<vector<int>,pair_vec> newPair(it->first,it->second);
				newCurrent.push_back(newPair);
			}
		}
		//cout << "Hi" << endl;

		lineToProcess[my_rank] = -1;
		
		pthread_mutex_lock(&mutex_Queue);
		sequenceList.insert(sequenceList.end(), newCurrent.begin(), newCurrent.end());
		availableThreads.push(my_rank);
		pthread_mutex_unlock(&mutex_Queue);
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
	if(argc != 4)
		Usage(argv[0]);

	thread_count = strtol(argv[1], NULL, 10);
	string fileName = argv[2];
	infile.open(fileName.c_str(),ifstream::in);
	minSupport = strtol(argv[3], NULL, 10);
}

void Usage(char* prog_name)
{
	fprintf(stderr, "usage: %s <number of threads> <name of file> <minimum support>\n", prog_name);
	exit(0);
}
