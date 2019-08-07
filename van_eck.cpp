#include <ctime>
#include <iomanip>
#include <iostream>
#include <vector>
#include <mpi.h>
#include <stdlib.h>
#include <cstdlib>

using namespace std;

int main ( int argc, char *argv[] );
int searchSeq ( vector<int> seq, int val , int argc, char *argv[] );
int searchFindings ( vector<int> finds );

bool valAppear ( vector<int> unique, int val, int arc, char *argv[] );

//TODO make unique array sorted in ascending order so i can concurrent binary search

int main ( int argc, char *argv[] ){
	
	int n, length = atoi(argv[1]), endVal, nextElement, ierr = 0, id;
	double lastNumSearchTime = 0, uniqueSearchTime = 0, timeHolder;
	vector<int> sequence;
	vector<int> unique;
			
	ierr = MPI_Init ( &argc, &argv );

	ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &id );	

	sequence.push_back(0);
	unique.push_back(-1);

	for(int z = 1; z < length; z++){	
		
		endVal = sequence.at(sequence.size() - 1);

		if ( id == 0 )
			timeHolder = MPI_Wtime();
		
		if( !valAppear( unique, endVal, argc, argv ) ){
			unique.push_back(endVal);
			sequence.push_back(0);

			if ( id == 0 ){
				timeHolder = MPI_Wtime() - timeHolder;
				uniqueSearchTime += timeHolder;
			}

		}else{

			if ( id == 0){
				timeHolder = MPI_Wtime() - timeHolder;
				uniqueSearchTime += timeHolder;
				timeHolder = MPI_Wtime();
			}
			sequence.push_back(searchSeq(sequence, endVal, argc, argv ));	

			if ( id == 0 ){
				timeHolder = MPI_Wtime() - timeHolder;
				lastNumSearchTime += timeHolder;
			}	
		}
	}		
			
	MPI_Finalize ( );
	
	if ( id == 0 ){
		
		cout << endl;
		cout << "Total Last Num Search Time: " << lastNumSearchTime << endl;
		cout << "Total Unique Num Search Time: " << uniqueSearchTime << endl;
	}
	
	if ( id == 0 ){	

		cout << endl  << "sequence: ";
		for(int c = 0; c < sequence.size(); c++){
			cout << sequence.at(c) << ", ";	
		}
	}

	return 0;
}

bool valAppear ( vector<int> unique, int val, int arc, char *argv[] ){
	
	bool found = false;
		
	for( int c = 0; c < unique.size() && !found; c++){	
			
		if ( unique.at(c) == val )
			found = true;
	}

	return found;
}

int searchFindings ( vector<int> finds ){
	
	int closestMatch = 0;
	bool found = false;

	for(int c = 0; c < finds.size() && !found; c++){
		if ( finds.at(c) > closestMatch ){
			found = true; 
			closestMatch = finds.at(c);
		}
	}

	for(int c = 0; c < finds.size(); c++){
		if( finds.at(c) > 0 && finds.at(c) < closestMatch )
			closestMatch = finds.at(c);
	}
	
	
	return closestMatch;
}
// if pos = -1 then searchVal did not match
// if pos = -2 then the tasks couldn't check another index
int searchSeq ( vector<int> seq, int val , int argc, char *argv[] ){


	bool found = false;	
	int ierr = 0, n, id;//TODO FIX THIS IEERR CODE TO ACTUALLY GET ERRORS
	int controller, factor = 0, pos = -1, searchVal = val, checkIndex;


	MPI_Comm_size ( MPI_COMM_WORLD, &n );
	
	if ( ierr != 0 ){
		cout << endl;
		cout << "van_eck - FATAL ERROR" << endl;
		cout << " MPI_Init returned nonzero ierr." << endl;
		exit ( 1 );
	}

	ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &id );	

	while ( !found ){

		checkIndex = (seq.size() - ( id + ( factor * n ) + 2 ));
		//Checking if the task can search within range or is past edge	
		if( checkIndex >= 0 ){
			
			
			// We subtract 2 from the index so we don't check the last element in the sequence
			if ( seq.at(checkIndex) == searchVal )
				pos = seq.size() - 1 - checkIndex;
			else 
				pos = -1;
		}else
			pos = -2;


		MPI_Reduce ( &pos, &controller, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD );

	
		if ( id == 0 && controller > -1 )
			found = true;
	
		MPI_Bcast ( &found, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD );

		
		// Checking if all tasks were used to search, if so then we 
		//incremebt factor by one so all corses can step forward.
		if ( !found ){
			MPI_Reduce ( &pos, &controller, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
			if ( id == 0 && controller > -2)
				factor++;

		MPI_Bcast ( &factor, 1, MPI_INT, 0, MPI_COMM_WORLD );
		}

	}

	
	vector<int> findings;

	MPI_Comm_size ( MPI_COMM_WORLD, &n );


	findings.resize ( n );


	MPI_Gather( &pos, 1, MPI_INT, findings.data(), 1, MPI_INT, 0, MPI_COMM_WORLD );


	int foundIndex = -1;
	
	if ( id == 0 ){
		foundIndex = searchFindings ( findings );
	}
	
	MPI_Bcast ( &foundIndex, 1, MPI_INT, 0, MPI_COMM_WORLD );	

	

	return foundIndex;
		
}
