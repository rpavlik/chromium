/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_error.h"
#include "cr_process.h"
#include "cr_string.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef WINDOWS
/* include something */
#else
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#endif


/*
 * Sleep/pause for the given time.
 */
void crSleep( unsigned int seconds )
{
#ifdef WINDOWS
  /* XXX need Windows solution */
#else
  sleep(seconds);
#endif
}


/*
 * Spawn (i.e. fork/exec) a new process.
 */
unsigned long crSpawn( const char *command, const char *argv[] )
{
#ifdef WINDOWS
	/* XXX need Windows solution */
	return 0;
#else
	pid_t pid;
	if ((pid = fork()) == 0)
	{
		/* I'm the child */
		int err = execvp(command, (char * const *) argv);
		crWarning("crSpawn failed (return code: %d)", err);
		return 0;
	}
	return (unsigned long) pid;
#endif
}


/*
 * Kill the named process.
 */
void crKill( unsigned long pid )
{
#ifdef WINDOWS
	/* XXX need Windows solution */
#else
	kill((pid_t) pid, SIGKILL);
#endif
}


/*
 * Return the name of the running process.
 * name[0] will be zero if anything goes wrong.
 */
void crGetProcName( char *name, int maxLen )
{
#ifdef WINDOWS
	/* XXX Need a Windows solution here */
	*name = 0;
#else
	/* Unix:
	 * Call getpid() to get our process ID.
	 * Then use system() to write the output of 'ps' to a temp file.
	 * Read/scan the temp file to map the process ID to process name.
	 * I'd love to find a better solution! (BrianP)
	 */
	FILE *f;
	pid_t pid = getpid();
	char *tmp, command[1000];

	/* init to NULL in case of early return */
	*name = 0;

	/* get a temporary file name */
	tmp = tmpnam(NULL);
	if (!tmp)
		return;

	/* pipe output of ps to temp file */
	sprintf(command, "ps > %s", tmp);
	system(command);

	/* open/scan temp file */
	f = fopen(tmp, "r");
	if (f) {
		char buffer[1000], cmd[1000];
		while (!feof(f)) {
			int id;
			fgets(buffer, 999, f);
			sscanf(buffer, "%d %*s %*s %999s", &id, cmd);
			if (id == pid) {
				crStrncpy(name, cmd, maxLen);
				break;
			}
		}
		fclose(f);
	}
	remove(tmp);
#endif
}


/*
 * Return current directory string.
 */
void crGetCurrentDir( char *dir, int maxLen )
{
#ifdef WINDOWS
  /* XXX need Windows solution here */
  dir[0] = 0;
#else
  if (!getcwd(dir, maxLen))
	dir[0] = 0;
#endif
}


#if 0
/* simple test harness */
int main(int argc, char **argv)
{
   char name[100];
   printf("argv[0] = %s\n", argv[0]);

   crGetProcName(name, 100);
   printf("crGetProcName returned %s\n", name);

   return 0;
}
#endif
