
#include "fMatrix.h"
#include "assert.h"
#include <cstdlib>

// Constructors

// Makes a 0x0 matrix.
fMatrix::fMatrix() {
    num_rows = 0;
    num_cols = 0;
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

