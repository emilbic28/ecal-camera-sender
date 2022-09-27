#include <iostream>
#include <string>
#include <thread>

#include <QCameraInfo>
#include <QtWidgets>

#include "camera.h"

Camera::Camera(eCAL::protobuf::CPublisher<foxglove::CompressedImage>& publisher, std::string& camera_name) :
  m_publisher(publisher), camera_name_(camera_name) ,photosTaken(0)
{
  const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
  if (cameras.isEmpty())
  {
    std::cerr << "No available camera to use" << std::endl;
    return;
  }

  for (const QCameraInfo &cameraInfo : cameras) {
      if (cameraInfo.deviceName().toStdString() == camera_name_)
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
  m_camera.reset(new QCamera(cameraInfo));
  m_camera.data()->setCaptureMode(QCamera::CaptureStillImage);

  m_imageCapture.reset(new QCameraImageCapture(m_camera.data()));
  m_imageCapture.data()->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);

  // connect
  QObject::connect(m_imageCapture.data(), &QCameraImageCapture::imageCaptured, [=] (int id, QImage img){
    ++photosTaken;
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
    m_publisher.Send(compressedImageProto);

    std::cout << "Sent photo number: " << photosTaken << " with size: " << compressedImageProto.ByteSizeLong() << std::endl;

    // limit the number of photos sent
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    emit photoSentSignal();
  });

  QObject::connect(m_imageCapture.data(), &QCameraImageCapture::readyForCaptureChanged, [=] (bool state) {
     if(state == true) {
         m_camera.data()->searchAndLock();
         m_imageCapture.data()->capture();
         m_camera.data()->unlock();
     }
  });

  QObject::connect(this, &Camera::photoSentSignal, [this](){
    if (isReadyForCapture())
    {
      m_camera.data()->searchAndLock();
      m_imageCapture.data()->capture();
      m_camera.data()->unlock();
    }
  });

  m_camera.data()->start();
}

void Camera::takePhoto()
{
  m_imageCapture.data()->capture();
}

bool Camera::isReadyForCapture()
{
  return m_imageCapture.data()->isReadyForCapture();
}

