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




#ifndef SCRIPTPP_SCRMAP_HPP_SENTRY
#define SCRIPTPP_SCRMAP_HPP_SENTRY

/*! \file scrmap.hpp
    \brief This file invents the ScriptMap class which is a hash table 
 */

#include "scrvar.hpp"

class ScriptSet {
    unsigned long dim;
    ScriptVariableInv *table; //!< Stores keys
    unsigned long itemcount;
public:
    ScriptSet(); 
    virtual ~ScriptSet();
    
    //! Find item 
    /*! Returns true if the item is there, false otherwise
     */
    bool Contains(const ScriptVariable &key) const;

    //! Add item 
    /*! Adds the item. 
        Returns true if there was one, false otherwise
     */
    bool AddItem(const ScriptVariable &key);

    //! Remove item 
    /*! Removes the item. 
        Returns true if there was one, false otherwise
     */
    bool RemoveItem(const ScriptVariable &key);

    //! Items count
    /*! Return total amount of items in the table */ 
    long Count() const { return itemcount; }

    //! Clear the table
    /*! Remove all existing items from the table */
    void Clear();

    //! Iterator to walk through the hash table
    class Iterator {
        const ScriptSet *tbl;
        int idx;
        int lim;
    public:
        //! The constructor
        Iterator(const ScriptSet &tbl);
        //! Iterate
        /*! Returns invalid ScriptVariable if the table is over */
        ScriptVariable GetNext();
    protected:
        bool GetNext(ScriptVariable &key, int &pos);        
    };

    friend class Iterator::Iterator;

private:
    int GetItemPosition(const ScriptVariable &s) const;
    int ProvideItemPosition(const ScriptVariable &s);
    void DoRemoveItem(int pos);
    void ResizeTable();

protected:
    //! Get the size of the table
    int GetTableSize() const { return dim; }
    //! Add the item, return the position in the table to which it is added
    int AddItemWithPos(const ScriptVariable &key);
    //! If the item is there, return its position, otherwise return -1
    int FindItemPos(const ScriptVariable &key) const;

    //! Resize start hook
    /*! Called right before the table resize takes place.  Should
        return a void* which points to user data used to perform
        resizing, e.g. the old payload array pointer.  The pointer
        is then passed to HookResizeReadd and HookResizeFinish
     */
    virtual void* HookResizeStart(int newsize) { return 0; }
    //! Resize re-add hook
    /*! Called for each item in the table, reporting its position
        within the old table and within the new table
     */
    virtual void HookResizeReadd(void *userdata, int oldpos, int newpos) {}
    //! Resize finish hook
    /*! Called after the resize is done */
    virtual void HookResizeFinish(void *userdata) {}

};    

class ScriptMap : private ScriptSet {
    ScriptVariableInv *val; //!< Stores values
public:
    ScriptMap();
    ~ScriptMap();

    bool AddItem(const ScriptVariable &key, const ScriptVariable &val);
    void SetItem(const ScriptVariable &key, const ScriptVariable &val);
    ScriptVariable GetItem(const ScriptVariable &key);

    ScriptVariable& operator[](const ScriptVariable& key);

    long Count() const { return ScriptSet::Count(); }
    void Clear();

    class Iterator : private ScriptSet::Iterator {
        ScriptVariable *valtbl;
    public:
        Iterator(const ScriptMap &tbl)
            : ScriptSet::Iterator(tbl), valtbl(tbl.val) {}
        bool GetNext(ScriptVariable &key, ScriptVariable &val) {
             int pos;
             if(ScriptSet::Iterator::GetNext(key, pos)) {
                 val = valtbl[pos];
                 return true;
             }
             return false;
        }
    };
    friend class Iterator;
private:
    virtual void* HookResizeStart(int newsize);
    virtual void HookResizeReadd(void *userdata, int oldpos, int newpos);
    virtual void HookResizeFinish(void *userdata);
};



#endif
