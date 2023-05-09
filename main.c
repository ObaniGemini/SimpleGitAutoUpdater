#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


#define BUF_SIZE 4096
#define TRUE 1

void Error( int cond, const char* err_msg ) {
	if( cond ) {
		printf("%s\n", err_msg);
		exit( 1 );
	}
}

int gitRevParseOutput( char* buffer ) {
	char tmp[BUF_SIZE] = { };

	int link[2];

	Error( pipe(link) == -1, "Error on pipe" );

	pid_t pid = fork();
	Error( pid == -1, "git rev-parse fork failed" );
	if( pid == 0 ) {
		dup2(link[1], STDOUT_FILENO);
		close(link[0]);
		close(link[1]);
		execl("/bin/git", "git", "rev-parse", "origin/main", NULL);
		Error( TRUE, "git rev-parse origin/main failed" );
	} else {
		close(link[1]);
		int nbytes = read( link[0], tmp, BUF_SIZE );
		if( strcmp( buffer, tmp ) != 0 ) {
			memset( buffer + nbytes, '\0', BUF_SIZE - nbytes );
			memcpy( buffer, tmp, nbytes );
			return 1;
		}
	}

	return 0;
}


int main( int argc, char* argv[] ) {
	if( argc != 2 ) {
		printf("Wrong arguments\nExpected: %s <command>\n", argv[ 0 ]);
		return 1;
	}

	char last_commit[BUF_SIZE] = { };
	gitRevParseOutput( last_commit );

	while( TRUE ) {
		pid_t pid = fork();
		Error( pid == -1, "Main fork failed");

		if( pid == 0 ) {
			execl(argv[ 1 ], NULL);
			Error( TRUE, "execl failed" );
		} else {
			int status;
			while( TRUE ) {
				sleep( 30 );
				if( gitRevParseOutput( last_commit ) ) break; //update found				
			}
			kill( 0, SIGTERM );
		}
	}
	
	return 0;
}