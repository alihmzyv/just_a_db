//
// Created by Ali on 16.09.26.
//

#ifndef JUST_A_DB_B_PLUS_TREE_INDEX_H
#define JUST_A_DB_B_PLUS_TREE_INDEX_H

typedef struct BPlusTreeIndex BPlusTreeIndex;

typedef struct CurrentTupleId CurrentTupleId;

typedef enum {
    B_PLUS_TREE_OK = 0,
    B_PLUS_TREE_ERR_IO = -1,
    B_PLUS_TREE_ERR_OOM = -2
} BPlusTreeStatus;

int get_ctid(BPlusTreeIndex *index, void *value, u_int64_t bytes, CurrentTupleId** ctid, uint32_t* match_num);

#endif //JUST_A_DB_B_PLUS_TREE_INDEX_H
