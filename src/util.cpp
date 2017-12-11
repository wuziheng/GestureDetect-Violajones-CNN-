#include "util.h"

void draw_text(cv::Mat& img,std::string text,int x, int y,cv::Scalar color)
{
	cv::putText(img,text.c_str(),cv::Point(x,y),cv::FONT_HERSHEY_SIMPLEX,0.8,color,2,1);
}


void safe_enlarge(cv::Rect& tmp, const cv::Size size,const float scale_w,const float scale_h){
 	
	
	int tx(tmp.x);
	int ty(tmp.y);
	int th(tmp.height);
	int tw(tmp.width);
						
	float sw = (scale_w-1.0)/8*(size.width/tw)+1.0;
	float sh = (scale_h-1.0)/8*(size.width/th)+1.0;

	tmp.x = std::max(0,tx-int(tw*(sw-1)/2));
	tmp.y = std::max(0,ty-int(th*(sh-1)/2));
	tmp.width = std::min(size.width-tmp.x-1,int(tw*sw));
	tmp.height = std::min(size.height-tmp.y-1,int(th*sh));

}


bool is_match(const cv::Rect& bef,const cv::Rect& now){
	//if((now.x+now.width/2 > bef.x && now.x+now.width/2 <bef.x+bef.width) && (now.y+now.height/2>bef.y && now.y+now.height/2<bef.y+bef.height))
	if(cal_iou_max(bef,now)>0.6)
		return true;
	else
		return false;
}


float cal_iou(const cv::Rect& rectA, const cv::Rect& rectB){
    if (rectA.x > rectB.x + rectB.width) { return 0.; }  
    if (rectA.y > rectB.y + rectB.height) { return 0.; }  
    if ((rectA.x + rectA.width) < rectB.x) { return 0.; }  
    if ((rectA.y + rectA.height) < rectB.y) { return 0.; }  
    float colInt = std::min(rectA.x + rectA.width, rectB.x + rectB.width) - std::max(rectA.x, rectB.x);  
    float rowInt = std::min(rectA.y + rectA.height, rectB.y + rectB.height) - std::max(rectA.y, rectB.y);  
    float intersection = colInt * rowInt;  
    float areaA = rectA.width * rectA.height;  
    float areaB = rectB.width * rectB.height;  
    float intersectionPercent = intersection / (areaA + areaB - intersection);  
    return intersectionPercent;  
}


float cal_iou_max(const cv::Rect& rectA, const cv::Rect& rectB){
    if (rectA.x > rectB.x + rectB.width) { return 0.; }  
    if (rectA.y > rectB.y + rectB.height) { return 0.; }  
    if ((rectA.x + rectA.width) < rectB.x) { return 0.; }  
    if ((rectA.y + rectA.height) < rectB.y) { return 0.; }  
    float colInt = std::min(rectA.x + rectA.width, rectB.x + rectB.width) - std::max(rectA.x, rectB.x);  
    float rowInt = std::min(rectA.y + rectA.height, rectB.y + rectB.height) - std::max(rectA.y, rectB.y);  
    float intersection = colInt * rowInt;  
    float areaA = rectA.width * rectA.height;  
    float areaB = rectB.width * rectB.height;  
    float intersectionPercentA = intersection / (areaA);  
    float intersectionPercentB = intersection / (areaB);  
    return std::max(intersectionPercentA,intersectionPercentB);  
}


int search_original_proposal(const cv::Rect& now, const std::vector<std::pair<int,cv::Rect> >& proposals, float threshold){		
	for(int i = 0; i < proposals.size();++i){
		if(cal_iou(proposals[i].second, now)>threshold){
			return proposals[i].first;
		}
	}
	return 0; 
}
