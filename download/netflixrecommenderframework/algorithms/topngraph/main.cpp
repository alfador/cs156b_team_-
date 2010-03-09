#include <probe.h>
#include <cstdlib>
#include <fMatrix.h>
#include <iostream>
#include <movie.h>
#include <user.h>
#include <ctime>
#include <assert.h>
#include <cmath>

using std :: cout;
using std :: endl;

const float SINK_PROB = .3;     // Probability of going to the sink node
const float WEIGHT_SCALE = 1 - SINK_PROB;
const int FIRST_USER = 6;       // ID of first user
const int MAX_USER_ID = 2649429; // Maximal user id
const int num_iters = 0; // Number of iterations to run
const int MOVIE_LIMIT = 2000;   // For testing on smaller data.  Inclusive.


// Converts a rating (1-5) to a weight
// Adjust this function for your own purposes
inline float rating_to_weight(int rating) {
/*    switch (rating)
    {
        case 1:
        return .0001;

        case 2:
        return .0010;

        case 3:
        return .0100;

        case 4:
        return .1000;

        case 5:
        return 1;

        default:
        assert(false);
        return 0;
    }
*/
   return pow(rating, 20);
}

// Implements the OrderingAlgorithm interface 
class TopNGraphOrder : public OrderingAlgorithm
{
public:
    TopNGraphOrder(DataBase *db) : OrderingAlgorithm()
    {
        this->data = db;

        // Seed random number generator
        srand((unsigned int)time(NULL));
        
        // Initial data structures
        int total_movies = db->totalMovies();
        int total_users = db->totalUsers();
        float *movie_sum = new float[MOVIE_LIMIT];
        float *user_sum = new float[total_users];
        int *user_ids = new int[total_users];
        int *user_indices = new int[MAX_USER_ID];
        this->user_vector.setSize(1, MOVIE_LIMIT + 1);

        // Determine user indices
        cout << "Getting user indices." << endl;
        User u(db);
        u.setId(FIRST_USER);
        for (int i = 0; i < total_users; ++i) {
            user_ids[i] = u.id();
            u.next();
        }

        // Make the user_id->index lookup array.
        cout << "Getting reverse user indices." << endl;
        for (int i = 0; i < total_users; ++i) {
            user_indices[user_ids[i]] = i;
        }

        // Determine the sum of weights for each movie.
        cout << "Getting movie sums." << endl;
        Movie m(db);
        int report_interval = MOVIE_LIMIT / 10;
        for (int i = 1; i <= MOVIE_LIMIT; ++i) {
            if (i % report_interval == 0)
                cout << "Movie: " << i << endl;
            m.setId(i);
            int num_ratings = m.votes();
            float weight_sum = 0;
            // Add the weight of each user's rating.
            for (int j = 0; j < num_ratings; ++j) {
                weight_sum += rating_to_weight(m.score(j));
            }
            movie_sum[i - 1] = weight_sum;
        }
            
        // Determine the sum of weights for each user.
        cout << "Getting user sums." << endl;
        report_interval = total_users / 10;
        for (int i = 0; i < total_users; ++i) {
            if (i % report_interval == 0)
                cout << "User: " << i << endl;
            u.setId(user_ids[i]);
            int num_ratings = u.votes();
            float weight_sum = 0;
            // Sum the weights given to each movie.
            for (int j = 0; j < num_ratings; ++j)
                if (u.movie(j) <= MOVIE_LIMIT) {
                    weight_sum += rating_to_weight(u.score(j));
                }
            // In case a user hasn't rated any movies within the desired
            // movie limit.  This prevents divide by 0 errors.
            weight_sum = (weight_sum != 0) ? weight_sum : 1;

            user_sum[i] = weight_sum;
        }

        // Make the transition matrix.
        cout << "Making transition matrix." << endl;

        fMatrix trans_mat(MOVIE_LIMIT + 1, MOVIE_LIMIT + 1);

        report_interval = MOVIE_LIMIT / 10;
        Movie m1(db);
        for (int i = 1; i <= MOVIE_LIMIT; ++i) {
            // Set probabilities in row i - 1 
            if (i % report_interval == 0)
                cout << "Movie: " << i << endl;
            m1.setId(i);
            int num_ratings = m1.votes();
            for (int j = 0; j < num_ratings; ++j) {
                // Iterate through users that rated m1
                int u_id = m1.user(j);
                u.setId(u_id);
                float pm1u = rating_to_weight(m1.score(j)) / movie_sum[i - 1];
                int u_ratings = u.votes();
                for (int k = 0; k < u_ratings; ++k) {
                    // Iterate through ratings for u
                    int m2 = u.movie(k);
                    if (m2 <= MOVIE_LIMIT) {
                        float pum2 = rating_to_weight(u.score(k)) / 
                                     user_sum[user_indices[u_id]];
                        trans_mat.plusEntry(i - 1, m2 - 1,
                                            pm1u * pum2 * WEIGHT_SCALE);
                    }
                }
            }
            // Account for the sink node
            trans_mat.setEntry(i - 1, MOVIE_LIMIT, SINK_PROB);
        }

        // Give some back to movies
        float loop_back = WEIGHT_SCALE / total_movies;

        // Set probabilities in the sink row.
        for (int i = 0; i < MOVIE_LIMIT; ++i)
            trans_mat.setEntry(MOVIE_LIMIT, i, 0);
//            trans_mat.setEntry(MOVIE_LIMIT, i, loop_back);

        trans_mat.setEntry(MOVIE_LIMIT, MOVIE_LIMIT, 1);
//        trans_mat.setEntry(MOVIE_LIMIT, MOVIE_LIMIT, SINK_PROB);

        cout << "Building auxiliary matrices" << endl;
        // Make a summation matrix corresponding to expected number of visits.
        fMatrix visits(MOVIE_LIMIT + 1, MOVIE_LIMIT + 1);
        visits.addFMatrix(trans_mat);
        
        // Matrix corresponding to powers of the transition matrix
        fMatrix trans_powers(MOVIE_LIMIT + 1, MOVIE_LIMIT + 1);
        trans_powers.addFMatrix(trans_mat); // Starts of as first power.

        fMatrix temp(1, 1);

        for (int i = 0; i < num_iters; ++i) {
            cout << "Mult iter: " << i << endl;
            // Multiply and add to visits
            trans_powers.multiply(trans_mat, temp);
            visits.addFMatrix(temp);
            // Put result back in trans_powers
            trans_powers.setSize(MOVIE_LIMIT + 1, MOVIE_LIMIT + 1);
            trans_powers.addFMatrix(temp);
        }
        // Free up some memory
        trans_powers.setSize(1, 1);
        trans_mat.setSize(1, 1);

        // Put in expected_visits
        cout << "Storing..." << endl;
        this->expected_visits.setSize(MOVIE_LIMIT + 1, MOVIE_LIMIT + 1);
        this->expected_visits.addFMatrix(visits);

    }

    ~TopNGraphOrder()
    {
        //delete [] avgs;
    }

    void setUser(int id)
    {
        // Make the row vector of user weights, then multiply it with
        // expected_visits, storing the result in user_vector. 
        User u(data);
        this->user_weights.setSize(1, MOVIE_LIMIT + 1); // Fill with 0s
        u.setId(id);

        // Fill in user_weights.
        int num_ratings = u.votes();
        for (int j = 0; j < num_ratings; ++j)
            if (u.movie(j) <= MOVIE_LIMIT) 
                this->user_weights.setEntry(0, u.movie(j) - 1, 
                                      rating_to_weight(u.score(j)));
                

        // Zero out user_vector
        for (int i = 0; i < MOVIE_LIMIT + 1; ++i)
            this->user_vector.setEntry(0, i, -1);

    }

    // -1 if movie1 is better than movie2
    // +1 otherwise
    int order(int movie1, int movie2)
    {
        // Check if we can even use our expected visits vector.
        if (movie1 > MOVIE_LIMIT || movie2 > MOVIE_LIMIT) {
            cout << "RANDOM" << endl;
            int random = rand() % 2;
            return random == 0 ? -1 : 1;
        }
        float m1 = user_vector.getEntry(0, movie1 - 1);
        if (m1 < 0) {
            // Need to get entry.
            user_vector.setEntry(0, movie1 - 1,
                this->user_weights.multiplyVecByColumn(expected_visits,
                                                       movie1 - 1));
            m1 = user_vector.getEntry(0, movie1 - 1);
        }
        float m2 = user_vector.getEntry(0, movie2 - 1);
        if (m2 < 0) {
            // Need to get entry.
            user_vector.setEntry(0, movie2 - 1,
                this->user_weights.multiplyVecByColumn(expected_visits,
                                                       movie2 - 1));
            m2 = user_vector.getEntry(0, movie2 - 1);
        }

        if (m1 > m2)
            return -1;
        else
            return 1;
    }

private:
    
    fMatrix expected_visits;
    fMatrix user_vector;
    fMatrix user_weights;
    int *user_indices;
    DataBase *data;
};

int main(int , char **)
{
    DataBase db;
    db.load();
    Probe probe(&db);
    TopNGraphOrder top(&db);
    probe.runProbeOrdering(&top, "probe");
}

