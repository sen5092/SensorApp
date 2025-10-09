/**
 * @file    HardwareDataSource.cpp
 * @author  Scott Novak
 * @date    2025-10-01
 * @brief   Implementation of HardwareDataSource for capturing frames with OpenCV.
 *
 * @details
 * Defines the HardwareDataSource class methods, including frame capture (`grabFrame`),
 * metadata extraction (`readAll`), and snapshot writing (`grabFrameToJpeg`).
 * This component is used by the Sensor class to provide image-based readings.
 */
#include "HardwareDataSource.hpp"
#include "Logger.hpp"
#include "ICamera.hpp"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/types.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <utility>  // std::move


HardwareDataSource::HardwareDataSource(std::shared_ptr<ICamera> camera)
    : camera_(std::move(camera)) {
        logCameraInfo();
}


std::unordered_map<std::string, double> HardwareDataSource::readAll() {

    Logger::instance().info("Reading from the hardware.");

    std::unordered_map<std::string, double> values;
    cv::Mat frame;

    if (grabFrame(frame)) {
        values["frame_width"]  = static_cast<double>(frame.cols);
        values["frame_height"] = static_cast<double>(frame.rows);
        values["channels"]     = static_cast<double>(frame.channels());
        values["brightness"]   = cv::mean(frame)[0]; // simple metric
        values["frame_status"] = 1.0;

        // snapshot saved automatically for debugging
        cv::imwrite("last_frame.jpg", frame);
    } else {
        values["frame_width"] = 0.0;
        values["frame_status"] = 0.0;
    }

    return values;
}

bool HardwareDataSource::grabFrameToJpeg(const std::string& outfile) {
    cv::Mat frame;
    if (!grabFrame(frame)) {
        return false;
    }

    if (!cv::imwrite(outfile, frame)) {
        Logger::instance().error("Failed to write " + outfile);
        return false;
    }

    Logger::instance().info("Saved snapshot to " + outfile);
    return true;
}

bool HardwareDataSource::grabFrame(cv::Mat& frame) {

    if(!camera_ || !camera_->isOpened()) {
        Logger::instance().error("Camera is not opened.");
        return false;
    }

    if (!camera_->read(frame)) {
        Logger::instance().error("Failed to grab frame from camera.");
        return false;
    }

    if (frame.empty()) {
        Logger::instance().error("Captured frame is empty.");
        return false;
    }

    return true;
}


void HardwareDataSource::logCameraInfo() {

    cv::Mat frame;
    if(grabFrame(frame))
    {
        Logger::instance().info("Capturing test frame...");
        Logger::instance().debug("Backend: " + camera_->getBackendName());

        Logger::instance().debug("Captured a frame at resolution: " +
                                std::to_string(frame.cols) + "x" + std::to_string(frame.rows));

        cv::Scalar meanColor = cv::mean(frame);
        Logger::instance().debug("Mean pixel intensity: [" +
                                std::to_string(meanColor[0]) + ", " +
                                std::to_string(meanColor[1]) + ", " +
                                std::to_string(meanColor[2]) + "]");
    }
}

