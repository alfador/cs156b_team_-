'''
This script converts the movies files to user files.
'''

path_to_movies = 'training_set/'
path_to_users = 'users/'

num_movies = 17770


def id_to_movie(id):
    '''
    Converts a movie id to the file of a movie.  'id' is an integer.
    '''
    return open(path_to_movies + 'mv_' + '%07d' % id + '.txt', 'r')


def id_to_user(id):
    '''
    Converts a user id to a user file.  'id' is an integer.
    '''
    return open(path_to_users + 'usr_' + '%07d' % id + '.txt', 'a')

# Dictionary mapping user ids (strings) to lines to write.
users = {}

def flush_users():
    '''
    Flushes the dictionary 'users' out into their respective files.
    '''
    global users
    print 'Flushing users...'
    i = 1
    for user_id in users:
        print i
        i += 1
        user_file = id_to_user(int(user_id))
        user_file.writelines(users[user_id])
        user_file.close()
    users = {}


for movie_id in range(1, num_movies + 1):
#    if movie_id % 9000 == 0:
#        flush_users()
    print 'movie:', movie_id
    movie_file = id_to_movie(movie_id)
    movie_file.readline() # Skip movie id line
    for line in movie_file:
        [user_id, rating, date] = line.split(',')
        if not users.has_key(user_id):
            users[user_id] = []
        users[user_id].append(str(movie_id) + ',' + rating + ',' + date)
    movie_file.close()
flush_users()
