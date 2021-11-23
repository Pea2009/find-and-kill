/*	
 *	faq.c
 *	Find out processes those include a specific series of characters .Find them ,  kill them.
 *	 Time: 2021/11/23
 *	Author: Xuaka	
 *
 * */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include<signal.h>
#include <stdio.h>
#define NR_ALLOCATE 512*1024  //512KiB..
#define NR_PIDS 8 //store 8 pid at most ..

void faq_help(void){
	printf("faq.\n\
\tFind out processes those include a specific series of characters .Find them ,  kill them.\n\
\tusage:\n\
\tfaq -s <string>\n");
	exit(0);
}


void fork4ps(int fd){
	char* args[] = {"/bin/ps","-A",NULL};
	int child_pid;
	int child_exit_code;

	if(!(child_pid =fork())){//child
		close(1);
		dup2(fd,1);
		child_exit_code = execve("/bin/ps",args,NULL);
//execve() should not  return,but we cann't print any information now , stdout has been replaced...
		exit(child_exit_code);
	}
//parrent
	if(child_pid < 0){
		printf("fork() failed..");
		exit(0);
	}
	child_pid = wait(&child_exit_code);
	if(child_exit_code){
		printf("execve() failed..exit code : %d",child_exit_code);
		exit(0);
	}
}


int main(int nr_arg,char ** arg){
	char* arg3;
	char *base , *p , in, unused;
	pid_t pids[NR_PIDS];
	int nr_pids = 0 , st_pids;
	int fd;
	size_t err;

	if(nr_arg^3 ||
	(*(++arg))[0] ^0x2d ||
	(*(arg))[1]^0x73	)	//0x2d : '-'   0x73 : 's'   according to ascii code
		faq_help();

	if((fd = open("./tmp",O_TRUNC|O_CREAT|O_RDWR,S_IROTH|S_IWOTH ))<0  ){
		printf("open()  failed..");
		goto out;
	}

	fork4ps(fd);

	base = (char*)malloc(NR_ALLOCATE);
	if(!base){
		printf("malloc() failed..");
		goto close_out;
	}

	lseek(fd,0,SEEK_SET);
	err = read(fd,base,NR_ALLOCATE);
	if(err<=0){
		printf("read() failed..");
		goto free_out;
	}
	if(err == NR_ALLOCATE){
		printf("NR_ALLOCATE too small");
		goto free_out;
	}

	arg3 = *(++arg);
	while((p = strstr(base,arg3)) && ++nr_pids <= NR_PIDS ){
		int stepsBack = 0 , i = 0 , current_pid = 0;

		while(*(--p)-0x20)	//0x20 : space according to ascii code
			;
		++p;
//p is pointing to CMD now

		printf("find one! number: %d , full CMD : ",nr_pids);
reputchar:
		putchar(p[i]);
		p[i]++;	//avoid repeat
		if(p[++i]-0x0A)		//0x0A : '\n'
			goto reputchar;
		putchar('\n');


repeat:
		while(!(*(--p)-0x20))	//stop when *(--p) is not space
			;
		while(*(--p)-0x20)	//stop when *(--p) is space
			;
		if(++stepsBack<2)
			goto repeat;
		while(*(--p)-0x20)
			;
		++p;
//p is pointing to pid now
readd:
		current_pid *= 10;
		current_pid +=((*p)-0x30);
		if(*(++p)-0x20)
			goto readd;
		pids[nr_pids-1] = current_pid;
	}
	if(nr_pids > NR_PIDS){
		printf("NR_PIDS too small");
		goto free_out;
	}

	st_pids = nr_pids;
	while(st_pids){
		int i;
		printf("type the number of process appers over to kill the process\n \'a\' for all , \'n\' for none \n : ");
		scanf("%c%c",&in,&unused);
		if(in == 'n')
			break;
		if(in == 'a'){
			i = 0;
			while(i<nr_pids){
				kill(pids[i],SIGKILL);
				i++;
			}
			break;
		}
		i =(int)(in - 0x30);
		if(i>st_pids || !(pids[i - 1]) ){
			printf("invalid\n");
			continue;
		}
		kill(pids[i - 1] , SIGKILL);
		pids[i - 1] = 0;
		st_pids--;
	};



free_out:
	free(base);
close_out:
	close(fd);
out:
	return 0;
}
