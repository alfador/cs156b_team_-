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

#include "database.h"
#include "movie.h"
#include "user.h"

#include <qfile.h>
#include <qdir.h>
#include <qdebug.h>

#ifdef Q_OS_WIN
#include <winmmap.h>
#else
#include <sys/mman.h>
#endif

#define USERFILENAME  "users"
#define MOVIESFILENAME "movies"

DataBase::DataBase(const QString &rootPath) :
        storedmovies(0),
        storedvotes(0),
        m_totalVotes(0),
        storedUsers(0),
        storedUsersIndex(0),
        m_totalUsers(0),
        moviesFile(0),
        votesFile(0),
        userFile(0),
        userIndexFile(0),
        m_rootPath(rootPath)
{
    if (m_rootPath.isEmpty()) {
        QDir dir = QDir::current();
        while (!dir.isRoot()) {
            if (dir.entryList(QStringList(), QDir::AllDirs).contains("training_set")) {
                m_rootPath = QDir::current().relativeFilePath(dir.path());
                break;
            }
            dir.cdUp();
        }
    }
}

DataBase::~DataBase()
{
    if (moviesFile != 0) {
        munmap(storedmovies, moviesFile->size());
        delete moviesFile;
    }
    if (votesFile != 0) {
        munmap(storedvotes, votesFile->size());
        delete votesFile;
    }
    if (userFile != 0) {
        munmap(storedUsers, userFile->size());
        delete userFile;
    }
    if (userIndexFile != 0) {
        munmap(storedUsersIndex, userIndexFile->size());
        delete userIndexFile;
    }
}

QString DataBase::rootPath() const
{
    return m_rootPath;
}

void DataBase::saveDatabase(const QVector<uint> &vector, const QString &fileName)
{
    QFile database(fileName);
    if (database.open(QFile::WriteOnly)) {
        for (int i = 0; i < vector.count(); ++i)
            database.write((char*)(&vector.at(i)), sizeof(uint));
    } else {
        qWarning() << "Warning: Unable to save database:" << fileName;
    }
}

bool userlessthan(const int &s1, const int &s2)
{
    return DataBase::guser(s1) < DataBase::guser(s2);
}

void DataBase::generateMovieDatabase()
{
    qDebug() << "Generating movie database..";
    QVector<uint> votesIndex;
    QVector<uint> votes;
    votesIndex.resize(17770); // hardcoded so totalMovies() doesn't need a branch
    qFill(votesIndex.begin(), votesIndex.end(), 0);
    int percentDone = 0;
    uint c = 0;
    votesIndex[0] = 0;
    for (int i = 1; i < votesIndex.size() + 1; ++i) {
        QString fileName = QString(rootPath() + "/training_set/mv_%1.txt").arg(i, 7, 10, QChar('0'));
        QFile data(fileName);
        QVector<int> currentVotes;
        if (data.open(QFile::ReadOnly)) {
            QTextStream in(&data);
            while (!in.atEnd()) {
                QString line = in.readLine();
                int x = line.lastIndexOf(",") - 1;
                //QString date = line.right(line.length() - x - 1);
                uint vote = line.mid(x, 1).toInt();
                uint user = line.mid(0, line.lastIndexOf(",") - 2).toInt();
                if (x != -2 && user <= 0) {
                    qWarning() << "warning: found a user with id 0";
                    continue;
                }

                if (x != -2) {
                    currentVotes.append((vote << 29) | user);
                    ++c;
                }
            }
        } else {
            qWarning() << fileName << "doesn't exist!";
            return;
        }
        qSort(currentVotes.begin(), currentVotes.end(), userlessthan);
        for (int j = 0; j < currentVotes.size(); ++j)
            votes.append(currentVotes.at(j));
        if (i != 17770) {
            votesIndex[i] = c;
        } else {
            //qDebug() << i << c << votes.count();
        }
        int t = i / (17770 / 100);
        if (t != percentDone && t % 5 == 0) {
            percentDone = t;
            qDebug() << fileName << percentDone << "%" << i << c;
        }
    }
    qDebug() << "Generated movie database.  Saving...";
    saveDatabase(votes, rootPath() + "/" + MOVIESFILENAME + ".data");
    saveDatabase(votesIndex, rootPath() + "/" + MOVIESFILENAME + ".index");
    votesIndex.clear();
    votes.clear();
    qDebug() << "Database cache files have been created.";
}

void DataBase::generateUserDatabase()
{
    if (!isLoaded())
        return;
    qDebug() << "Generating user database...";
    Movie movie(this);
    QMap<int, QVector<uint> > users;
    for (int i = 1; i <= totalMovies(); ++i) {
        movie.setId(i);
        for (uint j = 0; j < movie.votes(); ++j) {
            uint user = movie.user(j);
            uint score = movie.score(j);
            users[user].append((score << 29) | i);
        }

        // some debug stuff;
        int t = i / (totalMovies() / 100);
        if (i % (totalMovies() / 100) == 0 && t % 5 == 0)
            qDebug() << t << "%";
    }
    qDebug() << "finshed initial sorting, sorting and saving...";

    QMapIterator<int, QVector<uint> > i(users);
    QVector<uint> userIndex;
    QVector<uint> userConverter;

    QFile database(rootPath() + "/" + USERFILENAME + ".data");
    if (!database.open(QFile::WriteOnly)) {
        qWarning() << "Warning: Unable to save user database";
        return;
    }

    int c = 0;
    userConverter.append(c);

    while (i.hasNext()) {
        i.next();
        // sort and append each user's votes
        int user = i.key();
        QVector<uint> userVotes = i.value();
        users.remove(i.key());
        qSort(userVotes.begin(), userVotes.end(), userlessthan);

        userIndex.append(user);
        for (int j = 0; j < userVotes.count(); ++j) {
            uint next = userVotes[j];
            database.write((char*)(&next), sizeof(uint));
            ++c;
        }
        userConverter.append(c);
    }
    for (int i = 0; i < userConverter.count(); ++i)
        userIndex.append(userConverter[i]);
    saveDatabase(userIndex, rootPath() + "/" + USERFILENAME + ".index");
    qDebug() << "Finished saving user database.";
}

bool load(QFile *file, uint **pointer)
{
    bool error = false;
    if (file->size() != 0
            && file->exists()
            && file->open(QFile::ReadOnly | QFile::Unbuffered)) {
        *pointer = (uint*)
                   mmap(0, file->size(), PROT_READ, MAP_SHARED,
                        file->handle(),
                        (off_t)0);
        if (*pointer == (uint*) - 1) {
            qWarning() << "mmap failed" << file->fileName();
            error = true;
        }
    } else {
        qWarning() << "unable to load database" << file->fileName();
        error = true;
    }
    return error;
}

bool DataBase::load()
{
    QString movieFileName = rootPath() + "/" + MOVIESFILENAME + ".index";
    if (!QFile::exists(movieFileName))
        generateMovieDatabase();

    moviesFile = new QFile(movieFileName);
    bool moviesFileError = ::load(moviesFile, &storedmovies);
    // Basic sanity check
    if (!moviesFileError && (storedmovies[0] != 0 || (storedmovies[1] != 547 && storedmovies[1] != 524))) {
        qWarning() << "Movie database error, possibly corrupt.  Expected [0] to be 0, but it is:"
        << storedmovies[0] << "or expected [1] to be 547/524, but it is:" << storedmovies[1];
        munmap(storedmovies, moviesFile->size());
        moviesFileError = true;
    }
    if (moviesFileError) {
        delete moviesFile;
        moviesFile = 0;
        return false;
    }

    votesFile = new QFile(rootPath() + "/" + MOVIESFILENAME + ".data");
    bool votesFileError = ::load(votesFile, &storedvotes);
    m_totalVotes  = votesFile->size() / 4;
    // Basic sanity check
    Movie m(this, 1);
    if (!votesFileError && (m.votes() != 547 && m.votes() != 524)) {
        qWarning() << "votes database error, needs updating or possibly corrupt.  Expect movie" << m.id() << "to have 547/524 votes, but it only has:" << m.votes();
        munmap(storedvotes, votesFile->size());
        votesFileError = true;
    }
    if (!votesFileError && m.findScore(1488844) != 3) {
        qWarning() << "votes database error, needs updating or possibly corrupt.  Expect movie" << m.id() << "to have rank of 3:" << m.findScore(1488844) << "for user 1488844.";
        munmap(storedvotes, votesFile->size());
        votesFileError = true;
    }
    if (votesFileError) {
        delete votesFile;
        votesFile = 0;
        return false;
    }



    QString userFileName = rootPath() + "/" + USERFILENAME + ".data";
    if (!QFile::exists(userFileName))
        generateUserDatabase();

    userFile = new QFile(userFileName);
    bool userFileError = ::load(userFile, &storedUsers);
    userIndexFile = new QFile(rootPath() + "/" + USERFILENAME + ".index");
    bool userIndexFileError = ::load(userIndexFile, &storedUsersIndex);
    if (userFileError || userIndexFileError) {
        delete userIndexFile;
        delete userFile;
        userIndexFile = 0;
        userFile = 0;
        return false;
    }
    m_totalUsers = (isLoaded()) ? userIndexFile->size() / 8 : 0;
    users.reserve(totalUsers());
    for (int i = 0; i < totalUsers(); ++i) {
        users.insert(storedUsersIndex[i], i);
    }
    // Basic sanity check that the database is ok and as we expect it to be
    User user(this, 6);
    if ((user.votes() != 626 && user.votes() != 623) || user.movie(0) != 30) {
        qWarning() << "Expected " << m.id() << " movie(0) to be user 30, but it is actually:" << user.movie(0);
        qWarning() << "OR user database error, possibly corrupt.  Expected " << user.id() << "to have 626 votes, but it only has:" << user.votes();
        qWarning() << "Delete users.index and users.data and restart this app to regenerate updated files.";
        munmap(storedUsers, userFile->size());
        munmap(storedUsersIndex, userIndexFile->size());
        delete userIndexFile;
        delete userFile;
        userIndexFile = 0;
        userFile = 0;
        users.clear();
        return false;
    }

    return true;
}

