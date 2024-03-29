#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

static int _null_open(int f, int fd) {
    int fd2;

    if ((fd2 = open("/dev/null", f)) < 0)
        return -1;
    
    if (fd2 == fd)
        return fd;

    if (dup2(fd2, fd) < 0)
        return -1;

    close(fd2);
    return fd;
}

int main( int argc, char const * const argv[] )
{
   int rval = 1 ;
   if( 2 <= argc ){
      int childPid = fork();
      if( 0 == childPid )
      {
         char fdDir[256];
         snprintf(fdDir, sizeof(fdDir), "/proc/%d/fd", getpid() );
         DIR *dir = opendir( fdDir );
         if( dir ) {
            dirent  *dirEntry ;
            unsigned tempCount = 0 ;
            while( 0 != ( dirEntry = readdir( dir ) ) ) {
               if( isdigit( dirEntry->d_name[0] ) ) {
                  int fdnum;
                  if (1 == sscanf( dirEntry->d_name, "%u", &fdnum )) {
                     close(fdnum);
                  }
               }
            }
      
            closedir( dir );
         }

         setsid();
         setpgid(0,0);

         if (_null_open(O_RDONLY, 0) < 0) {
            fprintf( stderr, "Failed to open /dev/null for STDIN: %s", strerror(errno));
         }
         
         if (_null_open(O_WRONLY, 1) < 0) {
            fprintf( stderr, "Failed to open /dev/null for STDOUT: %s", strerror(errno));
         }
         
         if (_null_open(O_WRONLY, 2) < 0) {
            fprintf( stderr, "Failed to open /dev/null for STDERR: %s", strerror(errno));
         }

         execve( argv[1], (char **)(argv+1), environ );
         perror( argv[1] );
         rval = errno ;
      } // child
      else if( 0 < childPid )
      {
         printf( "%d\n", childPid );
         rval = 0 ;
      } // parent, succeeded
      else
         rval = errno ;
   }
   else
      fprintf( stderr, "Usage: daemonize program [args...]\n" );
   return rval ;
}
