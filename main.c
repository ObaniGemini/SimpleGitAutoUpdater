#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


#define BUF_SIZE 4096
#define FALSE 0
#define TRUE 1

void Error( int cond, const char* err_msg ) {
	if( cond ) {
		printf("%s\n", err_msg);
		exit( 1 );
	}
}

int gitRevParseOutput( char* buffer ) {
	static char tmp[BUF_SIZE] = { };
	int link[2];
	pid_t pid;

	Error( pipe(link) == -1, "Error on pipe" );
	Error( (pid = fork()) == -1, "git rev-parse fork failed" );

	if( pid == 0 ) {
		Error( dup2(link[1], STDOUT_FILENO) == -1, "CHILD: dup2 error" );
		Error( close(link[0]) == -1, "CHILD: close(link[0]) error" );
		Error( close(link[1]) == -1, "CHILD: close(link[1]) error" );
		execl("/bin/git", "git", "rev-parse", "origin/HEAD", NULL);
		Error( TRUE, "CHILD: git rev-parse origin/HEAD failed" );
	} else {
		Error( close(link[1]) == -1, "PARENT: close(link[1]) error" );
		int nbytes = read( link[0], tmp, BUF_SIZE );
		Error( nbytes == -1, "PARENT: couldn't read on link[0]" );
		if( strcmp( buffer, tmp ) != 0 ) {
			memset( buffer + nbytes, '\0', BUF_SIZE - nbytes );
			memcpy( buffer, tmp, nbytes );
			return TRUE;
		}
	}

	return FALSE;
}


int main( int argc, char* argv[] ) {
	Error( argc != 2, "Wrong arguments\nExpected: gitupdater <program>\n" );

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
			sleep(1);
			while( TRUE ) {
				waitpid( pid, &status, WNOHANG );
				Error( WIFEXITED( status ), "Program exitted without our permission (error ?)" );
				
				sleep( 30 );

				if( gitRevParseOutput( last_commit ) ) break; //update found			
			}
			kill( pid, SIGTERM );
		}
	}
	
	return 0;
}