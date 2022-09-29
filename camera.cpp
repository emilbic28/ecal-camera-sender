#include <iostream>
#include <string>
#include <thread>

#include <QCameraInfo>
#include <QtWidgets>

#include "camera.h"

Camera::Camera(eCAL::protobuf::CPublisher<foxglove::CompressedImage>& publisher, std::string& cameraName, uint16_t width, uint16_t height) :
  publisher_(publisher), cameraName_(cameraName), width_(width), height_(height), photosTaken_(0)
{
  const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
  if (cameras.isEmpty())
  {
    std::cerr << "No available camera to use" << std::endl;
    return;
  }

  for (const QCameraInfo &cameraInfo : cameras) {
      if (cameraInfo.deviceName().toStdString() == cameraName_)
      {
        setCamera(cameraInfo);
        return;
      }
  }

  std::cout << "Selected camera not found, setting to default camera." << std::endl;
  setCamera(QCameraInfo::defaultCamera());
}

Camera::~Camera()
{
}

void Camera::setCamera(const QCameraInfo& cameraInfo)
{
  camera_.reset(new QCamera(cameraInfo));
  camera_.data()->setCaptureMode(QCamera::CaptureStillImage);
  camera_.data()->load();

  imageCapture_.reset(new QCameraImageCapture(camera_.data()));
  imageCapture_.data()->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);


  if (isGivenResolutionSupported())
  {
    QImageEncoderSettings imageSettings;
    imageSettings.setCodec("image/jpg");
    imageSettings.setResolution(width_, height_);
    imageCapture_.data()->setEncodingSettings(imageSettings);
  }
  else
  {
    std::cout << "Given resolution is not supported from the camera, setting default resolution. List of supported resolutions: ";
    auto resolutions = imageCapture_.data()->supportedResolutions();
    for (const auto& it : resolutions)
    {
      std::cout << it.width() << "x" << it.height() << ", ";
    }
    std::cout << std::endl;
  }

  // connect
  QObject::connect(imageCapture_.data(), &QCameraImageCapture::imageCaptured, [=] (int id, QImage img){
    ++photosTaken_;
    QByteArray buf;
    QBuffer buffer(&buf);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "JPG");
    std::string protoData(buf.constData(), buf.length());
    buffer.close();

    // create and send protobuf
    foxglove::CompressedImage compressedImageProto;
    compressedImageProto.set_data(protoData);
    compressedImageProto.set_format("jpg");
    publisher_.Send(compressedImageProto);

    std::cout << "Sent photo number: " << photosTaken_ << " with size: " << compressedImageProto.ByteSizeLong() << std::endl;

    // limit the number of photos sent
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    emit photoSentSignal();
  });

  QObject::connect(imageCapture_.data(), &QCameraImageCapture::readyForCaptureChanged, [=] (bool state) {
     if(state == true) {
         camera_.data()->searchAndLock();
         imageCapture_.data()->capture();
         camera_.data()->unlock();
     }
  });

  QObject::connect(this, &Camera::photoSentSignal, [this](){
    if (isReadyForCapture())
    {
      camera_.data()->searchAndLock();
      imageCapture_.data()->capture();
      camera_.data()->unlock();
    }
  });

  camera_.data()->start();
}

bool Camera::isGivenResolutionSupported()
{
  QList<QSize> resolutions = imageCapture_.data()->supportedResolutions();
  QSize givenResolution(width_, height_);

  return resolutions.contains(givenResolution);
}

bool Camera::isReadyForCapture()
{
  return imageCapture_.data()->isReadyForCapture();
}

