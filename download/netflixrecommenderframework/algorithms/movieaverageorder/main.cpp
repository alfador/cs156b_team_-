#include <probe.h>
#include <cstdlib>
#include <iostream>

using std :: cout;
using std :: endl;

class MovieAverageOrder : public OrderingAlgorithm, public Algorithm
{
public:
    MovieAverageOrder(DataBase *db) : OrderingAlgorithm(), Algorithm()
    {
        cout << "Training Averages" << endl;
        // Train all the movie averages, all 17770 should
        // easily fit in memory
        unsigned int total = db->totalMovies();
        avgs = new float[total + 1];

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

    void setMovie(int id)
    {
        currentMovieId = id;
    }

    double determine(int userId)
    {
        Q_UNUSED(userId);
        return avgs[currentMovieId];
    } 


private:
    float * avgs;

    unsigned int currentMovieId;
};

int main(int numArgs, char ** args)
{
    bool doOrder = true;
    if (numArgs == 2)
    {
        if (!strcmp(args[1], "order"))
            doOrder = true;
        else if (!strcmp(args[1], "rmse"))
            doOrder = false;
        else
        {
            cout << "Argument " << args[1] << " not recognized" << endl;
            cout << "Defaulting to ordering error" << endl;
            doOrder = true;
        } 
    }
    DataBase db;
    db.load();
    Probe probe(&db);
    MovieAverageOrder movieAvg(&db);
    
    if (doOrder)
        probe.runProbeOrdering(&movieAvg, "probe");
    else
        probe.runProbe(&movieAvg, "probe");
}

