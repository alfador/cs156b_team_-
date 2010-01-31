/**
 * Copyright (C) 2006-2007 Benjamin C. Meyer (ben at meyerhome dot net)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Benjamin Meyer nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef USER_H
#define USER_H

#include "database.h"
#include <qdebug.h>
class User
{

public:
    User(DataBase *db, int id = 0);

    User(const User &otherUser);

    /*
        The first valid id is 6
    */
    void setId(int number);

    /*
        Sets this user to the next user
        *warning* no validation is done to make sure there is a next user.
    */
    void next();

    inline int id() const
    {
        return m_id;
    }

    inline int votes() const
    {
        return m_size;
    }

    int seenMovie(int id) const;

    inline int movie(int x) const
    {
        return DataBase::guser(db->storedUsers[offset + x]);
    }

    inline int score(int x) const
    {
        return DataBase::gscore(db->storedUsers[offset + x]);
    }

    inline DataBase *dataBase() const
    {
        return db;
    }

private:
    DataBase *db;
    int m_id;
    uint offset;
    uint indexOffset;
    uint m_size;
};

#endif

