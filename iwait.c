#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
   char buf;
   int fd, i, poll_num;
   int *wd;
   nfds_t nfds;
   struct pollfd fds[1];

   if (argc < 2) {
       printf("Usage: %s PATH [PATH ...]\n", argv[0]);
       exit(EXIT_FAILURE);
   }

   // Create the file descriptor for accessing the inotify API
   fd = inotify_init1(IN_NONBLOCK);
   if (fd == -1) {
       perror("inotify_init1");
       exit(EXIT_FAILURE);
   }

   // Allocate memory for watch descriptors
   wd = calloc(argc, sizeof(int));
   if (wd == NULL) {
       perror("calloc");
       exit(EXIT_FAILURE);
   }

   /* Mark directories for events
      - file was opened
      - file was closed */
   for (i = 1; i < argc; i++) {
       wd[i] = inotify_add_watch(fd, argv[i],
                                 IN_OPEN | IN_CLOSE);
       if (wd[i] == -1) {
           fprintf(stderr, "Cannot watch '%s'\n", argv[i]);
           perror("inotify_add_watch");
           exit(EXIT_FAILURE);
       }
   }

   // Prepare for polling
   nfds = 2;

   // Inotify input
   fds[0].fd = fd;
   fds[0].events = POLLIN;

   // Wait for events 
   while (1) {
       sleep(1);
       poll_num = poll(fds, nfds, -1);
       if (poll_num == -1) {
           if (errno == EINTR)
               continue;
           perror("poll");
           exit(EXIT_FAILURE);
       }

       if (poll_num > 0) {

           if (fds[0].revents & POLLIN) {

               // Inotify events are available
               exit(EXIT_SUCCESS); 
           }
       }
   }

   // Close inotify file descriptor
   close(fd);

   free(wd);
   exit(EXIT_SUCCESS);
}
