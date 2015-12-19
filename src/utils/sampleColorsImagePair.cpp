#include <iostream>
#include <Eigen/Dense>
#include <datahelpers.h>
#include <opencv2/opencv.hpp>

#define DATA_DIM 2
#define MAX_WIDTH_VIZ 1000.0f
#define MAX_HEIGHT_VIZ 800.0f
#define MAX_SAMPLES 128

cv::Mat sampler;
cv::Mat curr_color;
cv::Mat last_color;

unsigned char samples_r[MAX_SAMPLES * 2];
unsigned char samples_g[MAX_SAMPLES * 2];
unsigned char samples_b[MAX_SAMPLES * 2];
int curr_sample;

void mouseCallback(int event, int x, int y, int flags, void* userdata)
{
    if(event == cv::EVENT_LBUTTONDOWN)
    {
        //cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
        
        cv::Vec3b s = sampler.at<cv::Vec3b>(y, x);
        
        samples_b[curr_sample] = s[0];
        samples_g[curr_sample] = s[1];
        samples_r[curr_sample] = s[2];
        
        printf("Sample %d: %d %d %d\n",
               curr_sample,
               (int)samples_r[curr_sample],
               (int)samples_g[curr_sample],
               (int)samples_b[curr_sample]);
        
        cv::circle(sampler, cv::Point(x, y), 3, cv::Scalar(0,0,255), -1);
        last_color = s;
        curr_sample++;
    }
    else if(event == cv::EVENT_MOUSEMOVE)
    {
        curr_color = sampler.at<cv::Vec3b>(y, x);
    }
}

/*
 * Allows to take pairs of color correspondences between two images
 */
int main(int argc, char** argv)
{
    if(argc < 4)
    {
        printf("Please specify path to two images and output file\n");
        return -1;
    }
    
    const char* imgfile1 = argv[1];
    const char* imgfile2 = argv[2];
    const char* outputfile = argv[3];
    
    cv::Mat img1, img2;
    
    img1 = cv::imread(imgfile1);
    img2 = cv::imread(imgfile2);
    
    if (!img1.data || !img2.data)
    {
        printf("No image data \n");
        return -1;
    }
    
    cv::Mat comp(cv::Size(img1.cols + img2.cols, cv::max(img1.rows, img2.rows)), CV_8UC3);
    
    cv::Mat color_preview(cv::Size(200, 100), CV_8UC3);
    
    curr_color = cv::Mat(cv::Size(100,100), CV_8UC3);
    curr_color = cv::Vec3b(0,0,0);
    
    last_color = cv::Mat(cv::Size(100,100), CV_8UC3);
    last_color = cv::Vec3b(0,0,0);
    
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
    
    cv::resize(comp, sampler, cv::Size(), scaleFactor, scaleFactor);
    
    const char* imageTitle = "Sample Colors";
    cv::namedWindow(imageTitle, cv::WINDOW_AUTOSIZE );
   
    
    cv::setMouseCallback(imageTitle, mouseCallback);
    
    char k;
    
    printf("Click to sample colors. Use 'Esc' or 'q' to quit, 'z' to undo, 'c' to clear\n\n");
    
    curr_sample = 0;
    
    while (1)
    {
        k = cv::waitKey(33);
        if(k == 27 || k == 'q') break;
        
        if(k == 'z')
            curr_sample--;
        if(k == 'c')
        {
            cv::resize(comp, sampler, cv::Size(), scaleFactor, scaleFactor);
            curr_sample = 0;
        }
        
        curr_color.copyTo(color_preview(cv::Rect(0, 0, 100, 100)));
        last_color.copyTo(color_preview(cv::Rect(100, 0, 100, 100)));
        
        cv::imshow("Color", color_preview);
        cv::imshow(imageTitle, sampler);
        
    }
    
    FILE *f = fopen(outputfile, "w");
    
    if (f == NULL)
    {
        printf("Error creating file!\n");
        return false;
    }
    
    fprintf(f, "%d\n", curr_sample / 2);
    
    for(int i = 0; i < curr_sample; ++i)
    {
        fprintf(f, "%d %d %d", samples_r[i], samples_g[i], samples_b[i]);
        fprintf(f, i % 2 ? "\n" : " ");
    }
    
    fclose(f);

    return 0;
}
