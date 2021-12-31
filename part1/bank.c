/*
 * bank.c
 *
 * Created on: Nov 16, 2021
 *     Author: Kelly Schombert
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "string_parser.h"
#include "account.h"


int numofTrans = 0;

// Create array of account objects; Called by main()
account* create_Accounts(const char* filename)
{

	FILE *inputFile = fopen(filename, "r");
	
	// Getting first line to find number of accounts
	int numAccts = 0;
	fscanf(inputFile, "%d", &numAccts);
	
	char *line_buf = NULL;
	double bal = 0;
	double rate = 0;
	size_t len = 0;

	char heading[12];

	account *acct_array;
        acct_array=(account*)malloc(sizeof(account) * (numAccts));
	
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

		sprintf(heading, "account %d\n", i);
		strcpy(newAcct.out_file, heading);
		
		acct_array[i] = newAcct; 
	}
	
	fclose(inputFile);
	free(line_buf);

	return acct_array;
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
				return 0;
			}else{
				return 1;
			}
		}
	} 
}

// Takes request tokens from file read in main() to process requested transaction; Called by main()
void* process_transaction(transact *demand)
{
	char *end;
	command_line request = demand->request;
	char *ttype = request.command_list[0];
	char *acct = request.command_list[1];
	char *acctpw = request.command_list[2];
	char *acct2;
	double amount;
	int target;
	int target2;

	char heading[50];
	// Find account object in account array
	for(int i = 0; i < demand->numofAccts; i++)
	{
		if(strcmp(demand->accts[i].account_number, acct) == 0)
		{
			target = i;
		}
	}

	
	if(strcmp(ttype, "T") == 0)
	{
		acct2 = request.command_list[3];
		amount = strtod(request.command_list[4], &end);
		
		// Find recieving account object in account array
		for(int i = 0; i < demand->numofAccts; i++)
		{
			if(strcmp(demand->accts[i].account_number, acct2) == 0)
			{
				target2 = i;
			}
		}
		
		demand->accts[target].balance -= amount;
		demand->accts[target2].balance += amount;
		demand->accts[target].transaction_tracter += amount;
		numofTrans++;
	}else if(strcmp(ttype, "C") == 0)
	{
		printf("Account #%s balance: %f\n", demand->accts[target].account_number, demand->accts[target].balance);
	}else if(strcmp(ttype, "D") == 0)
	{
		amount = strtod(request.command_list[3], &end);

		demand->accts[target].balance += amount;
		demand->accts[target].transaction_tracter += amount;
		numofTrans++;
	}else if(strcmp(ttype, "W") == 0)
	{
		amount = strtod(request.command_list[3], &end);

		demand->accts[target].balance -= amount;
		demand->accts[target].transaction_tracter += amount;
		numofTrans++;
	}else
	{
		printf("Invalid Request\n");
	}


	return NULL;
}

// When number of transactions run hits 5000, main() call this function to update each account balance; balance += total transaction amount * reward_rate
void* update_balance(transact *timer)
{
	char heading[50];
	
	for(int i = 0; i < timer->numofAccts; i++)
	{
		timer->accts[i].balance += (timer->accts[i].transaction_tracter * timer->accts[i].reward_rate);
		timer->accts[i].transaction_tracter = 0;
		
		sprintf(heading, "Current Balance:\t%.02f\n", timer->accts[i].balance);
		strcat(timer->accts[i].out_file, heading);
	}
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
	int numAccts = 0;
	fscanf(inputFile, "%d", &numAccts);

	account *acct_array = create_Accounts(argv[1]);
	transact demand;
	demand.accts = acct_array;
	demand.numofAccts = numAccts;

	// All the accounts have been made at this point
	// Read through the lines and process the transactions
	
	char *line_buf = NULL;
	size_t len = 0;
	// Move through first 50 lines of account work
	for(int i = 0; i <= (numAccts * 5); i++)
	{
		getline(&line_buf, &len, inputFile);
	}
	
	// Read transactions to EOF
	command_line token_buffer;
	char *acct;
	char *acctpw;
	while(getline(&line_buf, &len, inputFile) != -1)
	{
		token_buffer = str_filler(line_buf, " ");
		demand.request = token_buffer;
	
		acct = token_buffer.command_list[1];
		acctpw = token_buffer.command_list[2];
	
		// Check that account and password match
		int valid = check_pw(acct_array, &numAccts, acct, acctpw);
		if(valid == 1)
		{
			printf("Incorrect account password: %s request denied\n", token_buffer.command_list[0]);
		}else{
			process_transaction(&demand);
			if(numofTrans == 5000)
			{
				update_balance(&demand);
				numofTrans = 0;
			}
		}
		
		free_command_line(&token_buffer);
		memset(&token_buffer, 0, 0);
	}

	//printf("\nChecking values before update balance\n");
	//for(int check = 0; check < numAccts; check++)
	//{
	//	printf("Account %d: %0.2f\n", check, acct_array[check].balance);
	//}

	char heading[50];
	for(int m = 0; m < numAccts; m++)
	{
		sprintf(heading, "%s.txt", demand.accts[m].account_number);
		FILE *acct_out = fopen(heading, "w+");
		fprintf(acct_out, "%s", demand.accts[m].out_file);
		fclose(acct_out);	
	}


	FILE *outputFile = fopen("output.txt", "w+");
	for(int k = 0; k < numAccts; k++)
	{
		fprintf(outputFile, "%d balance:\t%.2f\n\n", k, demand.accts[k].balance);
	}

	free(acct_array);
	fclose(inputFile);
	fclose(outputFile);
	free(line_buf);
	exit(0);
}
