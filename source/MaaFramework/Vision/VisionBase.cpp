#include "VisionBase.h"

#include "Utils/NoWarningCV.hpp"

#include "Option/GlobalOptionMgr.h"
#include "Utils/ImageIo.h"
#include "Utils/Logger.h"
#include "Utils/StringMisc.hpp"
#include "Utils/Time.hpp"
#include "VisionUtils.hpp"

MAA_VISION_NS_BEGIN

VisionBase::VisionBase(cv::Mat image, std::string name)
    : image_(std::move(image))
    , name_(std::move(name))
{
    init_debug_draw();
}

cv::Mat VisionBase::image_with_roi(const cv::Rect& roi) const
{
    cv::Rect roi_corrected = correct_roi(roi, image_);
    return image_(roi_corrected);
}

cv::Mat VisionBase::draw_roi(const cv::Rect& roi, const cv::Mat& base) const
{
    cv::Mat image_draw = base.empty() ? image_.clone() : base;
    const cv::Scalar color(0, 255, 0);

    cv::putText(
        image_draw,
        name_,
        cv::Point(5, image_.rows - 5),
        cv::FONT_HERSHEY_SIMPLEX,
        1,
        color,
        2);

    cv::rectangle(image_draw, roi, color, 1);
    std::string flag = std::format("ROI: [{}, {}, {}, {}]", roi.x, roi.y, roi.width, roi.height);
    cv::putText(
        image_draw,
        flag,
        cv::Point(roi.x, roi.y - 5),
        cv::FONT_HERSHEY_PLAIN,
        1.2,
        color,
        1);

    return image_draw;
}

void VisionBase::handle_draw(const cv::Mat& draw)
{
    draws_.emplace_back(draw);

    if (save_draw_) {
        draw_paths_.emplace_back(save_image(draw));
    }
}

void VisionBase::handle_index(size_t total, int index)
{
    auto index_opt = pythonic_index(total, index);
    if (!index_opt) {
        preferred_index_ = SIZE_MAX;
        return;
    }

    preferred_index_ = *index_opt;
}

std::filesystem::path VisionBase::save_image(const cv::Mat& image) const
{
    std::string filename = std::format("{}_{}.png", name_, format_now_for_filename());
    auto filepath = GlobalOptionMgr::get_instance().log_dir() / "vision" / path(filename);
    MAA_NS::imwrite(filepath, image);
    LogDebug << "save image to" << filepath;
    return filepath;
}

void VisionBase::init_debug_draw()
{
    save_draw_ = GlobalOptionMgr::get_instance().save_draw();

    if (save_draw_) {
        debug_draw_ = true;
    }
    else {
#ifdef MAA_DEBUG
        debug_draw_ = true;
#else
        debug_draw_ = false;
#endif
    }
}

MAA_VISION_NS_END