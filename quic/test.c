#include <stdio.h>
#include <stdlib.h>

int main (int argc , char * argv[]) 
{
	int i;
	// affichage des arguments
	printf("Nombre d’arguments passes au programme : %d\n", argc);
	
	for(i = 0 ; i< argc ; i ++) 
	{
		printf(" argv[%d] : ‘%s’\n", i, argv[i]);
	}

	printf(" data = %d \n", (char const **)(argv + 5));
	return 0 ;
}