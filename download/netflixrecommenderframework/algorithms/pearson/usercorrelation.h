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

#ifndef USERCORRELATION_H
#define USERCORRELATION_H

class UserCorrelation
{

public:
    UserCorrelation();
    virtual ~UserCorrelation();

    /*
        Returns the correlation between user1 and user2
    */
    double correlation(const User &user1, const User &user2) = 0;
};

class PearsonCorrelation
{
public:
PearsonCorrelation : UserCorrelation()
    {}

    double mean(const QVector<int> &v)
    {
        int t = 0;
        for (int i = 0; i < v.count(); ++i)
            t += v.at(i);
        return t / (double)v.count();
    }

    double pearsonCorrelationCoefficient(const QVector<int> &x, const QVector<int> &y, int size)
    {
        double productOfDeviations = 0;
        double squareOfXDeviations = 0;
        double squareOfYDeviations = 0;
        double xMean = mean(x);
        double yMean = mean(y);
        for (int i = 0; i < size; ++i) {
            double xValueDeviation = x.at(i) - xMean;
            double yValueDeviation = y.at(i) - yMean;
            productOfDeviations += (xValueDeviation * yValueDeviation);
            squareOfXDeviations += (xValueDeviation * xValueDeviation);
            squareOfYDeviations += (yValueDeviation * yValueDeviation);
        }
        return productOfDeviations / sqrt(squareOfXDeviations * squareOfYDeviations);
    }

    double correlation(const User &user1, const User &user2, )
    {
        QVector<int> a;
        QVector<int> b;
        int size = user1.votes() > user2.votes() ? user1.votes() : user2.votes();
        for (int i = 0; i < size; ++i) {
            uint user1Vote = user1.score(i);
            uint user2Vote = user2.seenMovie(user1.movie(i));
            if (user2Vote == -1)
                continue;
            a.append(user1Vote);
            b.append(user1Vote);
        }
        return pearsonCorrelationCoefficient(a, b);
    }
}

#endif

