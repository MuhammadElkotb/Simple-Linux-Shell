#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <syscall.h>
#include <sysexits.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>

#define true 1
#define false 0
#define null NULL

#define Red "\033[0;31m\033[1m"

#define Cyan "\033[0;36m\033[1m"

#define Yellow "\033[0;33m\033[1m"

#define MAX_INPUT_LENGTH 256
#define MAX_ENV_VARS 64

FILE *fptr;


/**
 * @brief a string of max length 256, always holds the current directory 
 */
char currentDir[MAX_INPUT_LENGTH];

/**
 * @brief two strings used when starting and exiting the shell
 * 
 */
char *shellName = {"KOTB SHELL"};
char *shellExit = {"Exiting...."};

/**
 * @brief an array that holds all envrioment variables entered by the user.
 * 
 * the first dimension represents number of env varibales added.
 * the second dimenstion --> the first index holds the variable name.
 *                           the second index holds the value of the variable.
 * the third dimensoin represents the strings stored for each variable name and variable value.
 */
char currentEnv[MAX_ENV_VARS][2][MAX_INPUT_LENGTH];

/**
 * @brief a counter that represents the size of the of the first dimension of currentEnv
 *
 * used when iterating over currentEnv to check for an existence of a certain variable name.
 */
int ctrEnv = 0;


/**
 * @brief helper function that adjusts the color of the text in the terminal
 */
void resetColor(){
	printf("\033[0m");
}

/**
 * @brief helper function that prints @param n as integer ending with a newline
 * 
 * @param n 
 */
void printInt(int n){
    printf("%d\n", n);
}

/**
 * @brief helper function that prints @param string as a string ending with a newline
 * 
 * @param string 
 */
void printString(char *string){
    printf("%s\n", string);
}


/**
 * @brief signal Catcher that method that is called when a child process is terminated 
 * 
 * 
 * the parent process enters an endless loop waiting for a child to return a status exit code
 * the parent escapes the loop when the result of wait is 0, -1 or a PID for a background child.
 * this functions helps the parent process to reap zombie of child if it exists.
 * 
 */
void signalCatcher(){
    pid_t	pid;

    while (true) {
        
        pid = waitpid((pid_t)-1, null, WNOHANG);

        if (pid == 0){
            return;
        }
        else if (pid == -1)
        {
            return;
        }
        else{

            printf(Red"Terminated ");
            printf(Yellow": %d\n", pid);
            fprintf(fptr, "Background Child Terminated (zombie reaped) :- PID -> %d\n", (int)pid);
            fflush(fptr);
            resetColor();
            return;
        }
    }   
}

/**
 * @brief helper function that ensures that @param string and @param substr are equal or not.
 * 
 * if @param equals = 1
 *      then the function acts as isEqual function
 * if @param equals = 0
 *      then the function acts as only contains function where it returns 1 if @param string
 *      contains @param substr
 * 
 * @param string 
 * @param substr 
 * @param equals 
 * @return int 
 */
int contains(char *string, char *substr, int equals){

    int substrLen = strlen(substr);
    int stringLen = strlen(string);

    if(equals){
        if(stringLen != substrLen)
            return 0;
    }
   

    int ctr = 0;

    for(int i = 0; i < stringLen; i++){
        if(string[i] == substr[ctr]){
            ctr++;
        }
        else{
            ctr = 0;
        }
        if(ctr == substrLen){
            return 1;
        }
    }
    return 0;

}


/**
 * @brief helper function that takes input from stdin and store in @param string
 * 
 * @param string 
 */
void getInput(char *string){
    if(fgets(string, MAX_INPUT_LENGTH, stdin) == null){
        printString("COULD NOT GET INPUT");
    }
}

/**
 * @brief function that generates list of arguments from input 
 * 
 * the function seperates strings seperated by a space or a "&"
 * acts on @param input , stores result in @param parsedInput
 * 
 * returns the length of strings that are seperated by space or "&"
 * 
 * @param input 
 * @param parsedInput 
 * @return int 
 */
int parseInput(char *input, char parsedInput[][MAX_INPUT_LENGTH]){
    int ctr = 0;
    int ctr2 = 0;
    for(int i = 0; i < strlen(input); i++){
        if(input[i] == '&'){
            if(ctr2 != 0){
                ctr++;
            }
            parsedInput[ctr][0] = '&';
            ctr++;
            break;
        }
    
        if(isspace(input[i]) || input[i] == '\n' || input[i] == '&'){
            
            if(ctr2 != 0){
                ctr++;
            }
            ctr2 = 0;
        }
        else{
           
            parsedInput[ctr][ctr2] = input[i];
            ctr2++;
        }
        
    }
    return ctr;
}


/**
 * @brief function that takes strings seperated by space to place them in an args array
 * 
 * @param args 
 * @param parsedInput 
 * @param commmandCtr 
 */
void parseArgs(char *args[], char parsedInput[][MAX_INPUT_LENGTH], int commmandCtr){
    for(int i = 0; i < commmandCtr; i++){
        args[i] = parsedInput[i];
    }
    args[commmandCtr] = NULL;

}


/**
 * @brief function that takes list of arguments to be executed based on foreground int value
 * 
 * the function takes list of args to be executed and forks the process
 * if the child process fails it exits
 * 
 * if @param foreground is 1 then the parent waits for the child process to terminate 
 * if @param foreground is 0 then the parent doesn't wait for the child.
 * 
 * @param args 
 * @param foreground 
 */
void execute(char *args[], int foreground){
    pid_t pid = fork();


    if(pid == -1){
        printString("Could not Fork sorry");
    }
    if(pid == 0){
        execvp(args[0], args);
        printf("\"%s\" : ", args[0]);
        printString(Red"Command not found.");
        exit(EXIT_FAILURE);
    }
    else if( pid > 0 && foreground == 1){
        if(waitpid((pid_t)-1, null, WUNTRACED) > 0){
            fprintf(fptr, "Foreground Child Terminated :- PID -> %d\n", (int)pid);
            fflush(fptr);
        }
    }
    resetColor();
}

/**
 * @brief function that changes working directory based on @param args given.
 * 
 * @param args 
 */
void changeDir(char *args[]){
    if(args[1] == null){
        chdir(getenv("HOME"));
    }
    else if(contains(args[1], "~", 1)){
        strcpy(currentDir, "~");
        chdir(getenv("HOME"));
    }
    else if(contains(args[1], "..", 1)){
        strcpy(currentDir, "..");
        chdir(currentDir);

       
    }
    else{
        strcpy(currentDir, args[1]);
        if(chdir(currentDir) == 0){
            chdir(currentDir);
        }
        else{
            printf(Red"%s : No such File or Directory.\n", args[1]);
        }
    }
    resetColor();
    
    getcwd(currentDir, MAX_INPUT_LENGTH);
}

/**
 * @brief helper function that is called upon calling "exit()" command.
 * 
 */
void exitShell(){
    printString(Cyan);
    printf("\n\n                                                                      ");
    for(int i = 0; i < strlen(shellExit); i++){
        printf("%c",shellExit[i]);
        fflush(stdout);
        usleep(50000);
    }
    printf("\n");

    exit(EXIT_SUCCESS);
}

/**
 * @brief helper function that is called upon starting the shell.
 * 
 */
void startShell(){
    printString(Cyan);
	printf("                                                                      ");
	for(int i = 0; i < strlen(shellName); i++){
		printf("%c",shellName[i]);
		fflush(stdout);
		usleep(50000);
	}
    
	printf("\n\n");
}

/**
 * @brief function is called upon calling "export var=value" command.
 * 
 * the function adds the variable given in string @param input to the enviroment and stores it
 * in the global array currentEnv.
 * 
 * it checks if the given variable already exists 
 *                      if yes... it overwrites its current value by the given on.
 *                      if no... it adds the variable to the currenEnv array and increments ctrEnv
 * 
 * @param input 
 * @return int 
 */
int addVar(char *input){
    char value[MAX_INPUT_LENGTH];
    char var[MAX_INPUT_LENGTH];


    memset(value, 0, sizeof value);
    memset(var, 0, sizeof var);

    int equalIndex = -1;

    for(int i = 0; i < strlen(input); i++){
        if(input[i] == '='){
            equalIndex = i;
            break;
        }
    }
    if(equalIndex == -1)
        return -1;


    int ctrValue = 0;
    int ctrVar = 0;

    int foundVal = false;
    int foundQuote = false;
    for(int i = equalIndex + 1; i < strlen(input); i++){
        if(input[i] == '\"'){
            if(foundQuote == true){
                break;
            }
            foundQuote = true;
            continue;
        }
        if(foundQuote){
            value[ctrValue] = input[i];
            ctrValue++;
        }
        else{
            if(isspace(input[i]))
                break;
            value[ctrValue] = input[i];
            ctrValue++;
        }
        
    }
    for(int i = equalIndex - 1; i >= 0; i--){
        if(isspace(input[i]) || input[i] == '\"')
            break;
        
        var[ctrVar] = input[i];
        ctrVar++;
    }
    if(ctrVar == 0)
        return -2;
    for(int i = 0; i < ctrVar / 2; i++){
        char c = var[i];
        var[i] = var[ctrVar - i - 1];
        var[ctrVar - i - 1] = c;
    }

    
    setenv(var, value, 1);

    int varIndex = -1;
    
    for(int i = 0; i < ctrEnv; i++){
        if(contains(currentEnv[i][0], var, 1)){
            varIndex = i;
        }
    }

    if(varIndex != -1){
        memset(currentEnv[varIndex], 0, sizeof currentEnv[varIndex]);
        strcpy(currentEnv[varIndex][0], var);
        strcpy(currentEnv[varIndex][1], value);
    }
    else{
        memset(currentEnv[ctrEnv], 0, sizeof currentEnv[ctrEnv]);
        strcpy(currentEnv[ctrEnv][0], var);
        strcpy(currentEnv[ctrEnv][1], value);
        ctrEnv++;
    }

    return 1;
    
}


/**
 * @brief function that replaces all "$" with the coressponding variable name after the sign.
 * 
 * takes action on @param input 
 * stores the result in @param inputExec 
 * 
 * @param inputExec 
 * @param input 
 */
void evalExpression(char *inputExec, char *input){
    char var[MAX_INPUT_LENGTH];
    memset(var, 0, sizeof var);


    int ctrVar = 0;
    int ctrOut = 0;

    int foundSign = false;

    for(int i = 0; i < strlen(input); i++){
        if(input[i] == '$'){
            foundSign = true;
        }
        if(foundSign && !isspace(input[i]) && input[i] != '\n' && input[i] != '\"'){
            if(input[i] != '$'){
                var[ctrVar] = input[i];
                ctrVar++;
            }
        }
        if(!foundSign){
            inputExec[ctrOut] = input[i];
            ctrOut++;
        }
        if(foundSign){
            if(isspace(input[i]) || input[i] == '\n' || input[i] == '\"' || input[i] == '$'){
                if(var != null){
                    if(getenv(var) != null){
                        for(int k = 0; k < strlen(getenv(var)); k++){
                            inputExec[ctrOut] = getenv(var)[k];
                            ctrOut++;
                        }
                    }
                }
                ctrVar = 0;
                memset(var, 0, sizeof var);
                if(input[i] != '$'){
                    foundSign = false;
                    i--;
                }
            }
           
        }   
    }
}

/**
 * @brief function that echoes every given string that is between double quotes.
 * 
 * @param input 
 */
void echoPrint(char *input){
    int foundQuote = false;
    for(int i = 0; i < strlen(input); i++){
        if(input[i] == '\"'){
            if(foundQuote){
                break;
            }
            foundQuote = true;
            continue;
        }
        if(foundQuote){
            printf("%c", input[i]);
        }
    }
    printf("\n");
}


/**
 * @brief function that is called upon calling "export" command
 * 
 * prints all enviroment variables added during the current shell session.
 * 
 */
void printEnv(){
    for(int i = 0; i < ctrEnv; i++){
        printf("%s = ", currentEnv[i][0]);
        printf("%s\n", currentEnv[i][1]);
    }
}


/**
 * @brief function that returns 1 if the given @param args list has "&" as its last argument 
 * 
 *  returns 0 otherwise.
 * 
 * @param args 
 * @param commandCtr 
 * @return int 
 */
int checkForeground(char *args[], int commandCtr){
    int foreground = true;
    if(contains(args[commandCtr - 1], "&", 1)){
        foreground = false;
    }
    if(foreground == false){
        args[commandCtr - 1] = null; 
    }
		
    return foreground;
}


/**
 * @brief sets the working directory to be "HOME" upen starting the shell.
 * 
 */
void setUpEnv(){
    chdir(getenv("HOME"));
	getcwd(currentDir, MAX_INPUT_LENGTH);
	fptr = fopen(strcat(currentDir, "/testShell.txt"), "w");
	getcwd(currentDir, MAX_INPUT_LENGTH);
}




