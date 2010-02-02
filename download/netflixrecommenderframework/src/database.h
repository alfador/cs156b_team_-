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

#ifndef DATABASE_H
#define DATABASE_H

#include <qfile.h>
#include <qhash.h>

/**
    That data is stored in two files dump_v.data and dump_m.data
    dump_v.data contains the complete list of every vote and user in order
    of movie id.  dump_m.data contains an list matching up array id to the
    start location in the dump_v.data

    The files are mmaped into memory each vote taking up 32 bits.  This
    permits storing the entire list in memory.

    17770 = 100010101101010
    5     = 101

    | 3 bits for rating | 7 bits buffer | 22 bits for user id |
*/
class DataBase
{
    friend class Movie;
    friend class User;
public:
    DataBase(const QString &rootPath = "");
    ~DataBase();

    bool load();
    inline bool isLoaded() const
    {
        return (moviesFile != 0 && votesFile != 0);
    }
    QString rootPath() const;

    inline int totalUsers() const
    {
        return m_totalUsers;
    }
    inline int lastUser() const
    {
        return storedUsersIndex[totalUsers() - 1];
    }
    inline int totalVotes() const
    {
        return m_totalVotes;
    }
    inline int totalMovies() const
    {
        return (isLoaded()) ? 17770 : 0;
    }

    // functions to pull out the vote and user
    static inline uint gscore(uint x)
    {
        return (x >> 29);
    }
    static inline uint guser(uint x)
    {
        return ((x << 3) >> 3);
    }

    static void saveDatabase(const QVector<uint> &movies, const QString &fileName);

    inline int mapUser(int user) const
    {
        return users.value(user, -1);
    }

private:
    inline int lastUserSize() const
    {
        return userFile->size() / sizeof(uint) - storedUsersIndex[totalUsers()];
    }

    // For direct access by Movie and User Class
    uint *storedmovies;
    uint *storedvotes;
    uint m_totalVotes;

    uint *storedUsers;
    uint *storedUsersIndex;
    uint m_totalUsers;
    QHash<int, int> users;

    // internal
    void generateMovieDatabase();
    void generateUserDatabase();

    QFile *moviesFile;
    QFile *votesFile;
    QFile *userFile;
    QFile *userIndexFile;
    QString m_rootPath;
};

#endif
