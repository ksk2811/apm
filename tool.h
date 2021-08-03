#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdarg.h>

void send_data(char *msg);
time_t get_millisec();
int get_heap_super_global(char **msg, const char* name);
int get_super_global(char *msg, int len, const char* name);
char* snprintf_heap(char *msg, ...);
