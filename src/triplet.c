/**
 * SPICE DC Analysis
 * Written by Kosmas Sidiropoulos
 *
 **/
#include "triplet.h"
#include <stdio.h>
#include <stdlib.h>

/** Adds non-zero value to the linked list */
void add_triplet(Triplet **list, double value, int i, int j)
{
	if(*list == NULL)
	{
		*list = (Triplet *)malloc(sizeof(Triplet));
		(*list)->value = value;
		(*list)->i = i;
		(*list)->j = j;
		(*list)->next = NULL;
		return;
	}
	
	Triplet *cur = *list;
	while(cur->next != NULL)
		cur = cur->next;
	
	cur->next = (Triplet *)malloc(sizeof(Triplet));
	cur->next->value = value;
	cur->next->i = i;
	cur->next->j = j;
	cur->next->next = NULL;
}

/** Return a triplet value from the linked list which 
	is represented as Matrix[i][j] value. */
Triplet * get_triplet(Triplet *list, int i, int j)
{
	Triplet *cur = list;
	while(cur != NULL)
	{
		if(cur->i == i && cur->j == j)
			return cur;
		
		cur = cur->next;
	}
	
	return NULL;
}