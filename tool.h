#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

void send_data(char *msg);
time_t get_millisec();
int get_super_global(char *msg, int len, const char* name);
