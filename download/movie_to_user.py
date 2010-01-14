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


for movie_id in range(1, num_movies + 1):
    print 'movie:', movie_id
    movie_file = id_to_movie(movie_id)
    movie_file.readline() # Skip movie id line
    for line in movie_file:
        [user_id, rating, date] = line.split(',')
        user_file = id_to_user(int(user_id))
        user_file.write(str(movie_id) + ',' + rating + ',' + date + '\n')
        user_file.close()
    movie_file.close()
