#include <probe.h>
#include <user.h>

const unsigned int NUM_FEATURES = 10;      // Number of features to use
const unsigned int MAX_EPOCHS   = 200;     // Max epochs
const double MIN_IMPROVEMENT = 0.00001;    // Minimum improvement required to
                                           // continue minimization

const double INIT = 0.1;           // Initialization value range for
                                   // features

const double LRATE =      0.001;   // Learning rate parameter
const double REGULARIZE = 0.015;   // Regularization parameter used
                                   // to minimize over-fitting

class SvdOrder : public Algorithm, public OrderingAlgorithm
{

public:
    SvdOrder(DataBase *db);

    void setMovie(int id);
    double determine(int user);

    void setUser(int user);
    int  order(int movie1, int movie2);    

    void calculateFeatures();

    void saveFeatures(QString filename);
    void loadFeatures(QString filename);

private:
    // feature vectors for each users and movies
    QVector< QVector<float> >  userFeatures;
    QVector< QVector<float> >  movieFeatures; 

    unsigned int currentMovie;
    unsigned int currentUser;
    DataBase * currDb;

    inline double predictRating(short movieId, int custId) const;
};

SvdOrder::SvdOrder(DataBase *db) : Algorithm(), OrderingAlgorithm()
{

    currDb = db;

    qDebug() << "Allocating";
    // Allocate memory for the feature vectors
    userFeatures.resize(db->totalUsers());
    for (int i = 0; i < db->totalUsers(); i++)
        userFeatures[i].resize(NUM_FEATURES);
    
    movieFeatures.resize(db->totalMovies());
    for (int i = 0; i < db->totalMovies(); i++)
        movieFeatures[i].resize(NUM_FEATURES);

    qDebug() << "Init";
    // Initialize them all to some random starting point
    for (unsigned int f = 0; f < NUM_FEATURES; ++f) 
    {
        for (int i = 0; i < db->totalUsers(); ++i)
            userFeatures[i][f] = ((((float)rand())/RAND_MAX * 2) - 1) * INIT;

        for (int i = 0; i < db->totalMovies(); ++i)
            movieFeatures[i][f] = ((((float)rand())/RAND_MAX * 2) - 1) * INIT;
    }   

    qDebug() << "Done with construct";
}

/**
 * Iteratively train each feature on the entire data set
 * Once sufficient progress has been made, move on
 */
void SvdOrder::calculateFeatures()
{
    qDebug() << "Training";

    // Keep looping until you have passed the maximum number
    // of epochs or have stopped making significant progress
    double prevRMSE = 1e8;
    double RMSE     = 1e7;
    Movie movie(currDb);
    double averageRating = currDb->getAverageRating();
    for (unsigned int i = 0;
         i < MAX_EPOCHS && (prevRMSE - RMSE) > MIN_IMPROVEMENT;
         i++)
    {
        prevRMSE = RMSE;
        RMSE = 0;

        User user(currDb,6);
        for (int j = 0; j < currDb->totalUsers(); j++)
        {   
            for (int k = 0; k < user.votes(); k++)
            {
                int movieId = user.movie(k);
                float rating = user.score(k) - averageRating;

                int userIndex = j;
                int movieIndex = movieId - 1;

                float predict = predictRating(movieIndex, userIndex);

                float diff = predict - rating;
                RMSE += diff * diff;

                // Update all the feature vectors here
                for (unsigned int l = 0; l < NUM_FEATURES; l++)
                {
                    float oldUF = userFeatures[userIndex][l];
                    float oldMF = movieFeatures[movieIndex][l];

                    userFeatures[userIndex][l] -= LRATE * 
                                        (diff * oldMF * 2 + REGULARIZE * oldUF);
                    movieFeatures[movieIndex][l] -= LRATE * 
                                        (diff * oldUF * 2 + REGULARIZE * oldMF);
                }
            }
            user.next();
        }

        RMSE = sqrt(RMSE / currDb->totalVotes());
        qDebug() << "Epoch" << i + 1 << "RMSE: " << RMSE;
    }                

    qDebug() << "Done with training";
}

double SvdOrder::predictRating(short movieIndex, int userIndex) const
{
    double sum = 0;

    for (unsigned int i = 0; i < NUM_FEATURES; i++)
        sum += userFeatures[userIndex][i] * movieFeatures[movieIndex][i];

    return sum;
}

void SvdOrder::setMovie(int id)
{
    currentMovie = id;
}

/**
 * Loop through the entire list of finished features
 */
double SvdOrder::determine(int user)
{
    int movieIndex = currentMovie - 1;
    int userIndex  = currDb->mapUser(user);
    return currDb->getAverageRating() + predictRating(movieIndex, userIndex);
}

void SvdOrder :: setUser(int user)
{
    currentUser = currDb->mapUser(user);
}

int SvdOrder :: order(int movie1, int movie2)
{
    if (predictRating(movie1 - 1, currentUser) >
        predictRating(movie2 - 1, currentUser))
        return -1;
    else
        return 1;
}

void SvdOrder :: saveFeatures(QString filename)
{
    QFile out(filename);
    out.open(QFile :: WriteOnly);

    for (int i = 0; i < currDb->totalMovies(); i++)
    {
        for (unsigned int j = 0; j < NUM_FEATURES; j++)
        {
            out.write((char*)&movieFeatures[i][j], sizeof(float)); 
        }
    }

    for (int i = 0; i < currDb->totalUsers(); i++)
    {
        for (unsigned int j = 0; j < NUM_FEATURES; j++)
        {
            out.write((char*)&userFeatures[i][j], sizeof(float)); 
        }
    }
}

void SvdOrder :: loadFeatures(QString filename)
{
    QFile in(filename);
    in.open(QFile :: ReadOnly);

    for (int i = 0; i < currDb->totalMovies(); i++)
    {
        for (unsigned int j = 0; j < NUM_FEATURES; j++)
        {
            in.read((char*)&movieFeatures[i][j], sizeof(float)); 
        }
    }

    for (int i = 0; i < currDb->totalUsers(); i++)
    {
        for (unsigned int j = 0; j < NUM_FEATURES; j++)
        {
            in.read((char*)&userFeatures[i][j], sizeof(float)); 
        }
    }
}

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
            qDebug() << "Argument" << args[1] << "not recognized" << endl;
            qDebug() << "Defaulting to ordering error" << endl;
            doOrder = true;
        } 
    }

    DataBase db;
    db.load();
    Probe probe(&db);
    SvdOrder alg(&db);
    QFile f("Features.dat");

    if (f.exists())
        alg.loadFeatures("Features.dat");
    else
        alg.calculateFeatures();

    if (doOrder)
        probe.runProbeOrdering(&alg, "probe");
    else
        probe.runProbe(&alg, "probe");

    alg.saveFeatures("Features.dat");
}

