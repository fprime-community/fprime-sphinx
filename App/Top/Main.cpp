#include <stdlib.h>
#include <ctype.h>

#include <App/Top/Components.hpp>

void print_usage(const char* app) {
    (void) printf("Usage: ./%s [options]\n-p\tport_number\n-a\thostname/IP address\n",app);
}

#include <signal.h>
#include <stdio.h>

volatile sig_atomic_t terminate = 0;

static void sighandler(int signum) {
    exitTasks();
    terminate = 1;
}


int main(void) {
    (void) printf("Hit Ctrl-C to quit\n");

    // register signal handlers to exit program
    signal(SIGINT,sighandler);
    signal(SIGTERM,sighandler);

    // Give time for threads to exit
    (void) printf("Waiting for threads...\n");
    Os::Task::delay(1000);

    (void) printf("Exiting...\n");

    return 0;
}
