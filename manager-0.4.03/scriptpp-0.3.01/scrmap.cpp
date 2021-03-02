// +-------------------------------------------------------------------------+
// |                     Script Plus Plus vers. 0.3.00                       |
// | Copyright (c) Andrey Vikt. Stolyarov <crocodil_AT_croco.net>  2003-2009 |
// | ----------------------------------------------------------------------- |
// | This is free software.  Permission is granted to everyone to use, copy  |
// |        or modify this software under the terms and conditions of        |
// |                 GNU LESSER GENERAL PUBLIC LICENSE, v. 2.1               |
// |     as published by Free Software Foundation (see the file LGPL.txt)    |
// |                                                                         |
// | Please visit http://www.croco.net/software/scriptpp to get a fresh copy |
// | ----------------------------------------------------------------------- |
// |   This code is provided strictly and exclusively on the "AS IS" basis.  |
// | !!! THERE IS NO WARRANTY OF ANY KIND, NEITHER EXPRESSED NOR IMPLIED !!! |
// +-------------------------------------------------------------------------+




#include "scrmap.hpp"

//#include <stdio.h> // FOR DEBUGGING!!!

typedef unsigned long script_hash_t;


static const script_hash_t hash_multiplier = 0x9c406bb5;
// Recommended by Knuth

static const script_hash_t hash_xor_op = 0x12fade34;
// arbitrary

static const unsigned long hash_sizes[] = {
            11,        /* > 8         */
            17,        /* > 16        */
            37,        /* > 32        */
            67,        /* > 64        */
            131,       /* > 128       */
            257,       /* > 256       */
            521,       /* > 512       */
            1031,      /* > 1024      */
            2053,      /* > 2048      */
            4099,      /* > 4096      */
            8209,      /* > 8192      */
            16411,     /* > 16384     */
            32771,     /* > 32768     */
            65537,     /* > 65536     */
            131101,    /* > 131072    */
            262147,    /* > 262144    */
            524309,    /* > 524288    */
            1048583,   /* > 1048576   */
            2097169,   /* > 2097152   */
            4194319,   /* > 4194304   */
            8388617,   /* > 8388608   */
            16777259,  /* > 16777216  */
            33554467,  /* > 33554432  */
            67108879,  /* > 67108864  */
            134217757, /* > 134217728 */
            /* we don't need sizes larger than 2**27; */
            /* allocating such an array would crash the program anyway */
            0          /* end-of-table mark */
        };


static script_hash_t UniversalHash(script_hash_t l)
{
    return hash_multiplier * (l ^ hash_xor_op);
}

static script_hash_t CharStringSum(const char *s)
{
    register script_hash_t res = 7; // Arbitrary.

    register int i = 0;
    while(*s) {
        res ^= (((script_hash_t) *s) << (8*(i%4)));
        // Every byte is XOR'ed with 0th, 1st, 2nd, 3rd, 0th... byte of res.
        s++; i++;
    }
    return res;
}

static script_hash_t ScriptHash(const ScriptVariable &s)
{
    if(s.IsInvalid()) return 0;
    else return UniversalHash(CharStringSum(s.c_str()));
}

ScriptSet::ScriptSet()
{
    dim = hash_sizes[0];
    table = new ScriptVariableInv[dim];
    itemcount = 0;
}

ScriptSet::~ScriptSet()
{
    delete[] table;
}

bool ScriptSet::Contains(const ScriptVariable &key) const
{
    return table[GetItemPosition(key)].IsValid();
}

bool ScriptSet::AddItem(const ScriptVariable &key)
{
    int pos = ProvideItemPosition(key);
    if(table[pos].IsInvalid()) {
        table[pos] = key;
        itemcount++;
        return true;
    }
    return false;
}

int ScriptSet::AddItemWithPos(const ScriptVariable &key)
{
    int pos = ProvideItemPosition(key);
    if(table[pos].IsInvalid()) {
        table[pos].ScriptVariable::operator=(key);
        itemcount++;
    }
    return pos;
}

int ScriptSet::GetItemPosition(const ScriptVariable &key) const
{
    script_hash_t h = ScriptHash(key);
    int pos = h % dim;
    while(table[pos].IsValid()) {
        if(table[pos] == key) // found !
            return pos;
        pos = pos ? pos-1 : dim-1;
    }
    return pos;
}

int ScriptSet::FindItemPos(const ScriptVariable &key) const
{
    int pos = GetItemPosition(key);
    if(table[pos].IsInvalid()) return -1;
    return pos;
}

int ScriptSet::ProvideItemPosition(const ScriptVariable &key)
{
    int pos = GetItemPosition(key);
    if(table[pos].IsInvalid()) {
        // not found -- we'll need to reserve a slot
        if(itemcount * 3 > dim * 2) {
            // This is rare event so we can leave it unoptimized
            ResizeTable();
            // table changed; recompute the pos
            pos = GetItemPosition(key);
        }
    }
    return pos;
}

bool ScriptSet::RemoveItem(const ScriptVariable& key)
{
    int pos = ScriptHash(key) % dim;
    while(table[pos].IsValid()) {
        if(table[pos] == key) {
            DoRemoveItem(pos);
            return true;
        }
        pos = pos ? pos-1 : dim-1;
    }
    // Not found
    return false;
}

void ScriptSet::Clear()
{
    for(unsigned int i = 0; i< dim; i++)
        table[i].Invalidate();
    itemcount = 0;
}


void ScriptSet::DoRemoveItem(int pos)
{
    /* We implement the Don Knuth' algorythm of removing items */
    /* see vol. 3, section 6.4 */
    itemcount--;
    table[pos].Invalidate();
    for(long i = pos?pos-1:dim-1; table[i].IsValid(); i =i?i-1:dim-1) {
        long r = ScriptHash(table[i]) % dim;
        if( (i <= r && r < pos) ||
                ((pos < i) && (r < pos || i <= r))
          ) continue;
        table[pos] = table[i];
        table[i].Invalidate();
        pos = i;
    }
}

void ScriptSet::ResizeTable()
{
    unsigned long newdim = 0;
    for(int i=1; hash_sizes[i]; i++) {
        if(hash_sizes[i] > dim) {
            newdim = hash_sizes[i];
            break;
        }
    }
    //printf("    ** RESIZE(%ld): %ld -> %ld\n", Count(), dim, newdim);
    /* if(!newdim) // this actually can not happen */
    long olddim = dim;
    ScriptVariable *oldtable = table;
    dim = newdim;
    table = new ScriptVariableInv[newdim];
    itemcount = 0;
    void *user_values = HookResizeStart(newdim);
    for(long i = 0; i<olddim; i++) {
        if(oldtable[i].IsValid()) {
            int pos = AddItemWithPos(oldtable[i]);
            HookResizeReadd(user_values, i, pos);
        }
    }
    HookResizeFinish(user_values);
    delete [] oldtable;
}

ScriptSet::Iterator::Iterator(const ScriptSet &ref)
{
    tbl = &ref;
    idx = 0;
    lim = ref.dim;
}

ScriptVariable ScriptSet::Iterator::GetNext()
{
    ScriptVariable res;
    int pos;
    if(GetNext(res, pos)) {
        return res;
    } else {
        return ScriptVariableInv();
    }
}

bool ScriptSet::Iterator::GetNext(ScriptVariable &key, int &pos)
{
    while(idx<lim && tbl->table[idx].IsInvalid())
    {
        idx++;
    }
    if(idx>=lim) {
        return false;
    }
    key = tbl->table[idx];
    pos = idx;
    idx++;
    return true;
}

//////////////////////////////////////////////////

ScriptMap::ScriptMap()
    : ScriptSet()
{
    val = new ScriptVariableInv[GetTableSize()];
}

ScriptMap::~ScriptMap()
{
    delete [] val;
}

bool ScriptMap::AddItem(const ScriptVariable &key, const ScriptVariable &v)
{
    int pos = AddItemWithPos(key);
    if(val[pos].IsValid()) return false;
    val[pos] = v;
    return true;
}

void ScriptMap::SetItem(const ScriptVariable &key, const ScriptVariable &v)
{
    int pos = AddItemWithPos(key);
    val[pos] = v;
}

ScriptVariable ScriptMap::GetItem(const ScriptVariable &key)
{
    int pos = FindItemPos(key);
    if(pos == -1) return ScriptVariableInv();
    return val[pos];
}

ScriptVariable& ScriptMap::operator[](const ScriptVariable& key)
{
    int pos = AddItemWithPos(key);
    if(val[pos].IsInvalid()) val[pos] = "";
    return val[pos];
}

void ScriptMap::Clear()
{
    int c = GetTableSize();
    ScriptSet::Clear();
    for(int i=0; i<c; i++) val[i].Invalidate();
}

void* ScriptMap::HookResizeStart(int newsize)
{
    ScriptVariable *old_val = val;
    val = new ScriptVariableInv[newsize];
    return old_val;
}

void ScriptMap::HookResizeReadd(void *userdata, int oldpos, int newpos)
{
    val[newpos] = ((ScriptVariable*)userdata)[oldpos];
}

void ScriptMap::HookResizeFinish(void *userdata)
{
    ScriptVariable *p = (ScriptVariable*)userdata;
    delete[] p;
}
