#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


#define BUF_SIZE 4096
#define FALSE 0
#define TRUE 1
#define UPDATE_PERIOD 60

void Error( int cond, const char* err_fmt, ... ) {
	if( cond ) {
		va_list args;
		va_start(args, err_fmt);
		vprintf(err_fmt, args);
		va_end(args);
		exit( 1 );
	}
}

int gitRevParseOutput( char* buffer ) {
	static char tmp[BUF_SIZE] = { };
	int link[2];
	int status;
	pid_t pid = fork();
	if( pid == 0 ) {
		execl("/bin/git", "git", "pull", NULL);
		Error( TRUE, "git pull failed\n");
	} else {
		Error( waitpid( pid, &status, 0 ) == -1, "wait 'git pull' failed\n" );
	}

	Error( pipe(link) == -1, "Error on pipe\n" );
	Error( (pid = fork()) == -1, "git rev-parse fork failed\n" );

	if( pid == 0 ) {
		Error( dup2(link[1], STDOUT_FILENO) == -1, "CHILD: dup2 error\n" );
		Error( close(link[0]) == -1, "CHILD: close(link[0]) error\n" );
		Error( close(link[1]) == -1, "CHILD: close(link[1]) error\n" );
		execl("/bin/git", "git", "rev-parse", "origin/HEAD", NULL);
		Error( TRUE, "CHILD: git rev-parse origin/HEAD failed (%d)\n", errno );
	} else {
		Error( close(link[1]) == -1, "PARENT: close(link[1]) error\n" );
		int nbytes = read( link[0], tmp, BUF_SIZE );
		Error( nbytes == -1, "PARENT: couldn't read on link[0]\n" );
		if( strcmp( buffer, tmp ) != 0 ) {
			memset( buffer + nbytes, '\0', BUF_SIZE - nbytes );
			memcpy( buffer, tmp, nbytes );
			return TRUE;
		}
	}

	return FALSE;
}


int main( int argc, char* argv[] ) {
	Error( argc < 2, "Wrong arguments\nExpected: %s <program>\n", argv[ 0 ] );

	char last_commit[BUF_SIZE] = { };
	gitRevParseOutput( last_commit );


	while( TRUE ) {
		pid_t pid = fork();
		Error( pid == -1, "Main fork failed\n");

		if( pid == 0 ) {
			execvp( argv[1], argv + 1 );
			Error( TRUE, "execvpe failed (%d)\n", errno );
		} else {
			int status;
			sleep(1);
			while( TRUE ) {
				Error( waitpid( pid, &status, WNOHANG ) == -1, "wait '%s' failed\n", argv[1] );
				Error( WIFEXITED( status ), "Program exitted without our permission (%d)\n", status );
				
				sleep( UPDATE_PERIOD );

				if( gitRevParseOutput( last_commit ) ) {
					printf("Updating...\n");
					break;
				}			
			}
			kill( pid, SIGTERM );
		}
	}
	
	return 0;
}