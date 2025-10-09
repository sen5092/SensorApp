#pragma once
#include <opencv2/core/mat.hpp>
#include <opencv2/videoio.hpp>
#include <vector>
#include <string>

/**
 * @brief A lightweight mock implementation of an OpenCV camera.
 *
 * Provides a predictable sequence of generated test frames
 * for simulation or unit testing, matching the same interface
 * as a real cv::VideoCapture-backed camera.
 */
class MockCamera : public ICamera {
public:
    MockCamera() { generateTestFrames(); }

    bool open(int index = 0) {
        (void)index; // unused
        opened_ = true;
        return true;
    }

    bool isOpened() const { return opened_; }

    bool read(cv::Mat& frame) {
        if (!opened_ || index_ >= frames_.size()){
            return false;
        }
        frame = frames_[index_++].clone();
        return true;
    }

    void release() { opened_ = false; }

    double get(cv::VideoCaptureProperties props) {
        (void)props; // unused
        return 0.0;
    }

    std::string getBackendName() const {
        return "MockCameraBackend";
    }

private:
    void generateTestFrames() {
        frames_.reserve(10);
        for (int i = 0; i < 10; ++i) {
            // height, width, channels, color (BGR)
            // brightness = 20
            // status = 1
            cv::Mat frame(480, 640, CV_8UC3, cv::Scalar(i * 20, i * 10, i * 5));
            frames_.push_back(frame);
        }
    }

    std::vector<cv::Mat> frames_;
    size_t index_ = 0;
    bool opened_ = false;
};
