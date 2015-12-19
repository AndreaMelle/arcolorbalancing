//
//  datahelpers.h
//  ar-color-balancing
//
//  Created by Andrea Melle on 18/12/2015.
//
//

#ifndef datahelpers_h
#define datahelpers_h

namespace data
{
    template<typename T, const int dim>
    static bool tofile(const char* filename,
                       const char* format,
                       const int& num_points,
                       const Eigen::Matrix<T, Eigen::Dynamic, dim, Eigen::RowMajor>& pts,
                       const Eigen::Matrix<T, Eigen::Dynamic, 1>& vals)
    {
        FILE *f = fopen(filename, "w");
        
        if (f == NULL)
        {
            printf("Error creating file!\n");
            return false;
        }
        
        for(int i = 0; i < num_points; ++i)
        {
            const T* point = &(pts.data()[i * dim]);
            
            for(int j = 0; j < dim; ++j)
            {
                fprintf(f, format, point[j]);
                fprintf(f, " ");
            }
            
            fprintf(f, format, vals(i));
            fprintf(f, "\n");
        }
        
        fclose(f);
        return true;
    }
    
    // Ok this is bad. requires to know how many points in file
    template<typename T, const int dim>
    static bool fromfile(const char* filename,
                         const char* format,
                         const int& num_points,
                         Eigen::Matrix<T, Eigen::Dynamic, dim, Eigen::RowMajor>& pts,
                         Eigen::Matrix<T, Eigen::Dynamic, 1>& vals)
    {
        FILE *f = fopen(filename, "r");
        
        if (f == NULL)
        {
            printf("Error reading file!\n");
            return false;
        }
        
        pts = Eigen::Matrix<T, Eigen::Dynamic, dim, Eigen::RowMajor>::Zero(num_points, dim);
        vals = Eigen::Matrix<T, Eigen::Dynamic, 1>::Zero(num_points);
        
        T value;
        
        for(int i = 0; i < num_points; ++i)
        {
            for(int j = 0; j < dim; ++j)
            {
                fscanf(f,format,&value); //"%lf"
                pts(i, j) = value;
            }
            
            fscanf(f,format,&value);
            vals(i) = value;
        }
        
        fclose(f);
        return true;
    }
    
}


#endif /* datahelpers_h */
