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
  bool isReadyForCapture();

private slots:
  void setCamera(const QCameraInfo& cameraInfo);
private:
  QScopedPointer<QCamera> camera_;
  QScopedPointer<QCameraImageCapture> imageCapture_;

  eCAL::protobuf::CPublisher<foxglove::CompressedImage>& publisher_;

  std::string cameraName_;
  size_t photosTaken_;

signals:
  void photoSentSignal();
};
