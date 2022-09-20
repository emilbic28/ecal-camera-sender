#include <ios>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include <QMediaService>
#include <QMediaRecorder>
#include <QCameraViewfinder>
#include <QCameraInfo>
#include <QMediaMetaData>

#include <QMessageBox>
#include <QPalette>

#include <QtWidgets>

#include "Camera.h"

Camera::Camera(eCAL::protobuf::CPublisher<foxglove::CompressedImage>& publisher) : m_publisher(publisher), photosTaken(0)
{
  const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
  for (const QCameraInfo &cameraInfo : cameras) {
      if (cameraInfo.deviceName() == "/dev/video0")
      {
        setCamera(cameraInfo);
      }
  }
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
    compressedImageProto.set_frame_id(std::to_string(photosTaken));
    m_publisher.Send(compressedImageProto);

    std::cout << "Sent photo number: " << photosTaken << " with size: " << compressedImageProto.ByteSizeLong() << " and frame_id " << compressedImageProto.frame_id() << std::endl;

    // limit the number of photos sent
    // std::this_thread::sleep_for(std::chrono::milliseconds(30));
    emit photoSentSignal();
  });

  QObject::connect(m_imageCapture.data(), &QCameraImageCapture::readyForCaptureChanged, [=] (bool state) {
     if(state == true) {
         std::cout << "EAB: readyForCaptureChanged\n";
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

