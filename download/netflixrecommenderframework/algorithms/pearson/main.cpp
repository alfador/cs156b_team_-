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
#include <quickdatabase.h>

#include <qdebug.h>
#include <qvector.h>

/*
# Compute the Pearson correlation coefficient between two arrays of numbers:
#
#  SUM( (x[i] - Xmean)(y[i] - Ymean) ) / SQRT( SUM( (x[i] - Xmean)^2 )(SUM( (y[i] - Ymean)^2 ))   )
#
# It returns values in the range from -1.0 to 1.0
*/
double mean(const QVector<int> &v, int size)
{
    int t = 0;
    for (int i = 0; i < size; ++i)
        t += v.at(i);
    return t / (double)v.count();
}

double pearsonCorrelationCoefficient(const QVector<int> &x, const QVector<int> &y, int size)
{
    double productOfDeviations = 0;
    double squareOfXDeviations = 0;
    double squareOfYDeviations = 0;
    double xMean = mean(x, size);
    double yMean = mean(y, size);
    for (int i = 0; i < size; ++i) {
        double xValueDeviation = x.at(i) - xMean;
        double yValueDeviation = y.at(i) - yMean;
        productOfDeviations += (xValueDeviation * yValueDeviation);
        squareOfXDeviations += (xValueDeviation * xValueDeviation);
        squareOfYDeviations += (yValueDeviation * yValueDeviation);
    }
    return productOfDeviations / sqrt(squareOfXDeviations * squareOfYDeviations);
}

/**
    One of the basic algorithm is the average algorithm which takes
    the average of all the votes for a movie and guess's that for any user.

*/
bool first(const QPair<double, int> &a, const QPair<double, int> &b)
{
    return a.first > b.first;
};
class Pearson : public Algorithm
{

public:
    Pearson(DataBase *db) : currentMovie(db), user(db), qdb(new QuickDatabase(db))
    {
    }
    
    ~Pearson()
    {
        delete qdb;
    }

    void setMovie(int id)
    {
        currentMovie.setId(id);
    }


    double determine(int userId)
    {
        user.setId(userId);
        movieUsers.resize(currentMovie.votes());
        User otherUser(currentMovie.dataBase());

        int currentMovieId = currentMovie.id();
        for (uint i = 0; i < currentMovie.votes(); ++i) {
            int nextUser = currentMovie.user(i);
            if (nextUser == userId)
                continue;
            otherUser.setId(nextUser);
            int c = 0;

            int userVotes = otherUser.votes();
            if (x.size() < userVotes) {
                x.resize(userVotes);
                y.resize(userVotes);
            }

            int userNumber = user.dataBase()->mapUser(user.id());
            for (int j = 0; j < userVotes; ++j) {
                int movie = otherUser.movie(j);
                if (movie == currentMovieId)
                    continue;
                if (qdb->has(userNumber, movie)) {
                    x[c] = (user.seenMovie(movie));
                    y[c] = (otherUser.score(j));
                    ++c;
                }
            }
            if (c > ((user.votes() / 100) * 70)) {
                double result = pearsonCorrelationCoefficient(x, y, c);
                movieUsers[i] = QPair<double, int>(result, currentMovie.score(i));
            }
        }

        qSort(movieUsers.begin(), movieUsers.end(), first);
        uint total = 0;
        int i;
        for (i = 0; i < 200 && i < movieUsers.count(); ++i)
            total += movieUsers.at(i).second;
        if (i == 0)
            return 3;
        return total / (double)i;
    }

    QVector<int> x;
    QVector<int> y;

    QVector<QPair<double, int> > movieUsers;

    Movie currentMovie;
    User user;
    QuickDatabase *qdb;
};

int main(int , char **)
{
    DataBase db;
    db.load();
    Probe probe(&db);
    Pearson algorithm(&db);
    return probe.runProbe(&algorithm);
}

