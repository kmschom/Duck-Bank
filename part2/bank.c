/*
 * bank.c
 *
 * Created on: Nov 24, 2021
 *     Author: Kelly Schombert
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "string_parser.h"
#include "account.h"
#include <sys/syscall.h>

#define MAX_THREAD 10

pthread_t tid[MAX_THREAD], bank_thread;
pthread_mutex_t lock[MAX_THREAD];

int numofTrans = 0;
int numAccts = 0;
long size = 0;
account *acct_array;

// Create array of account objects; Called by main()
account* create_Accounts(const char* filename)
{

	FILE *inputFile = fopen(filename, "r");
	
	// Getting first line to find number of accounts
	//int numAccts = 0;
	fscanf(inputFile, "%d", &numAccts);
	
	char *line_buf = NULL;
	double bal = 0;
	double rate = 0;
	size_t len = 0;

	char heading[12];

	for(int i = 0; i < numAccts; i++)
	{
		account newAcct;
		
		// Get rid of empty line_buf
		getline(&line_buf, &len, inputFile);

		// Move past index line
		getline(&line_buf, &len, inputFile);

		fscanf(inputFile, "%s", newAcct.account_number);
		fscanf(inputFile, "%s", newAcct.password);
		fscanf(inputFile, "%le", &bal);
		newAcct.balance = bal;
		fscanf(inputFile, "%le", &rate);
		newAcct.reward_rate = rate;

		newAcct.transaction_tracter = 0;
		
		newAcct.ac_lock = lock[i];

		sprintf(heading, "account %d\n", i);
		strcpy(newAcct.out_file, heading);
		
		acct_array[i] = newAcct; 
	}
	
	fclose(inputFile);
	free(line_buf);
}

// Check correct password give for requested account; Called by main() before calling each proccess_transaction()
int check_pw(account *acct_array, int *numAccts, char *acct, char *pw)
{
	for(int i = 0; i < *numAccts; i++)
	{
		if(strcmp(acct_array[i].account_number, acct) == 0)
		{
			if(strcmp(acct_array[i].password, pw) == 0)
			{
				//printf("Matching pw: %s\n", acct_array[i].account_number);
				return 0;
			}else{
				return 1;
			}
		}
	} 
}

// When number of transactions run hits 5000, main() call this function to update each account balance; balance += total transaction amount * reward_rate
void* update_balance(void *arg)
{
	//printf("\n\n\n UPDATE BALANCE CALLED \n\n\n");
	char heading[50];
	
	for(int i = 0; i < numAccts; i++)
	{
		acct_array[i].balance += (acct_array[i].transaction_tracter * acct_array[i].reward_rate);
		acct_array[i].transaction_tracter = 0;
			
		sprintf(heading, "Current Balance:\t%.02f\n", acct_array[i].balance);
		strcat(acct_array[i].out_file, heading);
		//printf("Outfile %d\t %s\n", i, acct_array[i].out_file);
	}
	pthread_exit(NULL);
}

// Takes request tokens from file read in main() to process requested transaction; Called by main()
void* process_transaction(void *arg)
{
	printf("Read command called by %ld\n", pthread_self());
	char** tmp = (char **) arg;
	
	command_line token_buffer;
	char *acct;
	char *acctpw;
	char *acct2;
	double amount;
	int target;
	int target2;

	for(int index = 0; index < ((size/MAX_THREAD)-1); index++)
	{
		token_buffer = str_filler(tmp[index], " ");
		
		acct = token_buffer.command_list[1];
		acctpw = token_buffer.command_list[2];

		// Check that account and password match
		int valid = check_pw(acct_array, &numAccts, acct, acctpw);
		if(valid == 1)
		{
			//printf("Incorrect account password: %s request denied\n", token_buffer.command_list[0]);
		}else{
			// Find account object in account array
			for(int i = 0; i < numAccts; i++)
			{
				if(strcmp(acct_array[i].account_number, acct) == 0)
				{
					target = i;
				}
			}

			if(strcmp(token_buffer.command_list[0], "T") == 0)
			{
				//printf("*****Transfer*****\n");
				acct2 = token_buffer.command_list[3];
				amount = atof(token_buffer.command_list[4]);
		
				for(int j = 0; j < numAccts; j++)
				{
					if(strcmp(acct_array[j].account_number, acct2) == 0)
					{
						target2 = j;
					}
				}

				pthread_mutex_lock(&acct_array[target].ac_lock);
				acct_array[target].balance -= amount;
				acct_array[target].transaction_tracter += amount;
				pthread_mutex_unlock(&acct_array[target].ac_lock);

				pthread_mutex_lock(&acct_array[target2].ac_lock);
				acct_array[target2].balance += amount;
				pthread_mutex_unlock(&acct_array[target2].ac_lock);
		
				numofTrans++;
				//printf("After transfer: %f\n", demand->accts[target].balance);
			}else if(strcmp(token_buffer.command_list[0], "C") == 0)
			{
				//printf("*****Check balance*****\n");
				printf("Account #%s balance: %f\n", acct_array[target].account_number, acct_array[target].balance);
			
			}else if(strcmp(token_buffer.command_list[0], "D") ==0)
			{
				//printf("*****Deposit*****\n");
				amount = atof(token_buffer.command_list[3]);
				
				pthread_mutex_lock(&acct_array[target].ac_lock);
				acct_array[target].balance += amount;
				acct_array[target].transaction_tracter += amount;
				pthread_mutex_unlock(&acct_array[target].ac_lock);
				
				numofTrans++;
				//printf("After deposit: %f\n", demand->accts[target].balance);
			}else if(strcmp(token_buffer.command_list[0], "W") == 0)
			{
				//printf("*****Withdraw*****\n");
				amount = atof(token_buffer.command_list[3]);
				
				pthread_mutex_lock(&acct_array[target].ac_lock);
				acct_array[target].balance -= amount;
				acct_array[target].transaction_tracter += amount;
				pthread_mutex_unlock(&acct_array[target].ac_lock);
				
				numofTrans++;
				//printf("After withdraw: %f\n", demand->accts[target].balance);
			}else
			{
				printf("Invalid Request\n");
			}
		}
	
		free_command_line(&token_buffer);
		memset(&token_buffer, 0, 0);
	}
	
	pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
	// Check for correct args
	if(argc != 2)
	{
		printf("Error: 1 input file required\n");
		exit(-1);
	}

	// Check that file exists
	FILE *inputFile = fopen(argv[1], "r");
	if(inputFile == NULL)
	{
		printf("Error %s does not exist.\n", argv[2]);
		exit(-1);
	}
	fclose(inputFile);
	
	inputFile = fopen(argv[1], "r");
	fscanf(inputFile, "%d", &numAccts);

        acct_array=(account*)malloc(sizeof(account) * (numAccts));
	create_Accounts(argv[1]);
	// All the accounts have been made at this point
	
	char *line_buf = NULL;
	size_t len = 0;
	// Move through first 50 lines of account work
	for(int i = 0; i <= (numAccts * 5); i++)
	{
		getline(&line_buf, &len, inputFile);
	}

	while(getline(&line_buf, &len, inputFile) != -1)
	{
		size++;
	}
	//printf("Size: %ld\n", size);
	fclose(inputFile);

	inputFile = fopen(argv[1], "r");
	// Skip num of account line and account info lines
	for(int i = 0; i <= (numAccts * 5); i++)
	{
		getline(&line_buf, &len, inputFile);
	}
	
	// Create array that will hold commands for threads to start at
	char **comm_array = (char **)malloc(sizeof(char*) * size);
	for(int c = 0; c < size; c++)
	{
		comm_array[c] = (char*)malloc(sizeof(char) * 256);
	}

	for(int shit = 0; shit < size; shit++)
	{
		for(int hell = 0; hell < 256; hell++)
		{
			comm_array[shit][hell] = 0;
		}
	}

	int x = 0;
	while(getline(&line_buf, &len, inputFile) != -1)
	{
		int j = 0;
		while(line_buf[j] != 0)
		{
			comm_array[x][j] = line_buf[j];
			j++;
		}
		x++;
	}
	fclose(inputFile);
	free(line_buf);

	int error;
	// Divvying up sections of commands in array
	for(int a = 0; a < MAX_THREAD; a++)
	{
		int b = a * size/MAX_THREAD; // Numbers off the sections
		error = pthread_create(&(tid[a]), NULL, &process_transaction, (void *) (comm_array + b));
		if (error != 0)
		{
			printf("\nThread can't be created :[%s]", strerror(error));
			exit(EXIT_FAILURE);
		}	
	}

	// Create bank thread to run update_balance
	error = pthread_create(&bank_thread, NULL, &update_balance, &numofTrans);
	if(error != 0)
	{
		printf("\nThread can't be created:[%s]", strerror(error));
		exit(EXIT_FAILURE);
	}



	for(int j = 0; j < MAX_THREAD; j++)
	{
		pthread_join(tid[j], NULL);
		printf("thread %d joins\n", (j + 1));
	}

	printf("THREADS RETURNED\n");

	pthread_join(bank_thread, NULL);
	printf("BANK THREAD RETURNED\n");
	
	for(int t = 0; t < size; t++)
	{
		free(comm_array[t]);
	}
	free(comm_array);

	int numUpdates = 0;	
	update_balance(&numUpdates);

	char heading[50];
	for(int m = 0; m < numAccts; m++)
	{
		//printf("OutFile %d\t %s\n", m, demand.accts[m].out_file);
		sprintf(heading, "%s.txt", acct_array[m].account_number);
		FILE *acct_out = fopen(heading, "w+");
		fprintf(acct_out, "%s", acct_array[m].out_file);
		//printf("GOT HERE\n");
		fclose(acct_out);	
	}

	FILE *outputFile = fopen("output.txt", "w+");
	for(int k = 0; k < numAccts; k++)
	{
		fprintf(outputFile, "%d balance:\t%.2f\n\n", k, acct_array[k].balance);
	}

	fclose(outputFile);
	free(acct_array);
	exit(0);
}
