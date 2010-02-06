#include <probe.h>
#include <cstdlib>
#include "fMatrix.h"
#include <iostream>

using std :: cout;
using std :: endl;


class TopNGraphOrder : public OrderingAlgorithm
{
public:
    TopNGraphOrder(DataBase *db) : OrderingAlgorithm()
    {
        cout << "Training Averages" << endl;
        // Train all the movie averages, all 17770 should
        // easily fit in memory
        unsigned int total = db->totalMovies();
        avgs = new float[total];

        fMatrix probs(total + 1, total + 1);

        Movie m(db);
        
        for (unsigned int i = 1; i <= total; ++i)
        {
            m.setId(i);
            
            unsigned int numRatings = m.votes();
            double totalRating = 0.0;
            for (unsigned int j = 0; j < numRatings; ++j)
            {
                totalRating += m.score(j);
            }

            avgs[i] = totalRating / numRatings;
        } 
    }

    ~MovieAverageOrder()
    {
        delete [] avgs;
    }

    void setUser(int id)
    {
        Q_UNUSED(id);
    }

    int order(int movie1, int movie2)
    {
        if (avgs[movie1] <= avgs[movie2])
            return 1;
        else
            return -1;    
    }

private:
    
    float * avgs;
};

int main(int , char **)
{
    DataBase db;
    db.load();
    Probe probe(&db);
    MovieAverageOrder movieAvg(&db);
    probe.runProbeOrdering(&movieAvg, "probe");
}

