#ifndef __RBF_H__
#define __RBF_H__

#include <Eigen/Dense>
#include <iostream>

namespace rbf
{
//    template <typename T, const int dim>
//    inline T rad(const T* const p0, const T* const p1)
//    {
//        // Euclidean distance, arbitrary dimensions
//        T sum = 0;
//        T v = 0;
//        for(int i = 0; i < dim; ++i)
//        {
//            v = (p0[i] - p1[i]);
//            sum += v * v;
//        }
//        
//        return std::sqrt<T>(sum);
//    }
    
    // Abstract template class for RBF_fn
    template<typename T>
    class RBF_fn
    {
    public:
        RBF_fn() {}
        virtual ~RBF_fn() {}
        virtual T operator()(const T& r) const = 0;
    private:
        RBF_fn(const RBF_fn& other);
        RBF_fn& operator=(const RBF_fn& other);
    };

    template<typename T, const int dim, template<typename> class TRBF_fn>
    class RBF_interpolation
    {
    public:

        RBF_interpolation(const Eigen::Matrix<T, Eigen::Dynamic, dim>& in_pts,
                          const Eigen::Matrix<T, Eigen::Dynamic, 1>& in_vals,
                          bool in_normalize) : normalize(in_normalize)
        {
            n = in_pts.rows();
            assert(n == in_vals.size());
            
            pts = Eigen::Matrix<T, Eigen::Dynamic, dim>(in_pts);
            vals = Eigen::Matrix<T, Eigen::Dynamic, 1>(in_vals);
            
            fn = new TRBF_fn<T>();
            
            int i, j;
            T sum;
            
            Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> rbf(n, n);
            Eigen::Matrix<T, Eigen::Dynamic, 1> rhs(n);
            
            for(i = 0; i < n; ++ i)
            {
                sum = 0;
                
                for(j = 0; j < n; ++j)
                {
                    //T d = rad<T, dim>(&(pts.data()[i * dim]), &(pts.data()[j * dim]));
                    T d = (pts.row(i) - pts.row(j)).norm();
                    sum += ( rbf(i,j) = fn->operator()(d) );
                    
                    //std::cout << rbf(i,j) << std::endl;
                    
                }
                
                rhs(i) = normalize ? (sum * vals(i)) : vals(i);
            }
            
            //w = rbf.fullPivLu().solve(rhs);
            
            w = rbf.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(rhs);
            
            //std::cout << w << std::endl;
            
            double relative_error = (rbf * w - rhs).norm() / rhs.norm(); // norm() is L2 norm
            std::cout << "The relative error is: " << relative_error << std::endl;
            
        }
        
        T interpolate(const Eigen::Matrix<T, 1, dim>& in_pt)
        {
            T fval, sum = 0.0, sumw = 0.0;
            
            for(int i = 0; i < n; ++i)
            {
                T d = (in_pt - pts.row(i)).norm();
                fval = fn->operator()(d);
                sumw += w[i] * fval;
                sum += fval;
            }
            
            return normalize ? (sumw / sum) : sumw;
        }
        
        virtual ~RBF_interpolation() { delete fn; }

    private:
        RBF_interpolation(const RBF_interpolation& other);
        RBF_interpolation& operator=(const RBF_interpolation& other);

        Eigen::Matrix<T, Eigen::Dynamic, dim> pts;
        Eigen::Matrix<T, Eigen::Dynamic, 1> vals;

        Eigen::Matrix<T, Eigen::Dynamic, 1> w;
        
        int n;

        RBF_fn<T>* fn;

        bool normalize;


    };
    
    // Shepard interp
    template<typename T>
    class RBF_fn_Shepard : public RBF_fn<T>
    {
    public:
        RBF_fn_Shepard() : p(2.0) { }
        RBF_fn_Shepard(const T& in_p) : p(in_p) { }
        
        virtual T operator()(const T& r) const
        {
            return std::pow(r, -p);
        }
        
    private:
        T p;
    };
    
    // Normalized shepard interp
    template<typename T>
    class RBF_fn_NormShepard : public RBF_fn<T>
    {
    public:
        RBF_fn_NormShepard() : p(3.7975) { }
        RBF_fn_NormShepard(const T& in_p) : p(in_p) { }
        
        virtual T operator()(const T& r) const
        {
            return std::pow((1 + r), -p);
        }
        
    private:
        T p;
    };

};

#endif //__RBF_H__
