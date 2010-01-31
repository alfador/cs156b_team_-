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

#include "quickdatabase.h"

#include <user.h>

QuickDatabase::QuickDatabase(DataBase *db)
{
    int totalMovies = db->totalMovies();
    int totalUsers = db->totalUsers();
    size_t size = (size_t)(((double)totalMovies * totalUsers) / 8 + 1);
    buffer = (char*) malloc (size);
    if (buffer == NULL) {
        qWarning() << "Unable to malloc memory";
        buffer = 0;
        return;
    }

    User user(db, 6);
    for (double i = 0;  i < totalUsers; ++i) {
        for (int j = 0; j < user.votes(); ++j) {
            int movie = user.movie(j);
            int location = (int)((i * totalMovies + movie) / 8);
            int offset = (int)((i * totalMovies + movie) - (location * 8));
            buffer[location] |= (1 << offset);
        }
        user.next();

        int one = totalUsers / 10;
        if ((int)i % one == 0 && i != 0)
            printf("filling: %d%%\n", ((int)i / one * 10));
    }
}

QuickDatabase::~QuickDatabase()
{
    free (buffer);
}

