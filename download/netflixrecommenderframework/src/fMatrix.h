
class fMatrix {
public:
    // Constructors
    fMatrix();
    fMatrix(int rows, int cols);

    // Destructor
    ~fMatrix();

    // Accessors
    int rows();
    int cols();
    inline float getEntry(int row, int col) {
        return mat[row * num_cols + col];
    }

    // Mutators
    void setSize(int rows, int cols);
    inline void setEntry(int row, int col, float num) {
        mat[row * num_cols + col] = num;
    }

    inline void plusEntry(int row, int col, float num) {
        mat[row * num_cols + col] += num;
    }

    // Other
    void multiply(fMatrix& other, fMatrix& result);
    void randomize();

private:
    int num_rows;
    int num_cols;
    float *mat;
};
