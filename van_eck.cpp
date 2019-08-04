#include <ctime>
#include <iomanip>
#include <iostream>
#include <mpi.h>

using namespace std;

int main ( int argc, char *argv[] );
int searchSeq ( vector<int> seq, int val );
int searchFindings ( vector<int> finds );

bool valAppear ( vector<int> unique, int val );

int main ( int argc, char *argv[] ){
	
	int n, length;
	vector<int> sequence;

	cout << "Enter desired length of Van Eck sequence." << endl;
	cin >> length;	
}

int searchSeq ( vector<int> seq, int val ){

	bool found = false;	
	int ierr, n;
	int controller, factor = 1, pos = -1, searchVal = val;

	ierr = MPI_Init ( &argc, &argv );
	
	if ( ierr != 0 ){
		cout << endl;
		cout << "van_eck - FATAL ERROR" << endl;
		cout << " MPI_Init returned nonzero ierr." << endl;
		exit ( 1 );
	}

	ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &id );	

	if ( id == 0 )
		pos 1;

	while ( !found ){
		//Checking if the task can search within range or at edge	
		if( seq.size() - factor * id > 0 ){
			if ( seq( seq.size() - factor * id) == searchVal )
				pos = seq.size() - factor * id;
			else 
				pos = 0;
		}else
			pos = -1;


		MPI_Reduce ( &pos, &controller, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD );
	
		if ( id == 0 && controller > 0 )
		found = true;
	
		MPI_Bcast ( &found, 1, MPI_BOOL, 0, MPI_COMM_WORLD );
		
		// Checking if all tasks were used to search, if so then we 
		//incremebt factor by one so all corses can step forward.
		if ( !found ){
			MPI_Reduce ( &pos, &controller, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
			if ( id == 0 && controller > -1)
				factor++;

		MPI_Bcast ( &factor, 1, MPI_INT, 0, MPI_COMM_WORLD );
		}

	}

	vector<int> findings;
	int n;

	MPI_Comm_sie ( MPI_COMM_WORLD, &n );
	
	MPI_Gather( &pos, 1, MPI_INT, &findings, 1, MPI_INT, 0, MPI_COMM_WORLD );

	MPI_Finalize ( );
	
	// Calls search findings to determine minimum
	return	searchFindings ( findings );
		
}
