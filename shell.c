#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"


#define INPUT 0
#define OUTPUT 1
#define APPEND 2




void removeWhiteSpace(char* buf)
{
    if(buf[strlen(buf)-1]==' ' || buf[strlen(buf)-1]=='\n')
        buf[strlen(buf)-1]='\0';
    if(buf[0]==' ' || buf[0]=='\n')
        memmove(buf, buf+1, strlen(buf));
}



void tokenize_buffer(char** param,int *nr,char *buf,const char *c)
{
    char *token;
    token=strtok(buf,c);
    int pc=-1;
    while(token)
    {
        param[++pc]=malloc(sizeof(token)+1);
        strcpy(param[pc],token);
        removeWhiteSpace(param[pc]);
        token=strtok(NULL,c);
    }
    param[++pc]=NULL;
    *nr=pc;
}





void executeBasic(char** argv)
{
    if(fork()>0)
    {
        wait(NULL);
    }
    else
    {
        execvp(argv[0],argv);
            perror(ANSI_COLOR_RED   "invalid input (commande n'est pas valide ) "   ANSI_COLOR_RESET "\n");
        exit(1);
    }
}


void executePiped(char** buf,int nr)
{
    if(nr>10) return;

    int fd[10][2],i,pc;
    char *argv[100];

    for(i=0; i<nr; i++)
    {
        tokenize_buffer(argv,&pc,buf[i]," ");
        if(i!=nr-1)
        {
            if(pipe(fd[i])<0)
            {
                perror("pipe creating was not successfull\n");
                return;
            }
        }

        if(fork()==0)
        {
            if(i!=nr-1)
            {
                dup2(fd[i][1],1);
                close(fd[i][0]);
                close(fd[i][1]);
            }

            if(i!=0)
            {
                dup2(fd[i-1][0],0);
                close(fd[i-1][1]);
                close(fd[i-1][0]);
            }
            execvp(argv[0],argv);
            perror(ANSI_COLOR_RED   "invalid input "   ANSI_COLOR_RESET );
            exit(1);
        }

        if(i!=0)
        {
            close(fd[i-1][0]);
            close(fd[i-1][1]);
        }
        wait(NULL);
    }
}



void executeAsync(char** buf,int nr)
{
    int i,pc;
    char *argv[100];
    for(i=0; i<nr; i++)
    {
        tokenize_buffer(argv,&pc,buf[i]," ");
        if(fork()==0)
        {
            if(strstr(argv[0],"cd"))
            chdir(argv[1]);
            else{
            execvp(argv[0],argv);
            perror(ANSI_COLOR_RED   "invalid input "   ANSI_COLOR_RESET);
            exit(1);
            }
        }
        else
           wait(NULL);
    }
}


void executeRedirect(char** buf,int nr,int mode)
{
    int pc,fd;
    char *argv[100];
    removeWhiteSpace(buf[1]);
    tokenize_buffer(argv,&pc,buf[0]," ");
    if(fork()==0)
    {

        switch(mode)
        {
        case INPUT:
            fd=open(buf[1],O_CREAT|O_WRONLY, 0777);
            break;
        case OUTPUT:
            fd=open(buf[1],O_CREAT|O_WRONLY, 0777);
            break;
        case APPEND:
            fd=open(buf[1],O_WRONLY | O_APPEND);
            break;
        default:
            return;
        }

        if(fd<0)
        {
            perror("cannot open file\n");
            return;
        }

        switch(mode)
        {
        case INPUT:
            dup2(fd,0);
            break;
        case OUTPUT:
            dup2(fd,1);
            break;
        case APPEND:
            dup2(fd,1);
            break;
        default:
            return;
        }
        execvp(argv[0],argv);
        perror(ANSI_COLOR_RED   "invalid input (erreur de redirection) "   ANSI_COLOR_RESET);
        exit(1);
    }
    wait(NULL);
}




int main(char** argv,int argc)
{
    char buf[500],ch1[500],*buffer[100],buf2[500],buf3[500], *params1[100],*params2[100],*token,cwd[1024];
    int nr=0,i=0;
    FILE* temp;



    printf(ANSI_COLOR_GREEN "*****************Shell Linux******************"   ANSI_COLOR_RESET "\n");
    debut:
    while(1)
    {

        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {	char hostn[1204] = "";
	       gethostname(hostn, sizeof(hostn));
            printf(ANSI_COLOR_GREEN  "%s@%s %s %%  " ANSI_COLOR_RESET, hostn,hostn,cwd);
        }
        else 	perror("getcwd failed\n");

        fgets(buf, 500, stdin);
       strcpy(ch1,buf);

            temp=fopen("/home/nadim/history.txt","a+");
            fprintf(temp,"%s",ch1);
            fclose(temp);


		if(strchr(buf,'|')){
			tokenize_buffer(buffer,&nr,buf,"|");
			executePiped(buffer,nr);
		}


		else if(strstr(buf,";")){
			tokenize_buffer(buffer,&nr,buf,";");
			executeAsync(buffer,nr);
		}
		else if(strstr(buf,"||")){
			tokenize_buffer(buffer,&nr,buf,"||");
			executeAsync(buffer,nr);
		}
		else if(strstr(buf,"&&")){
			tokenize_buffer(buffer,&nr,buf,"&&");
            executeAsync(buffer,nr);
		}



		else if(strstr(buf,"./")){

                tokenize_buffer(params1,&nr,buf," ");

                FILE* temp1;
                if(temp1=fopen(params1[0],"r"))
                { printf("Mode shell:\n");
                char ch[200];

                while(fgets(ch,sizeof ch,temp1)!=NULL)
                {
                    printf("%d. %s\n",++i,ch);

                }
                   executeBasic(params1);
                }
                else
              {
               perror(ANSI_COLOR_RED   "invalid input (fichier batsh n'existe pas) "   ANSI_COLOR_RESET);
              exit(0);
              }
		}





        else if(strstr(buf,">>"))
        {
            tokenize_buffer(buffer,&nr,buf,">>");
            if(nr==2)executeRedirect(buffer,nr,APPEND);
            else printf("Incorrect output redirection!(has to to be in this form: command >> file)");
        }
        else if(strchr(buf,'>'))
        {
            tokenize_buffer(buffer,&nr,buf,">");
            if(nr==2)executeRedirect(buffer,nr, OUTPUT);
            else printf("Incorrect output redirection!(has to to be in this form: command > file)");
        }
        else if(strchr(buf,'<'))
        {
            tokenize_buffer(buffer,&nr,buf,"<");
            if(nr==2)executeRedirect(buffer,nr, INPUT);
            else printf("Incorrect input redirection!(has to to be in this form: command < file)");
        }


        else
        {  tokenize_buffer(params1,&nr,buf," ");
            if(strstr(params1[0],"cd"))
            {
                if(chdir(params1[1]))
                 perror(ANSI_COLOR_RED   "invalid input (dossier n'existe pas) "   ANSI_COLOR_RESET);
            }

            else if(strstr(params1[0],"history"))
            {
                temp=fopen("/home/nadim/history.txt","r");
                char ch[500];

                while(fgets(ch,sizeof ch,temp)!=NULL)
                {printf("%d. %s\n",++i,ch);}
            }


            else if(strstr(params1[0],"quit"))
            {
                exit(0);
            }
            else executeBasic(params1);
        }
    }
    return 0;
}
