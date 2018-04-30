/* This program is made to imitate how a vending machine works
Assignment 2 : 01076259 Operating Systems
Dept. Computer Engineering ,Fact. Engineering, KMITL
@author  Kittinan Ounlum Std Code : 58010101
*/

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define NUM_SUP 5
#define NUM_CON 8
#define MAX 100

pthread_mutex_t mutex[NUM_SUP];

time_t rawtime;
char * ttime;

struct Products {													// Struct of Products
	char name[255];
	int amount;
	int interval;
	int repeat;
};

struct Users {														//Strut of Users (Suppliers or Consumers)
	struct Products product;
	char type[255];
	time_t trigger_time;
};

struct Machine {													//Struct of Machine (unnecessary)
	struct Users user[NUM_SUP + NUM_CON];
} machine;

void *clock_(){														//Real-time clock
	while(1)
		rawtime = time(NULL);
}

void *operate(void *id) {											//Perform an action
	int n = (long)id;
	char type[255];
	strcpy(type,machine.user[n].type);
	if( n >= NUM_SUP)												//If this thread is consumer use index in supplier struct
		for(int i = NUM_SUP; i < NUM_SUP + NUM_CON; i++)
			for(int j = 0; j < NUM_SUP; j++){
				if(strcmp(machine.user[n].product.name, machine.user[j].product.name) == 0){
					n = j;
					break;
				}
			}		
			int interval = machine.user[n].product.interval;
			int counter = machine.user[n].product.repeat;
			bool cond;
			while(1){
				struct tm * tm = localtime(&rawtime);
				ttime = asctime(tm);
		ttime[strcspn(ttime, "\n")] = 0;							//Custom date and time format
		if ((rawtime - machine.user[n].trigger_time) > interval) {	//If the last operation hasn't done recently
			machine.user[n].trigger_time = time(NULL);				//Update time
			if(strcmp(type, "supplier") == 0)						//If this thread is Supplier
				cond = machine.user[n].product.amount < MAX;		//If Amount < MAX
			else if(strcmp(type, "consumer") == 0)					//If this thread is Consumer	
				cond = machine.user[n].product.amount > 0;			//If Amount > 0
			if (cond) {
				interval = machine.user[n].product.interval;
				counter = machine.user[n].product.repeat;
				if(strcmp(type, "supplier") == 0) {
					pthread_mutex_lock(&mutex[n]);
					machine.user[n].product.amount += 1;			//Supply product 1 unit
					pthread_mutex_unlock(&mutex[n]);
				}else if(strcmp(type, "consumer") == 0) {
					pthread_mutex_lock(&mutex[n]);
					machine.user[n].product.amount -= 1;			//Consume product 1 unit
					pthread_mutex_unlock(&mutex[n]);
				}
				FILE *file = fopen("output.txt", "a");		
				if(strcmp(type, "supplier") == 0) {
					fprintf(file, "%s : %s supplied 1 unit. Stock after = %d\n",
					ttime,											//Write output to a text file
					machine.user[n].product.name,
					machine.user[n].product.amount);
					printf("%s : \033[01;33m%s\033[0m supplied 1 unit. Stock after = \033[01;32m%d\033[0m\n",
					ttime,											//print the result
					machine.user[n].product.name,		
					machine.user[n].product.amount);
				}
				else if(strcmp(type, "consumer") == 0){	
					fprintf(file, "%s : %s consumed 1 unit. Stock after = %d\n",
						ttime,
						machine.user[n].product.name,
						machine.user[n].product.amount);
					printf("%s : \033[01;33m%s\033[0m consumed 1 unit. Stock after = \033[01;32m%d\033[0m\n",
						ttime,
						machine.user[n].product.name,
						machine.user[n].product.amount);
				}
				fclose(file); 
			} else {						
				FILE *file = fopen("output.txt", "a");
				if(strcmp(type, "supplier") == 0) {					//If amount >= MAX
					fprintf(file, "%s : %s supplier is going to wait . . .\n", 
						ttime, machine.user[n].product.name);
					printf("%s : \033[01;33m%s\033[0m supplier is going to wait . . .\n", 
						ttime, machine.user[n].product.name);
				}
				else if(strcmp(type, "consumer") == 0) {			//If amount == 0
					fprintf(file, "%s : %s consumer is going to wait . . .\n", 
						ttime, machine.user[n].product.name);
					printf("%s : \033[01;33m%s\033[0m consumer is going to wait . . .\n", 
						ttime, machine.user[n].product.name);
				}
				fclose(file);
				if(counter > 0)										//Check if repetition available
					counter--;
				else{
					if(interval * 2 <= 60)							//If not double the interval
						interval *= 2;	
					else
						interval = 60;
					counter = machine.user[n].product.repeat; 		//Reset repetition value
				}
			}
		}
	}
	//unreachable
	pthread_exit(NULL);
}

int main(void) {
	pthread_t thread[NUM_SUP + NUM_CON];			
	pthread_t time_t;
	pthread_create(&time_t, NULL, &clock_, NULL);
	
	char buff[255];
	char string[255];
	char num[2];
	for(int n = 0; n < NUM_SUP + NUM_CON; n++){						//Read the configurations
		if(n < NUM_SUP) {
			strcpy(string, "supplier");
			sprintf(num, "%d", n + 1 );
		} else {
			strcpy(string, "consumer");
			sprintf(num, "%d", n - NUM_SUP + 1 );
		}
		strcpy(machine.user[n].type, string);
		strcat(string, num);
		strcat(string, ".txt");	
		FILE *fp;
		fp = fopen(string, "r");
		for (int i = 0; i < 3; i++) {
			fgets(buff, 255, fp);
			buff[strcspn(buff, "\n")] = 0;
			if (i == 0)
				strcpy(machine.user[n].product.name, buff);
			else if (i == 1)
				sscanf(buff, "%d", &machine.user[n].product.interval);
			else if (i == 2)
				sscanf(buff, "%d", &machine.user[n].product.repeat);
		}
		fclose(fp);
		strcpy(string, "");
		if(n < NUM_SUP)
			pthread_mutex_init(&mutex[n], NULL);
		pthread_create(&thread[n], NULL, &operate, (void *)(long)n);//Create threads
		
	}
	pthread_join(time_t, NULL);										//Wait for timer to join (Never)
	return 0;
}
