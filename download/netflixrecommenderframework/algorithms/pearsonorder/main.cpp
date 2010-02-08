#include <probe.h>
#include <user.h>
#include <math.h>
#include <assert.h>

#include <qdebug.h>
#include <qvector.h>
#include <qfile.h>

#ifdef Q_OS_WIN
#include <winmmap.h>
#else
#include <sys/mman.h>
#endif

class PearsonOrder : public OrderingAlgorithm
{

public:
    PearsonOrder(DataBase *db) : currUser(db)
    {
        // Load the sorted correlation file 
        QString fileName = db->rootPath() + "correlation.dat";
        loadCorrelations(db, fileName);
    }
    
    ~PearsonOrder()
    {

    }

    int buildCorrelations(DataBase * db, QString filename)
    {
        // Construct the correlation coefficient matrix in memory
        // Hopefully this takes under 2GB of ram to hold. 
        unsigned int TOTAL_MOVIES = db->totalMovies();

        QVector<QVector <float> > pMat;
        pMat.resize(TOTAL_MOVIES);
        for (int i = 0; i < pMat.size(); i++)
        {
            pMat[i].reserve(i);
        }

        qDebug() << "Constructing Correlation Matrix";

        // Go through each movie
        Movie movie(db);
        User  user(db);

        // Keep rolling statistics to get correlation
        // at the end
        QVector<float> xSum(TOTAL_MOVIES, 0.0);
        QVector<float> ySum(TOTAL_MOVIES, 0.0);
        QVector<float> xySum(TOTAL_MOVIES, 0.0);
        QVector<float> x2Sum(TOTAL_MOVIES, 0.0);
        QVector<float> y2Sum(TOTAL_MOVIES, 0.0);
        QVector<int> numCommon(TOTAL_MOVIES, 0);

        for (unsigned int i = 0; i < TOTAL_MOVIES; i++)
        {
            qDebug() << i;

            // Reset all statistic counts
            xSum.fill(0.0);  
            ySum.fill(0.0);  
            xySum.fill(0.0);  
            x2Sum.fill(0.0);  
            y2Sum.fill(0.0);  
            numCommon.fill(0);

            movie.setId(i+1);
            int movieId1 = i + 1;

            // Go through each user that rated this movie
            for (unsigned int userI = 0; userI < movie.votes(); userI++)
            {
                // Now go through each movie this user has rated,
                // this gets all movies that has a common user with
                // the current movie
                user.setId(movie.user(userI));

                float rating1 = movie.score(userI);
 
                for (int movieI = 0; movieI < user.votes(); movieI++)
                {
                            
                    int movieId2 = user.movie(movieI);
                    float rating2 = user.score(movieI);

                    // Only care about movie ids larger than the current one
                    // as pearson correlations are symmetric, and we'll 
                    // calculate only lower to higher
                    if (movieId2 <= movieId1)
                        continue;
                   
                    // Update the statistic counts
                    int movie2index = movieId2 - 1;

                    xSum[movie2index] += rating1;
                    ySum[movie2index] += rating2;
                    xySum[movie2index] += rating1 * rating2;
                    x2Sum[movie2index] += rating1 * rating1;
                    y2Sum[movie2index] += rating2 * rating2;
                    numCommon[movie2index] ++;
                }
            }

            // All the information needed to calculate this movie's 
            // pearson coefficients with all other movies is available,
            // so now just get all the coefficients
            for (unsigned int j = i+1; j < TOTAL_MOVIES; j++)
            {
                float xMean = xSum[j] / numCommon[j];
                float yMean = ySum[j] / numCommon[j];
              
                float numer =  (xySum[j] - xSum[j] * yMean -
                                ySum[j] * xMean + xMean * yMean * numCommon[j]);

                float denom =  ( sqrt( (x2Sum[j] - 2 * xSum[j] * xMean  
                                          + xMean * xMean * numCommon[j]) *

                                       (y2Sum[j] - 2 * ySum[j] * yMean
                                          + yMean * yMean * numCommon[j])));

                float r = numer / denom;

                if (isnan(r))
                    r = 0.0;

                // Place the correlation in the jth row, ith column
                // forming a lower triangular matrix
                pMat[j].append(r);
            }
        } 

        // Save the correlation matrix to file
        qDebug() << "Saving to" << filename;
       
        QFile fout(filename); 
        if (! fout.open(QFile :: WriteOnly))
        {
            qDebug() << "Unable to open file";
            return -1;
        }

        fout.write((char*) &TOTAL_MOVIES, sizeof(unsigned int));

        for (int i = 0; i < pMat.size(); i++)
        {
            for (int j = 0; j < pMat[i].size(); j++)
            {
                fout.write((char *)(&pMat[i][j]), sizeof(float));
            }
        }        

        return 0;
    }

    int loadCorrelations(DataBase * db, QString fileName)
    {
        // Check if the correlation file is present, otherwise
        // construct it
        if (!QFile::exists(fileName))
            if (buildCorrelations(db, fileName) == -1)
            {
                qDebug() << "Unable to build correlations";
                return -1;
            }

        // Map the correlation matrix into memory
        qDebug() << "Mapping Correlation matrix";
        QFile fin(fileName);
        fin.open(QFile :: ReadOnly | QFile :: Unbuffered);

        pMat = (float *) mmap(0, fin.size(), PROT_READ, MAP_SHARED,
                                 fin.handle(), (off_t)0);
        
        if ((int)pMat == -1)
        {
            qWarning() << "Unable to map correlation matrix";
            return -1;
        } 

        numMovies = ((unsigned int*) pMat)[0];
        pMat++;

        return 0;
    } 

    float getCorr(int movieId1, int movieId2)
    {
        
        // Order the two ids so that movieId1 is smaller
        if (movieId2 > movieId1)
        {
            int temp = movieId1;
            movieId1 = movieId2;    
            movieId2 = temp;
        }

        // Same movie should have 0 correlation, so that it does not
        // affect its own rating
        if (movieId1 == movieId2)
            return 0;


        // The lower triangular matrix is stored at pMat (excluding the
        // main diagonal), so return the movieId2 row, movieId1 column
        
        // Use triangular numbers to get the index of
        // the row we need to start at.
        unsigned int i = movieId2 - 1;
        unsigned int ref = ((i-1) * (i)) >> 1; 

        return pMat[ref + movieId1 - 1];
    }

    float calcRating(int movieId)
    {
        // Check if the value was already calculated, if so just return
        // the pre-computed value
        if (cachedRatings.contains(QPair<unsigned int, unsigned short>
                                        (currUser.id(), movieId)))
            return cachedRatings[QPair<unsigned int, unsigned short>
                                      (currUser.id(), movieId)];

       
        // Otherwise, the value should be computed and then stored
        float corrSum = 0;
        float ratingSum = 0;
        int userMax = currUser.votes();

        for (int i = 0; i < userMax; i++)
        {
            int rating = currUser.score(i);
            int mov    = currUser.movie(i);
            float r    = getCorr(movieId, mov);

            corrSum   += fabs(r);
            ratingSum += r * rating; 
        }

        float rating = ratingSum / corrSum;

        if (isnan(rating))
            rating = 3.0;
  
        cachedRatings[QPair<unsigned int, unsigned short>
                                 (currUser.id(), movieId)] = rating;

        return rating;
    }

    void setUser(int id)
    {
        currUser.setId(id);
    }

    int order(int movieId1, int movieId2)
    {
        // Load up each movie's correlation vectors
        float rating1 = calcRating(movieId1);
        float rating2 = calcRating(movieId2);

        if (rating1 < rating2)
            return -1;
        else
            return 1;
    }

private:
    float * pMat;
    unsigned int numMovies; 
    User currUser;

    QHash< QPair<unsigned int, unsigned short>, float > cachedRatings;
};

int main(int , char **)
{
    DataBase db;
    db.load();
    Probe probe(&db);
    PearsonOrder algorithm(&db);
    probe.runProbeOrdering(&algorithm, "probe");
    return 0;
}

