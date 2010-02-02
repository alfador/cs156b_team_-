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

#ifndef MOVIE_H
#define MOVIE_H

#include <database.h>

class Movie
{

public:
    Movie(DataBase *db, int number = 1);
    void setId(int number);

    inline uint id() const
    {
        return m_id + 1;
    }
    inline uint votes() const
    {
        return m_size;
    }
    int findVote(uint user) const;

    inline int user(int x) const
    {
        return DataBase::guser(db->storedvotes[db->storedmovies[m_id] + x]);
    }

    inline int score(int x) const
    {
        return DataBase::gscore(db->storedvotes[db->storedmovies[m_id] + x]);
    }

    inline int findScore(uint user) const
    {
        return score(findVote(user));
    }

    inline DataBase *dataBase() const
    {
        return db;
    }

    /*!
        Return the movie that vote is cast upon
    */
    static Movie getMovie(DataBase *db, uint vote);

    /*!
        Returns the offset for the votes in the votes DataBase array
    */
    inline int dataBaseOffset() const
    {
        return db->storedmovies[m_id];
    }

private:
    DataBase *db;
    uint m_size;
    uint m_id;
    int totalMovies;
};

#endif

