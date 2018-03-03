#define MAX_LEN 255
#define CHAIN_LEN 512

/**
	This structure describes a Node(circuits) which
	refers to any point of a circuit where two or
	more circuit elements meet.
*/
typedef struct Node
{
	char name[MAX_LEN];
	
	int num_of_pconnections;
	struct Pair **p_connections;
	int num_of_nconnections;
	struct Pair **n_connections;
	
	struct Node *hash_table_next;
}Node;

/**
	This structure describes a circuit element using
	a name which can be anything, the type (I, V, R, C, L)
	of the element and it's value.
	There are also two Node structures that point to the
	elements that are connected with positive polarity and
	those with negative.
*/
typedef struct Pair
{
	Node *pconnected_with;
	Node *nconnected_with;
	char type;
	char name[MAX_LEN];
	double value;
}Pair;

/**
	When the spice file is parsed all
	the nodes are stored in this hash table
	structure.
*/
typedef struct NodeHashTable
{
	Node **node_hash_table;
}NodeHashTable;

/**
	A linked list that stores the
	nodes in the order the nodes
	were parsed.
*/
typedef struct LinkedList
{
	Node *node;
	struct LinkedList *next;
}LinkedList;

/**
	A dynamic array that stores all
	the m2 type circuit elements (L, V).
*/
typedef struct M2Table
{
	Pair **pairs;
	int total;
}M2Table;

void add2_list(LinkedList **ll, Node **node);
Node *init_node(char name[]);
Pair *create_pair(Node **p, M2Table **m2_elem, char name[], char type, double value, Node **n);
void print_node_pairs(Node *node);
void print_hash_table(NodeHashTable *ht);
unsigned int hash(char* s);
NodeHashTable *init_hash_table();
void hash_table_add_node(NodeHashTable **hash_table, Node **node);
Node *lookup_node(NodeHashTable **ht, LinkedList **ll, char *name, int *n);
void parse_file(char filename[], NodeHashTable **ht, LinkedList **ll, M2Table **m2_elem, int *n);
void print_node_order(LinkedList *ll);