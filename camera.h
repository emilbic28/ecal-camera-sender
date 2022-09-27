#pragma once

#include <QObject>
#include <QCamera>
#include <QCameraImageCapture>
#include <QScopedPointer>

#include <ecal/msg/protobuf/publisher.h>

#include "compressed_image.pb.h"

class Camera : public QObject
{
  Q_OBJECT
public:
  Camera(eCAL::protobuf::CPublisher<foxglove::CompressedImage>& publisher, std::string& camera_name);
  ~Camera();
  void takePhoto();
  bool isReadyForCapture();

private slots:
  void setCamera(const QCameraInfo& cameraInfo);
private:
  void processCapturedImage(int requestId, const QImage& img);

  QScopedPointer<QCamera> m_camera;
  QScopedPointer<QCameraImageCapture> m_imageCapture;

  eCAL::protobuf::CPublisher<foxglove::CompressedImage>& m_publisher;

  std::string camera_name_;
  size_t photosTaken;

signals:
  void photoSentSignal();
};
