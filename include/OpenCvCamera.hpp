// include/OpenCVCamera.hpp
#pragma once
#include "ICamera.hpp"
#include <opencv2/videoio.hpp>

class OpenCvCamera : public ICamera {
public:
    bool open(int index) override {
        return cap_.open(index);
    }

    [[nodiscard]] bool isOpened() const override {
        return cap_.isOpened();
    }

    bool read(cv::Mat& frame) override {
        return cap_.read(frame);
    }

    void release() override {
        cap_.release();
    }

    double get(cv::VideoCaptureProperties props) override {
        return cap_.get(props);
    }

    [[nodiscard]] std::string getBackendName() const override {
        return cap_.getBackendName();
    }

private:
    cv::VideoCapture cap_;
};
