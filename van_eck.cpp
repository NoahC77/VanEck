#include <ctime>
#include <iomanip>
#include <iostream>
#include <vector>
#include <mpi.h>
#include <cstdlib>

using namespace std;

int main ( int argc, char *argv[] );
int searchSeq ( vector<int> seq, int val , int argc, char *argv[] );
int searchFindings ( vector<int> finds );

bool valAppear ( vector<int> unique, int val );

int main ( int argc, char *argv[] ){
	
	int n, length;
	vector<int> sequence;

	//cout << "Enter desired length of Van Eck sequence." << endl;
	//cin >> length;	
	
	//Just testing we want it to get 6.	
	sequence.push_back(0);
	sequence.push_back(0);
  //      sequence.push_back(1);
//	sequence.push_back(0);
//	sequence.push_back(2);
//	sequence.push_back(0);
//	sequence.push_back(2);
//	sequence.push_back(2);
//	sequence.push_back(1);
	
	cout << "sequence: ";
	for(int c = 0; c < sequence.size(); c++){
		cout << sequence.at(c) << ", ";	
	}

	cout << endl << "searchSeq output: " << searchSeq ( sequence, 0, argc, argv ) << endl;
	
	return 0;
}

int searchFindings ( vector<int> finds ){
	
	cout << "Entered searchFindigns!" << endl;
	int closestMatch = 0;
	bool found = false;

	cout << "finds.size() = " << finds.size() << " needs to be less than 0" << endl;
	
	for( int c = 0; c < finds.size(); c++){
		cout << "[" <<  finds.at(c) << "]"; 
	}
	cout << endl;

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
	
	cout << "closest math = " << closestMatch << endl;	
	
	return closestMatch;
}
// if pos = -1 then searchVal did not match
// if pos = -2 then the tasks couldn't check another index
int searchSeq ( vector<int> seq, int val , int argc, char *argv[] ){

	cout << "searchSeq started..." << endl;

	bool found = false;	
	int ierr, n, id;
	int controller, factor = 0, pos = -1, searchVal = val, checkIndex;

	ierr = MPI_Init ( &argc, &argv );

	MPI_Comm_size ( MPI_COMM_WORLD, &n );
	cout << "Comm_size = " << n << endl;
	
	if ( ierr != 0 ){
		cout << endl;
		cout << "van_eck - FATAL ERROR" << endl;
		cout << " MPI_Init returned nonzero ierr." << endl;
		exit ( 1 );
	}

	ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &id );	

	cout << "Entering the unfound looking loop..." << endl;
	while ( !found ){

		checkIndex = (seq.size() - ( id + ( factor * n ) + 2 ));
		//Checking if the task can search within range or is past edge	
		if( checkIndex >= 0 ){
			
			cout << "Proc[" << id << "] checking sequence[" << checkIndex << endl;
			
			// We subtract 2 from the index so we don't check the last element in the sequence
			if ( seq.at(checkIndex) == searchVal )
				pos = checkIndex;
			else 
				pos = -1;
		}else
			pos = -2;


		MPI_Reduce ( &pos, &controller, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD );

		cout << "Task[" << id << "] controller = " << controller << endl;
	
		if ( id == 0 && controller > -1 )
			found = true;
	
		MPI_Bcast ( &found, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD );

		cout << "Bcast found = " << found << endl;
		
		// Checking if all tasks were used to search, if so then we 
		//incremebt factor by one so all corses can step forward.
		if ( !found ){
			MPI_Reduce ( &pos, &controller, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
			if ( id == 0 && controller > -2)
				factor++;

		MPI_Bcast ( &factor, 1, MPI_INT, 0, MPI_COMM_WORLD );
		}

	}

	cout << "First search done" << endl;
	
	vector<int> findings;

	MPI_Comm_size ( MPI_COMM_WORLD, &n );

	cout << "MPI_Comm_size = " << n << endl;

	findings.resize ( n );

	cout << "Task[" << id << "] resized findings to " << n << endl;

	MPI_Gather( &pos, 1, MPI_INT, findings.data(), 1, MPI_INT, 0, MPI_COMM_WORLD );


	int foundIndex = -1;
	
	if ( id == 0 ){
		cout << "Task[" << id << "] calling searchFindings" << endl;
		foundIndex = searchFindings ( findings );
		cout << "Task[" << id << "] finished searchFindings" << endl;
	}
	
	cout << "Bcasting final found index for return" << endl;
	MPI_Bcast ( &foundIndex, 1, MPI_INT, 0, MPI_COMM_WORLD );	

	cout << "Task[" << id << "] foundIndex = " << foundIndex << endl;
	
	MPI_Finalize ( );

	return foundIndex;
		
}
