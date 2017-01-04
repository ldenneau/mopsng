/*
File:        sb_graph.h
Author:      J. Kubica
Created:     Thu Jan 15 21:09:00 EDT 2004
Description: Header for a sparse boolean (unweighted) graph data structure.

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

#ifndef SB_GRAPH_H
#define SB_GRAPH_H

#include "amiv.h"

typedef struct sb_graph {
  int num_nodes;
  
  int*  max_outgoing;
  int*  num_outgoing;
  int** neighbor;
} sb_graph;


/* --- Memory related functions ------------------------ */
 
sb_graph* mk_empty_sb_graph(int num_nodes);

sb_graph* mk_copy_sb_graph(sb_graph* old);

void free_sb_graph(sb_graph* old);


/* --- Reference related functions --------------------- */ 

int safe_sb_graph_num_nodes(sb_graph* grph);

int safe_sb_graph_num_edges(sb_graph* grph, int node);

int safe_sb_graph_max_edges(sb_graph* grph, int node);

int safe_sb_graph_edge_ref(sb_graph* grph, int node, int edge_num);

bool sb_graph_edge_exists(sb_graph* grph, int from, int to);

void sb_graph_edge_set(sb_graph* grph, int from, int to);

ivec* mk_sb_graph_edges(sb_graph* grph, int node);

/* Allow a few speedups */
#ifdef AMFAST

#define sb_graph_num_nodes(X)     (X->num_nodes)
#define sb_graph_num_edges(X,i)   (X->num_outgoing[i])
#define sb_graph_max_edges(X,i)   (X->max_outgoing[i])
#define sb_graph_edge_ref(X,i,n)  (X->neighbor[i][n])
#define sb_graph_neighbor(X,i,n)  (X->neighbor[i][n])

#else

#define sb_graph_num_nodes(X)     (safe_sb_graph_num_nodes(X))
#define sb_graph_num_edges(X,i)   (safe_sb_graph_num_edges(X,i))
#define sb_graph_max_edges(X,i)   (safe_sb_graph_max_edges(X,i))
#define sb_graph_edge_ref(X,i,n)  (safe_sb_graph_edge_ref(X,i,n))
#define sb_graph_neighbor(X,i,n)  (safe_sb_graph_edge_ref(X,i,n))

#endif

/* --- IO related functions --------------------------- */ 

void fprintf_sb_graph(FILE* f, char* prefix, sb_graph* grph, char* postfix);

#endif
