

#include "funcs.c"




int main(int argc, char *argv[]){


	signal(SIGCHLD, signalCatcher);
	setUpEnv();

	char input[MAX_INPUT_LENGTH];
	char inputExec[MAX_INPUT_LENGTH];
	char parsedInput[MAX_INPUT_LENGTH][MAX_INPUT_LENGTH];
 
	startShell();


	while(true){


		memset(parsedInput, 0, sizeof parsedInput);
    	memset(inputExec, 0, sizeof inputExec);
		memset(input, 0, sizeof input);

		printf(Yellow"%s@~ ", currentDir);
		resetColor();

		getInput(input);

		evalExpression(inputExec, input);

		int commandCtr = parseInput(inputExec, parsedInput);

		
		if(strlen(parsedInput[0]) == 0){
			continue;
		}
		
		char *args[commandCtr + 1];

	
		parseArgs(args, parsedInput, commandCtr);


		if(contains(args[0], "exit", 1)){
			fclose(fptr);
			exitShell();
		}
		else if(contains(args[0], "cd", 1)){
			memset(currentDir, 0, sizeof currentDir);
			changeDir(args);

			continue;
		}
		else if(contains(args[0], "echo", 1)){
			echoPrint(inputExec);
			continue;
		}
		else if(contains(args[0], "export", 1)){
			int retVar = addVar(inputExec);
			if(args[1] == null){
				printEnv();
			}
			continue;
		}
	
		int foreground = checkForeground(args, commandCtr);

		
		execute(args, foreground);
	}
	
	fclose(fptr);

    return 0;
}

























