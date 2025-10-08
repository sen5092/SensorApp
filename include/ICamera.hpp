// include/ICamera.hpp
#pragma once
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

class ICamera {
public:
    ICamera() = default;
    virtual ~ICamera() = default;

    ICamera(const ICamera&) = delete;
    ICamera& operator=(const ICamera&) = delete;
    ICamera(ICamera&&) = delete;
    ICamera& operator=(ICamera&&) = delete;


    virtual bool open(int index) = 0;
    [[nodiscard]] virtual bool isOpened() const = 0;
    virtual bool read(cv::Mat& frame) = 0;
    virtual void release() = 0;
    virtual double get(cv::VideoCaptureProperties) = 0;
    [[nodiscard]] virtual std::string getBackendName() const = 0;

};