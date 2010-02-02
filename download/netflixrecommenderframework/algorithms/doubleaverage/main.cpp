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

#include <probe.h>
#include <user.h>

/**
    One of the basic algorithm is the average algorithm which takes
    the average of all the votes for a movie and guess's that for any user.
 */
class DoubleAverage : public Algorithm
{
public:
    DoubleAverage(DataBase *db) : currentMovie(db), user(db), currentMovieAverage(-1)
    {}

    void setMovie(int id)
    {
        currentMovie.setId(id);
        currentMovieAverage = -1;
    }

    double determine(int userId)
    {
        if (currentMovieAverage == -1) {
            currentMovieAverage = 0;
            uint currentMovieVotes = currentMovie.votes();
            for (uint i = 0; i < currentMovieVotes; ++i) {
                currentMovieAverage += currentMovie.score(i);
            }
            currentMovieAverage = currentMovieAverage / (double)(currentMovie.votes());
        }

        user.setId(userId);
        int pastVotes = 0;
        int userVotes = user.votes();
        for (int i = 0; i < userVotes; ++i) {
            pastVotes += user.score(i);
        }
        double currentUserAverage = pastVotes / (double)(user.votes());
        double r = (currentMovieAverage + currentUserAverage) / 2;
        r = qMax(r, 1.0);
        r = qMin(r, 5.0);
        return r;
    }

    Movie currentMovie;
    User user;
    double currentMovieAverage;
};

int main(int , char **)
{
    DataBase db;
    db.load();
    Probe probe(&db);
    DoubleAverage bf(&db);
    return probe.runProbe(&bf);
}

