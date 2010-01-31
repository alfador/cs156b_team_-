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
#include <qvector.h>
#include <qdebug.h>
#include <qfileinfo.h>

#ifdef Q_OS_WIN
#include <winmmap.h>
#else
#include <sys/mman.h>
#endif

#define MAGICID 344

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
        if (i >= 300)
            return 0;
        
        if (output == SubmitionFile) {
            printf("%f\n", guess);
        } else {
            if (guess != realValue)
                qDebug() << i << "movie:" << currentMovie << "user:" << user << "guess:" << guess << "correct:" << realValue << rmse.result();;
            rmse.addPoint(realValue, guess);
            int t = rmse.count() / (total / 100);
            if (t != percentDone) {
                percentDone = t;
                qDebug() << rmse.count() << percentDone << "%" << rmse.result();
            }
        }
    }

    if (output != SubmitionFile)
        qDebug() << rmse.result();
    return 0;
}

