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
#include <qtextstream.h>
#include <qdebug.h>

/**
    1) The probe.txt doesn't contain the ranking so a newprobe.txt is generated
    that contains the rank so it doesn't have to be calculated at run time every time.

    2) It is recommended that you remove the probe data from the training set so
    your algorithm wont become biased.

    GenerateBetterProbe does both #1 and #2, once done delete the old database files
    and replace the probe.txt file.
*/
class GenerateBetterProbe : public Algorithm
{
public:

    GenerateBetterProbe(DataBase *db, const QString &newProbeFileName) :
            Algorithm(), currentMovie(db)
    {
        newProbeFile = new QFile(newProbeFileName);
        if (!newProbeFile->open(QFile::WriteOnly)) {
            qWarning() << "unable to save" << newProbeFile->fileName();
        }
        stream = new QTextStream(newProbeFile);
    }

    ~GenerateBetterProbe()
    {
        delete stream;
        delete newProbeFile;
        scrubbMovie();
    }

    void setMovie(int id)
    {
        *stream << QString("%1:\n").arg(id);
        scrubbMovie();
        currentMovie.setId(id);
    }

    double determine(int user)
    {
        int realValue = currentMovie.findScore(user);
        *stream << QString("%1,%2\n").arg(realValue).arg(user);
        usersToRemove += user;
        return 0;
    }

    /*!
        Remove users listed in the probe from the movie file
    */
    void scrubbMovie()
    {
        QString fileName = QString(currentMovie.dataBase()->rootPath()
                                   + "/training_set/mv_%1").arg(currentMovie.id(), 7, 10, QChar('0'));
        QFile movieFile(fileName + ".txt");
        QFile newMovieFile(fileName + ".new");
        if (movieFile.open(QFile::ReadOnly) && newMovieFile.open(QFile::WriteOnly)) {
            QTextStream in(&movieFile);
            QTextStream out(&newMovieFile);
            while (!in.atEnd()) {
                QString line = in.readLine();
                uint user = line.mid(0, line.lastIndexOf(",") - 2).toInt();
                if (!usersToRemove.contains(user)) {
                    out << line << '\n';
                }
            }
            movieFile.close();
            newMovieFile.close();
            movieFile.remove();
            if (!newMovieFile.rename(fileName + ".txt")) {
                qWarning() << "unable to replace old movie file" << currentMovie.id();
            }
        } else {
            qWarning() << "unable to update movie" << currentMovie.id();
        }

        usersToRemove.clear();
    }

    Movie currentMovie;
    QFile *newProbeFile;
    QTextStream *stream;

    QVector<int> usersToRemove;
};

int main(int , char **)
{
    DataBase db;
    db.load();
    QString newProbeFileName = db.rootPath() + "/betterprobe.txt";
    GenerateBetterProbe gbp(&db, newProbeFileName);
    Probe probe(&db);
    probe.runProbe(&gbp);
    qDebug() << "A new probe file has been created:" << newProbeFileName;
    qDebug() << "It is recommended you replace probe.txt and remove the database files so the new one without the probe data can be generated.";
    return 0;
}

