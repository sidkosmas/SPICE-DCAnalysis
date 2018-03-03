/**
 * SPICE DC Analysis
 * Written by Kosmas Sidiropoulos
 *
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gsl/gsl_math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_splinalg.h>

#include "parser.h"
#include "triplet.h"

void fprint_node_order(LinkedList *ll);

int main(int argc, char *argv[])
{
	NodeHashTable *ht = init_hash_table();	/* The nodes saved in hash table. */
	LinkedList *ll = NULL;	/* The nodes saved in order they were parsed. */
	M2Table *m2_elem = NULL;	/* The (L, V) second type elements. */
	Triplet *A = NULL;
	int n = 0;	//Number of Nodes (with the ground).
	int i, j;
	
	if(argc < 2){
		printf("fatal error: no input files\nsimulation terminated.\n");
		exit(-1);
	}
	else
		parse_file(argv[1], &ht, &ll, &m2_elem, &n);

	
	printf("n = %d\n", n);
	printf("m2 = %d\n", m2_elem->total);

	/* Calculate A table */
 	LinkedList *nodes_row = ll;
	for(i=0; i<n-1; i++)
	{
//		if(i % 1000 == 0) printf("%d\n", i);
		LinkedList *nodes_col = ll;
		for(j=0; j<n-1; j++)
		{	
			double r_total = 0.0;
			if(i == j)
			{
				Node *t = lookup_node(&ht, NULL, nodes_col->node->name, NULL);
				int k;
				for(k=0; k<t->num_of_pconnections; k++)
				{
					if(t->p_connections[k]->type == 'R')
						r_total += 1.0/t->p_connections[k]->value;
				}
				for(k=0; k<t->num_of_nconnections; k++)
				{
					if(t->n_connections[k]->type == 'R')
						r_total += 1.0/t->n_connections[k]->value;
				}
			}
			else
			{
				int k;
				Node *r = lookup_node(&ht, NULL, nodes_row->node->name, NULL);
				for(k=0; k<r->num_of_pconnections; k++)
				{
					if(r->p_connections[k]->type == 'R')
						if(!strcmp(r->p_connections[k]->pconnected_with->name, nodes_col->node->name))
							r_total -= 1.0/r->p_connections[k]->value;
				}
				
				for(k=0; k<r->num_of_nconnections; k++)
				{
					if(r->n_connections[k]->type == 'R')
						if(!strcmp(r->n_connections[k]->nconnected_with->name, nodes_col->node->name))
							r_total -= 1.0/r->n_connections[k]->value;
				}
			}
			if(r_total != 0.0)
				add_triplet(&A, r_total, i, j);
			nodes_col = nodes_col->next;
		}
		nodes_row = nodes_row->next;
	}
	
	nodes_row = ll;
	for(i=0; i<(n-1); i++)
	{		
		for(j=(n-1); j<(n-1) + m2_elem->total; j++)
		{
			if(!strcmp(m2_elem->pairs[j - (n-1)]->pconnected_with->name, nodes_row->node->name))
			{
				add_triplet(&A, -1.0, j, i);
				add_triplet(&A, -1.0, i, j);
			}
			else if(!strcmp(m2_elem->pairs[j - (n-1)]->nconnected_with->name, nodes_row->node->name))
			{
				add_triplet(&A, 1.0, j, i);
				add_triplet(&A, 1.0, i, j);
			}
		}
		nodes_row = nodes_row->next;
	}
	
	/* Write A table in triplet form to file */
	FILE *fp = fopen("solution/A.txt", "w");
	while(A != NULL)
	{
		printf("%d %d %lf\n", A->i, A->j, A->value);
		fprintf(fp, "%d %d %lf\n", A->i, A->j, A->value);
		A = A->next;
	}
	fclose(fp);
	printf("\n");
	
	//print_node_order(ll);
	fprint_node_order(ll);

	/* Calculate and write table B (just values) */
	fp = fopen("solution/B.txt", "w");
	LinkedList *temp = ll;
	for(i=0; i<(n-1); i++)
	{
		double value = 0.0;
		Node *t = lookup_node(&ht, NULL, temp->node->name, NULL);
		
		for(j=0; j<t->num_of_pconnections; j++)
		{
			if(t->p_connections[j]->type == 'I'){
				value -= t->p_connections[j]->value;
			}
		}
		for(j=0; j<t->num_of_nconnections; j++)
		{
			if(t->n_connections[j]->type == 'I'){
				value += t->n_connections[j]->value;
			}
		}
		printf("%lf\n", value);
		fprintf(fp, "%lf\n", value);
		temp = temp->next;
	}
	
	for(i=(n-1); i<(n-1)+m2_elem->total; i++)
	{
		if(m2_elem->pairs[i - (n-1)]->type == 'V')
		{
			double value = m2_elem->pairs[i - (n-1)]->value;
			printf("%lf\n", value);
			fprintf(fp, "%lf\n", value);
		}
		else
		{
			printf("%lf\n", 0.0);
			fprintf(fp, "%lf\n", 0.0);
		}
	}
	printf("\n");
	fclose(fp);

	/*******************************
	*	AX = B solution using the
	*	GSL library.
	********************************/
	
	const size_t _n = n-1 + m2_elem->total;
    const double h = 1.0;           					/* grid spacing */
    gsl_spmatrix *gsl_A = gsl_spmatrix_alloc(_n, _n); 	/* triplet format */
    gsl_spmatrix *gsl_CA;                            	/* compressed format */
    gsl_vector *gsl_B = gsl_vector_alloc(_n);        	/* right hand side vector */
    gsl_vector *gsl_X = gsl_vector_alloc(_n);        	/* solution vector */

    /* Construct the sparse matrix for the finite difference equation */

    fp = fopen("solution/A.txt", "r");
    while(!feof(fp))
    {
        size_t i, j;
        double value;
        fscanf(fp, "%zu %zu %lf", &i, &j, &value);
        gsl_spmatrix_set(gsl_A, i, j, value);
    }
	fclose(fp);

    /* Scale by h^2 */
    gsl_spmatrix_scale(gsl_A, 1.0 / (h * h));

    /* Construct right hand side vector */
	
	fp = fopen("solution/B.txt", "r");
	int counter = 0;
    while(!feof(fp))
    {
		if(counter >= _n) break;
        double value;
        fscanf(fp, "%lf", &value);
		gsl_vector_set(gsl_B, counter, value);
		counter++;
    }
	fclose(fp);

    /* Convert to compressed column format */
    gsl_CA = gsl_spmatrix_ccs(gsl_A);

    const double tol = 1.0e-6;  	/* solution relative tolerance */
    const size_t max_iter = 100; 	/* maximum iterations */
    const gsl_splinalg_itersolve_type *T = gsl_splinalg_itersolve_gmres;
    gsl_splinalg_itersolve *work = gsl_splinalg_itersolve_alloc(T, _n, 0);
    size_t iter = 0;
    int status;

    /* Initial guess X = 0 */
    gsl_vector_set_zero(gsl_X);

    /* Solve the system AX = B */
    do
    {
        status = gsl_splinalg_itersolve_iterate(gsl_CA, gsl_B, tol, gsl_X, work);
    }
    while (status == GSL_CONTINUE && ++iter < max_iter);

    /* Output and write solution to file */
	fp = fopen("solution/X.txt", "w");
    for (i = 0; i < _n; ++i)
    {
        double x_gsl = gsl_vector_get(gsl_X, i);
		fprintf(fp, "%lf\n", x_gsl);
        printf("%lf\n", x_gsl);
    }

    gsl_splinalg_itersolve_free(work);

    gsl_spmatrix_free(gsl_A);
    gsl_spmatrix_free(gsl_CA);
    gsl_vector_free(gsl_B);
    gsl_vector_free(gsl_X);
	
	return 0;
}

void fprint_node_order(LinkedList *ll)
{
	LinkedList *cur = ll;
	int counter = 0;
	FILE *fp = fopen("solution/node_order.txt", "w");
	while(cur != NULL)
	{
		fprintf(fp, "%s->", cur->node->name);
		if(counter == 10){
			printf("\n");
			counter = 0;
		}
		counter++;
		cur = cur->next;
	}
	fclose(fp);
}