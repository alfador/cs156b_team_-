
#include "fMatrix.h"
#include "assert.h"
#include <cstdlib>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QByteArray>

// Constructors

// Makes a 1x1 matrix.
fMatrix::fMatrix() {
    num_rows = 1;
    num_cols = 1;
    mat = new float[1];
}

fMatrix::fMatrix(int rows, int cols) {
    num_rows = rows;
    num_cols = cols;
    mat = new float[rows * cols];
    for (int i = 0; i < rows * cols; i++)
        mat[i] = 0;
}

// Destructor
fMatrix::~fMatrix() {
    delete[]  mat;
}

// Accessors
int fMatrix::rows() {
    return num_rows;
}

int fMatrix::cols() {
    return num_cols;
}

// Mutators
void fMatrix::setSize(int rows, int cols) {
    delete[] mat;
    num_rows = rows;
    num_cols = cols;
    mat = new float[rows * cols];
    for (int i = 0; i < rows * cols; i++)
        mat[i] = 0;   
}

// Adds a matrix to this matrix
void fMatrix::addFMatrix(fMatrix& other) {
    assert(other.rows() == this->num_rows && other.cols() == this->num_cols);
    // Assume same size
    for (int i = 0; i < this->num_rows; i++)
        for (int j = 0; j < this->num_cols; j++)
            plusEntry(i, j, other.getEntry(i, j));
}

// Multiply two matrices
void fMatrix::multiply(fMatrix& other, fMatrix& result) {
    assert(this->cols() == other.rows());
    // Make the result matrix.
    result.setSize(this->rows(), other.cols());
    for (int row = 0; row < result.rows(); row++) {
        for (int col = 0; col < result.cols(); col++) {
            float sum = 0;
            for (int i = 0; i < this->cols(); i++)
                sum += this->getEntry(row, i) * other.getEntry(i, col);
            result.setEntry(row, col, sum);
        }
    }
}

// Fill the entries in the given matrix randomly.
void fMatrix::randomize() {
    for (int r = 0; r < this->rows(); r++)
        for (int c = 0; c < this->cols(); c++)
            this->setEntry(r, c, (float) rand() / RAND_MAX);
}


// Saves this matrix to file
// Returns +1 if an error occurs, 0 otherwise.
int fMatrix::toFile(QString filename) {
    QFile fOut(filename);

    // Check if we can even write to a file.
    if (! fOut.open(QFile :: WriteOnly)) {
        qDebug() << "Unable to open file for writing!";
        return 1;
    }

    // Write the matrix dimensions to file.
    fOut.write((char *) &num_rows, sizeof(int));
    fOut.write((char *) &num_cols, sizeof(int));

    // Write each entry to file.
    for (int i = 0; i < num_rows; i++)
        for (int j = 0; j < num_cols; j++) {
            float result = this->getEntry(i, j);
            fOut.write((char *) &result, sizeof(float));
        }
    fOut.flush();
    fOut.close();
    return 0;
}


// Loads a matrix from file, and stores it in this.
// Returns +1 if an error occurs, 0 otherwise.
int fMatrix::fromFile(QString filename) {
    QFile fIn(filename);

    // Check if we can read from file.
    if (! fIn.open(QFile :: ReadOnly)) {
        qDebug() << "Unable to open file for reading!";
        return 1;
    }

    // Read in the matrix dimensions.
    int rows, cols;
    fIn.read((char *)&rows, sizeof(int));
    fIn.read((char *)&cols, sizeof(int));

    // Resize
    this->setSize(rows, cols);

    // Read in each entry.
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            float entry;
            fIn.read((char *) &entry, sizeof(float));
            this->setEntry(r, c, entry);
        }

    fIn.close();
    return 0;
}

