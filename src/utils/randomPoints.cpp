#include <iostream>
#include <Eigen/Dense>
#include <datahelpers.h>

#define DATA_DIM 2

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        printf("Please enter the number of samples to generate.\n");
        return -1;
    }
    
    int n = atoi(argv[1]);
    char* filename = argv[2];
    
    srand((unsigned int) time(0));

    Eigen::Matrix<double, Eigen::Dynamic, DATA_DIM, Eigen::RowMajor> pts(n, DATA_DIM);
    Eigen::Matrix<double, Eigen::Dynamic, 1> vals(n, 1);
    Eigen::Matrix<double, Eigen::Dynamic, 1> ones(n, 1);
    
    pts = Eigen::Matrix<double, Eigen::Dynamic, DATA_DIM, Eigen::RowMajor>::Random(n, DATA_DIM);
    vals = Eigen::Matrix<double, Eigen::Dynamic, 1, Eigen::RowMajor>::Random(n, 1);
    ones = Eigen::Matrix<double, Eigen::Dynamic, 1>::Ones(n, 1);
    
    vals = (vals + ones) * 50.0;
    
    data::tofile<double, DATA_DIM>(filename,
                                   "%0.6lf",
                                   n,
                                   pts,
                                   vals);

    return 0;
}
