//
// Created by root on 10/30/17.
//
#include <string>
#include <vector>

#include "handcnn.hpp"

caffe_handcnn::~caffe_handcnn(void)
{
    delete HNet_;
}


int caffe_handcnn::load_model(const std::string &proto_model_dir)
{

    Caffe::set_mode(Caffe::CPU);

    HNet_=new Net<float>((proto_model_dir + "/det.prototxt"), caffe::TEST);
    HNet_->CopyTrainedLayersFrom(proto_model_dir + "/det.caffemodel");

    return 0;
}

std::vector<float> caffe_handcnn::classify(cv::Mat& img)
{
    cv::Mat working_img;
    float alpha=0.00392156;
    //float mean=127.5;


    img.convertTo(working_img, CV_32FC3);

    working_img=(working_img)*alpha;

    int img_h=working_img.rows;
    int img_w=working_img.cols;
    //working_img=working_img.t();
    cv::cvtColor(working_img, working_img, cv::COLOR_BGR2RGB);


    Blob<float>* input_blob = HNet_->input_blobs()[0];
    input_blob->Reshape(1, 3, img_h, img_w);

    HNet_->Reshape();

    std::vector<cv::Mat> input_channels;
    WrapInputLayer(&input_channels);

    cv::split(working_img, input_channels);

    HNet_->Forward();

    Blob<float>* output_layer = HNet_->output_blobs()[0];
    const float* begin = output_layer->cpu_data();
    const float* end = begin+output_layer->channels();
    return std::vector<float>(begin,end);
}


void caffe_handcnn:: WrapInputLayer(std::vector<cv::Mat>* input_channels){
    Blob<float>* input_layer = HNet_->input_blobs()[0];

    int width = input_layer->width();
    int height = input_layer->height();
    float* input_data =  input_layer->mutable_cpu_data();
    for (int i=0; i< input_layer->channels();++i){
        cv::Mat channel(height,width,CV_32FC1,input_data);
        input_channels->push_back(channel);
        input_data += width*height;
    }
}
