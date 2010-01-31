/**
 * Copyright (c) 2006-2007 Benjamin C. Meyer (ben at meyerhome dot net)
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

#include <QtTest/QtTest>
#include <database.h>
#include <movie.h>
#include <user.h>
#include <rmse.h>
#include <quickdatabase.h>

class Test : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void init();
    void cleanup();

private slots:
    void dataBase();
    void dabaBaseBadPath();

    void movie_data();
    void movie();

    void noDataBaseMovie();

    void rmse_data();
    void rmse();

    void user_data();
    void user();

    void userNext_data();
    void userNext();

    void quickdatabase();
};

void Test::init()
{}

void Test::cleanup()
{}

void Test::dataBase()
{
    DataBase db;
    QCOMPARE(db.rootPath(), QLatin1String("../../"));
    QCOMPARE(db.isLoaded(), false);
    QVERIFY(db.totalVotes() == 0);
    QCOMPARE(db.load(), true);
    QCOMPARE(db.isLoaded(), true);
    QVERIFY(db.totalMovies() > 0);
    QVERIFY(db.totalVotes() > 0);
}

void Test::dabaBaseBadPath()
{
    DataBase db(QLatin1String("X"));
    QCOMPARE(db.rootPath(), QLatin1String("X"));
    QCOMPARE(db.isLoaded(), false);
    QVERIFY(db.totalVotes() == 0);
    QCOMPARE(db.load(), false);
    QCOMPARE(db.isLoaded(), false);
    QVERIFY(db.totalVotes() == 0);
}

void Test::movie_data()
{
    QTest::addColumn<int>("id");
    QTest::addColumn<int>("votes");

    QTest::addColumn<int>("user");
    QTest::addColumn<int>("score");
    // Check what should be the first vote in the file and basic validation
    QTest::newRow("movie 1, first row")
    << 1 << 547
    << 1488844 << 3;

    QTest::newRow("movie 1, first row sorted")
    << 1 << 547
    << 915 << 5;

    QTest::newRow("movie 1, second row")
    << 1 << 547
    << 822109 << 5;

    QTest::newRow("movie 1, last row in movie file")
    << 1 << 547
    << 1815755 << 5;

    QTest::newRow("movie 1, last row sorted")
    << 1 << 547
    << 2647871 << 4;

    QTest::newRow("movie 2, first row")
    << 2 << 145
    << 2059652 << 4;

    QTest::newRow("movie 2, last row")
    << 2 << 145
    << 1272122 << 5;

    // findVote if not returning a offseted value will be beyond the size of the file
    QTest::newRow("movie 10001, user 27822")
    << 10001 << 158
    << 27822 << 5;

    // Last row isn't written correctly to the file
    QTest::newRow("movie 17769, first row")
    << 17769 << 6749
    << 1844276 << 1;

    QTest::newRow("movie 17770, first row")
    << 17770 << 921
    << 2031561 << 2;

    QTest::newRow("movie 17770, last row")
    << 17770 << 921
    << 453585 << 2;

    QTest::newRow("movie 17770, last row sorted")
    << 17770 << 921
    << 2647066 << 2;

    QTest::newRow("movie 2843, first row sorted")
    << 2843 << 1455
    << 6 << 1;
}

void Test::movie()
{
    DataBase db;
    db.load();
    Movie m(&db);
    QFETCH(int, id);
    QFETCH(int, votes);
    QFETCH(int, user);
    QFETCH(int, score);
    m.setId(id);
    QCOMPARE(m.id(), (uint)id);
    QCOMPARE(m.votes(), (uint)votes);
    int voteId = m.findVote(user);
    QVERIFY(voteId != -1);
    QCOMPARE(m.user(voteId), user);
    QCOMPARE(m.score(voteId), score);

}

void Test::noDataBaseMovie()
{
    DataBase db("");
    Movie m(&db);
    QVERIFY(m.votes() == 0);
}

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<double>)

void Test::rmse_data()
{
    QTest::addColumn<QList<int> > ("actual");
    QTest::addColumn<QList<double> > ("guess");
    QTest::addColumn<double>("result");

    // The example given in the tarball, result generated from running the perl script
    QTest::newRow("Official")
    << (QList<int>()    << 2   << 3   << 4)
    << (QList<double>() << 3.2 << 3.1 << 5.0)
    << 0.903696;

    // 0 check just to make sure
    QTest::newRow("1 guess, 0")
    << (QList<int>()    << 0   << 0)
    << (QList<double>() << 0.0 << 0.0)
    << 0.0;
}

void Test::rmse()
{
    RMSE rmse;
    QFETCH(QList<int>, actual);
    QFETCH(QList<double>, guess);
    QFETCH(double, result);
    QCOMPARE(guess.count(), actual.count());
    for (int i = 0; i < guess.count(); ++i) {
        rmse.addPoint(actual[i], guess[i]);
    }
    QCOMPARE(rmse.count(), guess.count());
    QCOMPARE(QString("%1").arg(rmse.result()), QString("%1").arg(result));
}

void Test::user_data()
{
    QTest::addColumn<int>("user");
    QTest::addColumn<int>("id");

    QTest::newRow("invalid") << 1 << -1;
    QTest::newRow("first valid") << 6 << 6;
    QTest::newRow("second valid") << 7 << 7;
    QTest::newRow("middle one") << 822109 << 822109;
    QTest::newRow("last user") << 2649429 << 2649429;
}

void Test::user()
{
    DataBase db;
    QVERIFY(db.load());
    QFETCH(int, user);
    QFETCH(int, id);

    QList<int> movies;
    QList<int> votes;
    Movie m_movie(&db);
    for (int i = 0; i < db.totalMovies(); ++i) {
        m_movie.setId(i + 1);
        int x = m_movie.findVote(user);
        if (x >= 0) {
            movies.append(i + 1);
            votes.append(m_movie.score(x));
        }
    }
    User userObject(&db, user);
    QCOMPARE(userObject.id(), id);
    QCOMPARE(userObject.votes(), votes.count());
    userObject.setId(id);
    QCOMPARE(userObject.votes(), votes.count());

    for (int i = 0; i < votes.count(); ++i) {
        int movie = userObject.movie(i);
        QVERIFY((bool)(movies.contains(movie)));
        int score = userObject.score(i);
        QCOMPARE(votes[movies.indexOf(movie)], score);
    }
}

void Test::userNext_data()
{
    QTest::addColumn<int>("start");
    QTest::addColumn<int>("next");

    QTest::newRow("default user") << 0 << 7;
    QTest::newRow("first user")   << 6 << 7;
    QTest::newRow("first skip")   << 8 << 10;
    QTest::newRow("middle user")  << 508 << 515;
}

void Test::userNext()
{
    QFETCH(int, start);
    QFETCH(int, next);

    DataBase db;
    QVERIFY(db.load());
    User user(&db, start);
    user.next();
    QCOMPARE(next, user.id());
}

void Test::quickdatabase()
{
    DataBase db;
    QVERIFY(db.load());
    QuickDatabase iv(&db);

    // Quick check a int overflow case
    iv.has(479906, 252);

    User user(&db, 6);
    for (int i = 0; i < db.totalUsers(); i += db.totalUsers() / 10) {
        int userNumber = db.mapUser(user.id());
        for (int j = 0; j < db.totalMovies(); ++j) {
            int m = user.seenMovie(j);
            QVERIFY(iv.has(userNumber, j) == (m != -1));
        }
        user.next();
    }
}

QTEST_MAIN(Test)
#include "autotests.moc"

