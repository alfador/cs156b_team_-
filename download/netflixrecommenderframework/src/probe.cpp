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

#include "probe.h"
#include "user.h"
#include <qvector.h>
#include <qstringlist.h>
#include <qpair.h>
#include <qhash.h>
#include <qdebug.h>
#include <qfileinfo.h>

#ifdef Q_OS_WIN
#include <winmmap.h>
#else
#include <sys/mman.h>
#endif

#define MAGICID 344
#define USER_PROBE_VERSION 1
// For testing on smaller data in ordering.  Inclusive.
const int MOVIE_LIMIT = 17770;

Probe::Probe(DataBase *db) : db(db), output(ProbeFile)
{};

void Probe::setOutput(Output type)
{
    output = type;
}

bool Probe::readProbeData(const QString &probeFileName)
{
    QVector<uint> probeData;
    QFile data(probeFileName);
    if (!data.open(QFile::ReadOnly)) {
        qWarning() << "Error: Unable to open probe file." << probeFileName;
        return false;
    }

    uint movie = 0;
    QTextStream in(&data);
    QString line;
    int count = 0;
    while (!in.atEnd()) {
        line = in.readLine();
        if (line.right(1) == ":") {
            movie = line.mid(0, line.length() - 1).toInt();
            if (movie <= 0) {
                qWarning() << "Error: Found movie with id 0 in probe file.";
                return false;
            }
            probeData.append(0);
            probeData.append(movie);
        } else {
            int realValue = 0;
            int user = 0;
            if (line[1] == ',' && QString(line[0]).toInt() < 6 ) {
                // modified probe
                realValue = QString(line[0]).toInt();
                user = line.mid(2).toInt();
            } else {
                if (line.contains(",")) {
                    // qualifing
                    realValue = 0;
                    user = line.mid(0, line.indexOf(",")).toInt();
                } else {
                    // default probe
                    user = line.toInt();
                    Movie m(db, movie);
                    realValue = m.findScore(user);
                }
            }
            if (user <= 0) {
                qWarning() << "Error: Found user with id 0 in probe file." << line << line.mid(0, line.indexOf(","));
                return false;
            }
            probeData.append(user);
            probeData.append(realValue);
            ++count;
        }
    }

    probeData.insert(0, count);
    probeData.insert(0, MAGICID);

    QFileInfo info(probeFileName);
    QString binaryFileName = info.path() + "/" + info.completeBaseName() + QLatin1String(".data");
    DataBase::saveDatabase(probeData, binaryFileName);
    qDebug() << "probe data saved to a database";
    return true;
}

bool Probe :: readProbeDataByUser(const QString &probeFileName)
{
    qDebug() << "Constructing probe data grouped by users";
    QFile data(probeFileName);
    if (!data.open(QFile::ReadOnly)) {
        qWarning() << "Error: Unable to open probe file." << probeFileName;
        return false;
    }

    // Create a map mapping users to the movie, ratings pair vector
    qDebug() << "Sorting probe by user";
    QMultiHash<unsigned int, 
               QPair <unsigned short, unsigned char> > userMap;

    uint movie = 0;
    QTextStream in(&data);
    QString line;
    while (!in.atEnd()) {
        line = in.readLine();
        if (line.right(1) == ":") {
            movie = line.mid(0, line.length() - 1).toInt();
            if (movie <= 0) {
                qWarning() << "Error: Found movie with id 0 in probe file.";
                return false;
            }
        } else {
            int realValue = 0;
            int user = 0;
            if (line[1] == ',' && QString(line[0]).toInt() < 6 ) {
                // modified probe
                realValue = QString(line[0]).toInt();
                user = line.mid(2).toInt();
            } else {
                if (line.contains(",")) {
                    qWarning() << "Error: Qualifying not supported";
                    return false;
                } else {
                    // default probe
                    user = line.toInt();
                    Movie m(db, movie);
                    realValue = m.findScore(user);
                }
            }
            if (user <= 0) {
                qWarning() << "Error: Found user with id 0 in probe file."   
                           << line << line.mid(0, line.indexOf(","));
                return false;
            }
            userMap.insert(user, QPair<unsigned short, unsigned char>
                                       (movie, realValue)); 
        }
    }

    
    // Insert the data into the compiled vector grouped by user
    qDebug() << "Compiling Probe data";
    QList<unsigned int> users = userMap.keys();
    QVector<uint> probeData;
    unsigned int prevUser = -1;
    unsigned int numUsers = 0;
    for (int i = 0; i < users.size(); ++i)
    {   
        // Avoid putting in the same user, the keys list will
        // have it multiple times in order.
        if (users[i] == prevUser)
            continue;

        numUsers++;
        prevUser = users[i];

        // Start with the id and then the number of movies
        probeData.append(users[i]);

        QList<QPair<unsigned short, unsigned char> > ratings 
                = userMap.values(users[i]);

        probeData.append(ratings.size());

        // Now add each movie with its rating
        for (int j = 0; j < ratings.size(); ++j)
        {
            probeData.append(ratings[j].first);
            probeData.append(ratings[j].second);
        }

    }    

    // Insert some header values. The order in the file is
    // MAGICID, Version, user count
    probeData.insert(0, numUsers);
    probeData.insert(0, USER_PROBE_VERSION);
    probeData.insert(0, MAGICID);

    QFileInfo info(probeFileName);
    QString binaryFileName = info.path() + "/"
                           + info.completeBaseName()
                           + QLatin1String("byuser.data");
    DataBase::saveDatabase(probeData, binaryFileName);
    qDebug() << "probe data saved to a database";
    return true;
}

int Probe::runProbe(Algorithm *algorithm, const QString &probeFileName)
{
    if (probeFileName.isEmpty() || !db->isLoaded())
        return -1;
    
    if (probeFileName == "qualifying")
        output = SubmitionFile;

    QString fileName = db->rootPath() + "/" + probeFileName;
    if (!QFile::exists(fileName + ".data"))
        if (!readProbeData(fileName + ".txt"))
            return -1;

    uint *probe;
    uint probeSize = 0;

    QFile file(fileName + ".data");
    if (file.size() != 0
            && file.exists()
            && file.open(QFile::ReadOnly | QFile::Unbuffered)) {
        probe = (uint*)
                mmap(0, file.size(), PROT_READ, MAP_SHARED,
                     file.handle(),
                     (off_t)0);
        if (probe == (uint*) - 1) {
            qWarning() << "probe mmap failed";
            return -1;
        }
        probeSize = file.size() / sizeof(uint);
    } else {
        qWarning() << "unable to load probe database";
        return -1;
    }

    RMSE rmse;
    if (probe[0] != MAGICID) {
        qWarning() << "probe file is invalid, please remove" << "\"" + fileName + ".data\"" << "so a new one can be generated";
        return -1;
    }
    int total = probe[1];
    int percentDone = 0;
    int currentMovie = -1;

    // Only print updates every once in a while to avoid having console
    // output be a bottleneck.
    int PROGRESS_INTERVAL = 1; // time in seconds to wait.
    clock_t referTime = clock();
 
    for (uint i = 2; i < probeSize; ++i) {
        if (probe[i] == 0) {
            currentMovie = -1;
            continue;
        }
        if (currentMovie == -1) {
            currentMovie = probe[i];
            algorithm->setMovie(currentMovie);
            if (output == SubmitionFile)
                printf("%d:\n", currentMovie);
            continue;
        }
        int user = probe[i++];
        int realValue = probe[i];
        double guess = algorithm->determine(user);
        
        if (output == SubmitionFile) {
            printf("%f\n", guess);
        } else {
            rmse.addPoint(realValue, guess);
            int t = rmse.count() / (total / 100);
            if (t != percentDone ||
               (clock() - referTime) / CLOCKS_PER_SEC > PROGRESS_INTERVAL) {

                percentDone = t;
                referTime = clock();
                qDebug() << i << "movie:" << currentMovie 
                              << "user:" << user
                              << "done:" << percentDone << "%"; 
            }
        }
    }

    if (output != SubmitionFile)
        qDebug() << rmse.result();
    return 0;
}

int Probe :: runProbeOrdering(OrderingAlgorithm * algorithm,
                              const QString &probeFileName)
{
    if (probeFileName.isEmpty() || !db->isLoaded())
        return -1;
    
    if (probeFileName == "qualifying")
    {
        qWarning() << "qualifying not supported";
        return -1;
    }

    QString fileName = db->rootPath() + "/" + probeFileName;
    if (!QFile::exists(fileName + "byuser.data"))
        if (!readProbeDataByUser(fileName + ".txt"))
            return -1;

    uint *probe;
    uint probeSize = 0;

    QFile file(fileName + "byuser.data");
    if (file.size() != 0
            && file.exists()
            && file.open(QFile::ReadOnly | QFile::Unbuffered)) {
        probe = (uint*)
                mmap(0, file.size(), PROT_READ, MAP_SHARED,
                     file.handle(),
                     (off_t)0);
        if (probe == (uint*) -1) {
            qWarning() << "probe mmap failed";
            return -1;
        }
        probeSize = file.size() / sizeof(uint);
    } else {
        qWarning() << "unable to load probe database";
        return -1;
    }

    if (probe[0] != MAGICID) {
        qWarning() << "probe file is invalid, please remove" << "\"" + fileName + ".data\"" << "so a new one can be generated";
        return -1;
    }

    if (probe[1] != USER_PROBE_VERSION) {
        qWarning() << "Incompatible file format detected. Please remove "
                   + fileName + "byuser.data so a new one can be generated";

    }

    int numUsers = probe[2];
    int counted_users = numUsers; // Users with any comparisons that count

    int probeI = 3;

    // Intervals to update progress in seconds
    int PROGRESS_INTERVAL = 1;
    clock_t referTime = clock();
 
    // Go through each user
    User currU(db);
    double errorSum = 0.0;
   
    qDebug() << "Running Error Calculation on probe";     
    for (int userI = 0; userI < numUsers; ++userI) 
    {
        int userId = probe[probeI++];
        int numRatings = probe[probeI++];

        currU.setId(userId);
        algorithm -> setUser(userId);

        int errors = 0;
        int numTests = 0;
    
        // Compare all the test set ratings against the training set.
        for (int i = 0; i < currU.votes(); i++)
        {
            for (int j = 0; j < numRatings; j++)
            {
                int movie1 = currU.movie(i);
                int rating1 = currU.score(i);
    
                int movie2 = probe[probeI + j * 2];
                int rating2 = probe[probeI + j * 2 + 1];

                // avoid testing if the two movies are actually rated the same
                if (rating1 == rating2 || movie1 > MOVIE_LIMIT
                                       || movie2 > MOVIE_LIMIT)
                    continue;

                // test the predicted order. This value is negative if there
                // is an error
                int testVal = (rating2 - rating1) *
                               algorithm->order(movie1, movie2);
                
                if (testVal < 0)
                    ++errors;

                ++ numTests;
            }
        }

        // Test the pairs within the test set itself
        for (int i = 0; i < numRatings; i++)
        {
            for (int j = i + 1; j < numRatings; j++)
            {
                int movie1 = probe[probeI + i * 2];
                int rating1 = probe[probeI + i * 2 + 1];
    
                int movie2 = probe[probeI + j * 2];
                int rating2 = probe[probeI + j * 2 + 1];

                // avoid testing if the two movies are actually rated the same
                if (rating1 == rating2 || movie1 > MOVIE_LIMIT
                                       || movie2 > MOVIE_LIMIT)
                    continue;

                // test the predicted order. This value is negative if there
                // is an error
                int testVal = (rating2 - rating1) *
                               algorithm->order(movie1, movie2);
                
                if (testVal < 0)
                    ++errors;

                ++ numTests;
            }
        }

        probeI += numRatings * 2;
        double errorFrac = 0.0;
        if (numTests > 0)
            errorFrac = ((double) errors) / numTests;
        else
            --counted_users;

         
        errorSum += errorFrac;

        if ((clock() - referTime) / CLOCKS_PER_SEC >= PROGRESS_INTERVAL)
        {
            referTime = clock();
            qDebug() << userI + 1 << "of" << numUsers << "users done";
        }
    }

    double avgError = errorSum / counted_users;
    qDebug() << "Counted users: " << counted_users;
    qDebug() << "Error calculation completed. Error is:" << avgError;

    return 0;
}

void Probe :: runQualifyingOrdering(OrderingAlgorithm * algorithm,
                                    const QString& filename)
{
    int numUsers = db->totalUsers();

    QFile qual(filename);
    qual.open(QFile :: ReadOnly | QFile :: Text);

    QVector<QVector<QPair<int, float> > > qualRatings;
    qualRatings.resize(numUsers);
 
    QTextStream in(&qual);
    QString line = in.readLine();
    int currUserId = -1;

    // Read in the ratings from the qualifying file given
    while (!line.isNull())
    {
        if (line.endsWith(":"))
        {
            // New User line`
            line.remove(":");
            currUserId = line.toInt();
        }    
        else
        {
            // Movie line
            QStringList parts = line.split(","); 
            int movieId = parts[0].toInt();
            float rating  = parts[1].toFloat();

            qualRatings[db->mapUser(currUserId)].append(
                QPair<int, float>(movieId, rating));    
        }

        line = in.readLine();
    }    

    // Intervals to update progress in seconds
    int PROGRESS_INTERVAL = 1;
    clock_t referTime = clock();
 
    // Go through each user
    User currU(db);
    currU.setId(6);
    double errorSum = 0.0;
    
    int counted_users = numUsers;
 
    qDebug() << "Running Error Calculation";     
    for (int userI = 0; userI < numUsers; ++userI)
    {
        int userId = currU.id();
        int numRatings = qualRatings[userI].size();

        algorithm -> setUser(userId);

        int errors = 0;
        int numTests = 0;
    
        // Compare all the test set ratings against the training set.
        for (int i = 0; i < currU.votes(); i++)
        {
            for (int j = 0; j < numRatings; j++)
            {
                int movie1 = currU.movie(i);
                int rating1 = currU.score(i);
    
                int movie2 = qualRatings[userI][j].first;
                int rating2 = qualRatings[userI][j].second;

                // avoid testing if the two movies are actually rated the same
                if (rating1 == rating2 || movie1 > MOVIE_LIMIT
                                       || movie2 > MOVIE_LIMIT)
                    continue;

                // test the predicted order. This value is negative if there
                // is an error
                int testVal = (rating2 - rating1) *
                               algorithm->order(movie1, movie2);
                
                if (testVal < 0)
                    ++errors;

                ++ numTests;
            }
        }

        // Test the pairs within the test set itself
        for (int i = 0; i < numRatings; i++)
        {
            for (int j = i + 1; j < numRatings; j++)
            {
                int movie1 = qualRatings[userI][i].first;
                int rating1 = qualRatings[userI][i].second;

                int movie2 = qualRatings[userI][j].first;
                int rating2 = qualRatings[userI][j].second;

               // avoid testing if the two movies are actually rated the same
                if (rating1 == rating2 || movie1 > MOVIE_LIMIT
                                       || movie2 > MOVIE_LIMIT)
                    continue;

                // test the predicted order. This value is negative if there
                // is an error
                int testVal = (rating2 - rating1) *
                               algorithm->order(movie1, movie2);
                
                if (testVal < 0)
                    ++errors;

                ++ numTests;
            }
        }

        double errorFrac = 0.0;
        if (numTests > 0)
            errorFrac = ((double) errors) / numTests;
        else
            --counted_users;
    
        currU.next();
     
        errorSum += errorFrac;

        if ((clock() - referTime) / CLOCKS_PER_SEC >= PROGRESS_INTERVAL)
        {
            referTime = clock();
            qDebug() << userI + 1 << "of" << numUsers << "users done";
        }
    }

    double avgError = errorSum / counted_users;
    qDebug() << "Counted users: " << counted_users;
    qDebug() << "Error calculation completed. Error is:" << avgError;


     
}
     
