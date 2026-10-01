// shm_processes.c - Lab 3 Part 2 (Sumayia Moore): strict alternation, shared memory
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

void ParentProcess(volatile int []);
void ClientProcess(volatile int []);

int main(void)
{
     int ShmID, status;
     int *ShmPTR;   // ShmPTR[0] = BankAccount, ShmPTR[1] = Turn
     pid_t pid;

     ShmID = shmget(IPC_PRIVATE, 2*sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) { printf("*** shmget error ***\n"); exit(1); }
     ShmPTR = (int *) shmat(ShmID, NULL, 0);
     if (ShmPTR == (int *) -1) { printf("*** shmat error ***\n"); exit(1); }
     ShmPTR[0] = 0;
     ShmPTR[1] = 0;

     pid = fork();
     if (pid < 0) { printf("*** fork error ***\n"); exit(1); }
     if (pid == 0) { ClientProcess(ShmPTR); exit(0); }

     ParentProcess(ShmPTR);
     wait(&status);
     shmdt((void *) ShmPTR);
     shmctl(ShmID, IPC_RMID, NULL);
     return 0;
}

void ParentProcess(volatile int M[])
{
     int account, balance, i;
     srandom(time(NULL) ^ getpid());
     for (i = 0; i < 25; i++) {
          sleep(random() % 6);
          account = M[0];
          while (M[1] != 0);          // wait for Turn == 0
          account = M[0];
          if (account <= 100) {
               balance = random() % 101;
               if (balance % 2 == 0) {
                    account += balance;
                    printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
               } else
                    printf("Dear old Dad: Doesn't have any money to give\n");
          } else
               printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
          M[0] = account;
          M[1] = 1;                   // child's turn
     }
}

void ClientProcess(volatile int M[])
{
     int account, balance, i;
     srandom(time(NULL) ^ getpid());
     for (i = 0; i < 25; i++) {
          sleep(random() % 6);
          account = M[0];
          while (M[1] != 1);          // wait for Turn == 1
          account = M[0];
          balance = random() % 51;
          printf("Poor Student needs $%d\n", balance);
          if (balance <= account) {
               account -= balance;
               printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
          } else
               printf("Poor Student: Not Enough Cash ($%d)\n", account);
          M[0] = account;
          M[1] = 0;                   // parent's turn
     }
}
