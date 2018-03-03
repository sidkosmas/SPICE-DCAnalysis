/**
	A linked list that saves a sparse matrix
	in triplet form.
*/
typedef struct Triplet
{
	double value;
	int i, j;
	struct Triplet *next;
}Triplet;

void add_triplet(Triplet **list, double value, int i, int j);
Triplet * get_triplet(Triplet *list, int i, int j);