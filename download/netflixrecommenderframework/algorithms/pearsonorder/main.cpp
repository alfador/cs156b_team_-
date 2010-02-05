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

#define K 100

bool compareByFirst(const QPair<float, unsigned short> &a,
                    const QPair<float, unsigned short> &b)
{
    return a.first > b.first;
};

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

        qDebug() << "Constructing Correlation Matrix";

        // Go through each movie
        Movie movie(db);
        User  user(db);

        // Keep a vector of pairs of ratings for each possible
        // common movie
        QVector< QVector< QPair< float, float> > > commonRatings;
        commonRatings.resize(TOTAL_MOVIES);

        for (unsigned int i = 0; i < TOTAL_MOVIES; i++)
        {
            qDebug() << i;

            // Reset the pairs of common ratings;
            for (unsigned int j = 0; j < TOTAL_MOVIES; j++)
            {
                commonRatings[j].resize(0);
            }

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
                            
                    // Update the common movie vectors for each vote here
                    int movieId2 = user.movie(movieI);
                    float rating2 = user.score(movieI);

                    // Only care about movie ids larger than the current one
                    // as pearson correlations are symmetric, and we'll 
                    // calculate only lower to higher
                    if (movieId2 <= movieId1)
                        continue;
                    
                    commonRatings[movieId2 - 1].append(QPair<float, float>
                                                       (rating1, rating2));
                }
            }

            // All the information needed to calculate this movie's 
            // pearson coefficients with all other movies is available,
            // so now just get all the coefficients
            for (unsigned int j = i+1; j < TOTAL_MOVIES; j++)
            {
                float xMean = 0.0;
                float yMean = 0.0; 
                const int NUM_COMMON = commonRatings[j].size();
  
                for (int k = 0; k < NUM_COMMON; k++)
                {
                    xMean += commonRatings[j][k].first;
                    yMean += commonRatings[j][k].second;
                }

                xMean /= NUM_COMMON;
                yMean /= NUM_COMMON;

                float xydiff = 0.0;
                float xdiff2 = 0.0;
                float ydiff2 = 0.0;

                for (int k = 0; k < NUM_COMMON; k++)
                {
                    float xdiff = commonRatings[j][k].first - xMean;
                    float ydiff = commonRatings[j][k].second - yMean;
                    xydiff += xdiff * ydiff;
                    xdiff2 += xdiff * xdiff;
                    ydiff2 += ydiff * ydiff;
                }

                float corr = xydiff / (sqrt(xdiff2) * sqrt(ydiff2));
                if (isnan(corr))
                    corr = 0.0;

                // Place the correlation in the jth row, ith column
                // forming a lower triangular matrix
                pMat[j].append(corr);
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

    void loadSortedCorrelation(int movieId,
                               QVector<QPair< float, unsigned short> >& corr)
    {
 
        // Load up the correlation row for this movie       
        // Note that only the lower triangular matrix is stored,
        // without the main diagonal. So the relevant partial
        // row and partial column must both be traversed.
        corr.resize(numMovies);
        int count = 0;

        // Use triangular numbers to get the index of
        // the row we need to start at.
        unsigned int i = movieId - 1;
        unsigned int ref = ((i-1) * (i)) >> 1; 

        for (unsigned int rowI = 0; rowI < i; rowI++)
        {
            corr[count++] =  QPair<float, unsigned short> 
                              (pMat[ref + rowI], rowI + 1);
        }

        // append the current movie with correlation 0, so that
        // it will not affect its own rating.
        corr[count++] = QPair<float, unsigned short>
                            (0, i + 1);

        // append the partial column. This requires adding
        // an increasing offset per row
        int offset = i;
        int currI = ref + offset + i;
        for (unsigned int colI = 0; colI < numMovies - i - 1; colI++)
        {
            corr[count++] = QPair<float, unsigned short> 
                                (pMat[currI], i + colI + 2);
            offset ++;
            currI += offset;
        } 

        // Now sort by correlation
        qSort(corr.begin(), corr.end(), compareByFirst);
    } 

    float calcRating(unsigned int movieId)
    {
        // Check if the value was already calculated, if so just return
        // the pre-computed value
        if (cachedRatings.contains(QPair<unsigned int, unsigned short>
                                        (currUser.id(), movieId)))
            return cachedRatings[QPair<unsigned int, unsigned short>
                                      (currUser.id(), movieId)];

        
        // Otherwise, the value should be computed and then stored
        loadSortedCorrelation(movieId, corr);

        float corrSum = 0;
        float ratingSum = 0;
        int numAdded = 0;
        int userMax = currUser.votes();

        for (unsigned int i = 0; i < numMovies
                                 && numAdded < userMax
                                 && numAdded < K; 
                          i++)
        {
            float r = corr[i].first;
            int mov = corr[i].second;

            int rating = currUser.seenMovie(mov);
            if (rating == -1)
                continue;

            ratingSum += rating * r;           
            if (r > 0)
                corrSum += r;
            else
                corrSum -= r;

            numAdded++;
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

    QVector< QPair<float, unsigned short> > corr;
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

