#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <err.h>


struct cell{
	pid_t child;
	char *fich;
	int unique;
	struct cell *next;
};


struct cell *first, *last;


void new_element(pid_t new_child, char *new_fich,
				int new_unique){
	struct cell *new;
	
	new=(struct cell *) malloc (sizeof(struct cell));
	new->child=new_child;
	new->fich=new_fich;
	new->unique=new_unique;
	new->next=NULL;
	
	if (first==NULL){
		first=new;
		last=new;
	}else{
		last->next=new;
		last=new;
	}
}

int print_list()
{
	int result=0;
	struct cell *aux=first;
	while(aux!=NULL){
		printf("%s",aux->fich);
		switch(aux->unique)
		{
			case 2:
				printf(" FILE ERROR (maybe dont exist)\n");
				result=1;
				break;
			case 1:
				printf(" YES\n");
				break;
			case 0:
				printf(" NO\n");
				break;
		}
		aux=aux->next;
	}
	return result;
}

void free_list()
{
	struct cell *aux;
	
	while (first!=NULL){
		aux=first->next;
		free(first);
		first=aux;
	}
}

void save_result_cmp(pid_t son, int result)
{
	struct cell *aux=first;
	int completed=0;
	do{
		if (aux->child==son){
			aux->unique=result;
			completed=1;
		}
		aux=aux->next;
	}while (!completed);
}


int compare_files(char *file_a, char *file_b)
{
	pid_t pid =fork();
	if (pid==0){
		execl("/usr/bin/cmp", "/usr/bin/cmp", "-s" ,file_a, file_b, NULL);
		err(1, NULL);
		free_list();
		exit(EXIT_SUCCESS);
	}else{
		int status;
		waitpid(pid, &status, 0);
		int result=0;
		if WIFEXITED(status){
    		if (WEXITSTATUS(status)){
    			result=1;
    		}
		} 
	return result;
	}	
}

void exit_dad(int result)
{
	switch(result)
	{
		case 0:
			exit(EXIT_SUCCESS);
		case 1:
			fprintf(stderr, "\nSome arguments not exists\n");
			exit(EXIT_FAILURE);
		case 2:
			fprintf(stderr, "Arguments incorrects, need at least two files\n");
			exit(EXIT_FAILURE);
	}
}

int 
son_code(char *file)
{
	int result=1;
	struct cell *aux=first;

	if (!(access(file, F_OK))){
		while(aux!=NULL){
			if (strcmp(file, aux->fich)){
				result=compare_files(file,aux->fich);
				if (!result){
					return result;
				}
			}
			aux=aux->next; 
		}
	}else{
		result=2;
	}	
	return result;
}

int 
main(int argc, char *argv[]) 
{
	if (argc<3){
		exit_dad(2);
	}
	first=(struct cell *)NULL;
	last=(struct cell *)NULL;
	struct cell *pointer=(struct cell *)NULL;
	
	for (int i=1; i<argc; i++){
		new_element(0, argv[i], 1);
	}
	
	pointer=first;
	for (int z=1; z<argc; z++){
		int proces=fork();
		if (proces==0){
			int unique=son_code(argv[z]);
			free_list();
			exit(unique);
		}else{
			pointer->child=proces;
			pointer=pointer->next;
		}
	}
	
	int status;
	
	for(int x=1;x<argc;x++){ // loop will run n times (n=5) 
    	pid_t cpid=waitpid(-1, &status, 0);
    	if WIFEXITED(status){
    		save_result_cmp(cpid,WEXITSTATUS(status));
    	} 
     }
    int result=print_list();
    free_list();
	exit_dad(result);
}
