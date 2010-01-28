'''
This script converts the separated train/probe movie/user data into 4 flat
files, two for each dataset (train/probe), for both movies and users.
'''

path_to_train_movies = 'train/movies/'
path_to_train_users = 'train/users/'
path_to_probe_movies = 'probe/movies/'
path_to_probe_users = 'probe/users/'
movie_probe_filename = 'probe/movies.txt'
user_probe_filename = 'probe/users.txt'
movie_train_filename = 'train/movies.txt'
user_train_filename = 'train/users.txt'

probe_movies_filename = 'probe_movies.txt'
probe_users_filename = 'probe_users.txt'
user_list_filename = 'user_list.txt'

num_movies = 17770

def line_comp(line1, line2):
    '''
    Compares two lines.
    '''
    int1 = int(line1.split(',')[0])
    int2 = int(line2.split(',')[0])
    return -1 if int1 < int2 else 0 if int1 == int2 else 1

def id_to_moviename(id):
    '''
    Converts a user id to a moviename (padded with 0s).  Input 'id' should be
    an integer.  Returns a string.
    '''
    return '%07d' % id

def id_to_username(id):
    '''
    Converts a user id to a username (padded with 0s).  Input 'id' should be an
    integer.  Returns a string.
    '''
    return '%07d' % id

## The new probe movie flat file
#probe_movie_file = open(movie_probe_filename, 'w')
#
## Parse probe movie files
#print 'Parsing movie probe files...'
#for movie_id in range(1, num_movies + 1):
#    print 'Current probe movie:', movie_id
#    # Add a line to indicate the start of each movie's data
#    probe_movie_file.write(str(movie_id) + ":\n")
#
#    try:
#        probe_movie = open(path_to_probe_movies + 'mv_' +
#                           id_to_moviename(movie_id) + '.txt', 'r')
#    except IOError:
#        continue
#    lines = probe_movie.readlines()
#    for i, line in enumerate(lines):
#        lines[i] = (int(line.split(',')[0]), line)
#    lines.sort()
#    for (_, line) in lines:
#        probe_movie_file.write(line)
#    probe_movie.close()
#
#probe_movie_file.close()
#
#
# Make list of users
print 'Making list of users...'
user_ids = []
user_ids_file = open(user_list_filename, 'r')
for line in user_ids_file:
    user_id = int(line[:-1]) # Take out \n
    user_ids.append(user_id)
user_ids_file.close()
#
## The new probe user flat file
#probe_user_file = open(user_probe_filename, 'w')
#print 'Parsing user probe files...'
#for user_id in user_ids:
#    print 'Current probe user id:', user_id
#    # Add a line to indicate the start of each user's data
#    probe_user_file.write(str(user_id) + ":\n")
#
#    probe_user = open(path_to_probe_users + 'usr_' +
#                       id_to_username(user_id) + '.txt', 'r')
#    lines = probe_user.readlines()
#    for i, line in enumerate(lines):
#        lines[i] = (int(line.split(',')[0]), line)
#    lines.sort()
#    for (_, line) in lines:
#        probe_user_file.write(line)
#    probe_user.close()
#
#probe_user_file.close()
#
#
## The new train movie flat file
#train_movie_file = open(movie_train_filename, 'w')
#
## Parse probe movie files
#print 'Parsing movie train files...'
#for movie_id in range(1, num_movies + 1):
#    print 'Current train movie:', movie_id
#    # Add a line to indicate the start of each movie's data
#    train_movie_file.write(str(movie_id) + ":\n")
#
#    train_movie = open(path_to_train_movies + 'mv_' +
#                       id_to_moviename(movie_id) + '.txt', 'r')
#    lines = train_movie.readlines()[1:]
#    for i, line in enumerate(lines):
#        lines[i] = (int(line.split(',')[0]), line)
#    lines.sort()
#    for (_, line) in lines:
#        train_movie_file.write(line)
#    train_movie.close()
#
#train_movie_file.close()


# The new train user flat file
train_user_file = open(user_train_filename, 'w')
print 'Parsing user train files...'
for user_id in user_ids:
    print 'Current train user id:', user_id
    # Add a line to indicate the start of each user's data
    train_user_file.write(str(user_id) + ":\n")

    train_user = open(path_to_train_users + 'usr_' +
                       id_to_username(user_id) + '.txt', 'r')
    lines = train_user.readlines()
    for i, line in enumerate(lines):
        lines[i] = (int(line.split(',')[0]), line)
    lines.sort()
    for (_, line) in lines:
        train_user_file.write(line)
    train_user.close()

train_user_file.close()

