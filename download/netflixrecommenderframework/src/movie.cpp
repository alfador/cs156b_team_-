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

#include "movie.h"
#include "binarysearch.h"

Movie::Movie(DataBase *db, int number) : db(db), m_size(0), m_id(0)
{
    totalMovies = db->totalMovies();
    if (!db->isLoaded()) {
        m_size = 0;
        m_id = 0;
        return;
    }
    if (number < 1)
        number = 1;
    if (number > totalMovies)
        number = totalMovies;
    setId(number);
}

// #1 has 547
// #2 has 145
// number must be > 0
void Movie::setId(int number)
{
    m_id = (number - 1);
    if (number < totalMovies) {
        // Exploit the fact that we have an array rather then doing:
        //m_size = db->storedmovies[m_id + 1] - db->storedmovies[m_id];
        // pointer fun is faster
        uint *x = &(db->storedmovies[number]);
        m_size = (*x);
        m_size -= (*(--x));
    } else {
        if (totalMovies == 0)
            m_size = 0;
        else
            m_size = db->totalVotes() - db->storedmovies[m_id];
    }
}

Movie Movie::getMovie(DataBase *db, uint vote)
{
    uint* i = qBinaryFind(db->storedmovies, (db->storedmovies + db->totalMovies() - 1), (uint)vote);
    uint movieId = (i - (db->storedmovies));
    // movie id's are offset 1
    return Movie(db, movieId + 1);
}

int Movie::findVote(uint user) const
{
    int start = db->storedmovies[m_id];
    int end = start + votes() - 1;
    int r = userBinarySearch(db->storedvotes, user, start, end);
    if (DataBase::guser(db->storedvotes[r]) == user) {
        return (r - db->storedmovies[m_id]);
    }
    return -1;
}

