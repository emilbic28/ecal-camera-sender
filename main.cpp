#include <ecal/ecal.h>
#include <ecal/msg/protobuf/publisher.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

#include <QCamera>
#include <QCameraInfo>
#include <QCoreApplication>
#include <QtWidgets>

#include "compressed_image.pb.h"
#include "Camera.h"

int main(int argc, char** argv)
{
  std::string parameterCamera = argv[0];
  std::cout << "EAB: given parameter for camera = " << parameterCamera << std::endl;
  QApplication app(argc, argv);  
  // Initialize eCAL and create a protobuf publisher
  eCAL::Initialize(argc, argv, "Image Sender");
  eCAL::protobuf::CPublisher<foxglove::CompressedImage> publisher("compressed_image_protobuf");

  Camera camera(publisher);

  // while (eCAL::Ok())
  // {
  //   std::cout << "EAB: camera is " << camera.isReadyForCapture() << std::endl;
  //   std::this_thread::sleep_for(std::chrono::milliseconds(500));
  // }

  
  return app.exec();
}