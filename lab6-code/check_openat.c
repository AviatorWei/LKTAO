#include <stdio.h>
#include <sys/time.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <sys/syscall.h>

int set_ticker(int n_msecs);
#define INTERVAL 10
int main()
{
	void cntdown(int);
	signal(SIGALRM, cntdown);
	if(set_ticker(INTERVAL) == -1)
		perror("set_ticker");
	else
	while(1){
		pause();
	}
	return 0;
}
void cntdown(int signum)
{
	static int num = 10;
	int res = syscall(213);
	syslog(LOG_DEBUG,"%d", res);
	num--;
	if (num < 0) exit(0);
}
int set_ticker(int n_secs)
{
	struct itimerval new_timeset;
	new_timeset.it_interval.tv_sec = n_secs;
	new_timeset.it_interval.tv_usec = 0;

	new_timeset.it_value.tv_sec = n_secs;
	new_timeset.it_value.tv_usec = 0;
	return setitimer(ITIMER_REAL, &new_timeset, NULL);
}
