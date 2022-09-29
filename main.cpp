#include <iostream>
#include <string>

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/publisher.h>

#include <QCoreApplication>
#include <QtWidgets>

#include "compressed_image.pb.h"
#include "camera.h"

int main(int argc, char** argv)
{
  std::string cameraName;
  std::string topicName;
  uint16_t width = 0;
  uint16_t height = 0;

  if (argc < 3)
  {
    std::cerr << "Invalid parameters, usage: ./protobuf_snd [topicName] [cameraName] [OPTIONAL_resolutionWidth] [OPTIONAL_resolutionHeight]" << std::endl;
    return 0;
  }
  else
  {
    if (argc == 5)
    {
      width = std::stoi(argv[3]);
      height = std::stoi(argv[4]);
    }

    topicName = argv[1];
    cameraName = argv[2];
  }

  QApplication app(argc, argv);

  // Initialize eCAL and create a protobuf publisher
  eCAL::Initialize(argc, argv, "Image Sender");
  eCAL::protobuf::CPublisher<foxglove::CompressedImage> publisher(topicName);

  Camera camera(publisher, cameraName, width, height);
  
  return app.exec();
}