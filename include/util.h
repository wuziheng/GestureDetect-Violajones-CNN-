#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>

void draw_text(cv::Mat& img,std::string text,int x, int y,cv::Scalar color);

void safe_enlarge(cv::Rect& tmp, const cv::Size size,const float scale_w,const float scale_h);

bool is_match(const cv::Rect& bef,const cv::Rect& now);

float cal_iou(const cv::Rect& rectA, const cv::Rect& rectB);

float cal_iou_max(const cv::Rect& rectA, const cv::Rect& rectB);

int search_original_proposal(const cv::Rect& now, const std::vector<std::pair<int,cv::Rect> >& proposals, float threshold=0.5);

