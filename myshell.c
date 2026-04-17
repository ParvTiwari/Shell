#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_HISTORY 100

char history_list[MAX_HISTORY][100];
int hist_count = 0;

void history(){
	for(int i=0; i<hist_count; i++){
		printf("%d %s\n", i+1, history_list[i]);
	}
}

void handle_sigint(int sig){
	printf("\n>>> ");
	fflush(stdout);
}

int main(){
	signal(SIGINT, handle_sigint);
	signal(SIGQUIT, SIG_IGN);

	while(1){
		char s[100];
		printf(">>> ");
		fgets(s, sizeof(s), stdin);
        s[strcspn(s, "\n")] = 0;
		
		if(strlen(s) > 0 && strcmp(s, "history") != 0){
			if(hist_count < MAX_HISTORY){
				strcpy(history_list[hist_count++], s);				
			}
		}
		
		if(strcmp(s, "exit") == 0) break;
		
		else if(strcmp(s, "history") == 0){
			history();
			continue;		
		}
		char *args[50];
		char *token = strtok(s, " ");
		int i = 0;
		
		while (token) {
		    args[i++] = token;
			token = strtok(NULL, " ");
	    }
	    
	    if(i == 0) continue;
	    
	    int background = 0;
	    if(i > 0 && strcmp(args[i-1], "&") == 0){
	        background = 1;
	        args[i-1] = NULL;
	    }
	    else{
	        args[i] = NULL;
	    }
	    
	    int out_redirect = 0;
	    char *outfile = NULL;
	    
	    int in_redirect = 0;
        char *infile = NULL;
        
        int err_redirect = 0;
        char *errfile = NULL;
	    
	    // int pipe_idx = -1;
	    int pipe_count = 0;

		for(int j=0; j<i; j++){
			if(strcmp(args[j], "|") == 0){
				pipe_count++;
			}
		}

		if(pipe_count > 0){
			if(strcmp(args[0], "|") == 0 || strcmp(args[i-1], "|") == 0){
				printf("Invalid pipe usage\n");
				continue;
			}
			char *cmds[10][10];
			int cmd_idx = 0, arg_idx = 0;

			for(int j=0; j<i; j++){
				if(strcmp(args[j], "|") == 0){
					cmds[cmd_idx][arg_idx] = NULL;
					cmd_idx++;
					arg_idx = 0;
				}
				else{
					cmds[cmd_idx][arg_idx++] = args[j];
				}
			}
			cmds[cmd_idx][arg_idx] = NULL;

			int total_cmds = cmd_idx + 1;

			int fd[10][2];

			for(int j=0; j<pipe_count; j++){
				if(pipe(fd[j]) < 0){
					perror("pipe failed");
					exit(1);
				}			
			}

			for(int j=0; j<total_cmds; j++){
				pid_t p = fork();
				if(p < 0){
					perror("fork failed");
					exit(1);
				}
				if(p == 0){
					signal(SIGINT, SIG_DFL);
					if(j == 0){
						dup2(fd[j][1], STDOUT_FILENO);
					}

					else if(j == total_cmds -1){
						dup2(fd[j-1][0], STDIN_FILENO);
					}

					else{
						dup2(fd[j-1][0], STDIN_FILENO);
						dup2(fd[j][1], STDOUT_FILENO);
					}

					for(int k=0; k<pipe_count; k++){
						close(fd[k][0]);
						close(fd[k][1]);
					}

					execvp(cmds[j][0], cmds[j]);
					perror("exec failed");
					exit(1);
				}
			}
			for(int j = 0; j < pipe_count; j++){
				close(fd[j][0]);
				close(fd[j][1]);
			}

			for(int j = 0; j < total_cmds; j++){
				wait(NULL);
			}

			continue;
		}

		int error_flag = 0;

	    for(int j=0; j<i; j++){
	        if(strcmp(args[j], ">") == 0){
	            if(args[j+1] == NULL){
                    printf("Missing output file\n");
					error_flag = 1;
                    break;
                }
	            out_redirect = 1;
	            outfile = args[j+1];
	            args[j] = NULL;
	        }
            else if(strcmp(args[j], "<") == 0){
				if(args[j+1] == NULL){
					printf("Missing input file\n");
					error_flag = 1;
					break;
                }
                in_redirect = 1;
                infile = args[j+1];
                args[j] = NULL;
            }
            else if(strcmp(args[j], "2>") == 0){
				if(args[j+1] == NULL){
					printf("Missing error file\n");
					error_flag = 1;
					break;
                }
                err_redirect = 1;
                errfile = args[j+1];
                args[j] = NULL;
            }
            /*else if(strcmp(args[j], "|") == 0){
                pipe_idx = j;
                args[j] = NULL;
                break;
            }*/
			else if(args[j][0] == '$'){
				char *val = getenv(args[j] + 1);
				if(val){
					args[j] = val;
				}
				else{
					args[j] = "";
				}
			}
	    }

		if(error_flag) continue;
        
		/*if(pipe_idx == 0 || pipe_idx == i-1){
            printf("Invalid pipe usage\n");
            continue;
        }
        if(pipe_idx != -1){
            char *cmd1[10];
            char *cmd2[10];
            for(int j=0; j<pipe_idx; j++){
                cmd1[j] = args[j];
            }
            cmd1[pipe_idx] = NULL;
            
            int k=0;
            for(int j=pipe_idx+1; j<i; j++){
                cmd2[k++] = args[j];
            }
            cmd2[k] = NULL;
            
            int fd[2];
            pipe(fd);
            
            if(fork() == 0){
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                
                execvp(cmd1[0], cmd1);
                perror("exec1 failed");
                exit(1);
            }
            
            if(fork() == 0){
                dup2(fd[0], STDIN_FILENO);
                close(fd[1]);
                close(fd[0]);
                
                execvp(cmd2[0], cmd2);
                perror("exec2 failed");
                exit(1);
            }
            
            close(fd[0]);
            close(fd[1]);
            
            wait(NULL);
            wait(NULL);
            
            continue;    
        }*/
        
		pid_t pid = fork();
		if(pid < 0){
		    fprintf(stderr, "Fork Failed");
		    return 1;
		}
		else if(pid == 0){
			signal(SIGINT, SIG_DFL);
			
		    if(out_redirect){
		        int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		        if(fd < 0){
		            perror("open failed");
		            exit(1);
		        }
		        dup2(fd, STDOUT_FILENO);
		        close(fd);
		    }
		    
		    if(in_redirect){
		        int fd = open(infile, O_RDONLY);
		        if(fd < 0){
		            perror("open input failed");
		            exit(1);
		        }
		        dup2(fd, STDIN_FILENO);
		        close(fd);
		    }
		    
		    if(err_redirect){
		        int fd = open(errfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		        if(fd < 0){
		            perror("open error file failed");
		            exit(1);
		        }
		        dup2(fd, STDERR_FILENO);
		        close(fd);
		    }
		    
			execvp(args[0], args);
			perror("exec failed");
			exit(1);
		}
		else{
		    if(background == 0){
    			wait(NULL);
			    printf("Child Complete\n");
		    }
		    else{
	            printf("Process running in background\n");
		    }
		}
	}
	return 0;
}