#pragma once

#include <ostream>
#include <vector>

#include "Conf/Conf.h"

MAA_SUPPRESS_CV_WARNINGS_BEGIN
#include <opencv2/features2d.hpp>
MAA_SUPPRESS_CV_WARNINGS_END

#include "VisionBase.h"
#include "VisionTypes.h"

MAA_VISION_NS_BEGIN

class FeatureMatcher : public VisionBase
{
public:
    struct Result
    {
        cv::Rect box {};
        int count = 0;

        json::value to_json() const
        {
            json::value root;
            root["box"] = json::array({ box.x, box.y, box.width, box.height });
            root["count"] = count;
            return root;
        }
    };

    using ResultsVec = std::vector<Result>;

public:
    void set_template(std::shared_ptr<cv::Mat> templ) { template_ = std::move(templ); }
    void set_param(FeatureMatcherParam param) { param_ = std::move(param); }

    ResultsVec analyze() const;

private:
    ResultsVec foreach_rois(const cv::Mat& templ) const;
    std::pair<std::vector<cv::KeyPoint>, cv::Mat> detect(const cv::Mat& image, bool green_mask) const;
    std::pair<std::vector<cv::KeyPoint>, cv::Mat> detect(const cv::Mat& image, const cv::Rect& roi) const;
    cv::FlannBasedMatcher create_matcher(const std::vector<cv::KeyPoint>& keypoints, const cv::Mat& descriptors) const;

    ResultsVec match(cv::FlannBasedMatcher& matcher, const std::vector<cv::KeyPoint>& keypoints_1,
                     const cv::Rect& roi_2) const;
    void draw_result(const cv::Mat& templ, const std::vector<cv::KeyPoint>& keypoints_1, const cv::Rect& roi,
                     const std::vector<cv::KeyPoint>& keypoints_2, const std::vector<cv::DMatch>& good_matches,
                     ResultsVec& results) const;
    void filter(ResultsVec& results, int count) const;

    FeatureMatcherParam param_;
    std::shared_ptr<cv::Mat> template_;
};

MAA_VISION_NS_END

MAA_NS_BEGIN

inline std::ostream& operator<<(std::ostream& os, const MAA_VISION_NS::FeatureMatcher::Result& res)
{
    os << res.to_json().to_string();
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const MAA_VISION_NS::FeatureMatcher::ResultsVec& resutls)
{
    json::array root;
    for (const auto& res : resutls) {
        root.emplace_back(res.to_json());
    }
    os << root.to_string();
    return os;
}

MAA_NS_END