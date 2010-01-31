/**
 * Copyright (C) 2007 Benjamin C. Meyer (ben at meyerhome dot net)
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

#ifndef QUICKDATABASE_H
#define QUICKDATABASE_H

#include "database.h"

/**
    Provides a two dimentaional array of users by movies where each cell is 1 bit,
    0 being a no vote
    1 being a vote

    This is an super fast alternative to using user.seen().  An althernative that
    would use less ram, but not be as fast would be to use a hash.

    Beware this uses a little over a GB of ram
 */
class QuickDatabase
{

public:
    QuickDatabase(DataBase *db);
    ~QuickDatabase();

    inline bool has(uint user, uint movie);

private:
    char *buffer;

};

/**
    Returns true if user has voted on movie otherwise false.
 */
bool QuickDatabase::has(uint user, uint movie)
{
    uint location = (user * 17770 + movie) / 8;
    uchar x = buffer[location];
    if (x == 0)
        return false;
    uint offset = (user * 17770 + movie) - (location * 8);
    return x & (1 << offset);
}

#endif
