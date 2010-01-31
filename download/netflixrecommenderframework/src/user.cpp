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

#include "user.h"
#include "binarysearch.h"
#include <qdebug.h>

User::User(const User &otherUser) : db(otherUser.db),
        m_id(otherUser.m_id),
        offset(otherUser.offset),
        indexOffset(otherUser.indexOffset),
        m_size(otherUser.m_size)
{}

User::User(DataBase *db, int id) : db(db)
{
    setId(id);
}

int User::seenMovie(int id) const
{
    uint r = userBinarySearch((db->storedUsers + offset), (uint)id, 0, m_size - 1);
    if (movie(r) == id)
        return score(r);
    return -1;
}

void User::setId(int number)
{
    int result = db->mapUser(number);
    if (!db->isLoaded() || result == -1) {
        m_id = -1;
        m_size = 0;
        offset = 0;
        indexOffset = 0;
        return;
    }
    m_id = number;
    indexOffset = result;
    result += db->totalUsers();
    offset = db->storedUsersIndex[result];
    m_size = db->storedUsersIndex[result + 1] - db->storedUsersIndex[result];
}

void User::next()
{
    // grab pointer to the the array
    // m_id = db->storedUsersIndex[++indexOffset];
    uint *y = &(db->storedUsersIndex[++indexOffset]);
    m_id = *y;
    // which we can then just walk
    //m_size = db->storedUsersIndex[result + 1] - db->storedUsersIndex[result];
    uint *x = y + db->totalUsers() + 1;
    m_size = (*x);
    m_size -= (*(--x));
    offset = *x;
}

