#include <probe.h>
#include <cstdlib>

class RandomOrder : public OrderingAlgorithm
{
public:
    RandomOrder(DataBase *db) : OrderingAlgorithm()
    {
        Q_UNUSED(db);
    }

    void setUser(int id)
    {
        Q_UNUSED(id);
    }

    int order(int movie1, int movie2)
    {
        Q_UNUSED(movie1);
        Q_UNUSED(movie2);

        return (rand() % 2) * 2 - 1;
    }

};

int main(int , char **)
{
    DataBase db;
    db.load();
    Probe probe(&db);
    RandomOrder random(&db);
    probe.runProbeOrdering(&random, "probe");
}

