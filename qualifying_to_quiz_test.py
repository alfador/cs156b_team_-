'''
Splits a qualifying submission into two files, one for quiz predictions
and one for test predictions.  These are sorted by user.
'''
import sys

judging_location = '/grandprize/judging.txt'

usage = 'python qualifying_to_quiz_test.py [submission_file] [base_output\
_name] [path_to_qualifying.txt] [path_to_judging.txt]'

# Check for valid input
if len(sys.argv) != 5:
    print usage
    exit()

submission_filename = sys.argv[1]
base_output_filename = sys.argv[2]
path_to_qualifying = sys.argv[3]
path_to_judging = sys.argv[4]

# Map userid (integer) to (quiz_ratings, test_ratings)
users = {}

# Open files
judging = open(path_to_judging, 'r')
qualifying = open(path_to_qualifying, 'r')
submission = open(submission_filename, 'r')

# Movie currently on
current_movie = 0

# Read through files, putting data in users
while True:
    judge_line = judging.readline().strip()
    qualifying_line = qualifying.readline().strip()
    submission_line = submission.readline().strip()

    # Check if we're done
    if judge_line == '':
        break

    # Check for new movie
    if judge_line.endswith(':'):
        assert judge_line == qualifying_line == submission_line # Sanity check
        current_movie = int(judge_line.strip(':'))
        print 'Current movie:', current_movie
        assert (1 <= current_movie <= 17770)
        continue

    # Have a rating
    user_id = int(qualifying_line.split(',')[0]) # User id
    guessed_rating = float(submission_line)      # Submission guess
    data_set = int(judge_line.split(',')[1])     # Quiz (0) or Test (1)

    # Add user if necessary
    if user_id not in users:
        users[user_id] = ([],[])

    # Add rating to users
    users[user_id][data_set].append((current_movie, guessed_rating))

# Close files
judging.close()
qualifying.close()
submission.close()

# Sort ratings by user
ratings = users.items()
ratings.sort()

# Make output files
quiz_filename = base_output_filename + '_quiz.txt'
test_filename = base_output_filename + '_test.txt'
quiz = open(quiz_filename, 'w')
test = open(test_filename, 'w')

# Write to output files
print 'Outputting...'
for uid, rates in ratings:
    quiz_ratings = rates[0]
    test_ratings = rates[1]
    # Add user to quiz if user has quiz ratings
    if quiz_ratings != []:
        quiz.write(str(uid) + ':\n')
        # Sort by movie id (default, as first element of tuple)
        quiz_ratings.sort()
        # Write ratings to file
        for movie_id, rating in quiz_ratings:
            quiz.write(str(movie_id) + ',' + str(rating) + '\n')
    # Add user to test if user has quiz ratings
    if test_ratings != []:
        test.write(str(uid) + ':\n')
        # Sort by movie id (default, as first element of tuple)
        test_ratings.sort()
        # Write ratings to file
        for movie_id, rating in test_ratings:
            test.write(str(movie_id) + ',' + str(rating) + '\n')

# Close files
quiz.close()
test.close()
