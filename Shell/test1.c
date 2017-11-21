#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX 100
#define LEN 100

//shell指令单个管道结构体
struct cmd_list{  
    int argc;  //单个管道参数个数
    char *argv[MAX];
};

struct cmd_list *cmdv[MAX];  //shell指令
int num;//shell管道个数
int flagdo;//是否为后台处理命令标记

//执行外部命令
void execute(char *argv[])
{
        int error;
        error=execvp(argv[0],argv);
        if (error==-1)  printf("failed!\n");
        exit(1);
}

//切分单个管道
void split_cmd(char *line)
{
     struct cmd_list * cmd = (struct cmd_list *)malloc(sizeof(struct cmd_list));
     cmdv[num++] = cmd;
     cmd->argc = 0;
     char *save;
     char *arg = strtok_r(line, " \t", &save);//切分空格
     while (arg)
     {
        cmd->argv[cmd->argc] = arg;
        arg = strtok_r(NULL, " \t", &save);
        cmd->argc++;
     }
     cmd->argv[cmd->argc] = NULL;
}

//切分管道
void split_pipe(char *line)
{
    char *save;
    char * cmd = strtok_r(line, "|", &save);
    while (cmd) {
        split_cmd(cmd);
        cmd = strtok_r(NULL, "|", &save);
    }
}

//执行管道命令
void do_pipe(int index)
{
    if (index == num - 1)
        execute(cmdv[index]->argv);
    int fd[2];
    pipe(fd);//创建管道，0读，1写
    if (fork() == 0)
    {
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        execute(cmdv[index]->argv);
    }
    dup2(fd[0], 0);
    close(fd[0]);
    close(fd[1]);
    do_pipe(index + 1);
}

//执行内部指令
int inner(char *line)
{
    char *save,*tmp[MAX];
    char t[LEN],p[LEN];
    strcpy(t,line);
    char *arg = strtok_r(line, " \t", &save);//切分空格
    int i=0;
    while (arg) {
        tmp[i] = arg;
        i++;//记录命令个数
        arg = strtok_r(NULL, " \t", &save);
     }
    tmp[i] = NULL;
    if (strcmp(tmp[i-1],"&")==0)//判断是否为后台处理命令
    {
        flagdo=1;
        i--;
    }
    if (strcmp(tmp[0],"exit")==0)//exit
    {
        exit(0);
        return 1;
    }
    else
    if (strcmp(tmp[0],"pwd")==0)//pwd
    {
        char buf[LEN];
        getcwd(buf,sizeof(buf));//得到当前路径
        printf("Current dir is:%s\n",buf);
        return 1;
    }
    else
    if (strcmp(tmp[0],"cd")==0)//cd
    {
        char buf[LEN];
        if (chdir(tmp[1])>=0)
        {
            getcwd(buf,sizeof(buf));
            printf("Current dir is:%s\n",buf);
        }
        else
        {
            printf("Error path!\n");
        }
        return 1;
    }
    else return 0;
}

//输入重定向
void cat_in(char *q)
{
    char t[30];
    int fd;
    if (q[0]=='<')
    {
        strcpy(t,q+1);
        fd=open(t,O_RDONLY);
        cmdv[0]->argv[cmdv[0]->argc-1]=NULL;  //默认重定向为参数的最后一个
        cmdv[0]->argc--;
        if (fd==-1)
        {
            printf("file open failed\n");
            return;
        }
        dup2(fd,0);
        close(fd);
    }
}

//输出重定向
void cat_out(char *q)
{
    char t[30];
    int fd;
    if (q[0]=='>')
    {
        strcpy(t,q+1);
        cmdv[num-1]->argv[cmdv[num-1]->argc-1]=NULL;
        cmdv[num-1]->argc--;
        fd=open(t,O_CREAT|O_RDWR,0666); //0666为权限
        if (fd==-1)
        {
            printf("file open failed\n");
            return;
        }
        dup2(fd,1);
        close(fd);
    }
}

int main()
{
    int i,pid;
    char buf[LEN],p[LEN];
    while (1)
    {
        fgets(buf,LEN,stdin);//读入shell指令
        if (buf[0]=='\n') continue;
        buf[strlen(buf)-1]='\0';
        strcpy(p,buf);
        int inner_flag;
        inner_flag=inner(buf);//内置指令执行
        if (inner_flag==0)
        {
            pid=fork();//建立新的进程
            if (pid==0)
            {
                split_pipe(p);//管道的切割
                //如果是后台处理命令将&符号删除
                if (strcmp(cmdv[num-1]->argv[cmdv[num-1]->argc-1],"&")==0)
                {
                    cmdv[num-1]->argc--;
                    cmdv[num-1]->argv[cmdv[num-1]->argc]=NULL;
                }
                //默认输入输出重定向都是最后一个参数，输入时第一个管道，输出是最后一个管道
                if (cmdv[0]->argv[cmdv[0]->argc-1]!=NULL)
                {
                    char q[LEN];
                    strcpy(q,cmdv[0]->argv[cmdv[0]->argc-1]);
                    cat_in(q);//输入重定向
                }
                if (cmdv[num-1]->argv[cmdv[num-1]->argc-1]!=NULL)
                {
                    char q[LEN];
                    strcpy(q,cmdv[num-1]->argv[cmdv[num-1]->argc-1]);
                    cat_out(q);//输出重定向
                }
                do_pipe(0);//执行管道
                exit(0);
            }
            if (flagdo==0)//非后台处理命令主进程才需等待子进程处理
                waitpid(pid,NULL,0);
        }
    }
    return 0;
}