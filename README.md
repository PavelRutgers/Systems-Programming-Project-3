Names: Pavel Sverdlov, Nyssa Rawat

NetIDs: ps1329, nr649

Summary: 
This project simulates a simple UNIX shell in C. The shell reads input using read() system call, tokenizes the input, parses it into structured commands, and executes commands. The program supports pipelines, redirection, wildcard expansion, and built in c commands like cd pwd or which. The shell operates in an interactive mode, mimicking the real shell terminal, and batch mode, which takes several commands all at once and executes all of them. In interactive mode, mysh prints a welcome and goodbye message, which it does not do in batch mode.

Design: 
The shell processes each line of input in multiple stages. First, input is read using shell_read_line, which wraps the read() system call. The line is then tokenized into a list of strings, split on whitespace and special symbols. The parser converts tokens into a structured job_t, which contains one or more command_t entries for pipelines. After parsing, wildcard expansion is performed on each command's arguments. Finally, the job is executed as either a command or a pipeline of commands.

Commands are represented with the command_t structure, storing arguments using argv, the number of arguments, and any input output redirection files the command is associated with. Multiple commands are grouped into a job, which allows a pipeline to be represented as an array of commands. Tokens are stored in the dynamically allocated token_list_t.

Testing Process:
We tested the shell incrementally by feature, beginning with input handling and then adding tokenization, parsing, execution, redirection, pipelines, and wildcard expansion. Our goal was to verify both correct behavior on valid input and graceful handling of invalid input. 

For initial testing, occuring shortly after setting up input.c and main.c's skeletons, we simply launched the shell in interactive mode to make sure the welcome and goodbye messages worked, the terminal would correctly exit upon the exit command or blank submission. After implementing the built in commands, we tested how the program would navigate directories and update the prompt accordingly, and ensure that invalid commands didn't crash the shell. We also verified that running the program in batch mode would not print a welcome or goodbye message and only execute the listed commands.

Next, we tested tokenization and parsing by adding additional print statements which would print out all of the tokens in a given command. This ensured whitespace separated commands were working properly and <, >, or | are treated as separate tokens from the other commands, and that comments (anything beginning with #) was ignored. We tested this through standard commands such as 

"echo hello world"

but also tested input redirection and pipeline commands such as 

"echo hi > out.txt"
and 
"ls | wc"

as well as incorrectly formed inputs to ensure that improper use of pipelines and redirection would not pass through our code. 

"cat <"
"echo hello > |" 
Above: improper command examples. 

After confirming that parsing was stable, we tested external command execution. We used commands such as echo hello, /bin/echo hi, ls, and invalid commands such as nosuchcommand. These tests verified that bare command names were searched in the required directories, that path-based commands containing / were executed directly, and that invalid commands produced an error without terminating the shell.

After implementing wildcard expansion, we tested it through commands like echo *.c and echo .*. We also combined this wildcard expansion with our other features, like "echo *.c | wc" which features a pipeline and a wildcard expansion. These tests verified that matching filenames were expanded correctly, unmatched patterns were preserved unchanged, hidden files were not matched by * unless the pattern began with ., and that wildcard expansion worked correctly before execution.

These tests verified that matching filenames were expanded correctly, unmatched patterns were preserved unchanged, hidden files were not matched by * unless the pattern began with ., and that wildcard expansion worked correctly before execution. The folder wc_test was created to test specifically the wildcard features, through files like a.c and b.c which were made to see if wildcard expansion could be used to highlight all files of a given suffex, and many parts of its dna were also implemented into the finalshell_test folder. That finalshell_test folder is where we performed a comprehensive test of all the commands in our shell environment, including the built in commands, pipelines, wildcards, and redirection. We also ensured that things such as writing to files worked correctly, overwriting files rather than appending to them. 

After all of our other testing was complete, we ran the program under valgrind and inputted 2 built in commands, a wildcard expansion, and a pipeline which wrote to a file, and in the end our code passed without leaking any information. 
