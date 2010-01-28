'''
This script converts the movies files to user files.
'''

path_to_movies = 'train/movies/'
path_to_users = 'train/users/'

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


def get_users(user_id_min, user_id_max):
    '''
    Converts to user files for all users within the user_id range, inclusive.
    '''
    for movie_id in range(1, num_movies + 1):
#        if movie_id % 9000 == 0:
#            flush_users()
        print 'movie:', movie_id
        movie_file = id_to_movie(movie_id)
        movie_file.readline() # Skip movie id line
        for line in movie_file:
            [user_id, rating, date] = line.split(',')
            if user_id_min <= int(user_id) <= user_id_max:
                if not users.has_key(user_id):
                    users[user_id] = []
                users[user_id].append(str(movie_id) + ',' + rating + ',' +
                                      date)
        movie_file.close()
    flush_users()

if __name__ == '__main__':
    # Split into several ranges so that the files are sequential and we don't
    # run out of memory.
    get_users(0000001, 0500000)
    get_users(0500001, 1000000)
    get_users(1000001, 1500000)
    get_users(1500001, 2000000)
    get_users(2000001, 3000000)


