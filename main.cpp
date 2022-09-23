#include <iostream>
#include <string>

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/publisher.h>

#include <QCoreApplication>
#include <QtWidgets>

#include "compressed_image.pb.h"
#include "Camera.h"

int main(int argc, char** argv)
{
  std::string parameterCamera;
  std::string topic_name;

  if (argc != 3)
  {
    std::cerr << "Invalid parameters, usage: ./protobuf_snd [topic_name] [camera_input]" << std::endl;
    return 0;
  }
  else
  {
    topic_name = argv[1];
    parameterCamera = argv[2];
  }

  QApplication app(argc, argv);

  // Initialize eCAL and create a protobuf publisher
  eCAL::Initialize(argc, argv, "Image Sender");
  eCAL::protobuf::CPublisher<foxglove::CompressedImage> publisher(topic_name);

  Camera camera(publisher, parameterCamera);
  
  return app.exec();
}