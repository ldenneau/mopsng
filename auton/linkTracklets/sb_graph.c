/*
File:        sb_graph.c
Author:      J. Kubica
Created:     Thu Jan 15 21:20:00 EDT 2004
Description: A sparse boolean (unweighted) graph data structure.

Copyright 2004, The Auton Lab, CMU

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "sb_graph.h"


/* --- Memory related functions ------------------------ */
 
sb_graph* mk_empty_sb_graph(int num_nodes) {
  sb_graph* res = AM_MALLOC(sb_graph);
  int i;

  my_assert(num_nodes > 0);

  res->num_nodes    = num_nodes;
  res->num_outgoing = AM_MALLOC_ARRAY(int,num_nodes);
  res->max_outgoing = AM_MALLOC_ARRAY(int,num_nodes);
  res->neighbor     = AM_MALLOC_ARRAY(int*,num_nodes);
  
  for(i=0;i<num_nodes;i++) {
    res->num_outgoing[i] = 0;
    res->max_outgoing[i] = 0;
    res->neighbor[i]     = NULL;
  }

  return res;
}


sb_graph* mk_copy_sb_graph(sb_graph* old) {
  sb_graph* res;
  int n,i,I,N;

  res = mk_empty_sb_graph(old->num_nodes);
  N   = old->num_nodes;

  for(n=0;n<N;n++) {
    I = sb_graph_num_edges(old,n);
    res->num_outgoing[n] = I;
    res->max_outgoing[n] = I;
    res->neighbor[n]     = AM_MALLOC_ARRAY(int,I);

    for(i=0;i<I;i++) {
      res->neighbor[n][i] = old->neighbor[n][i];
    }
  }
  res->num_nodes = old->num_nodes;

  return res;
}


void free_sb_graph(sb_graph* old) {
  int i, N;

  N = old->num_nodes;
  for(i=0;i<N;i++) {
    if(old->neighbor[i] != NULL) {
      AM_FREE_ARRAY(old->neighbor[i],int,old->max_outgoing[i]);
    }
  }
  AM_FREE_ARRAY(old->neighbor,int*,N);
  AM_FREE_ARRAY(old->max_outgoing,int,N);
  AM_FREE_ARRAY(old->num_outgoing,int,N);
  
  AM_FREE(old,sb_graph);
}


/* --- Reference related functions --------------------- */ 

int safe_sb_graph_num_nodes(sb_graph* grph) {
  return grph->num_nodes;
}


int safe_sb_graph_num_edges(sb_graph* grph, int node) {
  my_assert((node >= 0)&&(node < grph->num_nodes));
  return grph->num_outgoing[node];
}


int safe_sb_graph_max_edges(sb_graph* grph, int node) {
  my_assert((node >= 0)&&(node < grph->num_nodes));
  return grph->max_outgoing[node];
}


int safe_sb_graph_edge_ref(sb_graph* grph, int node, int edge_num) {
  my_assert((edge_num >= 0)&&(edge_num < sb_graph_num_edges(grph, node)));
  return grph->neighbor[node][edge_num];
}


bool sb_graph_edge_exists(sb_graph* grph, int from, int to) {
  int min, max, mid;
  bool found = FALSE;

  my_assert((from >= 0)&&(from < grph->num_nodes));
  my_assert((to   >= 0)&&(to   < grph->num_nodes));

  if(grph->neighbor[from] == NULL) {
    return FALSE;
  }

  max = grph->num_outgoing[from]-1;
  min = 0;

  while((found == FALSE)&&(max > min)) {
    mid = (int)((max+min)/2);

    if(grph->neighbor[from][mid] == to) {
      found = TRUE;
    } else {
      if(grph->neighbor[from][mid] < to) {
        if(min == mid) { 
          min++;
        } else {
          min = mid;
        }
      } else {
        if(max == mid) {
          max--;
        } else {
          max = mid;
        }
      }
    }
  }

  return (found || (grph->neighbor[from][min] == to));
}


void sb_graph_edge_set(sb_graph* grph, int from, int to) {
  int* temp;
  int  tempB;
  int N = sb_graph_max_edges(grph, from);
  int n = sb_graph_num_edges(grph, from);
  int i;

  if( sb_graph_edge_exists(grph, from, to) == FALSE ) {

    /* Create the array is needed */
    if(N == 0) {
      grph->neighbor[from]     = AM_MALLOC_ARRAY(int,2);
      grph->neighbor[from][0]  = 0;
      grph->neighbor[from][1]  = 0;
      grph->max_outgoing[from] = 2;
      N = 2;
    }

    /* Expand the array if needed */
    if(n == N) {
      temp = grph->neighbor[from];
      grph->neighbor[from] = AM_MALLOC_ARRAY(int,2*N);
 
      for(i=0;i<n;i++) {
        grph->neighbor[from][i] = temp[i];
      }
      for(i=n;i<2*N;i++) {
        grph->neighbor[from][i] = 0;
      }

      AM_FREE_ARRAY(temp,int,N);

      N = 2*N;
      grph->max_outgoing[from] = N;
    }

    /* Insert the new edge at the end and bubble it back into the
       correct location */
    grph->neighbor[from][n] = to;
    grph->num_outgoing[from] = n+1;
    
    while( (n>0)&&(grph->neighbor[from][n] < grph->neighbor[from][n-1]) ) {
      tempB = grph->neighbor[from][n];
      grph->neighbor[from][n]   = grph->neighbor[from][n-1];
      grph->neighbor[from][n-1] = tempB;
      n--;
    }
  }

}


ivec* mk_sb_graph_edges(sb_graph* grph, int node) {
  ivec* res;
  int N = sb_graph_num_edges(grph, node);
  int i;

  res = mk_ivec(N);
  for(i=0;i<N;i++) {
    ivec_set(res,i,sb_graph_edge_ref(grph,node,i));
  }

  return res;
}



void fill_sb_graph_connected_component_from(sb_graph* grph, int node, ivec* used, ivec* comp) {
  int N, i;

  /* Check if the current node has been used and add it */
  /* to the list. */
  if(ivec_ref(used,node) == 0) {
    add_to_ivec(comp,node);
    ivec_set(used,node,1);

    /* Recursively all the neighbors... */
    N = sb_graph_num_edges(grph,node);
    for(i=0;i<N;i++) {
      fill_sb_graph_connected_component_from(grph,sb_graph_neighbor(grph,node,i),used,comp);
    }
  }
}


/* --- IO related functions --------------------------- */

void fprintf_sb_graph(FILE* f, char* prefix, sb_graph* grph, char* postfix) {
  int N = grph->num_nodes;
  int I, i, n;

  fprintf(f,prefix);

  for(n=0;n<N;n++) {
    I = sb_graph_num_edges(grph,n);
    fprintf(f,"%i: ",n);
    if(I > 0) {
      for(i=0;i<I;i++) {
        fprintf(f,"%i ",sb_graph_edge_ref(grph, n, i));
      }
      fprintf(f,"\n");
    } else {
      fprintf(f,"EMPTY\n");
    }
  }

  fprintf(f,postfix);
}


/* Print out the graph in two columns */
void fprintf_sb_graph_thin(FILE* f, char* prefix, sb_graph* grph, char* postfix) {
  int N = grph->num_nodes;
  int I, i, n;

  fprintf(f,prefix);

  for(n=0;n<N;n++) {
    I = sb_graph_num_edges(grph,n);
    for(i=0;i<I;i++) {
      fprintf(f,"%i -- %i\n",n,sb_graph_edge_ref(grph, n, i));
    }
  }

  fprintf(f,postfix);
}



void sb_graph_tester_main() {
  sb_graph* g;

  g = mk_empty_sb_graph(5);
  sb_graph_edge_set(g,0,1);
  sb_graph_edge_set(g,0,3);
  sb_graph_edge_set(g,0,2);

  sb_graph_edge_set(g,1,4);
  sb_graph_edge_set(g,1,1);
  sb_graph_edge_set(g,1,3);
  sb_graph_edge_set(g,1,2);

  sb_graph_edge_set(g,3,3);
  sb_graph_edge_set(g,3,4);
  sb_graph_edge_set(g,3,3);

  fprintf_sb_graph(stdout,"",g,"\n");

  printf("%i's 2nd neighbor is %i\n",0,sb_graph_edge_ref(g,0,1));
  printf("%i's 1st neighbor is %i\n",1,sb_graph_edge_ref(g,1,0));
  printf("%i's 2nd neighbor is %i\n",3,sb_graph_edge_ref(g,3,1));

  printf("(0,1) = %i\n",sb_graph_edge_exists(g, 0, 1));
  printf("(3,1) = %i\n",sb_graph_edge_exists(g, 3, 1));

  free_sb_graph(g);
}
