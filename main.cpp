#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <list>
#include <Magick++.h> 

const std::string map = " .,:;ox%#@";
const unsigned int defaultFontPointSize = 4;

unsigned int decideFontOffset(Magick::Image& image) {
  unsigned int result = defaultFontPointSize;
  for (std::size_t i = 0; i < map.size(); ++i) {
    Magick::TypeMetric typeMetrics;
    image.fontTypeMetrics(map.substr(i, 1), &typeMetrics);
    result = std::max(result, static_cast<unsigned int> (typeMetrics.textWidth()));
    result = std::max(result, static_cast<unsigned int> (typeMetrics.textHeight()));
  }
  return result;
}

int RGB2Gray(int r, int g, int b) {
  return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}


char grayToASCII(int gray) {
  return map[(255-gray) * 10 / 256];
}

int main(int argc, char **argv) 
{
  if (argc < 2) {
      std::cout << "No input file" << "\n";
      return 1;
  }

  Magick::InitializeMagick(*argv);
  Magick::Image image;
  std::string fileName;
  std::string suffix;
  std::string textResult;
  try {
    image.read(argv[1]);
    fileName = argv[1];
    std::size_t index = fileName.find_last_of(".");
    if (index != std::string::npos) {
      suffix = fileName.substr(index);
      fileName = fileName.substr(0, index);
    }
  } catch (Magick::Exception& error) {
      std::cout << error.what() << "\n";
      return 1;
  }


  unsigned int width = image.columns();
  unsigned int height = image.rows();
  image.font("FreeMono");
  image.fillColor(Magick::Color("black"));
  image.fontPointsize(defaultFontPointSize);
  unsigned int fontOffset = decideFontOffset(image);

  Magick::Image outputImage(Magick::Geometry(width * fontOffset, height * fontOffset), Magick::Color("white"));
  outputImage.font("FreeMono");
  outputImage.fillColor(Magick::Color("black"));
  outputImage.fontPointsize(defaultFontPointSize);
  outputImage.type(Magick::TrueColorType);
  std::list<Magick::Drawable> asciiDrawList;

  Magick::PixelPacket *pixels = image.getPixels(0, 0, width, height);
  int range = std::pow(2, image.modulusDepth());
  for (unsigned int i = 0; i < height; ++i) {
    for (unsigned int j = 0; j < width; ++j) {
      Magick::Color color = pixels[i * width + j];
      int grayValue = RGB2Gray(color.redQuantum(), color.greenQuantum(), color.blueQuantum());
      char asciiValue = grayToASCII(grayValue / range);
      textResult += asciiValue;
      Magick::Color newColor = Magick::Color(grayValue, grayValue, grayValue);
      asciiDrawList.push_back(Magick::DrawableText(j * fontOffset, i * fontOffset, std::string(1, asciiValue)));
      if (j == width - 1) {
        textResult += "\n";
      }
    }
  }
  
  outputImage.draw(asciiDrawList);
  outputImage.write(fileName + "_image_result_high_res" + suffix);
  outputImage.resize(Magick::Geometry(width, height));
  outputImage.write(fileName + "_image_result" + suffix);
  std::string outTxtFileName = fileName + "_ascii_result" + ".txt";
  std::ofstream outTxtFile(outTxtFileName.c_str());
  outTxtFile << textResult;
  outTxtFile.close();

  return 0; 
}