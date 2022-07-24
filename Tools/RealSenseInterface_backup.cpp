#include "RealSenseInterface.h"
#include <glog/logging.h>
#include <librealsense2/rs.h>
#include <librealsense2/rs_advanced_mode.h>
#include <functional>

#ifdef WITH_REALSENSE
RealSenseInterface::RealSenseInterface(int inWidth, int inHeight, int inFps)
    : width(inWidth), height(inHeight), fps(inFps), device(nullptr), initSuccessful(true) {
  LOG(INFO) << "Initialize RealSense...";
  devices_list = ctx.query_devices();
  size_t device_count = devices_list.size();
  if (!device_count)
  {
    LOG(ERROR) << "No device detected. Is it plugged in?\n";
  }

  bool device_found = false;
  for (auto&& dev : devices_list) {
    // finding a device of D400 product line for working with HDR feature
    if (dev.supports(RS2_CAMERA_INFO_PRODUCT_LINE) &&
        std::string(dev.get_info(RS2_CAMERA_INFO_PRODUCT_LINE)) == "D400") {
      device = &dev;
      device_found = true;
      break;
    }
  }
  if (!device_found) {
    LOG(ERROR) << "No device from D400 product line detected. Is it plugged in?\n";
  } else {
    LOG(INFO) << "Device found";
  }

  cfg.enable_stream(RS2_STREAM_DEPTH);
  cfg.enable_stream(RS2_STREAM_COLOR);
  //  dev->enable_stream(rs::stream::depth, width, height, rs::format::z16, fps);
  //  dev->enable_stream(rs::stream::color, width, height, rs::format::rgb8, fps);

  latestDepthIndex.assign(-1);
  latestRgbIndex.assign(-1);

  for (int i = 0; i < numBuffers; i++) {
    uint8_t* newImage = (uint8_t*)calloc(width * height * 3, sizeof(uint8_t));
    rgbBuffers[i] = std::pair<uint8_t*, int64_t>(newImage, 0);
  }

  for (int i = 0; i < numBuffers; i++) {
    uint8_t* newDepth = (uint8_t*)calloc(width * height * 2, sizeof(uint8_t));
    uint8_t* newImage = (uint8_t*)calloc(width * height * 3, sizeof(uint8_t));
    frameBuffers[i] = std::pair<std::pair<uint8_t*, uint8_t*>, int64_t>(
        std::pair<uint8_t*, uint8_t*>(newDepth, newImage), 0);
  }

//  setAutoExposure(true);
//  LOG(INFO) << "Here...";
//  setAutoWhiteBalance(true);

  rgbCallback = new RGBCallback(lastRgbTime, latestRgbIndex, rgbBuffers);

  depthCallback =
      new DepthCallback(lastDepthTime, latestDepthIndex, latestRgbIndex, rgbBuffers, frameBuffers);

  //  dev->set_frame_callback(rs::stream::depth, *depthCallback);
  //  dev->set_frame_callback(rs::stream::color, *rgbCallback);
  cfg.enable_stream(RS2_STREAM_DEPTH);
  cfg.enable_stream(RS2_STREAM_COLOR);

  //  dev->start();
  LOG(INFO) << "Start pipe...";
  pipe.start(cfg);
}

RealSenseInterface::~RealSenseInterface() {
  if (initSuccessful) {
    //    dev->stop();
    pipe.stop();
    for (int i = 0; i < numBuffers; i++) {
      free(rgbBuffers[i].first);
    }

    for (int i = 0; i < numBuffers; i++) {
      free(frameBuffers[i].first.first);
      free(frameBuffers[i].first.second);
    }

    delete rgbCallback;
    delete depthCallback;
  }
}

void RealSenseInterface::setAutoExposure(bool value) {
  //  dev->set_option(rs::option::color_enable_auto_exposure, value);
  device->query_sensors().front().set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, 1);
}

void RealSenseInterface::setAutoWhiteBalance(bool value) {
  //  dev->set_option(rs::option::color_enable_auto_white_balance, value);
  device->query_sensors().front().set_option(RS2_OPTION_ENABLE_AUTO_WHITE_BALANCE, 1);
}

bool RealSenseInterface::getAutoExposure() {
  //  return dev->get_option(rs::option::color_enable_auto_exposure);
  return device->query_sensors().front().get_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE);
}

bool RealSenseInterface::getAutoWhiteBalance() {
  //  return dev->get_option(rs::option::color_enable_auto_white_balance);
  return device->query_sensors().front().get_option(RS2_OPTION_ENABLE_AUTO_WHITE_BALANCE);
}
#else

RealSenseInterface::RealSenseInterface(int inWidth, int inHeight, int inFps)
    : width(inWidth), height(inHeight), fps(inFps), initSuccessful(false) {
  errorText = "Compiled without Intel RealSense library";
}

RealSenseInterface::~RealSenseInterface() {}

void RealSenseInterface::setAutoExposure(bool value) {}

void RealSenseInterface::setAutoWhiteBalance(bool value) {}

bool RealSenseInterface::getAutoExposure() {
  return false;
}

bool RealSenseInterface::getAutoWhiteBalance() {
  return false;
}
#endif
