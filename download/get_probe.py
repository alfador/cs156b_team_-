'''
This script removes the probe examples from the training examples and puts
them in their own directory.
'''

path_to_training_movies = 'train/movies/'
path_to_training_users = 'train/users/'
path_to_probe_movies = 'probe/movies/'
path_to_probe_users = 'probe/users/'

probe_filename = 'probe.txt'
user_list_filename = 'user_list.txt'

num_movies = 17770



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

# Make dictionary of users
print 'Making dictionary of users...'
user_movies = {} # Maps user id to a list of movie ids in probe.
user_ids = open(user_list_filename, 'r')
for line in user_ids:
    user_id = int(line[:-1]) # Take out \n
    user_movies[user_id] = []
user_ids.close()

movie_users = {} # Maps movie id to a list of user ids in probe.

# Start parsing probe
print 'Parsing probe'
probe = open(probe_filename, "r")
for line in probe:
    if line.endswith(':\n'):
        # At a new movie
        movie_id = int(line[:-2]) # Take out :\n
        movie_users[movie_id] = []
    else:
        # Is a user id
        user_id = int(line[:-1]) # Take out \n
        movie_users[movie_id].append(user_id)
        user_movies[user_id].append(movie_id)

# Remove all probe lines from movie files, making the movie probe files
# at the same time        
print 'Preparing movie probe data'
for movie_id in movie_users:
    print 'Movie id:', movie_id
    # Open train_movie in w+ mode so that the file is truncated when done!
    train_movie = open(path_to_training_movies + 'mv_' + 
                       id_to_moviename(movie_id) + '.txt', 'r')
    probe_movie = open(path_to_probe_movies + 'mv_' +
                       id_to_moviename(movie_id) + '.txt', 'w')
    training_lines = train_movie.readlines()
    probe_lines = [] # List of lines to put in probe/lines to remove from train

    # Get lines that belong in probe
    for line in training_lines[1:]: # Ignore first line
        user_id = int(line.split(',')[0])
        if user_id in movie_users[movie_id]:
            # This line should be in probe
            probe_lines.append(line)

    # Remove these lines from the training lines
    for line in probe_lines:
        training_lines.remove(line)

    # Write the new training file and probe file
    train_movie.close()
    train_movie = open(path_to_training_movies + 'mv_' + 
                       id_to_moviename(movie_id) + '.txt', 'w')
    train_movie.writelines(training_lines)
    probe_movie.writelines(probe_lines)
    train_movie.close()
    probe_movie.close()

# Remove all probe lines from user files, making the user probe files
# at the same time        
print 'Preparing user probe data'
user_count = 0
for user_id in user_movies:
    print 'User count:', user_count
    user_count += 1
    # Open train_user in w+ mode so that the file is truncated when done!
    train_user = open(path_to_training_users + 'usr_' + 
                       id_to_username(user_id) + '.txt', 'r')
    probe_user = open(path_to_probe_users + 'usr_' +
                       id_to_username(user_id) + '.txt', 'w')
    training_lines = train_user.readlines()
    probe_lines = [] # List of lines to put in probe/lines to remove from train

    # Get lines that belong in probe
    for line in training_lines:
        movie_id = int(line.split(',')[0])
        if movie_id in user_movies[user_id]:
            # This line should be in probe
            probe_lines.append(line)

    # Remove these lines from the training lines
    for line in probe_lines:
        training_lines.remove(line)

    # Write the new training file and probe file
    train_user.close()
    train_user = open(path_to_training_users + 'usr_' + 
                       id_to_username(user_id) + '.txt', 'w')
    train_user.writelines(training_lines)
    probe_user.writelines(probe_lines)
    train_user.close()
    probe_user.close()
