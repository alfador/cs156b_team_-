#include <probe.h>
#include <cstdlib>
#include <iostream>
#include <assert.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qdebug.h>
#include <qstringlist.h>
#include <user.h>


using std :: cout;
using std :: endl;


class QuizFileOrder : public OrderingAlgorithm
{
public:
    QuizFileOrder(DataBase *db, QString filename) : OrderingAlgorithm(),
                                                    user(db)
    {
        // Open the quiz file
        QFile quizFile(filename);
        if (!quizFile.open(QIODevice::ReadOnly | QIODevice::Text))
            assert(false);

        // Read the file
        int active_uid = 0;
        QTextStream in(&quizFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            // Check if we have a new user
            if (line.endsWith(":")) {
                line.remove(":");
                active_uid = line.toInt();
                continue;
            }
            // Otherwise, have a movie, rating pair
            QStringList sList = line.split(",");
            int movieID = sList[0].toInt();
            float rating = sList[1].toFloat();
            // Key is (user, movie)
            QPair<int, int> rPair(active_uid, movieID);
            // Insert into hash table
            this->ratings.insert(rPair, rating);
        }

        // Close file
        quizFile.close();
    }

    ~QuizFileOrder()
    {
    }

    void setUser(int id)
    {
        // Set ids
        this->current_uid = id;
        this->user.setId(id);
    }


    // Return 1 if movie1 worse than movie2, -1 otherwise.
    int order(int movie1, int movie2)
    {
        float score1;
        float score2;
        // Check if the first rating is in train.
        score1 = (float) this->user.seenMovie(movie1);
        if (score1 == (float) -1) {
            // Need to get score from quiz
            QPair<int, int> rPair(this->current_uid, movie1);
            score1 = this->ratings.value(rPair);
        }

        // Check if the second rating is in train.
        score2 = (float) this->user.seenMovie(movie2);
        if (score2 == (float) -1) {
            // Need to get score from quiz
            QPair<int, int> rPair(this->current_uid, movie2);
            score2 = this->ratings.value(rPair);
        }

        // Return 1 if the rating for movie1 is worse than movie2's rating
        if (score1 < score2)
            return 1;
        return -1;
    }

private:
    // Store in ((user, movie), rating) format
    QHash<QPair<int, int>, float> ratings;
    int current_uid;
    User user;
};

int main(int numArgs, char ** args)
{
    DataBase db;
    db.load();
    Probe probe(&db);

    // Filename of input quiz file
    QString quizFileName(args[1]);

    QuizFileOrder qfOrd(&db, quizFileName);
    
}

