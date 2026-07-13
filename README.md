ST5004CEM Operating Systems and Security - Coursework
Student: Saugat Neupane
Student ID: 240553
Module: ST5004CEM
Institution: Softwarica College of IT & E-Commerce (Coventry University)
GitHub: github.com/SAUGAT960/ST5004CEM-OS-Coursework



About This Repository
This repository holds the C source code for the ST5004CEM coursework. It uses standard POSIX libraries to show four main OS concepts: process management, memory simulation, secure file operations, and network programming.



File Structure
task1 handles process management and threading using a producer-consumer setup, a round-robin scheduler, and deadlock prevention.
task2 simulates memory management with a paging system that runs FIFO and LRU page replacement.
task3 manages files securely using authentication, permissions, encryption, and logs.
task4_server runs a TCP socket server that handles multiple clients at the same time.
task4_client runs a TCP socket client to communicate with the server.
README.md explains how to compile and run the code.



Compilation & Execution Instructions
You need a Linux/Unix system with terminal access and the GCC compiler installed. On Ubuntu, install it using sudo apt install gcc.


Running the Tasks
For Task 1, compile with gcc task1 -o task1_output and run with ./task1_output.
For Task 2, compile with gcc task2 -o task2_output and run with ./task2_output.
For Task 3, compile with gcc task3 -o task3_output and run with ./task3_output.
For Task 4, open two terminals:
In Terminal 1, start the server by running gcc task4_server -o task4_server_output and then ./task4_server_output
In Terminal 2, compile the client with gcc task4_client -o task4_client_output. You can test it by running ./task4_client_output 1 "AUTH:admin", ./task4_client_output 2 "MSG:Hello Server", or ./task4_client_output 3 "QUIT".



Libraries Used
This project relies entirely on standard system libraries, so you do not need to install any third-party dependencies.

POSIX Threads (<pthread.h>) handles multi-threading, mutexes, and condition variables.
POSIX Semaphores (<semaphore.h>) provides counting semaphores for the producer-consumer model.
Standard I/O (<stdio.h>) manages console output and basic file operations.
Sandard Library (<stdlib.h>) handles memory allocation and random numbers.
String Library (<string.h>) handles string manipulation.
Time Library (<time.h>) provides timestamps and time delays.
UNIX Standard (<unistd.h>) provides standard system calls like sleep(), usleep(), and fork().
Sockets (<sys/socket.h>, <netinet/in.h>, <arpa/inet.h>) handle network programming.
Error Handling (<errno.h>) provides standard system error codes.



