#pragma once
#include "ICamera.hpp"
#include <vector>
#include <opencv2/core.hpp>

class MockCamera : public ICamera {
public:
    MockCamera();  // no data passed in
    bool open(int index) override;
    [[nodiscard]] bool isOpened() const override;
    bool read(cv::Mat& frame) override;
    void release() override;
    double get(cv::VideoCaptureProperties props) override;
    [[nodiscard]] std::string getBackendName() const override;

private:
    std::vector<cv::Mat> frames_;
    size_t index_ = 0;
    bool opened_ = false;

    void generateTestFrames();  // internal helper
};
