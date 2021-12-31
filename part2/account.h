#ifndef ACCOUNT_H_
#define ACCOUNT_H_

typedef struct
{
	char account_number[17];
	char password[9];
	double balance;
	double reward_rate;
	
	double transaction_tracter;
	
	char out_file[2264];
	
	pthread_mutex_t ac_lock;
}account;

typedef struct
{
	account *accts;
	int numofAccts;
	command_line request;
} transact;

#endif /* ACCOUNT_H_ */
