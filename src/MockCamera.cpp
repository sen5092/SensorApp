
// NOLINTBEGIN

#include "MockCamera.hpp"


#include <opencv2/videoio.hpp>
#include <opencv2/core/mat.hpp>
#include <vector>
#include <string>



MockCamera::MockCamera() {
    generateTestFrames();
}

void MockCamera::generateTestFrames() {
    for (int i = 0; i < 10; ++i) {
        // Simple fake image filled with a scalar color
        cv::Mat frame(480, 640, CV_8UC3, cv::Scalar(i * 20, i * 10, i * 5));
        frames_.push_back(frame);
    }
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
bool MockCamera::open(int index) {
    opened_ = true;
    return true;
}

bool MockCamera::isOpened() const {
    return opened_;
}

bool MockCamera::read(cv::Mat& frame) {
    if (!opened_ || index_ >= frames_.size()) {
        return false;
    }
    frame = frames_[index_++].clone();
    return true;
}

void MockCamera::release() {
    opened_ = false;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
double MockCamera::get(cv::VideoCaptureProperties props) {
    return 0.0; // Dummy implementation
}

std::string MockCamera::getBackendName() const {
    return "MockCameraBackend";
}

// NOLINTEND