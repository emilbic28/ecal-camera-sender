#pragma once

#include <QObject>

#include <ecal/msg/protobuf/publisher.h>

#include <QCamera>
#include <QCameraImageCapture>
#include <QScopedPointer>

#include "compressed_image.pb.h"

class Camera : public QObject
{
  Q_OBJECT
public:
  Camera(eCAL::protobuf::CPublisher<foxglove::CompressedImage>& publisher);
  ~Camera();
  void takePhoto();
  bool isReadyForCapture();
  /*std::string getByteArrayFromPhoto(const std::string& filename);*/

private slots:
  void setCamera(const QCameraInfo& cameraInfo);
private:
  void processCapturedImage(int requestId, const QImage& img);

  QScopedPointer<QCamera> m_camera;
  QScopedPointer<QCameraImageCapture> m_imageCapture;
  eCAL::protobuf::CPublisher<foxglove::CompressedImage>& m_publisher;

  size_t photosTaken;

signals:
  void photoSentSignal();
};
