// Copyright (c) 2014, ZhaoStudio
// Author: CptZhao<zhaonanyu@gmail.com>
// Created: 2014-10-18
// Description:
// code structure enhancement

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<dirent.h>

struct proc_pid_stat
{
	int pid;
	int ppid;
	char comm[50];
	char status;
};

void do_ps();

void parse_stat(struct proc_pid_stat* pps, char buf[1024]);

void parse_pid(struct proc_pid_stat* pps, char word[50]);
void parse_comm(struct proc_pid_stat* pps, char word[50]);
void parse_status(struct proc_pid_stat* pps, char word[50]);
void parse_ppid(struct proc_pid_stat* pps, char word[50]);

int main()
{
	do_ps();
}

void do_ps()
{
	// print table head
	printf("pid\tCMD\t\t  STAT  ppid\n");

	char pid_path[20];
	DIR* dir_ptr;
	struct proc_pid_stat pps;
	struct dirent* p_dir;
	int fd_stat;
	char stat_content[1024];

	if((dir_ptr = opendir("/proc")) == NULL){
		fprintf(stderr, "open /proc dir failed");
	}else{
		while((p_dir = readdir(dir_ptr)) != NULL)
		{
			// if this dir is a process rsrc dir
			if(p_dir->d_name[0]>='0'&&p_dir->d_name[0]<='9')
			{
				// generate pid_path string
				strcpy(pid_path, "/proc/");
				strcat(pid_path, p_dir->d_name);
				strcat(pid_path, "/stat");
				// get proc_pid_stat struct
				fd_stat = open(pid_path, O_RDONLY);
				read(fd_stat, stat_content, 1024);
				parse_stat(&pps, stat_content);

				// display
				printf("%d\t%-20.20s%c\t%d\n", pps.pid, pps.comm, pps.status, pps.ppid);
				close(fd_stat);
			}
		}
		closedir(dir_ptr);
	}
}

void parse_stat(struct proc_pid_stat* pps, char buf[1024])
{
	int i, count, word_i;
	char word[50];
	word_i = i = count = 0;
	for(i; i<strlen(buf); i++)
	{
		// rewrite is need, paser /proc/[pid]/stat file into a string[], for cputime calculation properse.
		if(buf[i] == ' ')
		{
			count++;
			word[word_i] = '\0';

			switch(count)
			{
				case 1:
					parse_pid(pps, word);
					break;
				case 2:
					parse_comm(pps, word);
					break;
				case 3:
					parse_status(pps, word);
					break;
				case 4:
					parse_ppid(pps, word);
					break;
				default:
					break;
			}
			word_i = 0;
			continue;
		}
		word[word_i] = buf[i];
		word_i++;
	}
}

void parse_pid(struct proc_pid_stat* pps, char word[50])
{
	pps->pid = atoi(word);
}

void parse_comm(struct proc_pid_stat* pps, char word[50])
{
	strcpy(pps->comm, word);
}

void parse_status(struct proc_pid_stat* pps, char word[50])
{
	pps->status = word[0];
}

void parse_ppid(struct proc_pid_stat* pps, char word[50])
{
	pps->ppid = atoi(word);
}
