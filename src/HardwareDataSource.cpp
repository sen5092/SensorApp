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

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <unordered_map>
#include <string>
#include <iostream>
#include "HardwareDataSource.hpp"
#include "Logger.hpp"


std::unordered_map<std::string, double> HardwareDataSource::readAll() {

    Logger::instance().info("Reading from the hardware.");

    std::unordered_map<std::string, double> values;
    cv::Mat frame;

    if (grabFrame(frame)) {
        values["frame_width"]  = static_cast<double>(frame.cols);
        values["frame_height"] = static_cast<double>(frame.rows);
        values["channels"]     = static_cast<double>(frame.channels());
        values["brightness"]   = cv::mean(frame)[0]; // simple metric

        // snapshot saved automatically for debugging
        cv::imwrite("last_frame.jpg", frame);
        values["frame_status"] = 1.0;
    } else {
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
        std::cerr << "[OpenCV] Failed to write " << outfile << "\n";
        return false;
    }

    std::cout << "[OpenCV] Saved " << outfile
              << " (" << frame.cols << "x" << frame.rows << ")\n";
    return true;
}

bool HardwareDataSource::grabFrame(cv::Mat& frame) {
    cv::VideoCapture cap(0, cv::CAP_AVFOUNDATION);
    if (!cap.isOpened()) {
        std::cerr << "[OpenCV] Could not open default camera.\n";
        return false;
    }

    if (!cap.read(frame) || frame.empty()) {
        std::cerr << "[OpenCV] Failed to read a frame.\n";
        return false;
    }

    return true;
}

