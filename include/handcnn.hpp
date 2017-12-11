//
// Created by root on 10/30/17.
//

#ifndef FACE_DEMO_MASTER_HANDCNN_HPP
#define FACE_DEMO_MASTER_HANDCNN_HPP

#include <string>
#include <vector>

#include <caffe/caffe.hpp>
#include <caffe/layers/memory_data_layer.hpp>

#include <opencv2/opencv.hpp>

using namespace caffe;


// A wrapper class of caffe classify, include load_model(), classify() 
// public load_model() : Load .caffemodel in assigned dir
// public classify(): Forward Net to get the classify reshult
// protected WrapInputLayer() : Reshape the input img(cv::Mat) into Net.input_layer(caffe::Blob) 
// private data HNet(caffe:Net)

class caffe_handcnn{
public:
    caffe_handcnn()=default;

    int load_model(const std::string& model_dir);

    std::vector<float> classify(cv::Mat& img);

    ~caffe_handcnn();

protected:

    void WrapInputLayer(std::vector<cv::Mat>* input_channels);

private:

    Net<float> * HNet_;

};



#endif //FACE_DEMO_MASTER_HANDCNN_HPP
