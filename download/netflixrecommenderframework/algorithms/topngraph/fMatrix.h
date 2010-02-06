
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
    float getEntry(int row, int col);

    // Mutators
    void setEntry(int row, int col, float num);
    void setSize(int rows, int cols);

    // Other
    void multiply(fMatrix& other, fMatrix& result);
    void randomize();

private:
    int num_rows;
    int num_cols;
    float *mat;
};
