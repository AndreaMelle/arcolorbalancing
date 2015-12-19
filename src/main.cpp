#include <iostream>
#include <rbf.h>
#include <colors.h>
#include <datahelpers.h>
#include <opencv2/opencv.hpp>

#define DATA_DIM 3
#define R 0
#define G 1
#define B 2

#define MAX_WIDTH_VIZ 1000.0f
#define MAX_HEIGHT_VIZ 800.0f



int main(int argc, char** argv)
{
    
    if(argc < 3)
    {
        printf("Please enter the color samples file.\n");
        return -1;
    }
    
    const char* color_samples_file = argv[2];
    
    FILE *f = fopen(color_samples_file, "r");
    
    if (f == NULL)
    {
        printf("Error reading file.\n");
        return false;
    }
    
    int num_samples = 0;
    int fres = fscanf(f,"%d\n",&num_samples);
    
    if (fres == EOF || num_samples == 0)
    {
        printf("File format error.\n");
        return false;
    }
    
    Eigen::Matrix<float, Eigen::Dynamic, DATA_DIM, Eigen::RowMajor> support
        = Eigen::Matrix<float, Eigen::Dynamic, DATA_DIM, Eigen::RowMajor>::Zero(num_samples, DATA_DIM);
    
    Eigen::Matrix<float, Eigen::Dynamic, 1> values[3];
    
    values[R] = Eigen::Matrix<float, Eigen::Dynamic, 1>::Zero(num_samples);
    values[G] = Eigen::Matrix<float, Eigen::Dynamic, 1>::Zero(num_samples);
    values[B] = Eigen::Matrix<float, Eigen::Dynamic, 1>::Zero(num_samples);
    
    int v = 0;
    unsigned char*   rgb    = (unsigned char*)malloc(sizeof(unsigned char) * num_samples * DATA_DIM * 2);
    float*           frgb   = (float*)malloc(sizeof(float) * num_samples * DATA_DIM * 2);
    float*           lab    = (float*)malloc(sizeof(float) * num_samples * DATA_DIM * 2);
    
    color::IColorConversion<float>* rgbToLab = color::CreateColorConversion<float>(color::sRGB_to_CIELAB, color::RGB);
    color::IColorConversion<float>* labToRgb = color::CreateColorConversion<float>(color::CIELAB_sRGB, color::RGB);
    
    for(int i = 0; i < num_samples; ++i)
    {
        for(int j = 0; j < 2 * DATA_DIM; ++j)
        {
            fscanf(f,"%d ",&v);
            rgb[i * (2 * DATA_DIM) + j] = (unsigned char)v;
        }
    }
    
    color::RGB255_to_RGB01(rgb, frgb, num_samples * 2);
    rgbToLab->convert(frgb, lab, num_samples * 2);
    
//    for(int i = 0; i < num_samples * 2 * DATA_DIM; i+=3)
//    {
//        color::print_RGB(&rgb[i]);
//        color::print_RGB(&frgb[i]);
//        color::print_Lab(&lab[i]);
//    }
    
    for(int i = 0; i < num_samples; ++i)
    {
        float ci[3];
        float di[3];
        
        for(int j = 0; j < DATA_DIM; ++j)
        {
            ci[j] = lab[i * (2 * DATA_DIM) + j];
            support(i,j) = ci[j];
        }
        
        for(int j = 0; j < DATA_DIM; ++j)
        {
            di[j] = lab[i * (2 * DATA_DIM) + j + DATA_DIM];
            values[j](i) = di[j] - ci[j];
        }
    }
    
    
    free(rgb);
    free(frgb);
    free(lab);
    
    //std::cout << support << std::endl;

    rbf::RBF_interpolation<float, DATA_DIM, rbf::RBF_fn_NormShepard>* rbf[3];
    
    rbf[R] = new rbf::RBF_interpolation<float, DATA_DIM, rbf::RBF_fn_NormShepard>(support, values[R], true);
    rbf[G] = new rbf::RBF_interpolation<float, DATA_DIM, rbf::RBF_fn_NormShepard>(support, values[G], true);
    rbf[B] = new rbf::RBF_interpolation<float, DATA_DIM, rbf::RBF_fn_NormShepard>(support, values[B], true);
    
    
    const char* imgfile1 = argv[1];
    //const char* imgfile2 = argv[2];
    
    cv::Mat img1, img2;
    
    img1 = cv::imread(imgfile1);
    img2 = cv::imread(imgfile1);
    
    if (!img1.data || !img2.data)
    {
        printf("No image data \n");
        return -1;
    }
    
    const int channels = img2.channels();
    
    assert(img2.depth() == CV_8U && channels == 3);
    
    unsigned char crgb[3];
    float cfrgb[3];
    float clab[3];
    Eigen::Matrix<float, 1, DATA_DIM> val;
    
    cv::MatIterator_<cv::Vec3b> it, end;
    for( it = img2.begin<cv::Vec3b>(), end = img2.end<cv::Vec3b>(); it != end; ++it)
    {
        
        crgb[B] = (*it)[R];
        crgb[G] = (*it)[G];
        crgb[R] = (*it)[B];
        
        color::RGB255_to_RGB01(crgb, cfrgb);
        rgbToLab->convert(cfrgb, clab, 1);
        
        val(R) = clab[R];
        val(G) = clab[G];
        val(B) = clab[B];
        
        clab[R] += rbf[R]->interpolate(val);
        clab[G] += rbf[G]->interpolate(val);
        clab[B] += rbf[B]->interpolate(val);
        
        labToRgb->convert(clab, cfrgb, 1);
        color::RGB01_to_RGB255(cfrgb, crgb);
        
        (*it)[R] = crgb[B];
        (*it)[G] = crgb[G];
        (*it)[B] = crgb[R];
    }
    
    cv::Mat comp(cv::Size(img1.cols + img2.cols, cv::max(img1.rows, img2.rows)), CV_8UC3);
    
    img1.copyTo(comp(cv::Rect(0, 0, img1.cols, img1.rows)));
    img2.copyTo(comp(cv::Rect(img1.cols, 0, img2.cols, img2.rows)));
    
    float scaleFactor = 1.0f;
    
    if(comp.rows > comp.cols)
    {
        scaleFactor = MAX_HEIGHT_VIZ / (float)comp.rows;
    }
    else
    {
        scaleFactor = MAX_WIDTH_VIZ / (float)comp.cols;
    }
    
    cv::Mat compSmall;
    cv::resize(comp, compSmall, cv::Size(), scaleFactor, scaleFactor);
    
    const char* imageTitle = "Sample Colors";
    cv::namedWindow(imageTitle, cv::WINDOW_AUTOSIZE );
    
    char k;
    
    while (1)
    {
        k = cv::waitKey(33);
        if(k == 27 || k == 'q') break;
        
        cv::imshow(imageTitle, compSmall);
    }
    
    delete rgbToLab;
    delete labToRgb;
    delete rbf[R];
    delete rbf[G];
    delete rbf[B];
    
    return 0;
}
