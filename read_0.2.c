// Copyright (c) 2014, ZhaoStudio
// Author: CptZhao<zhaonanyu@gmail.com>
// Created: 2014-10-18
// Description:
// Write a ps lf command on my own, with the help of HeChuan(github UID:Hechuan)
// This project still in progress
// Log:
// 2014-10-18 Basic func is working, can't parse F VSZ RSS WCHAN TTY TIME yet.
// 2014-10-19 Code structure enhancement
// 2014-10-19 Parser for F TTY are working now. Can't parse VSZ RSS WCHAN yet. TIME's parser works not well yet.
// 2014-10-20 VSZ parser now works

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
	char tty_nr[10];		/* The controlling terminal of the process */
	int flags;				/* The kernel flags word of the process */
	unsigned long utime;	/* Amount of time that this process has been scheduled in user mode */
	unsigned long stime;	/* Amount of time that this process has been scheduled in kernel mode */
	long cutime;			/* Amount of time that this process's waited-for children have been scheduled in user mode */
	long cstime;			/* Amount of time that this process's waited-for children have been scheduled in kernel mode */
	long priority;
	long nice;
	long vsize;				/* Virtual memory size in bytes */
	long cputime;			/* not presented in /proc/[pid]/stat file. cputime = utime + stime + cutime + cstime */
};

void do_ps();

void parse_stat(struct proc_pid_stat* pps, char buf[1024]);

void parse_pid(struct proc_pid_stat* pps, char word[50]);
void parse_comm(struct proc_pid_stat* pps, char word[50]);
void parse_status(struct proc_pid_stat* pps, char word[50]);
void parse_ppid(struct proc_pid_stat* pps, char word[50]);
void parse_tty_nr(struct proc_pid_stat* pps, char word[50]);
void parse_flags(struct proc_pid_stat* pps, char word[50]);
void parse_priority(struct proc_pid_stat* pps, char word[50]);
void parse_nice(struct proc_pid_stat* pps, char word[50]);
void parse_vsize(struct proc_pid_stat*pps, char word[50]);
void parse_utime(struct proc_pid_stat* pps, char word[50]);
void parse_stime(struct proc_pid_stat* pps, char word[50]);
void parse_cutime(struct proc_pid_stat* pps, char word[50]);
void parse_cstime(struct proc_pid_stat* pps, char word[50]);
void parse_cputime(struct proc_pid_stat*pps);

int main()
{
	do_ps();
}

/**
 * ps cmd handler
 */
void do_ps()
{
	// print table head
	printf("pid\tCMD\t\t  STAT  ppid  TTY\tPRI   NI\tTIME\tF\tVSZ\n");

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

				// print process detail
				printf("%d\t%-20.20s%c\t%d\t%s\t%ld\t%ld\t%ld\t%d\t%ld\n", pps.pid, pps.comm, pps.status, pps.ppid, pps.tty_nr, pps.priority, pps.nice, pps.cputime, pps.flags, pps.vsize);
				close(fd_stat);
			}
		}
		closedir(dir_ptr);
	}
}

/**
 * parse a single stat file in /proc/[pid]/stat
 */
void parse_stat(struct proc_pid_stat* pps, char buf[1024])
{
	int i, count, word_i;
	char word[50];
	word_i = i = count = 0;
	for(i; i<strlen(buf); i++)
	{
		// switch wasn't a good code style, rewrite is needed
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
				case 7:
					parse_tty_nr(pps, word);
					break;
				case 9:
					parse_flags(pps, word);
					break;
				case 14:
					parse_utime(pps, word);
					break;
				case 15:
					parse_stime(pps, word);
					break;
				case 16:
					parse_cutime(pps, word);
					break;
				case 17:
					parse_cstime(pps, word);
				case 18:
					parse_priority(pps, word);
					break;
				case 19:
					parse_nice(pps, word);
					break;
				case 23:
					parse_vsize(pps, word);
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
	parse_cputime(pps);
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

/**
 * parse TTY from tty_nr in /proc/[pid]/stat
 * tty_nr was stored as a int in stat this parser will parse it into string
 * result eg: pts/27
 */
void parse_tty_nr(struct proc_pid_stat* pps, char word[50])
{
	/**
	 * tty_nr means the controlling terminal of the process.
	 * The minor device number is contained in the combination of bits 31 to 20 and 7 to 0
	 * The major device number is in bits 15 to 8
	 *
	 * For parse this param see /proc/tty drivers file
	 * major  minor     TTY
	 *     4  1-63      tty console
	 *     4  64-111    ttyS serial
	 *   136  0-1048575 pty:slave
	 *   128  0-1048575 pty:master
	 **/
	int tty_nr = atoi(word);
	int tty_nr_minor;
	int tty_nr_major;
	char tty_nr_minor_str[10];
	char tty[10];
	if(word[0] == '0')
	{
		strcpy(pps->tty_nr, "?");
	}else{
		tty_nr_minor = tty_nr;
		tty_nr_minor = tty_nr_minor<<24;
		tty_nr_minor = tty_nr_minor>>24;
		

		tty_nr_major = tty_nr;
		tty_nr_major = tty_nr_major>>8;

		switch(tty_nr_major)
		{
			case 4:
				if(tty_nr_minor<64){
				strcpy(tty, "tty/");
				}else{
					strcpy(tty, "ttyS/");
				}
				break;
			case 136:
				strcpy(tty, "pts/");
				break;
			case 128:
				strcpy(tty, "ptm/");
				break;
			case 216:
				strcpy(tty, "rfcomm/");
				break;
			default:
				break;
		}
		sprintf(tty_nr_minor_str, "%d", tty_nr_minor);
		strcat(tty, tty_nr_minor_str);
		strcpy(pps->tty_nr, tty);

	}
}

/**
 * parse the F column from flags param in /proc/[pid]/stat saved as unsigned
 * PROCESS FLAGS
 *  1 - forked but didn't exec
 *  4 - used super-user privileges
 */
void parse_flags(struct proc_pid_stat* pps, char word[50])
{
	/**
	 * PF_FORKNOEXEC and PF_SUPERPRIV both defined in sched.h see PF_*
	 * flags %u (%lu before Linux 2.6.22) 9th param in /proc/[pid]/stat
	 * - The kernel flags word of the process. For bit meanings
	 */
	int flags = atoi(word);
	if((flags & 0x00000040) == 0x00000040)			/* PF_FORKNOEXEC */
	{
		pps->flags = 1;
	}else if((flags & 0x00000100) == 0x00000100)	/* PF_SUPERPRIV */
	{
		pps->flags = 4;
	}else{
		pps->flags = 0;
	}
}

void parse_priority(struct proc_pid_stat* pps, char word[50])
{
	pps->priority = atoi(word);
}

void parse_nice(struct proc_pid_stat* pps, char word[50])
{
	pps->nice = atoi(word);
}

void parse_vsize(struct proc_pid_stat* pps, char word[50])
{
	pps->vsize = atoi(word)/1024;
}

void parse_utime(struct proc_pid_stat* pps, char word[50])
{
	pps->utime = atoi(word);
}

void parse_stime(struct proc_pid_stat* pps, char word[50])
{
	pps->stime = atoi(word);
}

void parse_cutime(struct proc_pid_stat* pps, char word[50])
{
	pps->cutime = atoi(word);
}

void parse_cstime(struct proc_pid_stat* pps, char word[50])
{
	pps->cstime = atoi(word);
}

void parse_cputime(struct proc_pid_stat* pps)
{
	pps->cputime = pps->utime + pps->stime + pps->cutime + pps->cstime;
}

