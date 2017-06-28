#ifndef _UNIX_C_INC_
#define _UNIX_C_INC_ 1

#include <unistd.h>

void getShellOutput(char * command, char* res, size_t n){
	int pipefd[2];
	int l;
	pipe(pipefd);
	if(fork()){
		l=read(pipefd[0], res, n);
		if(l>=0)res[l]=0;
		close(pipefd[0]);
		close(pipefd[1]);
	} else {
		close(1);
		dup2(pipefd[1],1);
		execlp("sh", "sh", "-c", command, NULL);
	}
}

void bindProcess(char* command, int fd[3], int * pid, int one_out){
  int ipipe[2], opipe[2], epipe[2];
  pipe(ipipe);
  pipe(opipe);
  pipe(epipe);
  if((*pid=fork())){
    close(ipipe[0]);
    close(opipe[1]);
    close(epipe[1]);
    fd[0] = ipipe[1];
    fd[1] = opipe[0];
    fd[2] = epipe[0];
    fcntl(fd[1], F_SETFL, O_NONBLOCK);
    fcntl(fd[2], F_SETFL, O_NONBLOCK);
  } else {
    close(0);
    close(1);
    close(2);
    dup2(ipipe[0], 0);
    dup2(opipe[1], 1);
    if(one_out){
      dup2(opipe[1], 2);
    }else{
      dup2(epipe[1], 2);
    }
    setsid();
    execlp("sh", "sh", "-c", command, NULL);
  }
}

#endif 
