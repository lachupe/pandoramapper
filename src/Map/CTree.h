/*
 *  Pandora MUME mapper
 *
 *  Copyright (C) 2000-2009  Azazello
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TREE_H 
#define TREE_H 

#include <QVector>

#define ALPHABET_SIZE	27	/*  26 letters */
#define A_SIZE		ALPHABET_SIZE
#define MAX_HASH_LEN	150	/* this caps top length of the tree thread */


struct TTree {
    TTree *leads[A_SIZE];	/* pointers to the next part letter */
    QVector<unsigned int> ids;
};

struct levels_data_type {
  unsigned long leads;
  unsigned long nodes;
  unsigned long items;
};

class CTree {
  int divingDelete(TTree *p, char *part, unsigned int id);
  void genHash(const char *name, char *hash);
  void deleteAll(TTree *t);
    
  /* for gathering debug info only*/
  struct levels_data_type levels_data[MAX_HASH_LEN];
  void calculateInfo(TTree *t, int level, int single);
  long debugSingles;

public:
  TTree *root;


  CTree();

  void addName(const char *name, unsigned int id);	
  void resetTTree(TTree *t);
  TTree * findByName(const char *name);
  void deleteItem(const char *name, unsigned int id);
  void reinit();
  void printTreeStats();
  void removeId(unsigned int id, TTree *t);
};

extern class CTree NameMap;

#endif


