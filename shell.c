#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define LINE_SIZE 255
#define ARG_SIZE 20
#define HISTORY_SIZE 50
#define GREEN  "\x1B[32m"
#define NORMAL  "\x1B[0m"
#define BLUE  "\x1B[34m"
#define RED  "\x1B[31m"
struct record{
    char line[LINE_SIZE];
};


void printEnvironment();
void checkUpperCase(char *command);
int isMyCommand(char ** parsed);
void addHistory(char * line, struct record * history,int * lineNum);
void execute(char ** parsed);
void parse(char * cmd , char ** parsed);
char * readCommandLine(struct record * history, int * cursor);
int arrowKeyCheck(char * line , struct record * history, int * cursor);

/*Checks if the arrow key is entered*/
int arrowKeyCheck(char * line , struct record * history, int * cursor){
    if(line[0] == '\033' && line[1]=='['){
        if(line[2] == 'A'){
            (*cursor) --;
            printEnvironment();
            printf("%s",history[*cursor].line);
            return 1;

        }
        else if(line[2] == 'B')
            return 2;
    }
    return 0;

}

/*prints user and directory information*/
void printEnvironment(){

    char * username = getenv("USER");
    long size;
    char *directory;

    size = pathconf(".",_PC_PATH_MAX);
    if ((directory = (char *)malloc((size_t)size)) != NULL)
        getcwd(directory, (size_t)size);

    printf("%s%s: %s%s $%s",GREEN,username,BLUE,directory,NORMAL);

}


void checkUpperCase(char *command){
/*if command's last character before null is 'L'
then converts all command string to lower case*/
    int i =0;
    if(command[ strlen(command) -1] == 'L'){
        while( !(command[i] == 'L'  && command[i+1] == NULL ) ) {
            //if upper case convert to lower case
            if (command[i] >= 'A' && command[i] <= 'Z')
                 command[i] = command[i] + 'a' - 'A';
            i++;
        }
        command[i] = '\0';
    }

}

int isMyCommand(char ** parsed){

    if( strcmp(parsed[0],"cd") == 0){
        chdir(parsed[1]);
        return 1;
    }
    else if(strcmp(parsed[0],"exit") == 0){
        exit(EXIT_SUCCESS);
        return 1;
    }
    else if(strcmp(parsed[0],"help") == 0){
        printf("* This shell is written using only standard c libraries and modules.\n"
                "* Supports cd , exit ,help and most of UNIX shell commands.\n"
                "* You can press the up arrow key and then enter key for previous command.\n"
                "  NOTE:It can only call previous command and does not support editing.\n"
                "* Extra feature: If you incorrectly enter the command with uppercase ,it is okay.\n"
                "You just need to 'L' character to the end of command without any space\n"
                "  EXAMPLE : 'LS -AL' converted to 'ls - a' automaticly before execution \n"
                "* Pipe '|' symbol is NOT supported\n");
        return 1;
    }


    return 0;

}

void addHistory(char * line, struct record * history,int * lineNum){

    //if history is full then lineNum is reset and last record is move to the head of history
    if(*lineNum >= HISTORY_SIZE ){
        strcpy(history[0].line,history[*(lineNum -1)].line);
        *lineNum = 1;
    }
    strcpy(history[*lineNum].line,line);
    //printf("%s copied to history \n",history[*lineNum].line);
    (*lineNum)++;
    return ;

}
void execute(char ** parsed){

    if( isMyCommand(parsed) ){
        return;
    }

    pid_t child_pid;

    if ((child_pid = fork()) < 0) {
        printf("error: forking child process failed\n");
        return;
    }

    // The child process
    if (child_pid == 0) {
        if (execvp(parsed[0],parsed)<0) {
            printf("%serror: command execution failed\n%s",RED,NORMAL);
            exit(0);
        }
    }
    //The parent process
    else {
        wait(NULL);
    }


}
void parse(char * cmd , char ** parsed){

    int i = 0;
    do{
        parsed[i] = strsep(&cmd, " ");
        i++;

    }while(parsed[i-1] != NULL);

}
char * readCommandLine(struct record * history, int * cursor){

    int c;
    int index = 0;
    char *buffer = (char *) malloc(sizeof(char) * LINE_SIZE);

    if (!buffer) {
    printf("Memory Allocation error\n");
    exit(EXIT_FAILURE);
    }

    c = getchar();
    while (c != EOF && c != '\n'){
        buffer[index] = c;
        index++;

        c = getchar();

        if ( *cursor != 0 && arrowKeyCheck(buffer,history,cursor) != 0){
        //cursor 0 means no command history
        // arrowKeyCheck returns 0 when command is not the arrow key

            strcpy(buffer,history[*cursor].line);
             do{
                c = getchar();
             }while (c != EOF && c != '\n');
             return buffer;
        }

    }
    buffer[index] = '\0';

    checkUpperCase(buffer);

    return buffer;

}

int main(){

    int lineNum = 0; // number of entered line
    char * line;
    char *parsed[ARG_SIZE];
    char *parsedwithPipe[ARG_SIZE];
    int arrow;
    int  cursor;
    struct record history[HISTORY_SIZE];
    printf("Hello %s \nYou can use the \"help\" command to "
    "get information about the shell\n",getenv("USER"));

    while(1){
        cursor = lineNum;
        printEnvironment();
        line = readCommandLine(history,&cursor);
        if(arrow == 0){
            addHistory(line,history,&lineNum);
            parse(line,parsed);
            execute(parsed);
        }

    }
    return 0;
}


