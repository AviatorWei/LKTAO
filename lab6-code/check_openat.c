#include <stdio.h>
#include <time.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/syscall.h>

int main()
{
	clock_t t;
	t = clock();
	while(1){
		clock_t tmp;
		tmp = clock();
		printf("%d\n",difftime(tmp,t)/CLOCKS_PER_SEC*1000);
		if ((difftime(tmp, t)/CLOCKS_PER_SEC)*1000 == 60){
			
			int res = syscall(200);
			syslog(LOG_DEBUG,"%d", res);
		}

	}
	return 0;
}
