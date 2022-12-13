/* query.c - implementation of the queries */

#include "quasiMC.h"

extern int verbose;

/* ------------------------------------------------------------------------- */

void CheckQueryArraySize(QUERIES *queries, int num_add)
/* checks if 'numadd' new nodes can be added to query array else reallocates
   query array to twice its size. */
{
  if (queries->num_queries + num_add > queries->query_array_size) {
    /* reallocate */
    do
      queries->query_array_size *= 2; /* multiply by two */
    while (queries->num_queries + num_add > queries->query_array_size);

    if ((queries->query = (QUERY *)realloc(
             queries->query, queries->query_array_size * sizeof(QUERY))) ==
        NULL) {
      fprintf(stderr, "QuasiMC - cannot reallocate memory for queries.\n");
      exit(0);
    }

    if (verbose >= 1)
      fprintf(stderr, "QuasiMC - queries reallocated. New size is %d.\n",
              queries->query_array_size);
  }
  return;
}

/* ------------------------------------------------------------------------- */

void InitQuery(QUERIES *queries)
/* initialize the query array - should only be called once */
{
  queries->num_queries = 0;
  queries->query_array_size = DEFAULT_QUERY_SIZE;
  if ((queries->query = (QUERY *)malloc(queries->query_array_size *
                                        sizeof(QUERY))) == NULL) {
    fprintf(stderr, "QuasiMC - cannot allocate memory for queries.\n");
    exit(0);
  }
  memset(queries->query, 0, queries->query_array_size * sizeof(QUERY));
  return;
}

/* ------------------------------------------------------------------------- */

void FreeQuery(QUERIES *queries)
/* free the query array from memory */
{
  if (queries->query != NULL)
    free(queries->query);
  queries->query = NULL;
  return;
}

/* ------------------------------------------------------------------------- */

void ResetQuery(QUERIES *queries)
/* clear the query array to all zeros */
{
  if (queries->query != NULL) {
    memset(queries->query, 0, queries->query_array_size * sizeof(QUERY));
    queries->num_queries = 0;
  }
  return;
}

/* ------------------------------------------------------------------------- */
