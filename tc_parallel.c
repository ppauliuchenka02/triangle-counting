#ifdef PARALLEL

#include "types.h"
#include "graph.h"
#include "tc_parallel.h"
#include <unistd.h>
#include <omp.h>

#define PBODY(foo)				\
  int numThreads;				\
  UINT_t *mycount;				\
  _Pragma("omp parallel")			\
  {							      \
  int myID = omp_get_thread_num();			      \
  if (myID == 0) {					      \
    numThreads = omp_get_num_threads();			      \
    mycount = (UINT_t *)calloc(numThreads, sizeof(UINT_t));   \
    assert_malloc(mycount);				      \
  }							      \
  _Pragma("omp barrier")				      \
  _Pragma("omp for schedule(dynamic)")			      \
  foo							      \
  _Pragma("omp for reduction(+:count)")			      \
  for (int i = 0; i < numThreads ; i++)			      \
    count += mycount[i];				      \
  }


#define PBODY2(foo, bar)				\
  int numThreads;					\
  UINT_t *mycount;					\
  _Pragma("omp parallel")				\
  {							\
  int myID = omp_get_thread_num();			      \
  if (myID == 0) {					      \
      numThreads = omp_get_num_threads();		      \
      mycount = (UINT_t *)calloc(numThreads, sizeof(UINT_t)); \
      assert_malloc(mycount);				      \
      bar						      \
  }							      \
  _Pragma("omp barrier")				      \
  _Pragma("omp for schedule(dynamic)")			      \
  foo							      \
  _Pragma("omp for reduction(+:count)")			      \
  for (int i = 0; i < numThreads ; i++)			      \
    count += mycount[i];				      \
  }

#define myCount mycount[myID]

  

UINT_t tc_triples_P(const GRAPH_TYPE *graph) {
  /* Algorithm: for each triple (i, j, k), determine if the three triangle edges exist. */
  
  UINT_t count = 0;

  const UINT_t n = graph->numVertices;

  PBODY(
	for (UINT_t i = 0; i < n; i++)
	  for (UINT_t j = 0; j < n; j++)
	    for (UINT_t k = 0; k < n; k++)
	      if (check_edge(graph, i, j) && check_edge(graph, j, k) && check_edge(graph, k, i))
		myCount++;
	);

  return (count/6);
}

UINT_t tc_triples_DO_P(const GRAPH_TYPE *graph) {
  /* Algorithm: for each triple (i, j, k), determine if the three triangle edges exist. */
  /* Direction oriented. */
  
  UINT_t count = 0;

  const UINT_t n = graph->numVertices;

  PBODY(
	  for (UINT_t i = 0; i < n; i++)
	    for (UINT_t j = i; j < n; j++)
	      for (UINT_t k = j; k < n; k++)
		if (check_edge(graph, i, j) && check_edge(graph, j, k) && check_edge(graph, k, i))
		  myCount++;
	);

  return count;
}

UINT_t tc_wedge_P(const GRAPH_TYPE *graph) {
  /* Algorithm: For each vertex i, for each open wedge (j, i, k), determine if there's a closing edge (j, k) */
  UINT_t count = 0;

  const UINT_t* restrict Ap = graph->rowPtr;
  const UINT_t* restrict Ai = graph->colInd;
  const UINT_t n = graph->numVertices;

  PBODY(
	for (UINT_t i = 0; i < n; i++) {
	  UINT_t s = Ap[i];
	  UINT_t e = Ap[i + 1];
    
	  for (UINT_t j = s; j < e; j++) {
	    UINT_t neighbor1 = Ai[j];

	    for (UINT_t k = s; k < e; k++) {
	      UINT_t neighbor2 = Ai[k];
	      
	      if (neighbor1 != neighbor2) {
		UINT_t s_n1 = Ap[neighbor1];
		UINT_t e_n1 = Ap[neighbor1 + 1];
		
		for (UINT_t l = s_n1; l < e_n1; l++) {
		  if (Ai[l] == neighbor2) {
		    myCount++;
		    break;
		  }
		}
	      }
	    }
	  }
	}
	);

  return (count/6);
}


UINT_t tc_wedge_DO_P(const GRAPH_TYPE *graph) {
  /* Algorithm: For each vertex i, for each open wedge (j, i, k), determine if there's a closing edge (j, k) */
  /* Direction oriented. */
  
  UINT_t count = 0;

  const UINT_t* restrict Ap = graph->rowPtr;
  const UINT_t* restrict Ai = graph->colInd;
  const UINT_t n = graph->numVertices;

  PBODY(
	for (UINT_t i = 0; i < n; i++) {
	  UINT_t s = Ap[i];
	  UINT_t e = Ap[i + 1];

	  for (UINT_t j = s; j < e; j++) {
	    UINT_t neighbor1 = Ai[j];
	    if (neighbor1 > i) {

	      for (UINT_t k = s; k < e; k++) {
		UINT_t neighbor2 = Ai[k];

		if ((neighbor1 != neighbor2) && (neighbor2 > neighbor1)) {
		  UINT_t s_n1 = Ap[neighbor1];
		  UINT_t e_n1 = Ap[neighbor1 + 1];
	  
		  for (UINT_t l = s_n1; l < e_n1; l++) {
		    if (Ai[l] == neighbor2) {
		      myCount++;
		      break;
		    }
		  }
		}
	      }
	    }
	  }
	}
	);

  return count;
}


UINT_t tc_intersectMergePath_P(const GRAPH_TYPE *graph) {
  /* Algorithm: For each edge (i, j), find the size of its intersection using a linear scan. */
  
  UINT_t count = 0;

  const UINT_t* restrict Ap = graph->rowPtr;
  const UINT_t* restrict Ai = graph->colInd;
  const UINT_t n = graph->numVertices;

  PBODY(
	for (UINT_t v = 0; v < n; v++) {
	  UINT_t b = Ap[v  ];
	  UINT_t e = Ap[v+1];
	  for (UINT_t i=b ; i<e ; i++) {
	    UINT_t w = Ai[i];
	    myCount += intersectSizeMergePath(graph, v, w);
	  }
	}
	);

  return (count/6);
}

UINT_t tc_intersectMergePath_DO_P(const GRAPH_TYPE *graph) {
  /* Algorithm: For each edge (i, j), find the size of its intersection using a linear scan. */
  /* Direction oriented. */
  
  UINT_t count = 0;

  const UINT_t* restrict Ap = graph->rowPtr;
  const UINT_t* restrict Ai = graph->colInd;
  const UINT_t n = graph->numVertices;

  PBODY(
	for (UINT_t v = 0; v < n; v++) {
	  UINT_t b = Ap[v  ];
	  UINT_t e = Ap[v+1];
	  for (UINT_t i=b ; i<e ; i++) {
	    UINT_t w = Ai[i];
	    if (v < w)
	      myCount += intersectSizeMergePath(graph, v, w);
	  }
	}
	);

  return (count/3);
}


UINT_t tc_intersectBinarySearch_P(const GRAPH_TYPE *graph) {
  /* Algorithm: For each edge (i, j), find the size of its intersection using a binary search. */

  UINT_t count = 0;

  const UINT_t* restrict Ap = graph->rowPtr;
  const UINT_t* restrict Ai = graph->colInd;
  const UINT_t n = graph->numVertices;

  PBODY(
	for (UINT_t v = 0; v < n; v++) {
	  UINT_t b = Ap[v  ];
	  UINT_t e = Ap[v+1];
	  for (UINT_t i=b ; i<e ; i++) {
	    UINT_t w  = Ai[i];
	    myCount += intersectSizeBinarySearch(graph, v, w);
	  }
	}
	);

  return (count/6);
}

UINT_t tc_intersectBinarySearch_DO_P(const GRAPH_TYPE *graph) {
  /* Algorithm: For each edge (i, j), find the size of its intersection using a binary search. */
  /* Direction oriented. */

  UINT_t count = 0;

  const UINT_t* restrict Ap = graph->rowPtr;
  const UINT_t* restrict Ai = graph->colInd;
  const UINT_t n = graph->numVertices;

  PBODY(
	for (UINT_t v = 0; v < n; v++) {
	  UINT_t b = Ap[v  ];
	  UINT_t e = Ap[v+1];
	  for (UINT_t i=b ; i<e ; i++) {
	    UINT_t w  = Ai[i];
	    if (v < w)
	      myCount += intersectSizeBinarySearch(graph, v, w);
	  }
	}
	);

  return (count/3);
}


UINT_t tc_intersectPartition_P(const GRAPH_TYPE *graph) {
  /* Algorithm: For each edge (i, j), find the size of its intersection using a binary search-based partition. */
  
  UINT_t count = 0;

  const UINT_t *restrict Ap = graph->rowPtr;
  const UINT_t *restrict Ai = graph->colInd;
  const UINT_t n = graph->numVertices;

  PBODY(
	for (UINT_t v = 0; v < n ; v++) {
	  UINT_t b = Ap[v  ];
	  UINT_t e = Ap[v+1];
	  for (UINT_t i=b ; i<e ; i++) {
	    UINT_t w  = Ai[i];
	    myCount += searchLists_with_partitioning((UINT_t *)Ai, (INT_t) Ap[v], (INT_t)Ap[v+1]-1, (UINT_t *)Ai, (INT_t)Ap[w], (INT_t)Ap[w+1]-1);
	  }
	}
	);

  return (count/6);
}

UINT_t tc_intersectPartition_DO_P(const GRAPH_TYPE *graph) {
  /* Algorithm: For each edge (i, j), find the size of its intersection using a binary search-based partition. */
  /* Direction oriented. */
  
  UINT_t count = 0;

  const UINT_t *restrict Ap = graph->rowPtr;
  const UINT_t *restrict Ai = graph->colInd;
  const UINT_t n = graph->numVertices;

  PBODY(
	for (UINT_t v = 0; v < n; v++) {
	  UINT_t b = Ap[v  ];
	  UINT_t e = Ap[v+1];
	  for (UINT_t i=b ; i<e ; i++) {
	    UINT_t w  = Ai[i];
	    if (v < w)
	      myCount += searchLists_with_partitioning((UINT_t *)Ai, (INT_t) Ap[v], (INT_t)Ap[v+1]-1, (UINT_t *)Ai, (INT_t)Ap[w], (INT_t)Ap[w+1]-1);
	  }
	}
	);

  return (count/3);
}


UINT_t tc_intersectHash_P(const GRAPH_TYPE *graph) {
  /* Algorithm: For each edge (i, j), find the size of its intersection using a hash. */

  UINT_t count = 0;

  bool *Hash;
  
  const UINT_t* restrict Ap = graph->rowPtr;
  const UINT_t* restrict Ai = graph->colInd;
  const UINT_t n = graph->numVertices;

  PBODY2(
	for (UINT_t v = 0; v < n ; v++) {
	  UINT_t b = Ap[v  ];
	  UINT_t e = Ap[v+1];
	  for (UINT_t i=b ; i<e ; i++) {
	    UINT_t w  = Ai[i];
	    myCount += intersectSizeHash(graph, Hash + (myID * n), v, w);
	  }
	}
	,
	 Hash = (bool *)calloc(n * omp_get_num_threads(), sizeof(bool));
	 assert_malloc(Hash);
	);

  free(Hash);

  return (count/6);
}




UINT_t tc_intersectHash_DO_P(const GRAPH_TYPE *graph) {
  /* Algorithm: For each edge (i, j), find the size of its intersection using a hash. */
  /* Direction oriented. */

  UINT_t count = 0;

  bool *Hash;

  const UINT_t* restrict Ap = graph->rowPtr;
  const UINT_t* restrict Ai = graph->colInd;
  const UINT_t n = graph->numVertices;
  const UINT_t m = graph->numEdges;

  PBODY2(
	 for (UINT_t v = 0; v < n ; v++) {
	   UINT_t b = Ap[v  ];
	   UINT_t e = Ap[v+1];
	   for (UINT_t i=b ; i<e ; i++) {
	     UINT_t w  = Ai[i];
	     if (v < w)
	       myCount += intersectSizeHash(graph, Hash + (myID * n), v, w);
	   }
	   }
	 ,
	 Hash = (bool *)calloc(n * omp_get_num_threads(), sizeof(bool));
	 assert_malloc(Hash);
	 );

  free(Hash);

  return (count/3);
}



void bfs_mark_horizontal_edges_P(const GRAPH_TYPE *graph, const UINT_t startVertex, UINT_t* restrict level, Queue* queue, bool* visited, bool* horiz) {
  const UINT_t *restrict Ap = graph->rowPtr;
  const UINT_t *restrict Ai = graph->colInd;
  const UINT_t n = graph->numVertices;

  UINT_t listsize;

  UINT_t *list = (UINT_t *)calloc(n, sizeof(UINT_t));
  assert_malloc(list);

  visited[startVertex] = true;
#if 0
  enqueue(queue, startVertex);
#else
  UINT_t liststart = 0;
  list[liststart] = startVertex;
  UINT_t listend = 1;
  listsize = listend - liststart;

  UINT_t curlistend = listend;
#endif
  level[startVertex] = 1;

  while (
#if 0
  !isEmpty(queue)
#else
  listsize > 0
#endif
	 ) {
#pragma omp parallel for schedule(dynamic)
    for (UINT_t i = liststart ; i<listend ; i++) {
#if 0
      UINT_t v = dequeue(queue);
#else
      listsize = curlistend - liststart;
      UINT_t v = list[i];
#endif
      for (UINT_t i = Ap[v]; i < Ap[v + 1]; i++) {
	UINT_t w = Ai[i];
	if (!visited[w])  {
	  horiz[i] = false;
	  visited[w] = true;
#if 0
	  enqueue(queue, w);
#else
#pragma omp critical
	  {
	    list[listend++] = w;
	  }
#endif
	  level[w] = level[v] + 1;
	}
	else {
	  horiz[i] = (level[w] == 0) || (level[w] == level[v]);
	}
      }
    }
#pragma omp single
    {
      curlistend = listend;
      liststart += listsize;
    }
#pragma omp barrier
  }
  
  free(list);
}



UINT_t tc_bader_bfs1_P(const GRAPH_TYPE *graph) {
  /* Bader's new algorithm for triangle counting based on BFS */
  /* Uses Hash array to detect triangles (v, w, x) if x is adjacent to v */
  /* For level[], 0 == unvisited. Needs a modified BFS starting from level 1 */
  /* Mark horizontal edges during BFS */
  /* Direction orientied. */
  UINT_t* restrict level;
  UINT_t c1, c2;
  static bool *Hash;
  bool *horiz;
  bool *visited;
  const UINT_t *restrict Ap = graph->rowPtr;
  const UINT_t *restrict Ai = graph->colInd;
  const UINT_t n = graph->numVertices;
  const UINT_t m = graph->numEdges;

  level = (UINT_t *)calloc(n, sizeof(UINT_t));
  assert_malloc(level);
  
  visited = (bool *)calloc(n, sizeof(bool));
  assert_malloc(visited);

  horiz = (bool *)malloc(m * sizeof(bool));
  assert_malloc(horiz);

  Queue *queue = createQueue(n);

  for (UINT_t v = 0 ; v < n ; v++) {
    if (!level[v])
      bfs_mark_horizontal_edges(graph, v, level, queue, visited, horiz);
  }

  static int numThreads;
  static UINT_t *myc1;
  static UINT_t *myc2;

  c1 = 0; c2 = 0;
#pragma omp parallel
  {
    int myID = omp_get_thread_num();
    if (myID==0) {
      numThreads = omp_get_num_threads();
      
      myc1 = (UINT_t *)calloc(numThreads, sizeof(UINT_t));
      assert_malloc(myc1);
      myc2 = (UINT_t *)calloc(numThreads, sizeof(UINT_t));
      assert_malloc(myc2);
  
      Hash = (bool *)calloc(n * numThreads, sizeof(bool));
      assert_malloc(Hash);
    }
#pragma omp barrier
#pragma omp for schedule(dynamic)
    for (UINT_t v = 0 ; v < n ; v++) {
      bool *myHash = Hash + (n * myID);
    
      const UINT_t s = Ap[v  ];
      const UINT_t e = Ap[v+1];
      const UINT_t l = level[v];

      for (UINT_t p = s ; p<e ; p++)
	myHash[Ai[p]] = true;

      for (UINT_t j = s ; j<e ; j++) {
	if (horiz[j]) {
	  const UINT_t w = Ai[j];
	  if (v < w) {
	    for (UINT_t k = Ap[w]; k < Ap[w+1] ; k++) {
	      UINT_t x = Ai[k];
	      if (myHash[x]) {
		if (level[x] != l) {
		  myc1[myID]++;
		}
		else {
		  myc2[myID]++;
		}
	      }
	    }
	  }
	}
      }

      for (UINT_t p = s ; p<e ; p++)
	myHash[Ai[p]] = false;
    }

#pragma omp for reduction(+:c1,c2)
    for (int i = 0; i < numThreads ; i++) {
      c1 += myc1[i];
      c2 += myc2[i];
    }
  }

  free_queue(queue);

  free(Hash);
  free(visited);
  free(level);
  free(horiz);

  return c1 + (c2/3);
}


static UINT_t tc_bader_bfs_core_P(const GRAPH_TYPE* graph, void (*f)(const GRAPH_TYPE*, const UINT_t, UINT_t *, bool *)) {
  /* Bader's new algorithm for triangle counting based on BFS */
  /* Uses Hash array to detect triangles (v, w, x) if x is adjacent to v */
  /* For level[], 0 == unvisited. Needs a modified BFS starting from level 1 */
  /* Mark horizontal edges during BFS */
  /* Direction orientied. */
  UINT_t* restrict level;
  UINT_t c1, c2;
  static bool *Hash;
  bool *visited;

  const UINT_t *restrict Ap = graph->rowPtr;
  const UINT_t *restrict Ai = graph->colInd;
  const UINT_t n = graph->numVertices;
  const UINT_t m = graph->numEdges;

  level = (UINT_t *)calloc(n, sizeof(UINT_t));
  assert_malloc(level);
  
  visited = (bool *)calloc(n, sizeof(bool));
  assert_malloc(visited);

  Queue *queue = createQueue(n);

  for (UINT_t v = 0 ; v < n ; v++) {
    if (!visited[v])
      (*f)(graph, v, level, visited);
  }

  static int numThreads;
  static UINT_t *myc1;
  static UINT_t *myc2;

  c1 = 0; c2 = 0;
#pragma omp parallel
  {
    int myID = omp_get_thread_num();
    if (myID==0) {
      numThreads = omp_get_num_threads();
      
      myc1 = (UINT_t *)calloc(numThreads, sizeof(UINT_t));
      assert_malloc(myc1);
      myc2 = (UINT_t *)calloc(numThreads, sizeof(UINT_t));
      assert_malloc(myc2);
  
      Hash = (bool *)calloc(n * numThreads, sizeof(bool));
      assert_malloc(Hash);
    }
#pragma omp barrier
#pragma omp for schedule(dynamic)
    for (UINT_t v = 0 ; v < n ; v++) {
      bool *myHash = Hash + (n * myID);
    
      const UINT_t s = Ap[v  ];
      const UINT_t e = Ap[v+1];
      const UINT_t l = level[v];

      for (UINT_t p = s ; p<e ; p++)
	myHash[Ai[p]] = true;

      for (UINT_t j = s ; j<e ; j++) {
	const UINT_t w = Ai[j];
	if ((v<w) && (l == level[w])) {
	  for (UINT_t k = Ap[w]; k < Ap[w+1] ; k++) {
	    UINT_t x = Ai[k];
	    if (myHash[x]) {
	      if (level[x] != l) {
		myc1[myID]++;
	      }
	      else {
		myc2[myID]++;
	      }
	    }
	  }
	}
      }

      for (UINT_t p = s ; p<e ; p++)
	myHash[Ai[p]] = false;
    }

#pragma omp for reduction(+:c1,c2)
    for (int i = 0; i < numThreads ; i++) {
      c1 += myc1[i];
      c2 += myc2[i];
    }
  }
  
  free_queue(queue);

  free(visited);
  free(Hash);
  free(level);

  return c1 + (c2/3);
}


UINT_t tc_bader_bfs3_P(const GRAPH_TYPE *graph) {
  return tc_bader_bfs_core_P(graph, bfs_visited);
}

UINT_t tc_bader_bfs_hybrid_P(const GRAPH_TYPE *graph) {
  return tc_bader_bfs_core_P(graph, bfs_hybrid_visited);
}

UINT_t tc_bader_bfs_hybrid2_P(const GRAPH_TYPE *graph) {
  return tc_bader_bfs_core_P(graph, bfs_hybrid_visited_P);
}

UINT_t tc_bader_bfs_chatgpt_P(const GRAPH_TYPE *graph) {
  return tc_bader_bfs_core_P(graph, bfs_chatgpt_P);
}


UINT_t tc_forward_hash_P(const GRAPH_TYPE *graph) {
  
/* Schank, T., Wagner, D. (2005). Finding, Counting and Listing All Triangles in Large Graphs, an Experimental Study. In: Nikoletseas, S.E. (eds) Experimental and Efficient Algorithms. WEA 2005. Lecture Notes in Computer Science, vol 3503. Springer, Berlin, Heidelberg. https://doi.org/10.1007/11427186_54 */

  register UINT_t s, t;
  register UINT_t b, e;
  UINT_t count = 0;

  const UINT_t* restrict Ap = graph->rowPtr;
  const UINT_t* restrict Ai = graph->colInd;
  const UINT_t n = graph->numVertices;
  const UINT_t m = graph->numEdges;

  bool* Hash = (bool *)calloc(m, sizeof(bool));
  assert_malloc(Hash);

  UINT_t* Size = (UINT_t *)calloc(n, sizeof(UINT_t));
  assert_malloc(Size);
  
  UINT_t* A = (UINT_t *)calloc(m, sizeof(UINT_t));
  assert_malloc(A);

  for (s = 0; s < n ; s++) {
    b = Ap[s  ];
    e = Ap[s+1];
    for (UINT_t i=b ; i<e ; i++) {
      t  = Ai[i];
      if (s<t) {
	count += intersectSizeHash_forward_P(graph, Hash, s, t, A, Size);
	A[Ap[t] + Size[t]] = s;
	Size[t]++;
      }
    }
  }

  free(A);
  free(Size);
  free(Hash);
  
  return count;
}



#endif
