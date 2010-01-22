import sqlite3


conn = sqlite3.connect("Netflix.db")

# Create the table in memory
conn.cursor().execute("create temp table temp_ratings(userId int,\
                                                      movieId int,\
                                                      rating int,\
                                                      date date)");

for i in range(1, 17771):
   
    filename = "./training_set/mv_%07d.txt" % i 
    f = open(filename, "r")

    print "Doing", filename
    
    f.readline() # ignore first line
    
    for line in f:
      
        line = line.strip() 
        parts = line.split(",")
        userId = int(parts[0])
        rating = int(parts[1])
        date = parts[2]
 
        conn.cursor().execute("insert into temp_ratings\
                               (userId, movieId, rating,\
                               date) values (?, ?, ?, ?)", 
                              (userId, i, rating, date))


    f.close()

# Dump all the data into a real table
print "Dumping"
conn.cursor().execute("create table ratings(userId int,\
                                            movieId int,\
                                            rating int,\
                                            date date)")

conn.cursor().execute("insert into ratings (userId, movieId, rating, date)\
                     select userId, movieId, rating, date from temp_ratings")
conn.commit()
