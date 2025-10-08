/**
 * @file HardwareDataSource.hpp
 * @brief Defines the HardwareDataSource class responsible for real sensor (camera) input.
 *
 * The `HardwareDataSource` class provides a concrete implementation of a data source
 * that interfaces directly with hardware sensors using OpenCV. It captures frames from
 * a connected camera, extracts image metadata (such as resolution, channel count, and
 * brightness), and prepares the data for the Sensor subsystem.
 *
 * This class is conditionally compiled when `USE_OPENCV` is enabled and is intended to
 * serve as the real-world counterpart to the `SimulationDataSource` used for development
 * and testing.
 *
 * ### Responsibilities:
 * - Initialize and manage an OpenCV `cv::VideoCapture` device.
 * - Verify and request camera access authorization (`ensureCameraAuthorized()`).
 * - Read and process video frames for analysis or transmission.
 * - Extract relevant metadata including frame width, height, channels, and mean intensity.
 * - Provide the acquired data in a structured format for downstream components.
 *
 * @date Created October 2025
 * @author Scott Novak
 */


#pragma once

#include "ICamera.hpp"
#include <string>
#include <unordered_map>
#include <random>
#include <opencv2/core.hpp>

class HardwareDataSource {

    private:
        // --- Internal helpers ---
        void logCameraInfo();
        bool grabFrameToJpeg(const std::string& outfile);
        bool grabFrame(cv::Mat& frame);  // internal helper

        std::shared_ptr<ICamera> camera_;

    public:

        // --- Construction ---
        explicit HardwareDataSource(std::shared_ptr<ICamera> camera);

        // --- Data Generation ---
        std::unordered_map<std::string, double> readAll();

};