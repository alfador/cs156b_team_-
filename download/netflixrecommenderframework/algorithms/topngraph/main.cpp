#include <probe.h>
#include <cstdlib>
#include <fMatrix.h>
#include <iostream>
#include <movie.h>
#include <user.h>

using std :: cout;
using std :: endl;

const float SINK_PROB = .1;     // Probability of going to the sink node
const float WEIGHT_SCALE = 1 - SINK_PROB;
const int FIRST_USER = 6;       // ID of first user
const int MAX_USER_ID = 2649429; // Maximal user id

// Converts a rating (1-5) to a weight
// Adjust this function for your own purposes
inline float rating_to_weight(int rating) {
    return rating;
}

// Implements the OrderingAlgorithm interface 
class TopNGraphOrder : public OrderingAlgorithm
{
public:
    TopNGraphOrder(DataBase *db) : OrderingAlgorithm()
    {
        
        // Train all the movie averages, all 17770 should
        // easily fit in memory
        int total_movies = db->totalMovies();
        int total_users = db->totalUsers();
        float *movie_sum = new float[total_movies];
        float *user_sum = new float[total_users];
        int *user_ids = new int[total_users];
        int *user_indices = new int[MAX_USER_ID];

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
        int report_interval = total_movies / 100;
        for (int i = 1; i <= total_movies; ++i) {
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
        report_interval = total_users / 100;
        for (int i = 0; i < total_users; ++i) {
            if (i % report_interval == 0)
                cout << "User: " << i << endl;
            u.setId(user_ids[i]);
            int num_ratings = u.votes();
            float weight_sum = 0;
            // Sum the weights given to each movie.
            for (int j = 0; j < num_ratings; ++j) {
                weight_sum += rating_to_weight(u.score(j));
            }
            user_sum[i] = weight_sum;
        }

        // Make the transition matrix.
        cout << "Making transition matrix." << endl;
        fMatrix trans_mat(total_movies + 1, total_movies + 1);
        report_interval = total_movies / 17770;
        Movie m1(db);
        for (int i = 1; i <= total_movies; ++i) {
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
                    float pum2 = rating_to_weight(u.score(k)) / 
                                 user_sum[user_indices[u_id]];
                    trans_mat.plusEntry(i - 1, m2 - 1,
                                        pm1u * pum2 * WEIGHT_SCALE);
                }
            }
            // Account for the sink node
            trans_mat.setEntry(i - 1, total_movies, SINK_PROB);
        }

        // Set probabilities in the sink row.
        for (int i = 0; i < total_movies; ++i)
            trans_mat.setEntry(total_movies, i, 0);

        trans_mat.setEntry(total_movies, total_movies, 1);
    }

    ~TopNGraphOrder()
    {
        //delete [] avgs;
    }

    void setUser(int id)
    {
        Q_UNUSED(id);
    }

    int order(int movie1, int movie2)
    {
       return 1;
    }

private:
    
    //float * avgs;
};

int main(int , char **)
{
    DataBase db;
    db.load();
//    Probe probe(&db);
    TopNGraphOrder top(&db);
//    probe.runProbeOrdering(&movieAvg, "probe");
}

