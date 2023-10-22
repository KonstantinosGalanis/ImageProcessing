#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

class YUVImage;
class GSCImage;

class Pixel {
public:
  virtual ~Pixel() = default;
};

class GSCPixel : public Pixel {
private:
  unsigned char value;

public:
  GSCPixel() = default;

  GSCPixel(const GSCPixel &p) { value = p.value; }

  GSCPixel(unsigned char value) { this->value = value; }

  unsigned char getValue() const { return value; }

  void setValue(unsigned char value) { this->value = value; }
};

class RGBPixel : public Pixel {
private:
  unsigned char red;
  unsigned char green;
  unsigned char blue;

public:
  RGBPixel() = default;

  RGBPixel(const RGBPixel &p) {
    red = p.red;
    green = p.green;
    blue = p.blue;
  }

  RGBPixel(unsigned char r, unsigned char g, unsigned char b) {
    red = r;
    green = g;
    blue = b;
  }

  int getRed() const { return red; }

  int getGreen() const { return green; }

  int getBlue() const { return blue; }

  void setRed(unsigned char r) { red = r; }

  void setGreen(unsigned char g) { green = g; }

  void setBlue(unsigned char b) { blue = b; }
};

class YUVPixel : public Pixel {
private:
  unsigned char y;
  unsigned char u;
  unsigned char v;

public:
  YUVPixel() = default;

  YUVPixel(const YUVPixel &p) {
    y = p.y;
    u = p.u;
    v = p.v;
  }

  YUVPixel(unsigned char y, unsigned char u, unsigned char v) {
    this->y = y;
    this->u = u;
    this->v = v;
  }

  unsigned char getY() const { return y; }

  unsigned char getU() const { return u; }

  unsigned char getV() const { return v; }

  void setY(unsigned char y) { this->y = y; }

  void setU(unsigned char u) { this->u = u; }

  void setV(unsigned char v) { this->v = v; }
};

class Image {
protected:
  int width;
  int height;
  int max_luminocity;

public:
  virtual ~Image() {}
  int getWidth() const { return width; }
  int getHeight() const { return height; }
  int getMaxLuminocity() const { return max_luminocity; }
  void setWidth(int width) { this->width = width; }
  void setHeight(int height) { this->height = height; }
  void setMaxLuminocity(int lum) { this->max_luminocity = lum; }

  virtual Image &operator+=(int times) = 0;
  virtual Image &operator*=(double factor) = 0;
  virtual Image &operator!() = 0;
  virtual Image &operator~() = 0;
  virtual Image &operator*() = 0;
  virtual Pixel &getPixel(int row, int col) const = 0;

  friend std::ostream &operator<<(std::ostream &out, Image &image);
};

class RGBImage : public Image {
private:
  RGBPixel **pixels;

public:
  RGBImage() {
    width = 0;
    height = 0;
    max_luminocity = 255;
    pixels = nullptr;
  }

  RGBImage(const RGBImage &img) {
    width = img.width;
    height = img.height;
    max_luminocity = img.max_luminocity;
    pixels = new RGBPixel *[height];
    for (int i = 0; i < height; i++) {
      pixels[i] = new RGBPixel[width];
      for (int j = 0; j < width; j++) {
        pixels[i][j] = img.pixels[i][j];
      }
    }
  }

  RGBImage(std::istream &stream) {
    stream.seekg(0);

    std::string magicNumber;
    stream >> magicNumber;
    if (magicNumber != "P3") {
      return;
    }

    stream >> width >> height >> max_luminocity;
    pixels = new RGBPixel *[height];
    for (int i = 0; i < height; i++) {
      pixels[i] = new RGBPixel[width];
      for (int j = 0; j < width; j++) {
        int red, green, blue;
        stream >> red >> green >> blue;
        pixels[i][j] = RGBPixel(red, green, blue);
      }
    }
  }

  ~RGBImage() {
    for (int i = 0; i < height; i++) {
        delete[] pixels[i];
    }
    delete[] pixels;
}

  RGBImage(const YUVImage &yuvImage);
  RGBImage(const GSCImage &yuvImage);

  RGBImage &operator=(const RGBImage &img) {
    if (this == &img) {
      return *this;
    }

    for (int i = 0; i < height; i++) {
      delete[] pixels[i];
    }
    delete[] pixels;

    width = img.width;
    height = img.height;
    max_luminocity = img.max_luminocity;
    pixels = new RGBPixel *[height];
    for (int i = 0; i < height; i++) {
      pixels[i] = new RGBPixel[width];
      for (int j = 0; j < width; j++) {
        pixels[i][j] = img.pixels[i][j];
      }
    }

    return *this;
  }

  virtual Image &operator+=(int times) override {
    if (times > 0) {
	 times %= 4;
      for (int t = 0; t < times; t++) {
        RGBPixel **rotatedPixels = new RGBPixel *[width];
        for (int i = 0; i < width; i++) {
          rotatedPixels[i] = new RGBPixel[height];
          for (int j = 0; j < height; j++) {
            rotatedPixels[i][j] = pixels[height - j - 1][i];
          }
        }

        std::swap(width, height);
        for (int i = 0; i < width; i++) {
          delete[] pixels[i];
        }
        delete[] pixels;
        pixels = rotatedPixels;
      }
    } else if (times < 0) {
      times = std::abs(times);
    times %= 4;
      for (int t = 0; t < times; t++) {
        RGBPixel **rotatedPixels = new RGBPixel *[width];
        for (int i = 0; i < width; i++) {
          rotatedPixels[i] = new RGBPixel[height];
          for (int j = 0; j < height; j++) {
            rotatedPixels[i][j] = pixels[j][width - i - 1];
          }
        }

        std::swap(width, height);
        for (int i = 0; i < width; i++) {
          delete[] pixels[i];
        }
        delete[] pixels;
        pixels = rotatedPixels;
      }
    }

    return *this;
  }

  virtual Image &operator*=(double factor) override {
    int newWidth = static_cast<int>(width * factor);
    int newHeight = static_cast<int>(height * factor);

    RGBPixel **resizedPixels = new RGBPixel *[newHeight];
    for (int i = 0; i < newHeight; i++) {
      resizedPixels[i] = new RGBPixel[newWidth];
      for (int j = 0; j < newWidth; j++) {
        int r1 = std::min(static_cast<int>(std::floor(i / factor)), height - 1);
        int r2 = std::min(static_cast<int>(std::ceil(i / factor)), height - 1);
        int c1 = std::min(static_cast<int>(std::floor(j / factor)), width - 1);
        int c2 = std::min(static_cast<int>(std::ceil(j / factor)), width - 1);

        int redSum = pixels[r1][c1].getRed() + pixels[r1][c2].getRed() +
                     pixels[r2][c1].getRed() + pixels[r2][c2].getRed();
        int greenSum = pixels[r1][c1].getGreen() + pixels[r1][c2].getGreen() +
                       pixels[r2][c1].getGreen() + pixels[r2][c2].getGreen();
        int blueSum = pixels[r1][c1].getBlue() + pixels[r1][c2].getBlue() +
                      pixels[r2][c1].getBlue() + pixels[r2][c2].getBlue();

        int newRed = static_cast<int>(redSum / 4);
        int newGreen = static_cast<int>(greenSum / 4);
        int newBlue = static_cast<int>(blueSum / 4);

        resizedPixels[i][j] = RGBPixel(newRed, newGreen, newBlue);
      }
    }

    for (int i = 0; i < height; i++) {
      delete[] pixels[i];
    }
    delete[] pixels;

    width = newWidth;
    height = newHeight;
    pixels = resizedPixels;

    return *this;
  }

  virtual Image &operator!() override {
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        pixels[i][j].setRed(max_luminocity - pixels[i][j].getRed());
        pixels[i][j].setGreen(max_luminocity - pixels[i][j].getGreen());
        pixels[i][j].setBlue(max_luminocity - pixels[i][j].getBlue());
      }
    }
    return *this;
  }

  virtual Image &operator~() override {
	  return *this;
  }

  virtual Image &operator*() override {
    for (int i = 0; i < getHeight(); i++) {
      for (int j = 0; j < getWidth() / 2; j++) {
       std::swap(pixels[i][j], pixels[i][width - j - 1]);
      }
    }

    return *this;
  }

  virtual Pixel &getPixel(int row, int col) const override {
    return pixels[row][col];
  }
};

class YUVImage : public Image {
private:
  YUVPixel **pixels;
  int max_luminocity = 235;

public:
  YUVImage() {
    width = 0;
    height = 0;
    max_luminocity = 235;
    pixels = nullptr;
  }

  YUVImage(const YUVImage &img) {
    width = img.width;
    height = img.height;
    max_luminocity = img.max_luminocity;

    pixels = new YUVPixel *[height];
    for (int i = 0; i < height; i++) {
      pixels[i] = new YUVPixel[width];
      for (int j = 0; j < width; j++) {
        pixels[i][j] = img.pixels[i][j];
      }
    }
  }

  YUVImage(const RGBImage &rgbImage) {
    width = rgbImage.getWidth();
    height = rgbImage.getHeight();

    pixels = new YUVPixel *[height];
    for (int i = 0; i < height; i++) {
      pixels[i] = new YUVPixel[width];
      for (int j = 0; j < width; j++) {
        const Pixel &pixel = rgbImage.getPixel(i, j);
        const RGBPixel &rgbPixel = dynamic_cast<const RGBPixel &>(pixel);

        int y1 = static_cast<int>(((66 * rgbPixel.getRed() + 129 * rgbPixel.getGreen() + 25 * rgbPixel.getBlue() + 128) >> 8) + 16);
        int u1 = static_cast<int>(((-38 * rgbPixel.getRed() - 74 * rgbPixel.getGreen() + 112 * rgbPixel.getBlue() + 128) >> 8) + 128);
        int v1 = static_cast<int>(((112 * rgbPixel.getRed() - 94 * rgbPixel.getGreen() - 18 * rgbPixel.getBlue() + 128) >> 8) + 128);
		  
        unsigned char y = static_cast<unsigned char>(y1);
        unsigned char u = static_cast<unsigned char>(u1);
        unsigned char v = static_cast<unsigned char>(v1);
		  
        pixels[i][j] = YUVPixel(y, u, v);
      }
    }
  }

 ~YUVImage() {
    for (int i = 0; i < height; i++) {
        delete[] pixels[i];
    }
    delete[] pixels;
}

  virtual Image &operator+=(int times) override {return *this;}
  virtual Image &operator*=(double factor) override {return *this;}
  virtual Image &operator!() override {return *this;}
  virtual Image &operator~() override {
    int histogram[236] = {0};
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        unsigned char luminance = pixels[i][j].getY();
        histogram[luminance]++;
      }
    }
    
    // Calculate probability distribution
    double probabilityDistribution[236];
    for (int i = 0; i <= 235; i++) {
      probabilityDistribution[i] = static_cast<double>(histogram[i]) / (width * height);
    }

    // Calculate cumulative probability distribution
    double cumulativeDistribution[236];
    cumulativeDistribution[0] = probabilityDistribution[0];
    for (int i = 1; i <= 235; i++) {
      cumulativeDistribution[i] = cumulativeDistribution[i - 1] + probabilityDistribution[i];
    }

    // Calculate new luminance values
    int newLuminance[236];
    for (int i = 0; i <= 235; i++) {
      newLuminance[i] = static_cast<int>(cumulativeDistribution[i] * 235);
    }

    // Apply luminance transformation to the image
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        unsigned char currentLuminance = pixels[i][j].getY();
        int newPixelValue1 = static_cast<int>(newLuminance[currentLuminance]);
		unsigned char newPixelValue = static_cast<unsigned char>(newPixelValue1);
		pixels[i][j].setY(newPixelValue);
      }
    }
    
    
    return *this;
  }

  virtual Image &operator*() override {return *this;}
  virtual Pixel &getPixel(int row, int col) const override {
    return pixels[row][col];
  }
};

RGBImage::RGBImage(const YUVImage &yuvImage) {
  width = yuvImage.getWidth();
  height = yuvImage.getHeight();

  pixels = new RGBPixel *[height];
  for (int i = 0; i < height; i++) {
    pixels[i] = new RGBPixel[width];
    for (int j = 0; j < width; j++) {
      const Pixel &pixel = yuvImage.getPixel(i, j);
      const YUVPixel &yuvPixel = dynamic_cast<const YUVPixel &>(pixel);

      int y = static_cast<int>(yuvPixel.getY());
      int u = static_cast<int>(yuvPixel.getU());
      int v = static_cast<int>(yuvPixel.getV());

      int red1 = ((298 * (y - 16) + 409 * (v - 128) + 128) >> 8);
      int green1 = ((298 * (y - 16) - 100 * (u - 128) - 208 * (v - 128) + 128) >> 8);
      int blue1 = ((298 * (y - 16) + 516 * (u - 128) + 128) >> 8);
      
      if (red1 < 0) {
        red1 = 0;
	  }
      else if (red1 > 255) {
        red1 = 255;
	  }

      if (green1 < 0) {
        green1 = 0;
	  }
      else if (green1 > 255) {
        green1 = 255;
	  }

      if (blue1 < 0) {
        blue1 = 0;
	  }
      else if (blue1 > 255) {
        blue1 = 255;
	  }

      unsigned char red = static_cast<unsigned char>(red1);
      unsigned char green = static_cast<unsigned char>(green1);
      unsigned char blue = static_cast<unsigned char>(blue1);

      pixels[i][j] = RGBPixel(red, green, blue);
    }
  }

  delete &yuvImage;
}

class GSCImage : public Image {
private:
  GSCPixel **pixels;
  int max_luminocity = 255;

public:
  GSCImage() {
    width = 0;
    height = 0;
    max_luminocity = 255;
    pixels = nullptr;
  }

  GSCImage(const GSCImage &img) {
    width = img.width;
    height = img.height;
    max_luminocity = img.max_luminocity;

    pixels = new GSCPixel *[height];
    for (int i = 0; i < height; i++) {
      pixels[i] = new GSCPixel[width];
      for (int j = 0; j < width; j++) {
        pixels[i][j] = img.pixels[i][j];
      }
    }
  }

  GSCImage(const RGBImage &grayscaled) {
    width = grayscaled.getWidth();
    height = grayscaled.getHeight();
    max_luminocity = grayscaled.getMaxLuminocity();

    pixels = new GSCPixel *[height];
    for (int i = 0; i < height; i++) {
      pixels[i] = new GSCPixel[width];
      for (int j = 0; j < width; j++) {
        const Pixel &pixel = grayscaled.getPixel(i, j);
        const RGBPixel &rgbPixel = dynamic_cast<const RGBPixel &>(pixel);
        unsigned char grayValue = static_cast<unsigned char>(rgbPixel.getRed() * 0.3 + rgbPixel.getGreen() * 0.59 + rgbPixel.getBlue() * 0.11);
        pixels[i][j] = GSCPixel(grayValue);
      }
    }
  }

  GSCImage(const RGBImage &grayscaled, int dontMind) {
    width = grayscaled.getWidth();
    height = grayscaled.getHeight();
    max_luminocity = grayscaled.getMaxLuminocity();

    pixels = new GSCPixel *[height];
    for (int i = 0; i < height; i++) {
      pixels[i] = new GSCPixel[width];
      for (int j = 0; j < width; j++) {
        const Pixel &pixel = grayscaled.getPixel(i, j);
        const RGBPixel &rgbPixel = dynamic_cast<const RGBPixel &>(pixel);
        unsigned char grayValue = static_cast<unsigned char>(rgbPixel.getRed());
        pixels[i][j] = GSCPixel(grayValue);
      }
    }
  }

  GSCImage(std::istream &stream) {
    stream.seekg(0);

    std::string magicNumber;
    stream >> magicNumber;
    if (magicNumber != "P2") {
      return;
    }

    stream >> width >> height >> max_luminocity;

    pixels = new GSCPixel *[height];
    for (int i = 0; i < height; i++) {
      pixels[i] = new GSCPixel[width];
      for (int j = 0; j < width; j++) {
        int pixelValue;
        stream >> pixelValue;
        pixels[i][j] = GSCPixel(static_cast<unsigned char>(pixelValue));
      }
    }
  }

~GSCImage() {
    for (int i = 0; i < height; i++) {
        delete[] pixels[i];
    }
    delete[] pixels;
}

  GSCImage &operator=(const GSCImage &img) {
    if (this != &img) {
      for (int i = 0; i < height; i++) {
        delete[] pixels[i];
      }
      delete[] pixels;

      width = img.width;
      height = img.height;
      max_luminocity = img.max_luminocity;

      pixels = new GSCPixel *[height];
      for (int i = 0; i < height; i++) {
        pixels[i] = new GSCPixel[width];
        for (int j = 0; j < width; j++) {
          pixels[i][j] = img.pixels[i][j];
        }
      }
    }
    return *this;
  }

  virtual Image &operator+=(int times) override {
    if (times > 0) {
     times %= 4;
      for (int t = 0; t < times; t++) {
        GSCPixel **rotatedPixels = new GSCPixel *[width];
        for (int i = 0; i < width; i++) {
          rotatedPixels[i] = new GSCPixel[height];
          for (int j = 0; j < height; j++) {
            rotatedPixels[i][j] = pixels[height - j - 1][i];
          }
        }

        std::swap(width, height);
        for (int i = 0; i < width; i++) {
          delete[] pixels[i];
        }
        delete[] pixels;
        pixels = rotatedPixels;
      }
    } else if (times < 0) {
      times = std::abs(times);
      times %= 4;
      for (int t = 0; t < times; t++) {
        GSCPixel **rotatedPixels = new GSCPixel *[width];
        for (int i = 0; i < width; i++) {
          rotatedPixels[i] = new GSCPixel[height];
          for (int j = 0; j < height; j++) {
            rotatedPixels[i][j] = pixels[j][width - i - 1];
          }
        }

        std::swap(width, height);
        for (int i = 0; i < width; i++) {
          delete[] pixels[i];
        }
        delete[] pixels;
        pixels = rotatedPixels;
      }
    }

    return *this;
  }

  virtual Image &operator*=(double factor) override {
    int newWidth = static_cast<int>(width * factor);
    int newHeight = static_cast<int>(height * factor);

    GSCPixel **resizedPixels = new GSCPixel *[newHeight];
    for (int i = 0; i < newHeight; i++) {
      resizedPixels[i] = new GSCPixel[newWidth];
      for (int j = 0; j < newWidth; j++) {
        int r1 = std::min(static_cast<int>(std::floor(i / factor)), height - 1);
        int r2 = std::min(static_cast<int>(std::ceil(i / factor)), height - 1);
        int c1 = std::min(static_cast<int>(std::floor(j / factor)), width - 1);
        int c2 = std::min(static_cast<int>(std::ceil(j / factor)), width - 1);

        int value = static_cast<int>((pixels[r1][c1].getValue() + pixels[r1][c2].getValue() +
                     pixels[r2][c1].getValue() + pixels[r2][c2].getValue())/4);
		  
        resizedPixels[i][j] = GSCPixel(static_cast<unsigned char>(value));
      }
    }

    for (int i = 0; i < height; i++) {
      delete[] pixels[i];
    }
	delete[] pixels;

    width = newWidth;
    height = newHeight;
    pixels = resizedPixels;

    return *this;
  }

  virtual Image &operator!() override {
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        unsigned char value = pixels[i][j].getValue();
        pixels[i][j].setValue(max_luminocity - value);
      }
    }
    return *this;
  }

  virtual Image &operator~() override {
    // Calculate histogram
    int histogram[256] = {0};

    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        unsigned char luminance = pixels[i][j].getValue();
        histogram[luminance]++;
      }
    }

    // Calculate probability distribution
    double probabilityDistribution[256];
    for (int i = 0; i <= 255; i++) {
      probabilityDistribution[i] = static_cast<double>(histogram[i]) / (width * height);
    }

    // Calculate cumulative probability distribution
    double cumulativeDistribution[256];
    cumulativeDistribution[0] = probabilityDistribution[0];
    for (int i = 1; i <= 255; i++) {
      cumulativeDistribution[i] = cumulativeDistribution[i - 1] + probabilityDistribution[i];
    }

    // Calculate new luminance values
    int newLuminance[256];
    for (int i = 0; i <= 255; i++) {
      newLuminance[i] = static_cast<unsigned char>(cumulativeDistribution[i] * 255);
    }

    // Apply luminance transformation to the image
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        unsigned char currentLuminance = pixels[i][j].getValue();
		
        unsigned char newPixelValue =  static_cast<unsigned char>(newLuminance[currentLuminance]);
        pixels[i][j].setValue(newPixelValue);
      }
    }

    return *this;
  }

  virtual Image &operator*() override {
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width / 2; j++) {
        std::swap(pixels[i][j], pixels[i][width - j - 1]);
      }
    }
    return *this;
  }

  virtual Pixel &getPixel(int row, int col) const override {
    return pixels[row][col];
  }
};

RGBImage::RGBImage(const GSCImage &gscImage) {
  width = gscImage.getWidth();
  height = gscImage.getHeight();

  pixels = new RGBPixel *[height];
  for (int i = 0; i < height; i++) {
    pixels[i] = new RGBPixel[width];
    for (int j = 0; j < width; j++) {
      const Pixel &pixel = gscImage.getPixel(i, j);
      const GSCPixel &gscPixel = dynamic_cast<const GSCPixel &>(pixel);

      int value = static_cast<int>(gscPixel.getValue());
		
      unsigned char red = static_cast<unsigned char>(value);
      unsigned char green = static_cast<unsigned char>(value);
      unsigned char blue = static_cast<unsigned char>(value);

      pixels[i][j] = RGBPixel(red, green, blue);
    }
  }

  delete &gscImage;
}

std::ostream &operator<<(std::ostream &out, Image &image) {
  out << "P2" << std::endl;
  out << image.getWidth() << " " << image.getHeight() << std::endl;
  out << image.getMaxLuminocity() << std::endl;
  for (int i = 0; i < image.getHeight(); i++) {
    for (int j = 0; j < image.getWidth(); j++) {
      out << static_cast<int>(
                 static_cast<GSCPixel &>(image.getPixel(i, j)).getValue())
          << " ";
    }
    out << std::endl;
  }
  return out;
}

class Token {
private:
  std::string name;
  Image *ptr;

public:
  Token(const std::string &n = "", Image *p = nullptr) : name(n), ptr(p) {}
  std::string getName() const { return name; }
  Image *getPtr() const { return ptr; }
  void setName(const std::string &n) { name = n; }
  void setPtr(Image *p) { ptr = p; }
};

Image *readNetpbmImage(const char *filename) {
  std::ifstream f(filename);
  if (!f.is_open()) {
    std::cout << "[ERROR] Unable to open " << filename << std::endl;
  }
  Image *img_ptr = nullptr;
  std::string type;

  if (f.good() && !f.eof())
    f >> type;
  if (!type.compare("P3")) {
    img_ptr = new RGBImage(f);
  } else if (!type.compare("P2")) {
    img_ptr = new GSCImage(f);
  } else if (f.is_open()) {
    std::cout << "[ERROR] Invalid file format" << std::endl;
  }
  return img_ptr;
}

bool tokenExists(const std::vector<Token> &tokens,
                 const std::string &tokenName) {
  return std::find_if(tokens.begin(), tokens.end(),
                      [tokenName](const Token &token) {
                        return token.getName() == tokenName;
                      }) != tokens.end();
}

Token *findToken(const std::vector<Token> &tokenDatabase,
                 const std::string &token) {
  for (const Token &t : tokenDatabase) {
    if (t.getName() == token) {
      return const_cast<Token *>(&t);
    }
  }
  return nullptr;
}

bool fileExists(const std::string &filename) {
  std::ifstream file(filename);
  return file.good();
}

bool exportPGMImage(const GSCImage *image, const std::string &filename) {
  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cout << "[ERROR] Unable to create file\n";
    return false;
  }

  int width = image->getWidth();
  int height = image->getHeight();

  file << "P2\n";
  file << width << " " << height << " "
       << "255\n";

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      const Pixel &pixel = image->getPixel(y, x);
      const GSCPixel &gscPixel = dynamic_cast<const GSCPixel &>(pixel);
      unsigned char luminosity = gscPixel.getValue();
      file << static_cast<int>(luminosity) << "\n";
    }
  }
  return true;
}

bool exportPPMImage(const RGBImage *image, const std::string &filename) {
  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cout << "[ERROR] Unable to create file\n";
    return false;
  }

  int width = image->getWidth();
  int height = image->getHeight();

  file << "P3\n";
  file << width << " " << height << " "
       << "255\n";

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      const Pixel &pixel = image->getPixel(y, x);
      const RGBPixel &rgbPixel = dynamic_cast<const RGBPixel &>(pixel);
      unsigned char red = rgbPixel.getRed();
      unsigned char green = rgbPixel.getGreen();
      unsigned char blue = rgbPixel.getBlue();
      file << static_cast<int>(red) << " " << static_cast<int>(green) << " "
           << static_cast<int>(blue) << "\n";
    }
  }
  return true;
}

bool exportYUVImage(const YUVImage *image, const std::string &filename) {
  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cout << "[ERROR] Unable to create file\n";
    return false;
  }

  int width = image->getWidth();
  int height = image->getHeight();

  file << "P3\n";
  file << width << " " << height << " "
       << "255\n";

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      const Pixel &pixel = image->getPixel(y, x);
      const YUVPixel &yuvPixel = dynamic_cast<const YUVPixel &>(pixel);
      unsigned char y = yuvPixel.getY();
      unsigned char u = yuvPixel.getU();
      unsigned char v = yuvPixel.getV();
      file << static_cast<int>(y) << " " << static_cast<int>(u) << " "
           << static_cast<int>(v) << "\n";
    }
  }
  return true;
}

void deleteToken(std::vector<Token> &tokenDatabase,
                 const std::string &tokenName) {
  auto tokenIterator = std::find_if(
      tokenDatabase.begin(), tokenDatabase.end(),
      [&](const Token &token) { return token.getName() == tokenName; });

  if (tokenIterator != tokenDatabase.end()) {
    delete tokenIterator->getPtr();
    tokenDatabase.erase(tokenIterator);
    std::cout << "[OK] Delete " << tokenName << std::endl;
  } else {
    std::cout << "[ERROR] Token " << tokenName << " not found!" << std::endl;
  }
}

Image &rotate(Image &image, int times) { return image += times; }

Image &resize(Image &image, double factor) { return image *= factor; }

Image &mirrorVertical(Image &image) { return *image; }

Image &reverseBrightness(Image &image) { return !image; }

Image &histogramEqualization(Image &image) { return ~image; }

int main() {
  std::vector<Token> tokenDatabase;
  int afterEq = 0;

  while (true) {
    std::string line;
    std::getline(std::cin, line);

    std::istringstream iss(line);
    std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};

    if (tokens.empty()) {
      continue;
    }

    if (tokens[0] == "i" && tokens.size() >= 4) {
      std::string filename = tokens[1];
      std::string token = tokens[3];

      if (token[0] != '$') {
        std::cout << "\n-- Invalid command! --" << std::endl;
        continue;
      }

      if (tokenExists(tokenDatabase, token)) {
        std::cout << "[ERROR] Token " << token << " exists" << std::endl;
        continue;
      }

      Image *img = readNetpbmImage(filename.c_str());

      if (img != nullptr) {
        tokenDatabase.push_back(Token(token, img));
        std::cout << "[OK] Import " << token << std::endl;
      }
    } else if (tokens[0] == "r" && tokens.size() >= 4 && tokens[2] == "clockwise") {
      std::string token = tokens[1];

      if (token[0] != '$') {
        std::cout << "\n-- Invalid command! --" << std::endl;
        continue;
      }

      Token *tokenPtr = findToken(tokenDatabase, token);
      if (tokenPtr == nullptr) {
        std::cout << "[ERROR] Token " << token << " not found!" << std::endl;
        continue;
      }

      int times = std::stoi(tokens[3]);

      Image *imagePtr = tokenPtr->getPtr();
      *imagePtr = rotate(*imagePtr, times);
      std::cout << "[OK] Rotate " << token << std::endl;
    } else if (tokens[0] == "s" && tokens.size() >= 4) {
      std::string token = tokens[1];

      if (token[0] != '$') {
        std::cout << "\n-- Invalid command! --" << std::endl;
        continue;
      }

      Token *tokenPtr = findToken(tokenDatabase, token);
      if (tokenPtr == nullptr) {
        std::cout << "[ERROR] Token " << token << " not found!" << std::endl;
        continue;
      }

      double factor = std::stod(tokens[3]);

      Image *imagePtr = tokenPtr->getPtr();
      *imagePtr = resize(*imagePtr, factor);
      std::cout << "[OK] Scale " << token << std::endl;
    } else if (tokens[0] == "g" && tokens.size() >= 2) {
      std::string token = tokens[1];

      if (token[0] != '$') {
        std::cout << "\n-- Invalid command! --" << std::endl;
        continue;
      }

      Token *tokenPtr = findToken(tokenDatabase, token);
      if (tokenPtr == nullptr) {
        std::cout << "[ERROR] Token " << token << " not found!" << std::endl;
        continue;
      }

      Image *imagePtr = tokenPtr->getPtr();
      if (dynamic_cast<GSCImage *>(imagePtr)) {
        std::cout << "[NOP] Already grayscale " << token << std::endl;
      } else if (dynamic_cast<RGBImage *>(imagePtr)) {
        RGBImage *rgbImage = static_cast<RGBImage *>(imagePtr);
        GSCImage *gscImage = new GSCImage(*rgbImage);
        delete rgbImage;
        tokenPtr->setPtr(gscImage);
        std::cout << "[OK] Grayscale " << token << std::endl;
      }
    }
    else if (tokens[0] == "m" && tokens.size() >= 2) {
      std::string token = tokens[1];

      if (token[0] != '$') {
        std::cout << "\n-- Invalid command! --" << std::endl;
        continue;
      }

      Token *tokenPtr = findToken(tokenDatabase, token);
      if (tokenPtr == nullptr) {
        std::cout << "[ERROR] Token " << token << " not found!" << std::endl;
        continue;
      }

      Image *imagePtr = tokenPtr->getPtr();
      *imagePtr = mirrorVertical(*imagePtr);
      std::cout << "[OK] Mirror " << token << std::endl;
    }
    if (tokens[0] == "n" && tokens.size() >= 2) {
      std::string token = tokens[1];

      if (token[0] != '$') {
        std::cout << "\n-- Invalid command! --" << std::endl;
        continue;
      }

      Token *tokenPtr = findToken(tokenDatabase, token);
      if (tokenPtr == nullptr) {
        std::cout << "[ERROR] Token " << token << " not found!" << std::endl;
        continue;
      }

      Image *imagePtr = tokenPtr->getPtr();
      *imagePtr = reverseBrightness(*imagePtr);
      std::cout << "[OK] Color Inversion " << token << std::endl;
    } else if (tokens[0] == "d") {
      std::string token = tokens[1];
      deleteToken(tokenDatabase, token);
    } else if (tokens[0] == "q") {
      for (const Token &token : tokenDatabase) {
        delete token.getPtr();
      }
      tokenDatabase.clear();
      break;
    } else if (tokens[0] == "z" && tokens.size() >= 2) {
      std::string token = tokens[1];

      if (token[0] != '$') {
        std::cout << "\n-- Invalid command! --" << std::endl;
        continue;
      }

      Token *tokenPtr = findToken(tokenDatabase, token);
      if (tokenPtr == nullptr) {
        std::cout << "[ERROR] Token " << token << " not found!" << std::endl;
        continue;
      }

      Image *imagePtr = tokenPtr->getPtr();
      if (dynamic_cast<GSCImage *>(imagePtr)) {
        GSCImage *gscImage = static_cast<GSCImage *>(imagePtr);
		RGBImage *rgbImage = new RGBImage(*gscImage);
		YUVImage *yuvImage = new YUVImage(*rgbImage);
        histogramEqualization(*yuvImage);
		*rgbImage = RGBImage(*yuvImage);
		GSCImage *gscImage2 = new GSCImage(*rgbImage,afterEq);
		delete rgbImage;
		tokenPtr->setPtr(gscImage2);
        std::cout << "[OK] Equalize " << token << std::endl;
      } else if (dynamic_cast<RGBImage *>(imagePtr)) {
        RGBImage *rgbImage = static_cast<RGBImage *>(imagePtr);
        YUVImage *yuvImage = new YUVImage(*rgbImage);
        histogramEqualization(*yuvImage);
		*rgbImage = RGBImage(*yuvImage);
		tokenPtr->setPtr(rgbImage);
        std::cout << "[OK] Equalize " << token << std::endl;
      }
    } else if (tokens[0] == "e" && tokens.size() >= 4) {
      std::string token = tokens[1];
      std::string filename = tokens[3];

      if (token[0] != '$') {
        std::cout << "\n-- Invalid command! --" << std::endl;
        continue;
      }

      Token *tokenPtr = findToken(tokenDatabase, token);

      if (tokenPtr == nullptr) {
        std::cout << "[ERROR] Token " << token << " not found!" << std::endl;
        continue;
      }

      if (fileExists(filename)) {
        std::cout << "[ERROR] File exists" << std::endl;
        continue;
      }

      bool success = false;
      Image *imagePtr = tokenPtr->getPtr();
      if (dynamic_cast<GSCImage *>(imagePtr)) {
        success = exportPGMImage(static_cast<GSCImage *>(imagePtr), filename);
      } else if (dynamic_cast<RGBImage *>(imagePtr)) {
        success = exportPPMImage(static_cast<RGBImage *>(imagePtr), filename);
      } else if (dynamic_cast<YUVImage *>(imagePtr)) {
        success = exportYUVImage(static_cast<YUVImage *>(imagePtr), filename);
      }

      if (success) {
        std::cout << "[OK] Export " << token << std::endl;
      } else {
        std::cout << "[ERROR] Unable to create file" << std::endl;
      }
    }
  }

  return 0;
}