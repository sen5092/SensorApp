

#pragma once

#include "IDataSource.hpp"

#include <string>
#include <unordered_map>
#include <random>
#include <opencv2/core.hpp>

class HardwareDataSource : public IDataSource {

    private:
        static void logCameraInfo();
        // --- Internal helpers ---
        static bool grabFrameToJpeg(const std::string& outfile);
        static bool grabFrame(cv::Mat& frame);  // internal helper

    public:

        // --- Construction ---
        HardwareDataSource();

        // --- Data Generation ---
        //double generate(const std::string &metricName);

        std::unordered_map<std::string, double> readAll() override;

        [[nodiscard]] static bool ensureCameraAuthorized() const;



};