#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <iostream>
#include <stdio.h>
#include <gflags/gflags.h>
#include "handcnn.hpp"
#include "util.h"

//#define DEBUG ;

using resq = std::pair<int,cv::Rect>;


const int MinVjSize = 70;
const int MaxVjSize = 100;
const float VjScale = 1.15;
const float EnlargeScale = 1.4;

void pre_process(const cv::Mat& img, cv::Mat& res, std::vector<resq>& proposal,const std::vector<resq>& _Proposals, int scale);
void pre_process(const cv::Mat& img, cv::Mat& res, int scale);
int  detect_vj(const cv::Mat& img, resq& proposal, std::vector<cv::Rect>& tmp, int bad_label_max);
void proposal_detect(std::vector<std::pair<int,cv::Rect> >& proposal, std::vector<std::vector<cv::Rect> >& total, std::vector<std::pair<int,cv::Rect> >& _Proposals,int bad_label_max);
void check_bad(std::vector<resq>& Proposals, const std::vector<resq>& CatchProposal, std::vector<resq>& BadProposals);


//wrap this process to clear the main frame of detection
void post_process();
void subframe_detect();


DEFINE_int32(frame_control, 3 , "use to control frame rate");
DEFINE_int32(bad_label_max, 24 , "use to control frame rate");
DEFINE_int32(use_con, 3 , "use to control continuity");
DEFINE_int32(use_cnn, 0 , "control use cnn or not");
DEFINE_string(fist, "model/xml/ah1_xml/cascade_12.xml", "fist  xml path (ah1_13)");
DEFINE_string(palm, "model/xml/ah_xml/cascade_13.xml", "palm  xml path (ah_13)");
DEFINE_string(greet,"model/xml/g1_xml/cascade_13.xml" , "greet xml path (g1_15)");
DEFINE_string(six,"model/xml/s3_xml/cascade_14.xml" , "six   xml path (s3_15)");
DEFINE_string(thumb,"model/xml/t3_xml/cascade_11.xml" , "thumb xml path (t3_11)");
//DEFINE_string(thumb,"model/backup/t2_xml/cascade_10.xml" , "thumb xml path (t3_11)");
//DEFINE_string(thumb,"model/backup/k2_xml/cascade_11.xml" , "thumb xml path (t3_11)");

//fist 0 ,palm 1 ,greet 2,six 3 ,thumb 4


cv::Mat img;
cv::Mat tmp_img; // keep for debug of proposal

std::vector<std::vector<resq>> TotalA(5,std::vector<resq>());         //keep result for vj detect
std::vector<std::vector<cv::Rect>> Total(5,std::vector<cv::Rect>());  //keep result after post_process
std::vector<cv::Rect> t_fists;                                        //tmp container for subframe vj result

cv::CascadeClassifier Fist, Greet, Six, Palm, Thumb;
caffe_handcnn tmp;

int main(int argc, char* argv[])
{

	std::vector<resq> Proposals;	//proposals according to last frame proposals and this frame
	std::vector<resq> BadProposals; //labeled bad proposals
	std::vector<resq> CatchProposal;
	
	// cmd flags control	
	int frame_control(1);
	int use_cnn(0);

	gflags::ParseCommandLineFlags(&argc, &argv, true);
	
	frame_control = FLAGS_frame_control;
	use_cnn = FLAGS_use_cnn;
	
	
	if(use_cnn == 1)	
		tmp.load_model("./model/caffemodel/");
		
	std::cout<<"frame_ctrl :"<<frame_control<<std::endl;
	std::cout<<"use_cnn    :"<<use_cnn<<std::endl;


	cv::VideoCapture cap(0);
	char ch;

	// load xml
	Fist.load(FLAGS_fist); //cascade is good
	Palm.load(FLAGS_palm); //cascade is good
	Greet.load(FLAGS_greet); // 13 is best
	Six.load(FLAGS_six);//15~17 all great
	Thumb.load(FLAGS_thumb);


	// count time
	double time_past=0;
	int frame_cnt = 0;

	int T=2;
	double avg_fps=0;
	int64 s,s1,s2,s3,s4;
	double time_pre = 0, time_vj = 0, time_after = 0 ;
	double time_pf = 0,time_cap=0;
	double time_show = 0;

 
	//cv::Mat after_pre;	
	while(true)
	{
		int64 start = cv::getTickCount();
		
		cap>>img;
		
		s = cv::getTickCount();

		s1 = cv::getTickCount();
		check_bad(Proposals,CatchProposal,BadProposals);	
		time_pre+=(cv::getTickCount()-s1)/cv::getTickFrequency();
		
		if (frame_cnt%frame_control==0){
			
			std::vector<resq> pro;
			s1 = cv::getTickCount();
			// preprocess to detect proposals		
			pre_process(img, tmp_img, pro , Proposals, 4);
			time_pre+=(cv::getTickCount()-s1)/cv::getTickFrequency();

			// sequential vj detection 
			s2 = cv::getTickCount();
			proposal_detect(pro, Total, Proposals ,FLAGS_bad_label_max);
			time_vj+=(cv::getTickCount()-s2)/cv::getTickFrequency();
			
			// postprocess to raise the vj detection's precision 
			s3 = cv::getTickCount();
			post_process();
			time_after+=(cv::getTickCount()-s3)/cv::getTickFrequency();
		}

		else{
			//subframe detect to speed_up and raise vj detection's precision
			s2 = cv::getTickCount();
			subframe_detect();
			time_vj+=(cv::getTickCount()-s2)/cv::getTickFrequency();
		}
			

		CatchProposal.clear();
		
		time_pf += (cv::getTickCount()-s)/double(cv::getTickFrequency());
		

		// putText if a vj result with label >= use_con(confirm for 3 frame continuity)
		int index(0);
		for(auto hand : TotalA){
			for(resq res: hand){
				cv::Rect it = res.second;
				//cnn interface to raise the precision if needed (but slow)
				/*if(use_cnn && res.first>FLAGS_use_con){
					cv::Mat roi = img(it);
					cv::resize(roi,roi,cv::Size(20,20));
					auto fist_cnn_res = tmp.classify(roi);	
					auto k = std::distance(fist_cnn_res.begin(),std::max_element(fist_cnn_res.begin(),fist_cnn_res.end()));
					if(k){
						cv::putText(img,std::to_string(k),cv::Point(it.x,it.y),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(0,0,255),2,1);
						cv::rectangle(img,cv::Point(it.x,it.y),cv::Point(it.x+ it.width,it.y+ it.height),cv::Scalar(0,255,255),1,8,0);
					}
				}
				else*/
				{
					if(res.first>=FLAGS_use_con){
						cv::putText(img,std::to_string(index),cv::Point(it.x,it.y),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(0,0,255),2,1);
						cv::rectangle(img,cv::Point(it.x,it.y),cv::Point(it.x+ it.width,it.y+ it.height),cv::Scalar(0,255,255),1,8,0);
						CatchProposal.push_back(res);	
						std::cout<<"frame_cnt: "<<frame_cnt<<"  index: "<<index<<std::endl;
						//printf("frame_cnt: %d , index : %d \n",frame_cnt,index);	
					}
				}

			}
			index++;
		}
			
		
		//time record for per frame	
		//time_pf += (cv::getTickCount()-s)/double(cv::getTickFrequency());
		time_cap +=(s-start)/double(cv::getTickFrequency());
		time_past += (cv::getTickCount() - start) / double(cv::getTickFrequency());
		frame_cnt++;
		
		//if(time_past >= T)
		if(frame_cnt==12)
		{
			avg_fps = (double)frame_cnt / time_pf;
			//printf("average fps:%3.2f, time_pre: %f,time_vj:%f ms,time_after:%f ms, time_pf:%f ms , time_cap:%f ms, time_show: %f ms in %d second\n", avg_fps,time_pre/frame_cnt*1000, time_vj/frame_cnt*1000, time_after/frame_cnt*1000,time_pf/frame_cnt*1000,time_cap/frame_cnt*1000,time_show/frame_cnt*1000, T);
			printf("average fps:%3.2f, time_pre: %f,time_vj:%f ms,time_after:%f ms, time_pf:%f ms in %d second\n", avg_fps,time_pre/frame_cnt*1000, time_vj/frame_cnt*1000, time_after/frame_cnt*1000,time_pf/frame_cnt*1000, T);
			
			frame_cnt = 0;
			time_cap = 0;
			time_past = 0;
			time_pre = 0;
			time_pf = 0;
			time_vj = 0;
			time_after = 0;
			time_show = 0;
		}
		

		char fps_str[256] ;
		sprintf(fps_str,"%s %d","FPS : ",(int)avg_fps);
		draw_text(img,fps_str,10,50,cv::Scalar(0,255,0));
		
#ifdef DEBUG
		cv::imshow("Mask",tmp_img);
#endif
		
		s4=cv::getTickCount();		
		cv::imshow("Gesture Recognition",img);
		ch=cv::waitKey(1); //26ms to wait camera cap and dma to designed cpu buffer
		time_show+=(cv::getTickCount()-s4)/double(cv::getTickFrequency());

		if(ch==27)
		{
			break;
		}

	}
	return 0;
}



void pre_process(const cv::Mat& img, cv::Mat& res, std::vector<resq>& proposal, const std::vector<resq>& _Proposals,int scale)
{
		
	//resize to speed
	cv::Mat small(img.rows/scale,img.cols/scale,CV_8UC3);
	cv::resize(img,small,small.size(),0,0,cv::INTER_NEAREST);
	
	//convert to yuv and mask
	cv::Mat yuv(small);
	cv::cvtColor(small,yuv,36);
	vector<cv::Mat> channels;
	cv::split(yuv,channels);
	auto Y = channels.at(0);
	auto Cr = channels.at(1);
	auto Cb = channels.at(2);

	//using U & V threshold to detect roi
	for(int j= 0;j<Y.rows;j++){
		uchar* currentCr = Cr.ptr<uchar>(j);
		uchar* currentCb = Cb.ptr<uchar>(j);
		for(int i = 0;i<Y.cols;i++){
			if((currentCr[i]>137)&&(currentCr[i]<175)&&(currentCb[i]>100)&&(currentCb[i]<118));
			else{
				
				small.at<cv::Vec3b>(j,i)[0] =0; 
				small.at<cv::Vec3b>(j,i)[1] =0; 
				small.at<cv::Vec3b>(j,i)[2] =0; 
			}
		}
	}


	// resize dilate + findcontour locate roi to cv::Rect
	int next_scale(4); //this next_scale(resize procedure) is designed to expand dilate size with in the same time but scarifice accuracy
	
	cv::Mat gray;
	cv::Mat n_small(img.rows/next_scale,img.cols/next_scale,CV_8UC3);
	
	if(next_scale!=4)
		cv::resize(small,n_small,n_small.size(),0,0,cv::INTER_NEAREST);
	else
		n_small = small;

	cv::cvtColor(n_small,gray,CV_RGB2GRAY);
	cv::threshold(gray,gray,40,255,cv::THRESH_BINARY);
	
	
	int dilation_size(1);
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(dilation_size*2+1,dilation_size*2+1),cv::Point(dilation_size,dilation_size));
	cv::dilate(gray,gray,element);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(gray,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_NONE);	
	
	
	int box_min(MinVjSize/next_scale*8/10);
	for(auto i:contours){
		cv::Rect r0=cv::boundingRect(cv::Mat(i));
		if(r0.width>box_min && r0.height>box_min){
				safe_enlarge(r0,gray.size(),EnlargeScale,EnlargeScale);
				int j(0);
				j = search_original_proposal(cv::Rect(r0.x*next_scale, r0.y*next_scale, r0.width*next_scale, r0.height*next_scale),_Proposals);  	
				proposal.push_back(std::make_pair(j,cv::Rect(r0.x*next_scale, r0.y*next_scale, r0.width*next_scale, r0.height*next_scale)));
#ifdef DEBUG
				
				if(j<=4)
					std::cout<<"labels: "<<j<<" contours: "<<fabs(cv::contourArea(i,true))<< " width: "<<r0.width*4<<" height: "<<r0.height*4<<std::endl;
				cv::rectangle(small,cv::Rect(r0.x*next_scale/scale,r0.y*next_scale/scale,r0.width*next_scale/scale,r0.height*next_scale/scale),cv::Scalar(255,255,255),1);
#endif
		}
	}
	
#ifdef DEBUG
	cv::resize(small,res,img.size(),0,0,cv::INTER_CUBIC);	
#else
	res = img;	
#endif
		
}


// No proposal version
void pre_process(const cv::Mat& img, cv::Mat& res, int scale)
{
	//resize to spped
	cv::Mat small(img.rows/scale,img.cols/scale,CV_8UC3);
	cv::resize(img,small,small.size(),0,0,cv::INTER_NEAREST);
	
	//convert to yuv and mask
	cv::Mat yuv(small);
	cv::cvtColor(small,yuv,36);
	vector<cv::Mat> channels;
	cv::split(yuv,channels);
	auto Y = channels.at(0);
	auto Cr = channels.at(1);
	auto Cb = channels.at(2);
	
	for(int j= 0;j<Y.rows;j++){
		uchar* currentCr = Cr.ptr<uchar>(j);
		uchar* currentCb = Cb.ptr<uchar>(j);
		for(int i = 0;i<Y.cols;i++){
			if((currentCr[i]>137)&&(currentCr[i]<175)&&(currentCb[i]>100)&&(currentCb[i]<118));
			else{
				
				small.at<cv::Vec3b>(j,i)[0] =0; 
				small.at<cv::Vec3b>(j,i)[1] =0; 
				small.at<cv::Vec3b>(j,i)[2] =0; 
			}
		}
	}
	cv::resize(small,res,img.size(),0,0,cv::INTER_NEAREST);	
	
}


// This function is an wrapper of posp process to advance the continuity, all Variable are global
void post_process(){
	//TotalA has 5 hands count
	//examine result from viola-jones with last frame result to erase FP(false positive) 

	for(int i=0;i<5;++i){
		if(TotalA[i].size()==0 && Total[i].size()!=0){
			for(int j=0; j<Total[i].size(); ++j)
				TotalA[i].push_back(make_pair(1,Total[i][j]));
		}
		else if(TotalA[i].size()>0){
			// At first, all minus 1 
			for(int k=0; k<TotalA[i].size(); ++k)
				TotalA[i][k].first--;
			
			vector<resq> tmp;
			for(int j=0; j<Total[i].size(); ++j){
				int match(0);
				for (int k=0; k<TotalA[i].size(); ++k){
					if(is_match(TotalA[i][k].second,Total[i][j])){
						TotalA[i][k].first+=2; //if match add 2
						TotalA[i][k].second = Total[i][j];
						tmp.push_back(TotalA[i][k]);
						match=1;
						break;
					}
				}
				if(match==0)
					tmp.push_back(make_pair(1,Total[i][j]));
			}
			
			TotalA[i] = tmp;
			
			// Erase the result have label 0(count by last frame by not show in this frame)
			// All these labels are initialize with 1 in the first frame, next frame it will be sub 1 at first and if matched add 2, to ensure the continuity 
			std::sort(TotalA[i].begin(),TotalA[i].end(),[](resq A,resq B){return A.first>B.first;});
			for(auto itr = TotalA[i].begin(); itr != TotalA[i].end(); itr++){
				if(itr->first ==0)
				{
					TotalA[i].erase(itr, TotalA[i].end());
					break;
				}
			}
		}			
	}
}


void proposal_detect(std::vector<std::pair<int,cv::Rect> >& proposal, std::vector<std::vector<cv::Rect> >& total, std::vector<std::pair<int,cv::Rect> >& _Proposals,int bad_label_max){
		if(proposal.size()==0)
			return ;
		else{
			for(int i=0;i<5;++i)
				total[i].clear();
			std::vector<cv::Rect> t_total;

			_Proposals.clear();
			for(int i=0; i<proposal.size(); ++i){
				int j = 0;
				j = detect_vj(img,proposal[i],t_total,bad_label_max);
				
				// detected and keep result
				if (t_total.size()>0 && j < 5)
					for(auto t: t_total)
			 			total[j].push_back(cv::Rect(t.x+proposal[i].second.x, t.y+proposal[i].second.y, t.width, t.height));
				
				
				//save proposals for next frame use
				_Proposals.push_back(make_pair(j,proposal[i].second));
			}
			return ;
		}	
}


void check_bad(std::vector<resq>& Proposals, const std::vector<resq>& CatchProposal, std::vector<resq>& BadProposals)
{
	for(auto &pro: Proposals){
		int marked(0);
		for(auto res: CatchProposal){
			if(cal_iou_max(pro.second,res.second)>0.8){
					//std::cout<<"frame: "<<frame_cnt<<" catched proposals pro label: "<<pro.first<<"  res label: "<<res.first<<std::endl;
				marked = 1;
				break;
			}
		}
			
		if(marked==0){
				//std::cout<<"frame: "<<frame_cnt<<" uncatched proposals: "<< pro.first<<std::endl;
			if(BadProposals.size()==0)
				BadProposals.push_back(pro);
			else{
				int badmark(0);
				for(auto &bad: BadProposals){
					if(cal_iou(bad.second,pro.second)>0.7){
						bad.second=pro.second;
						bad.first++;
						if(bad.first>1000)
							bad.first=1000;
						pro.first=bad.first;
							//std::cout<<"bad: "<<bad.first<<"   pro: "<<pro.first<<"  BadSize: "<<BadProposals.size()<<std::endl;
						badmark = 1;
						break;
					}
				}
				if(badmark==0)
					BadProposals.push_back(pro);
			}
		}
	}
		
	if(BadProposals.size()>3)
		BadProposals.clear();
	
	/*		
	std::sort(BadProposals.begin(),BadProposals.end(),[](resq A,resq B){return A.second.y<B.second.y;});
	for(auto itr = BadProposals.begin(); itr != BadProposals.end(); itr++){
		if(itr != BadProposals.begin())
		{
			itr->first=5;
		}
	}*/

}



void subframe_detect()
{
	for(int j = 0;j < 5; ++j){
				std::vector<resq>& hand = TotalA[j];

				if (hand.size()==0);
				else{
					int qsize = hand.size();
					for(int i=0;i<qsize;++i){
					
						cv::Rect roi(hand[i].second);
						safe_enlarge(roi,img.size(),EnlargeScale,EnlargeScale); 	
						cv::Mat track = img(roi);
						
						cv::Mat tmp_track(track.size(),CV_8UC3);
						pre_process(track,tmp_track,4);	

						
						switch(j){
							case 0:{
								Fist.detectMultiScale(tmp_track,t_fists,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
								break;
							}
							case 1:{
								Palm.detectMultiScale(tmp_track,t_fists,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
								break;
							}
							case 2:{
								Greet.detectMultiScale(tmp_track,t_fists,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
								break;
							}
							case 3:{
								Six.detectMultiScale(tmp_track,t_fists,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
								break;
							}
							case 4:{
								Thumb.detectMultiScale(tmp_track,t_fists,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
								break;
							}
						}

						if(t_fists.size()>=1){
							auto it = t_fists[0];

							//using momentum to eliminate jitter
							hand[i].second.x = (std::max(0,it.x+roi.x)+hand[i].second.x*(FLAGS_frame_control-1))/FLAGS_frame_control;
							hand[i].second.y = (std::max(0,it.y+roi.y)+hand[i].second.y*(FLAGS_frame_control-1))/FLAGS_frame_control;
							hand[i].second.width = (std::min(img.size().width-hand[i].second.x-1,it.width)+hand[i].second.width*(FLAGS_frame_control-1))/FLAGS_frame_control;
							hand[i].second.height =(std::min(img.size().height-hand[i].second.y-1,it.height)+hand[i].second.height*(FLAGS_frame_control-1))/FLAGS_frame_control;	
							hand[i].first++;
						}
						else
							hand[i].first--;

					}
				}
			}

}



//wrapper of 5 viola-jones detector in sequence
//arrange the sequence order by proposal index
//int detect_debug_frame = 0;
int detect_vj(const cv::Mat& img, resq& proposal, std::vector<cv::Rect>& tmp, int bad_label_max){
	if(proposal.first>=bad_label_max)
		return 5;
	else{
		
		/*
		cv::Mat _roi = img(proposal.second).clone();
		cv::imshow("pose",_roi);
		vector<cv::Mat> channels;
		cv::split(_roi,channels);
		for(int i=0;i<3;++i)	
			cv::equalizeHist(channels[i],channels[i]);
		cv::merge(channels,_roi);
		cv::imshow("equa pose",_roi);
		cv::waitKey(1);
		
		
		string a="save/d_pic_";
		string b=".jpg";	

		cv::Mat tmp_roi = img(proposal.second);
		
		if(proposal.first==3 && detect_debug_frame<300){
			std::cout<<"detect_debug_frame: "<<detect_debug_frame<<"  save!"<<std::endl;
			a+=std::to_string(detect_debug_frame);
			a+=b;
			cv::imwrite(a,tmp_roi);
			detect_debug_frame++;
		}
		*/
		
		cv::Mat tmp_roi = img(proposal.second);

		if(proposal.first==0){
			int k(5);
			for(int j=0; j<5; ++j){
				switch(j){
					case 0:{
						Fist.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 0;
						break;
					}
					case 1:{
						Palm.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 1;
						break;
					}
					case 2:{
						Greet.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 2;
						break;
					}
					case 3:{
						Six.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 3;
						break;
					}
					case 4:{
						Thumb.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize-10,MinVjSize-10),cv::Size(MaxVjSize+10,MaxVjSize+10));   //thumb is hard to search
						//Thumb.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));   //thumb is hard to search
						k = 4;
						break;
					}
				}
				if(tmp.size()>0){
					proposal.first = k;
					return k;
				}
			}
			proposal.first = k;
			return k;
		}
		if(proposal.first==1){
			int k(5);
			for(int j=0; j<5; ++j){
				switch(j){
					case 4:{
						Fist.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 0;
						break;
					}
					case 0:{
						Palm.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 1;
						break;
					}
					case 1:{
						Greet.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 2;
						break;
					}
					case 2:{
						Six.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 3;
						break;
					}
					case 3:{
						//Thumb.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						Thumb.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize-10,MinVjSize-10),cv::Size(MaxVjSize+10,MaxVjSize+10));   //thumb is hard to search
						k = 4;
						break;
					}
				}
				if(tmp.size()>0){
					proposal.first = k;
					return k;
				}
			}
			proposal.first = 5;
			return 5;
		}
		if(proposal.first==2){
			int k(5);
			for(int j=0; j<5; ++j){
				switch(j){
					case 3:{
						Fist.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 0;
						break;
					}
					case 4:{
						Palm.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 1;
						break;
					}
					case 0:{
						Greet.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 2;
						break;
					}
					case 1:{
						Six.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 3;
						break;
					}
					case 2:{
						//Thumb.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						Thumb.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize-10,MinVjSize-10),cv::Size(MaxVjSize+10,MaxVjSize+10));   //thumb is hard to search
						k = 4;
						break;
					}
				}
				if(tmp.size()>0){
					proposal.first = k;
					return k;
				}
			}
			proposal.first = 5;
			return 5;
		}
		if(proposal.first==3){
			int k(5);
			for(int j=0; j<5; ++j){
				switch(j){
					case 2:{
						Fist.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 0;
						break;
					}
					case 3:{
						Palm.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 1;
						break;
					}
					case 4:{
						Greet.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 2;
						break;
					}
					case 0:{
						Six.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 3;
						break;
					}
					case 1:{
						//Thumb.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						Thumb.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize-10,MinVjSize-10),cv::Size(MaxVjSize+10,MaxVjSize+10));   //thumb is hard to search
						k = 4;
						break;
					}
				}
				if(tmp.size()>0){
					proposal.first = k;
					return k;
				}
			}
			proposal.first = 5;
			return 5;
		}
		if(proposal.first==4){
			int k(5);
			for(int j=0; j<5; ++j){
				switch(j){
					case 1:{
						Fist.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 0;
						break;
					}
					case 2:{
						Palm.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 1;
						break;
					}
					case 3:{
						Greet.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 2;
						break;
					}
					case 4:{
						Six.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 3;
						break;
					}
					case 0:{
						//Thumb.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						Thumb.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize-10,MinVjSize-10),cv::Size(MaxVjSize+10,MaxVjSize+10));   //thumb is hard to search
						k = 4;
						break;
					}
				}
				if(tmp.size()>0){
					proposal.first = k;
					return k;
				}
			}
			proposal.first = 5;
			return 5;
		}
		else{
			int k(5);
			for(int j=0; j<5; ++j){
				switch(j){
					case 0:{
						Fist.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 0;
						break;
					}
					case 1:{
						Palm.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 1;
						break;
					}
					case 2:{
						Greet.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 2;
						break;
					}
					case 3:{
						Six.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						k = 3;
						break;
					}
					case 4:{
						//Thumb.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize,MinVjSize),cv::Size(MaxVjSize,MaxVjSize));
						Thumb.detectMultiScale(tmp_roi,tmp,VjScale,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(MinVjSize-10,MinVjSize-10),cv::Size(MaxVjSize+10,MaxVjSize+10));   //thumb is hard to search
						k = 4;
						break;
					}
				}
				if(tmp.size()>0){
					proposal.first = k;
					return k;
				}
			}
			proposal.first++;
			return proposal.first++;
		}
	}
}



