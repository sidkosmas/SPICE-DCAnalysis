/**
 * SPICE DC Analysis
 * Written by Kosmas Sidiropoulos
 *
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

void add2_list(LinkedList **ll, Node **node)
{
	if(*ll == NULL) 
	{
		*ll = (LinkedList *)malloc(sizeof(LinkedList));
		(*ll)->node = *node;
		(*ll)->next = NULL;
		return;
	}
	LinkedList *cur = *ll;
	while(cur->next != NULL)
		cur = cur->next;
	
	cur->next = (LinkedList *)malloc(sizeof(LinkedList));
	cur->next->node = *node;
	cur->next->next = NULL;
}

Node *init_node(char name[])
{
	Node *new_node = (Node *)malloc(sizeof(Node));
	strcpy(new_node->name, name);
	new_node->p_connections = NULL;
	new_node->num_of_pconnections = 0;
	new_node->n_connections = NULL;
	new_node->num_of_nconnections = 0;
	
	new_node->hash_table_next = NULL;
	return new_node;
}

Pair *create_pair(Node **p, M2Table **m2_elem, char name[], char type, double value, Node **n)
{
	if(*p == NULL) return NULL;
	
	/** Circuit element creation. */
	Pair *new_pair = (Pair *)malloc(sizeof(Pair));
	new_pair->type = type;
	new_pair->value = value;
	new_pair->pconnected_with = *n;
	strcpy(new_pair->name, name);
	
	if(type == 'V' || type == 'L')
	{
		if(*m2_elem == NULL) 
		{
			*m2_elem = (M2Table *)malloc(sizeof(M2Table));
			(*m2_elem)->pairs = (Pair **)malloc(sizeof(Pair));
			(*m2_elem)->pairs[0] = new_pair;
			(*m2_elem)->total++;
		}
		else
		{
			(*m2_elem)->pairs = realloc((*m2_elem)->pairs, ((*m2_elem)->total + 1) * sizeof(Pair));
			(*m2_elem)->pairs[(*m2_elem)->total] = new_pair;
			(*m2_elem)->total++;
		}
	}
	
	//Positive
	if(!(*p)->num_of_pconnections){
		(*p)->p_connections = (Pair **)malloc(sizeof(Pair));
		(*p)->p_connections[0] = new_pair;
	}
	else{
		(*p)->p_connections = realloc((*p)->p_connections, 
			((*p)->num_of_pconnections + 1) * sizeof(Pair));
		(*p)->p_connections[(*p)->num_of_pconnections] = new_pair;
	}
	(*p)->num_of_pconnections++;
	
	//Negative
	new_pair->nconnected_with = *p;
	if(!(*n)->num_of_nconnections){
		(*n)->n_connections = (Pair **)malloc(sizeof(Pair));
		(*n)->n_connections[0] = new_pair;
	}
	else{
		(*n)->n_connections = realloc((*n)->n_connections, 
			((*n)->num_of_nconnections + 1) * sizeof(Pair));
		(*n)->n_connections[(*n)->num_of_nconnections] = new_pair;
	}
	(*n)->num_of_nconnections++;

	return new_pair;
}

void print_node_pairs(Node *node)
{
	if(node == NULL) return;
	int i;
	printf("Node : %s\n", node->name);
	printf("Positive Connections\n");
	for(i=0; i<node->num_of_pconnections; i++)
		printf("\t%s %c %lf\n\tConnected with %s\n", 
			node->p_connections[i]->name, 
			node->p_connections[i]->type, 
			node->p_connections[i]->value,
			node->p_connections[i]->pconnected_with->name);
	 
	printf("Negative Connections\n");
	for(i=0; i<node->num_of_nconnections; i++)
		printf("\t%s %c %lf\n\tConnected with %s\n", 
			node->n_connections[i]->name, 
			node->n_connections[i]->type, 
			node->n_connections[i]->value,
			node->n_connections[i]->nconnected_with->name);
	
	printf("\n");
	
}

void print_hash_table(NodeHashTable *ht)
{
	int i;
	for(i=0; i<CHAIN_LEN; i++)
		if(ht->node_hash_table[i] != NULL){
			printf("Hash val = %d\n", i);
			print_node_pairs(ht->node_hash_table[i]);
		}
}

unsigned int hash(char* s)
{
    unsigned int hval = 0;
    while (*s != '\0')
    {
        hval = (hval << 4) + *s;
        s++;
    }
    return hval % CHAIN_LEN;
}

NodeHashTable *init_hash_table()
{
	NodeHashTable *new_hash_table;
	new_hash_table = (NodeHashTable *)malloc(sizeof(NodeHashTable));
	
	new_hash_table->node_hash_table = (Node **)malloc(sizeof(Node) * CHAIN_LEN);
	int i;
	for(i=0; i<CHAIN_LEN; i++)
		new_hash_table->node_hash_table[i] = NULL;
	
	return new_hash_table;
}

void hash_table_add_node(NodeHashTable **hash_table, Node **node)
{
	unsigned int hash_value = hash((*node)->name);
	if((*hash_table)->node_hash_table[hash_value] == NULL)
	{
		(*hash_table)->node_hash_table[hash_value] = (Node *)malloc(sizeof(Node));
		(*hash_table)->node_hash_table[hash_value] = *node;
		return;
	}
	
	Node *cur = (*hash_table)->node_hash_table[hash_value];
	while(cur->hash_table_next != NULL)
		cur = cur->hash_table_next;
	
	cur->hash_table_next = (Node *)malloc(sizeof(Node));
	cur->hash_table_next = *node;
}

Node *lookup_node(NodeHashTable **ht, LinkedList **ll, char *name, int *n)
{
	unsigned int hash_value = hash(name);
	if((*ht)->node_hash_table[hash_value] == NULL)
	{
		Node *new_node = init_node(name);
		hash_table_add_node(ht, &new_node);
		if(strcmp(name, "0"))
			add2_list(ll, &new_node);
		(*n)++;
		return new_node;
	}
	else{
		if(!strcmp((*ht)->node_hash_table[hash_value]->name, name))
			return (*ht)->node_hash_table[hash_value];
		else{
			Node *cur = (*ht)->node_hash_table[hash_value]->hash_table_next;
			while(cur != NULL){
				if(!strcmp(cur->name, name))
					return cur;
				cur = cur->hash_table_next;
			}
		}
	}
	
	Node *new_node = init_node(name);
	hash_table_add_node(ht, &new_node);
	if(strcmp(name, "0"))
		add2_list(ll, &new_node);
	(*n)++;
	return new_node;
}

void parse_file(char filename[], NodeHashTable **ht, LinkedList **ll, M2Table **m2_elem, int *n)
{
	FILE *fp;
	fp = fopen(filename, "r");
	
	while(!feof(fp)){
		char element_name[MAX_LEN];
		char high_voltage[MAX_LEN];
		char low_voltage[MAX_LEN];
		double value;
		
		fscanf(fp, "%s %s %s %lf\n", element_name, high_voltage, low_voltage, &value);
		printf("%s %s %s %lf\n", element_name, high_voltage, low_voltage, value);
		Node *pos = lookup_node(ht, ll, high_voltage, n);
		Node *neg = lookup_node(ht, ll, low_voltage, n);
		
		char type = element_name[0];
		create_pair(&pos, m2_elem, element_name, type, value, &neg);
	}
	fclose(fp);
	printf("Parsing %s completed!\n", filename);
}

void print_node_order(LinkedList *ll)
{
	LinkedList *cur = ll;
	int counter = 0;
	while(cur != NULL)
	{
		printf("%s->", cur->node->name);
		if(counter == 10){
			printf("\n");
			counter = 0;
		}
		cur = cur->next;
	}
}